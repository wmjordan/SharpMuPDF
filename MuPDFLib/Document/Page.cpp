#include "Page.h"

#pragma unmanaged
DLLEXP int RunPage(fz_context* ctx, fz_page* page, fz_device* dev, fz_matrix ctm, fz_cookie* cookie) {
	MuTry(ctx, fz_run_page(ctx, page, dev, ctm, cookie));
}
DLLEXP int RunPageContents(fz_context* ctx, fz_page* page, fz_device* dev, fz_matrix ctm, fz_cookie* cookie) {
	MuTry(ctx, fz_run_page_contents(ctx, page, dev, ctm, cookie))
}
DLLEXP int RunPageAnnotations(fz_context* ctx, fz_page* page, fz_device* dev, fz_matrix ctm, fz_cookie* cookie) {
	MuTry(ctx, fz_run_page_annots(ctx, page, dev, ctm, cookie))
}
DLLEXP int RunPageWidgets(fz_context* ctx, fz_page* page, fz_device* dev, fz_matrix ctm, fz_cookie* cookie) {
	MuTry(ctx, fz_run_page_widgets(ctx, page, dev, ctm, cookie))
}

#pragma managed
MuPDF::PdfArray^ MuPDF::Page::GetPageBox(PageBoxType boxType) {
	pdf_obj* box;
	switch (boxType) {
	case MuPDF::PageBoxType::Media:
		box = pdf_dict_get_inheritable(Context::Ptr, PagePtr, PDF_NAME(MediaBox));
		break;
	case MuPDF::PageBoxType::Crop:
		box = pdf_dict_get_inheritable(Context::Ptr, PagePtr, PDF_NAME(CropBox));
		break;
	case MuPDF::PageBoxType::Bleed:
		box = pdf_dict_get_inheritable(Context::Ptr, PagePtr, PDF_NAME(BleedBox));
		break;
	case MuPDF::PageBoxType::Trim:
		box = pdf_dict_get_inheritable(Context::Ptr, PagePtr, PDF_NAME(TrimBox));
		break;
	case MuPDF::PageBoxType::Art:
		box = pdf_dict_get_inheritable(Context::Ptr, PagePtr, PDF_NAME(ArtBox));
		break;
	default:
		return nullptr;
	}
	return GcWrap(PdfArray, box);
}

void MuPDF::Page::Run(Device^ dev, Matrix ctm, Cookie^ cookie) {
	if (!RunPage(Context::Ptr, _page, dev->Ptr, ctm, Unwrap(cookie))) {
		throw MuException::FromContext();
	}
}

void MuPDF::Page::RunContents(Device^ dev, Matrix ctm, Cookie^ cookie) {
	if (!RunPageContents(Context::Ptr, _page, dev->Ptr, ctm, Unwrap(cookie))) {
		throw MuException::FromContext();
	}
}

void MuPDF::Page::RunAnnotations(Device^ dev, Matrix ctm, Cookie^ cookie) {
	if (!RunPageAnnotations(Context::Ptr, _page, dev->Ptr, ctm, Unwrap(cookie))) {
		throw MuException::FromContext();
	}
}

void MuPDF::Page::RunWidgets(Device^ dev, Matrix ctm, Cookie^ cookie) {
	if (!RunPageWidgets(Context::Ptr, _page, dev->Ptr, ctm, Unwrap(cookie))) {
		throw MuException::FromContext();
	}
}
