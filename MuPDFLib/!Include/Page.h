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

public enum class RedactImageMethod
{
	/// <summary>
	/// Do not change images at all.
	/// </summary>
	None,
	/// <summary>If the image intrudes across the redaction region(even if clipped), remove it.</summary>
	Remove,
	/// <summary>If the image intrudes across the redaction region(even if clipped), replace the bit that intrudes with black pixels..</summary>
	RemovePixels,
	/// <summary>
	/// If the image, when clipped, intrudes across the redaction
	///  region, remove it completely.Note: clipped is a rough estimate
	///  based on the bbox of clipping paths.
	///
	///  Essentially this says "remove any image that has visible parts
	///  that extend into the redaction region".
	/// 
	/// This method can effectively 'leak' invisible information during
	///  the redaction phase, so should be used with caution.
	/// </summary>
	RemoveUnlessInvisible
};

public enum class RedactLineArtMethod {
	None,
	RemoveIfCovered,
	RemoveIfTouched
};

public enum class RedactTextMethod {
	/* Remove any text that overlaps with the redaction region,
	 * however slightly. This is the default option, and is the
	 * correct option for secure behaviour. */
	Remove,
	/* Do not remove any text at all as part of this redaction
	 * operation. Using this option is INSECURE! Use at your own
	 * risk. */
	Keep
};

public ref class PdfRedactOptions {
public:
	property bool BlackBoxes;
	property RedactImageMethod ImageMethod;
	property RedactLineArtMethod LineArt;
	property RedactTextMethod Text;
internal:
	// implicitly convert PdfRedactOptions to pdf_redact_options
	operator pdf_redact_options() {
		pdf_redact_options opts;
		opts.black_boxes = BlackBoxes;
		opts.image_method = (int)ImageMethod;
		opts.line_art = (int)LineArt;
		opts.text = (int)Text;
		return opts;
	}
};


public ref class Page sealed : IDisposable, IEquatable<Page^> {
public:
	property int PageNumber {
		// we don't use _pageNumber here, since pages can be reordered after editing operations
		int get() { return pdf_lookup_page_number(Context::Ptr, _pdfPage->doc, _pdfPage->obj); }
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

	/// <summary>
	/// Clip contents, links and annotations within this page.
	/// </summary>
	/// <param name="box">The preserved region within this page.</param>
	void Clip(Box box) {
		auto r = (fz_rect)box;
		pdf_clip_page(Context::Ptr, _pdfPage, &r);
	}

	void SyncPage() {
		pdf_sync_page(Context::Ptr, _pdfPage);
	}
	/// <summary>
	/// Remove cached links and annotations within this page.
	/// </summary>
	void NukePage() {
		pdf_nuke_page(Context::Ptr, _pdfPage);
	}
	void SyncLinks() {
		pdf_sync_links(Context::Ptr, _pdfPage);
	}
	/// <summary>
	/// Remove cached links within this page.
	/// </summary>
	void NukeLinks() {
		pdf_nuke_links(Context::Ptr, _pdfPage);
	}
	void SyncAnnotations() {
		pdf_sync_annots(Context::Ptr, _pdfPage);
	}
	/// <summary>
	/// Remove cached annotations within this page.
	/// </summary>
	void NukeAnnotations() {
		pdf_nuke_annots(Context::Ptr, _pdfPage);
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
	MuPDF::PdfObject^ GetAssociatedFile(int index) {
		return MuPDF::PdfObject::Wrap(pdf_page_associated_file(Context::Ptr, _pdfPage, index));
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
	property pdf_page* Ptr {
		pdf_page* get() { return _pdfPage; }
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
