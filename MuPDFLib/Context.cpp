#include "MuPDF.h"
#include <thread>
#include <mutex>
#include <iostream>

using namespace System::Threading;

#pragma unmanaged
DLLEXP fz_stream* OpenFile(fz_context* ctx, const wchar_t* filePath) {
	fz_stream* r;
	MuTryReturn(ctx, fz_open_file_w(ctx, filePath), r);
}

DLLEXP pdf_document* OpenPdfDocumentStream(fz_context* ctx, fz_stream* stream) {
	pdf_document* r;
	MuTryReturn(ctx, pdf_open_document_with_stream(ctx, stream), r);
}

DLLEXP int CloseDocumentWriter(fz_context* ctx, fz_document_writer* writer) {
	MuTry(ctx, fz_close_document_writer(ctx, writer));
}

struct internal_state {
	/* Constructor. */
	internal_state() {
		m_locks.user = this;
		m_locks.lock = lock;
		m_locks.unlock = unlock;
		m_ctx = nullptr;
		reinit(true);
	}

	void reinit(bool multithreaded) {
		fz_drop_context(m_ctx);
		m_multithreaded = multithreaded;
		m_ctx = fz_new_context(NULL /*alloc*/, (multithreaded) ? &m_locks : nullptr, FZ_STORE_DEFAULT);
		fz_register_document_handlers(m_ctx);
	}
	static void lock(void* user, int lock) {
		internal_state* self = (internal_state*)user;
		assert(self->m_multithreaded);
		self->m_mutexes[lock].lock();
	}
	static void unlock(void* user, int lock) {
		internal_state* self = (internal_state*)user;
		assert(self->m_multithreaded);
		self->m_mutexes[lock].unlock();
	}
	~internal_state() {
		fz_drop_context(m_ctx);
	}

	bool                m_multithreaded;
	fz_context* m_ctx;
	std::mutex          m_mutex;    /* Serialise access to m_ctx. fixme: not actually necessary. */

	/* Provide thread support to mupdf. */
	std::mutex          m_mutexes[FZ_LOCK_MAX];
	fz_locks_context    m_locks;
};

static internal_state  s_state;

#pragma managed

MuPDF::Context^ MuPDF::Context::Instance::get() {
	if (_Instance) {
		return _Instance;
	}
	if (_MainInstance->_disposed) {
		return nullptr;
	}
	return _Instance = gcnew Context(fz_clone_context(s_state.m_ctx), true);
}

MuPDF::Colorspace^ MuPDF::Context::GetColorspace(ColorspaceKind kind) {
	return gcnew Colorspace(GetFzColorspace(kind));
}

MuPDF::Context^ MuPDF::Context::MakeMainContext() {
	return gcnew Context(s_state.m_ctx, false);
}

void MuPDF::Context::ReleaseHandle() {
	if (_isCloned) {
		fz_drop_context(_context);
		_context = NULL;
		_disposed = true;
		return;
	}
	// HACK: 
	// The following statement often causes AccessViolationException for unknown reason.
	// Since the main instance is a static one which is finalized only when the program exits,
	//   we skip it and let the OS do the dirty job.
	// fz_drop_context(_context);
	_context = NULL;
	_disposed = true;
}

fz_colorspace* MuPDF::Context::GetFzColorspace(ColorspaceKind kind) {
	switch (kind) {
		case ColorspaceKind::Rgb: return fz_device_rgb(Ptr);
		case ColorspaceKind::Cmyk: return fz_device_cmyk(Ptr);
		case ColorspaceKind::Gray: return fz_device_gray(Ptr);
		case ColorspaceKind::Bgr: return fz_device_bgr(Ptr);
		case ColorspaceKind::Lab: return fz_device_lab(Ptr);
	}
	throw gcnew MuException("Invalid colorspace kind.");
}

