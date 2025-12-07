#include "MuPDF.h"
#include <thread>
#include <shared_mutex>

using namespace System::Runtime::InteropServices;
using namespace System::Threading;
using namespace MuPDF;

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

typedef void (ErrorHandler)(bool error, const char* message);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ErrorCallbackDelegate(bool isError, const char* message);

struct internal_state
{
	internal_state() {
		m_locks.user = this;
		m_locks.lock = lock;
		m_locks.unlock = unlock;
		m_ctx = nullptr;
		reinit(true);
	}
	~internal_state() {
		fz_drop_context(m_ctx);
	}

	void onError(void* user, const char* msg) const {
		fprintf(stderr, "Error: %s\n", msg);
		if (m_error_handler) {
			m_error_handler(true, msg);
		}
	}

	void onWarning(void* user, const char* msg) const {
		fprintf(stderr, "Warning: %s\n", msg);
		if (m_error_handler) {
			m_error_handler(false, msg);
		}
	}

	static void onNativeError(void* user, const char* msg) {
		internal_state* me = static_cast<internal_state*>(user);
		if (me) {
			me->onError(user, msg);
		}
	}

	static void onNativeWarning(void* user, const char* msg) {
		internal_state* me = static_cast<internal_state*>(user);
		if (me) {
			me->onWarning(user, msg);
		}
	}

	void reinit(bool multithreaded) {
		fz_drop_context(m_ctx);
		m_multithreaded = multithreaded;
		m_ctx = fz_new_context(NULL /*alloc*/, (multithreaded) ? &m_locks : nullptr, FZ_STORE_DEFAULT);
		fz_set_error_callback(m_ctx, onNativeError, this);
		fz_set_warning_callback(m_ctx, onNativeWarning, this);
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

	bool                m_multithreaded;
	fz_context* m_ctx;
	std::shared_mutex          m_mutex;    /* Serialise access to m_ctx. fixme: not actually necessary. */

	/* Provide thread support to mupdf. */
	std::shared_mutex          m_mutexes[FZ_LOCK_MAX];
	fz_locks_context    m_locks;

	ErrorHandler* m_error_handler;
};

static internal_state  s_state;

#pragma managed

Context^ Context::Current::get() {
	if (_Instance) {
		return _Instance;
	}
	if (_MainInstance->_disposed) {
		return nullptr;
	}
	return _Instance = gcnew Context(fz_clone_context(s_state.m_ctx), true);
}

Colorspace^ Context::GetColorspace(ColorspaceKind kind) {
	return gcnew Colorspace(GetFzColorspace(kind));
}

Context^ Context::MakeMainContext() {
	return gcnew Context(s_state.m_ctx, false);
}

void Context::SetErrorCallback(System::Action<bool, String^>^ callback) {
	auto me = Current;
	if (callback) {
		if (_errorCallback == callback) {
			return;
		}
		if (_errorCallbackHandle.IsAllocated) {
			_errorCallbackHandle.Free();
		}
		_errorCallback = callback;
		StaticMethodToFunctionPointer(Context::ErrorCallback, ErrorCallbackDelegate, ErrorHandler, _errorCallbackHandle, s_state.m_error_handler)
	}
	else if (_errorCallback) {
		_errorCallback = nullptr;
		_errorCallbackHandle.Free();
		s_state.m_error_handler = NULL;
	}
}

void Context::ErrorCallback(bool isError, const char* message) {
	if (_errorCallback) {
		_errorCallback(isError, DecodeUTF8(message));
	}
}

void Context::ReleaseHandle() {
	if (_isCloned) {
		if (_errorCallback) {
			s_state.m_error_handler = NULL;
			_errorCallback = nullptr;
		}
		if (_errorCallbackHandle.IsAllocated) {
			_errorCallbackHandle.Free();
		}
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

fz_colorspace* Context::GetFzColorspace(ColorspaceKind kind) {
	switch (kind) {
		case ColorspaceKind::Rgb: return fz_device_rgb(Ptr);
		case ColorspaceKind::Cmyk: return fz_device_cmyk(Ptr);
		case ColorspaceKind::Gray: return fz_device_gray(Ptr);
		case ColorspaceKind::Bgr: return fz_device_bgr(Ptr);
		case ColorspaceKind::Lab: return fz_device_lab(Ptr);
	}
	throw gcnew MuException("Invalid colorspace kind.");
}

