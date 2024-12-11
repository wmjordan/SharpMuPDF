#include "mupdf/fitz.h"
#include "mupdf/pdf.h"
#include "MuPDF.h"

#ifndef __PAGE
#define __PAGE

#pragma once
using namespace System;
namespace MuPDF {

public enum class PageBoxType {
	Media,
	Crop,
	Bleed,
	Trim,
	Art,
	Unknown
};

public ref class Page sealed : IDisposable, IEquatable<Page^> {
public:
	property int PageNumber {
		int get() { return _pageNumber; }
	}

	/// <summary>
	/// Determine the page size in points, taking page rotation into account. The page size is taken to be the crop box if it exists (visible area after cropping), otherwise the media box will be used (possibly including printing marks).
	/// </summary>
	property Box Bound {
		Box get() { return pdf_bound_page(Context::Ptr, _pdfPage, FZ_CROP_BOX); }
	}
	property Box MediaBox {
		Box get() { return pdf_dict_get_rect(Context::Ptr, _pdfPage->obj, (pdf_obj*)PdfNames::MediaBox); }
	}
	property Box CropBox {
		Box get() { return pdf_dict_get_rect(Context::Ptr, _pdfPage->obj, (pdf_obj*)PdfNames::CropBox); }
	}
	property Box ArtBox {
		Box get() { return pdf_dict_get_rect(Context::Ptr, _pdfPage->obj, (pdf_obj*)PdfNames::ArtBox); }
	}
	property Box BleedBox {
		Box get() { return pdf_dict_get_rect(Context::Ptr, _pdfPage->obj, (pdf_obj*)PdfNames::BleedBox); }
	}
	property Box TrimBox {
		Box get() { return pdf_dict_get_rect(Context::Ptr, _pdfPage->obj, (pdf_obj*)PdfNames::TrimBox); }
	}
	property int Rotation {
		int get() {
			return pdf_dict_get_int(Context::Ptr, PagePtr, PDF_NAME(Rotate));
		}
	}
	property float UserUnit {
		float get() {
			return pdf_dict_get_real_default(Context::Ptr, PagePtr, PDF_NAME(UserUnit), 1);
		}
	}
	property bool HasTransparency {
		bool get() { return pdf_page_has_transparency(Context::Ptr, _pdfPage); }
	}
	property int AssociatedFileCount {
		int get() { return pdf_count_page_associated_files(Context::Ptr, _pdfPage); }
	}
	property PdfDictionary^ PdfObject {
		PdfDictionary^ get() {
			return gcnew PdfDictionary(_pdfPage->obj);
		}
	}
	property PdfDictionary^ Resources {
		PdfDictionary^ get() {
			auto r = pdf_page_resources(Context::Ptr, _pdfPage);
			return r ? gcnew PdfDictionary(r) : nullptr;
		}
	}
	/// <summary>
	/// Gets the /Contents object, which can be a stream or an array, from the page dictionary
	/// </summary>
	property MuPDF::PdfObject^ Contents {
		MuPDF::PdfObject^ get() {
			auto c = pdf_page_contents(Context::Ptr, _pdfPage);
			return c ? MuPDF::PdfObject::Wrap(c) : nullptr;
		}
	}
	property MuPDF::TextPage^ TextPage {
		MuPDF::TextPage^ get() {
			return _textPage != nullptr ? _textPage : (_textPage = gcnew MuPDF::TextPage(fz_new_stext_page_from_page(Context::Ptr, _page, NULL)));
		}
	}

	Box BoundPageBox(PageBoxType boxType) {
		return pdf_bound_page(Context::Ptr, _pdfPage, (fz_box_type)boxType);
	}
	void SetPageBox(PageBoxType boxType, Box box) {
		pdf_set_page_box(Context::Ptr, _pdfPage, (fz_box_type)boxType, box);
	}
	PdfArray^ GetPageBox(PageBoxType boxType);
	array<Byte>^ GetContentBytes() {
		auto c = pdf_page_contents(Context::Ptr, _pdfPage);
		if (c) {
			auto s = gcnew Stream(pdf_open_contents_stream(Context::Ptr, _pdfPage->doc, c));
			return s->ReadAll();
		}
		return nullptr;
	}

	void Run(Device^ dev, Matrix ctm, Cookie^ cookie);
	void Run(Device^ dev, Cookie^ cookie) {
		Run(dev, Matrix::Identity, cookie);
	}
	void RunContents(Device^ dev, Matrix ctm, Cookie^ cookie);
	void RunContents(Device^ dev, Cookie^ cookie) {
		RunContents(dev, Matrix::Identity, cookie);
	}
	void RunAnnotations(Device^ dev, Matrix ctm, Cookie^ cookie);
	void RunAnnotations(Device^ dev, Cookie^ cookie) {
		RunAnnotations(dev, Matrix::Identity, cookie);
	}
	void RunWidgets(Device^ dev, Matrix ctm, Cookie^ cookie);
	void RunWidgets(Device^ dev, Cookie^ cookie) {
		RunWidgets(dev, Matrix::Identity, cookie);
	}
	void FlattenInheritablePageItems() {
		pdf_flatten_inheritable_page_items(Context::Ptr, _pdfPage->obj);
	}
	void RefreshPageCache() {
		pdf_sync_page(Context::Ptr, _pdfPage);
		RefreshTextPage();
	}
	void RefreshTextPage() {
		if (_textPage) {
			delete _textPage;
			_textPage = nullptr;
		}
	}

	Equatable(Page, _page)

internal:
	Page(fz_page* page, int pageNumber) : _page(page) {
		_pdfPage = pdf_page_from_fz_page(Context::Ptr, page);
		_pageNumber = pageNumber;
		pdf_flatten_inheritable_page_items(Context::Ptr, _pdfPage->obj);
	};
	~Page() {
		ReleaseHandle();
	}
	property pdf_obj* PagePtr {
		pdf_obj* get() { return _pdfPage->obj; }
	}
protected:
	!Page() {
		ReleaseHandle();
	}
private:
	fz_page* _page;
	pdf_page* _pdfPage;
	int _pageNumber;
	MuPDF::TextPage^ _textPage;
	array<Byte>^ _contents;

	void ReleaseHandle() {
		fz_drop_page(Context::Ptr, _page);
		if (_textPage) {
			delete _textPage;
			_textPage = nullptr;
		}
		_page = NULL;
		_pdfPage = NULL;
	}
};
};

#endif // !__PAGE
