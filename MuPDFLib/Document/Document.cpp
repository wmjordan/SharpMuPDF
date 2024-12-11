#include "Document.h"
#include "MuException.h"
#include "Vcclr.h"

#pragma unmanaged
static fz_document* OpenDocumentWithStream(fz_context* ctx, fz_stream* stream) {
	fz_document* s;
	MuTryReturn(ctx, fz_open_document_with_stream(ctx, ".pdf", stream), s);
}

static fz_page* LoadPage(fz_context* ctx, fz_document* doc, int pn) {
	fz_page* p;
	MuTryReturn(ctx, fz_load_page(ctx, doc, pn), p);
}

DLLEXP int PdfSaveDocument(fz_context* ctx, pdf_document* doc, const wchar_t* filePath, const pdf_write_options* options) {
	char* utf8path = NULL;
	fz_try(ctx) {
		utf8path = fz_utf8_from_wchar(ctx, filePath);
		pdf_save_document(ctx, doc, utf8path, options);
	}
	fz_always(ctx) {
		fz_free(ctx, utf8path);
	}
	fz_catch(ctx) {
		return 0;
	}
	return 1;
}

DLLEXP int PdfSaveSnapshot(fz_context* ctx, pdf_document* doc, const wchar_t* filePath) {
	char* utf8path = NULL;
	fz_try(ctx) {
		utf8path = fz_utf8_from_wchar(ctx, filePath);
		pdf_save_snapshot(ctx, doc, utf8path);
	}
	fz_always(ctx) {
		fz_free(ctx, utf8path);
	}
	fz_catch(ctx) {
		return 0;
	}
	return 1;
}

#pragma managed
MuPDF::Document::Document(fz_stream* stream) {
	OpenStream(stream);
}

void MuPDF::Document::OpenStream(fz_stream* stream) {
	fz_context* ctx = Context::Ptr;
	fz_document* doc = OpenDocumentWithStream(ctx, stream);
	if (doc) {
		_document = doc;
		_stream = stream;
		InitTrailer();
		return;
	}
	else {
		fz_drop_stream(ctx, stream);
	}
	throw MuException::FromContext();
}

void MuPDF::Document::InitTrailer() {
	fz_context* ctx = Context::Ptr;
	_pdf = pdf_document_from_fz_document(ctx, _document);
	if (!_pdf) {
		throw gcnew MuException("Document is not PDF.");
	}
	_trailer = pdf_trailer(ctx, _pdf);
	if (!_trailer) {
		throw gcnew MuException("Missing document trailer.");
	}
	_pageCount = fz_count_pages(ctx, _document);
}

MuPDF::Page^ MuPDF::Document::LoadPage(int pageNumber) {
	fz_page* p = ::LoadPage(Context::Ptr, _document, pageNumber);
	if (p) {
		return gcnew Page(p, pageNumber);
	}
	throw MuException::FromContext();
}

MuPDF::PdfDictionary^ MuPDF::Document::NewPage(Box mediaBox, int rotate, PdfDictionary^ resources, array<Byte>^ contents) {
	pin_ptr<Byte> c = &contents[0];
	auto b = fz_new_buffer_from_copied_data(Context::Ptr, c, contents->Length);
	return gcnew PdfDictionary(pdf_add_page(Context::Ptr, _pdf, mediaBox, rotate, resources ? resources->Ptr : NULL, b));
}

void MuPDF::Document::Save(String^ filePath, WriterOptions^ options) {
	pin_ptr<const wchar_t> p = PtrToStringChars(filePath);
	pdf_write_options w = options->ToNative();
	auto r = PdfSaveDocument(Context::Ptr, _pdf, (const wchar_t*)p, (const pdf_write_options*)&w);
	if (!r) {
		throw MuException::FromContext();
	}
}

void MuPDF::Document::SaveSnapshot(String^ filePath) {
	pin_ptr<const wchar_t> p = PtrToStringChars(filePath);
	auto r = PdfSaveSnapshot(Context::Ptr, _pdf, (const wchar_t*)p);
	if (!r) {
		throw MuException::FromContext();
	}
}

bool MuPDF::Document::CheckPassword(String^ password) {
	const char* c = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(password);
	int r = fz_authenticate_password(Context::Ptr, _document, c);
	delete c;
	return r;
}

void MuPDF::Document::Reopen() {
	if (!_path) {
		throw gcnew System::InvalidOperationException("Could not reopen a document without path");
	}
	Stream^ s = gcnew Stream(_path);
	try {
		OpenStream(s->Ptr);
	}
	catch (Exception^) {
		delete s;
		throw;
	}
}

pdf_write_options MuPDF::WriterOptions::ToNative() {
	pdf_write_options r{};
	r.do_incremental = Incremental;
	r.do_pretty = Pretty;
	r.do_ascii = Ascii;
	r.do_compress = Compress;
	r.do_compress_images = CompressImages;
	r.do_compress_fonts = CompressFonts;
	r.do_decompress = Decompress;
	r.do_garbage = Garbage;
	r.do_linear = Linear;
	r.do_clean = Clean;
	r.do_sanitize = Sanitize;
	r.do_appearance = Appearance;
	r.do_encrypt = Decrypt;
	r.dont_regenerate_id = DoNotRegenerateId;
	r.do_snapshot = Snapshot;
	r.do_preserve_metadata = PreserveMetadata;
	r.do_use_objstms = UseObjectStreams;
	r.compression_effort = CompressionEffort;
	if (OwnerPassword) {
		System::Runtime::InteropServices::Marshal::Copy(OwnerPassword, 0, (IntPtr)(void*)&r.opwd_utf8, OwnerPassword->Length);
	}
	if (UserPassword) {
		System::Runtime::InteropServices::Marshal::Copy(UserPassword, 0, (IntPtr)(void*)&r.upwd_utf8, UserPassword->Length);
	}
	return r;
}
