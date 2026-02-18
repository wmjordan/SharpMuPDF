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
using namespace MuPDF;

PdfRedactOptions::operator pdf_redact_options() {
	pdf_redact_options opts;
	opts.black_boxes = BlackBoxes;
	opts.image_method = (int)ImageMethod;
	opts.line_art = (int)LineArt;
	opts.text = (int)Text;
	return opts;
}

int Page::PageNumber::get() {
	// we don't use _pageNumber here, since pages can be reordered after editing operations
	return pdf_lookup_page_number(Context::Ptr, _pdfPage->doc, _pdfPage->obj);
}

Box Page::Bound::get() { return pdf_bound_page(Context::Ptr, _pdfPage, FZ_CROP_BOX); }

Box Page::MediaBox::get() { return pdf_dict_get_rect(Context::Ptr, _pdfPage->obj, (pdf_obj*)PdfNames::MediaBox); }

Box Page::CropBox::get() { return pdf_dict_get_rect(Context::Ptr, _pdfPage->obj, (pdf_obj*)PdfNames::CropBox); }

Box Page::ArtBox::get() { return pdf_dict_get_rect(Context::Ptr, _pdfPage->obj, (pdf_obj*)PdfNames::ArtBox); }

Box Page::BleedBox::get() { return pdf_dict_get_rect(Context::Ptr, _pdfPage->obj, (pdf_obj*)PdfNames::BleedBox); }

Box Page::TrimBox::get() { return pdf_dict_get_rect(Context::Ptr, _pdfPage->obj, (pdf_obj*)PdfNames::TrimBox); }

PdfDictionary^ Page::PdfObject::get() {
	return gcnew PdfDictionary(_pdfPage->obj);
}

MuPDF::PdfObject^ Page::Contents::get() {
	auto c = pdf_page_contents(Context::Ptr, _pdfPage);
	return c ? MuPDF::PdfObject::Wrap(c) : nullptr;
}

PdfDictionary^ Page::Resources::get() {
	auto r = pdf_page_resources(Context::Ptr, _pdfPage);
	return r ? gcnew PdfDictionary(r) : nullptr;
}

TextPage^ Page::TextPage::get() {
	return _textPage != nullptr ? _textPage : (_textPage = gcnew MuPDF::TextPage(fz_new_stext_page_from_page(Context::Ptr, _page, NULL)));
}

PdfArray^ Page::GetPageBox(PageBoxType boxType) {
	pdf_obj* box;
	switch (boxType) {
	case PageBoxType::Media:
		box = pdf_dict_get_inheritable(Context::Ptr, PagePtr, PDF_NAME(MediaBox));
		break;
	case PageBoxType::Crop:
		box = pdf_dict_get_inheritable(Context::Ptr, PagePtr, PDF_NAME(CropBox));
		break;
	case PageBoxType::Bleed:
		box = pdf_dict_get_inheritable(Context::Ptr, PagePtr, PDF_NAME(BleedBox));
		break;
	case PageBoxType::Trim:
		box = pdf_dict_get_inheritable(Context::Ptr, PagePtr, PDF_NAME(TrimBox));
		break;
	case PageBoxType::Art:
		box = pdf_dict_get_inheritable(Context::Ptr, PagePtr, PDF_NAME(ArtBox));
		break;
	default:
		return nullptr;
	}
	return GcWrap(PdfArray, box);
}

array<Byte>^ Page::GetContentBytes() {
	auto c = pdf_page_contents(Context::Ptr, _pdfPage);
	if (c) {
		auto s = gcnew Stream(pdf_open_contents_stream(Context::Ptr, _pdfPage->doc, c));
		return s->ReadAll();
	}
	return nullptr;
}

void Page::Run(Device^ dev, Matrix ctm, Cookie^ cookie) {
	if (!RunPage(Context::Ptr, _page, dev->Ptr, ctm, Unwrap(cookie))) {
		throw MuException::FromContext();
	}
}

void Page::RunContents(Device^ dev, Matrix ctm, Cookie^ cookie) {
	if (!RunPageContents(Context::Ptr, _page, dev->Ptr, ctm, Unwrap(cookie))) {
		throw MuException::FromContext();
	}
}

void Page::RunAnnotations(Device^ dev, Matrix ctm, Cookie^ cookie) {
	if (!RunPageAnnotations(Context::Ptr, _page, dev->Ptr, ctm, Unwrap(cookie))) {
		throw MuException::FromContext();
	}
}

void Page::RunWidgets(Device^ dev, Matrix ctm, Cookie^ cookie) {
	if (!RunPageWidgets(Context::Ptr, _page, dev->Ptr, ctm, Unwrap(cookie))) {
		throw MuException::FromContext();
	}
}

MuPDF::PdfObject^ Page::GetAssociatedFile(int index) {
	return MuPDF::PdfObject::Wrap(pdf_page_associated_file(Context::Ptr, _pdfPage, index));
}

void Page::RefreshTextPage() {
	DisposeObject(_textPage);
}

Page::!Page() {
	DropHandle(_page, fz_drop_page);
	DisposeObject(_textPage);
	_pdfPage = NULL;
}
