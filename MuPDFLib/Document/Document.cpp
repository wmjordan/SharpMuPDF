#include "Document.h"
#include "MuException.h"
#include <vcclr.h>
using namespace System::Runtime::InteropServices;

#pragma unmanaged
static fz_document* OpenDocumentWithStream(fz_context* ctx, fz_stream* stream) {
	fz_document* s;
	MuTryReturn(ctx, fz_open_document_with_stream(ctx, ".pdf", stream), s);
}

static fz_page* LoadPage(fz_context* ctx, fz_document* doc, int pn) {
	fz_page* p;
	MuTryReturn(ctx, fz_load_page(ctx, doc, pn), p);
}

static int LookupPageNumber(fz_context* ctx, pdf_document* doc, pdf_obj* pageRef) {
	int n = -1;
	fz_try(ctx) {
		n = pdf_lookup_page_number(ctx, doc, pageRef);
	}
	fz_catch(ctx) {
		n = -2;
	}
	return n;
}

static pdf_obj* LoadPageDict(fz_context* ctx, pdf_document* doc, int pn) {
	pdf_obj* p;
	MuTryReturn(ctx, pdf_lookup_page_obj(ctx, doc, pn), p);
}

static fz_rect BoundPage(fz_context* ctx, pdf_document* doc, int pn) {
	fz_matrix page_ctm;
	fz_rect rect;
	fz_try(ctx) {
		pdf_obj* p = pdf_lookup_page_obj(ctx, doc, pn);
		pdf_page_obj_transform_box(ctx, p, &rect, &page_ctm, FZ_CROP_BOX);
		rect = fz_transform_rect(rect, page_ctm);
	}
	fz_catch(ctx) {
		rect = fz_invalid_rect;
	}
	return rect;
}

static bool DeleteObject(fz_context* ctx, pdf_document* doc, int num) {
	MuTry(ctx, pdf_delete_object(ctx, doc, num));
}

static bool LoadNameTree(fz_context* ctx, pdf_document* doc, pdf_obj* name, pdf_obj** nameTree) {
	bool r;
	fz_try(ctx) {
		*nameTree = pdf_load_name_tree(ctx, doc, name);
		r = true;
	}
	fz_catch(ctx) {
		*nameTree = PDF_NULL;
		r = false;
	}
	return r;
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

static int GraftPages(fz_context* ctx, int pageTo, int numberOfPages, pdf_document* dest, pdf_document* src, int pageFrom)
{
	pdf_graft_map* map = pdf_new_graft_map(ctx, dest);
	fz_try(ctx) {
		if (pageTo < 0) {
			for (int i = 0; i < numberOfPages; i++) {
				pdf_graft_mapped_page(ctx, map, -1, src, pageFrom++);
			}
		}
		else {
			for (int i = 0; i < numberOfPages; i++) {
				pdf_graft_mapped_page(ctx, map, pageTo++, src, pageFrom++);
			}
		}
	}
	fz_always(ctx)
		pdf_drop_graft_map(ctx, map);
	fz_catch(ctx)
		return 0;
	return 1;
}

static int GraftPage(fz_context* ctx, pdf_graft_map* map, int pageTo, pdf_document* src, int pageFrom) {
	MuTry(ctx, pdf_graft_mapped_page(ctx, map, pageTo, src, pageFrom));
}

#pragma managed

using namespace MuPDF;

PdfDictionary^ Document::Trailer::get() {
	return gcnew PdfDictionary(_trailer);
}

PdfDictionary^ Document::Root::get() {
	return gcnew PdfDictionary(pdf_dict_get(Context::Ptr, _trailer, PDF_NAME(Root)));
}

PdfDocumentInfo^ Document::Info::get() {
	return gcnew PdfDocumentInfo(pdf_dict_get(Context::Ptr, _trailer, PDF_NAME(Info)));
}

Document^ Document::Open(String^ filePath) {
	Stream^ s = gcnew Stream(filePath);
	try {
		auto doc = gcnew Document(s->Ptr);
		doc->FilePath = filePath;
		return doc;
	}
	catch (Exception^) {
		delete s;
		throw;
	}
}

Document^ Document::Open(array<Byte>^ memoryFile)
{
	Stream^ s = gcnew Stream(memoryFile);
	try {
		auto doc = gcnew Document(s->Ptr);
		doc->FilePath = String::Empty;
		return doc;
	}
	catch (Exception^) {
		delete s;
		throw;
	}
}

Document::Document(fz_stream* stream) {
	OpenStream(stream);
}

void Document::OpenStream(fz_stream* stream) {
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

void Document::InitTrailer() {
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

Page^ Document::LoadPage(int pageNumber) {
	fz_page* p = ::LoadPage(Context::Ptr, _document, pageNumber);
	if (p) {
		return gcnew Page(p, pageNumber);
	}
	throw MuException::FromContext();
}

int Document::LookupPageNumber(PdfObject^ pageObj) {
	int n = ::LookupPageNumber(Context::Ptr, _pdf, pageObj->Ptr);
	if (n == -2) {
		throw MuException::FromContext();
	}
	return n;
}

PdfDictionary^ Document::GetPageDictionary(int pageNumber) {
	auto p = ::LoadPageDict(Context::Ptr, _pdf, pageNumber);
	if (p) {
		return gcnew PdfDictionary(p);
	}
	throw MuException::FromContext();
}

Box Document::BoundPage(int pageNumber) {
	return ::BoundPage(Context::Ptr, _pdf, pageNumber);
}

PdfDictionary^ Document::NewPage(Box mediaBox, int rotate, PdfDictionary^ resources, array<Byte>^ contents) {
	pin_ptr<Byte> c = &contents[0];
	auto b = fz_new_buffer_from_copied_data(Context::Ptr, c, contents->Length);
	return gcnew PdfDictionary(pdf_add_page(Context::Ptr, _pdf, mediaBox, rotate, resources ? resources->Ptr : NULL, b));
}

PdfDictionary^ Document::NewDictionary(int capacity) {
	return gcnew PdfDictionary(pdf_new_dict(Context::Ptr, _pdf, capacity));
}

PdfArray^ Document::NewArray(int capacity) {
	return gcnew PdfArray(pdf_new_array(Context::Ptr, _pdf, capacity));
}

PdfArray^ Document::NewBox(Box box) {
	return gcnew PdfArray(pdf_new_rect(Context::Ptr, _pdf, box));
}

PdfArray^ Document::NewMatrix(Matrix matrix) {
	return gcnew PdfArray(pdf_new_matrix(Context::Ptr, _pdf, matrix));
}

void Document::DeleteObject(int objNum) {
	if (!::DeleteObject(Context::Ptr, _pdf, objNum)) {
		throw MuException::FromContext();
	}
}

void Document::GraftPagesFrom(Document^ srcDoc, int pageFrom, int numberOfPages, int pageTo) {
	if (!::GraftPages(Context::Ptr, pageTo, numberOfPages, _pdf, srcDoc->_pdf, pageFrom)) {
		throw MuException::FromContext();
	}
	RefreshPageCount();
}

void Document::GraftPagesFrom(Document^ srcDoc, System::Collections::Generic::IEnumerable<int>^ srcPages, int pageTo)
{
	auto ctx = Context::Ptr;
	pdf_graft_map* map = pdf_new_graft_map(ctx, _pdf);
	auto src = srcDoc->_pdf;
	MuException^ err = nullptr;
	if (pageTo < 0) {
		for each(int num in srcPages) {
			if (!::GraftPage(ctx, map, -1, src, num)) {
				err = MuException::FromContext();
				goto RETURN;
			}
		}
	}
	else {
		for each(int num in srcPages) {
			if (!::GraftPage(ctx, map, pageTo++, src, num)) {
				err = MuException::FromContext();
				goto RETURN;
			}
		}
	}
	RETURN:
	RefreshPageCount();
	pdf_drop_graft_map(ctx, map);
	if (err) {
		throw MuException::FromContext();
	}
}

void Document::SetPageLabel(int index, PageLabelStyle style, String^ prefix, int start)
{
	IntPtr p_prefix = Marshal::StringToHGlobalAnsi(prefix);
	char* p = static_cast<char*>(p_prefix.ToPointer());
	pdf_set_page_labels(Context::Ptr, _pdf, index, (pdf_page_label_style)style, p, start);
	Marshal::FreeHGlobal(p_prefix);
}

PdfDictionary^ Document::LoadNameTree(PdfNames name) {
	auto ctx = Context::Ptr;
	pdf_obj* r;
	if (!::LoadNameTree(ctx, _pdf, (pdf_obj*)name, &r)) {
		throw MuException::FromContext();
	}
	return pdf_is_dict(ctx, r) ? gcnew PdfDictionary(r, false) : nullptr;
}

PdfObject^ Document::GetAssociatedFile(int index) {
	return MuPDF::PdfObject::Wrap(pdf_document_associated_file(Context::Ptr, _pdf, index));
}

void Document::Save(String^ filePath, WriterOptions^ options) {
	pin_ptr<const wchar_t> p = PtrToStringChars(filePath);
	pdf_write_options w = options ? options->ToNative() : pdf_write_options();
	auto r = PdfSaveDocument(Context::Ptr, _pdf, (const wchar_t*)p, (const pdf_write_options*)&w);
	if (!r) {
		throw MuException::FromContext();
	}
}

void Document::SaveSnapshot(String^ filePath) {
	pin_ptr<const wchar_t> p = PtrToStringChars(filePath);
	auto r = PdfSaveSnapshot(Context::Ptr, _pdf, (const wchar_t*)p);
	if (!r) {
		throw MuException::FromContext();
	}
}

bool Document::CheckPassword(String^ password) {
	const char* c = (char*)(void*)Marshal::StringToHGlobalAnsi(password);
	int r = fz_authenticate_password(Context::Ptr, _document, c);
	delete c;
	return r;
}

void Document::Reopen() {
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

void Document::ReleaseHandle() {
	fz_context* ctx = Context::Ptr;
	fz_drop_document(ctx, _document);
	fz_drop_stream(ctx, _stream);
	_document = NULL;
	_pdf = NULL;
	_trailer = NULL;
	_stream = NULL;
}

pdf_write_options WriterOptions::ToNative() {
	pdf_write_options r{};
	r.do_incremental = Incremental;
	r.do_pretty = Pretty;
	r.do_ascii = Ascii;
	r.do_compress = (int)CompressionMode;
	r.do_compress_images = CompressImages;
	r.do_compress_fonts = CompressFonts;
	r.do_decompress = Decompress;
	r.do_garbage = (int)Garbage;
	r.do_linear = Linear;
	r.do_clean = Clean;
	r.do_sanitize = Sanitize;
	r.do_appearance = Appearance;
	r.do_encrypt = (int)Encrypt;
	r.dont_regenerate_id = DoNotRegenerateId;
	r.do_snapshot = Snapshot;
	r.do_preserve_metadata = PreserveMetadata;
	r.do_use_objstms = UseObjectStreams;
	r.compression_effort = CompressionEffort;
	r.permissions = (int)Permissions;
	r.do_labels = AddLabels;
	if (OwnerPassword) {
		System::Runtime::InteropServices::Marshal::Copy(OwnerPassword, 0, (IntPtr)(void*)&r.opwd_utf8, OwnerPassword->Length);
	}
	if (UserPassword) {
		System::Runtime::InteropServices::Marshal::Copy(UserPassword, 0, (IntPtr)(void*)&r.upwd_utf8, UserPassword->Length);
	}
	return r;
}
