#ifndef __PAGE
#define __PAGE

#pragma once
#include "Fitz.h"
#include "Pdf.h"
#include "MuPDF.h"

using namespace System;

namespace MuPDF {

	ref class PdfObject;
	ref class PdfArray;
	ref class PdfDictionary;
	ref class Device;

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
	operator pdf_redact_options();
};

public ref class Page sealed : IDisposable, IEquatable<Page^> {
public:
	property int PageNumber { int get(); }

	/// <summary>
	/// Determine the page size in points, taking page rotation into account. The page size is taken to be the crop box if it exists (visible area after cropping), otherwise the media box will be used (possibly including printing marks).
	/// </summary>
	property Box Bound { Box get(); }
	property Box MediaBox { Box get(); }
	property Box CropBox { Box get(); }
	property Box ArtBox { Box get(); }
	property Box BleedBox {
		Box get();
	}
	property Box TrimBox {
		Box get();
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
	property PdfDictionary^ PdfObject { PdfDictionary^ get(); }
	property PdfDictionary^ Resources { PdfDictionary^ get(); }
	/// <summary>
	/// Gets the /Contents object, which can be a stream or an array, from the page dictionary
	/// </summary>
	property MuPDF::PdfObject^ Contents {
		MuPDF::PdfObject^ get();
	}
	property MuPDF::TextPage^ TextPage { MuPDF::TextPage^ get(); }

	Box BoundPageBox(PageBoxType boxType) {
		return pdf_bound_page(Context::Ptr, _pdfPage, (fz_box_type)boxType);
	}
	void SetPageBox(PageBoxType boxType, Box box) {
		pdf_set_page_box(Context::Ptr, _pdfPage, (fz_box_type)boxType, box);
	}
	PdfArray^ GetPageBox(PageBoxType boxType);
	array<Byte>^ GetContentBytes();

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
	MuPDF::PdfObject^ GetAssociatedFile(int index);
	void RefreshPageCache() {
		pdf_sync_page(Context::Ptr, _pdfPage);
		RefreshTextPage();
	}
	void RefreshTextPage();

	Equatable(Page, _page)

internal:
	Page(fz_page* page, int pageNumber) : _page(page) {
		_pdfPage = pdf_page_from_fz_page(Context::Ptr, page);
		_pageNumber = pageNumber;
		pdf_flatten_inheritable_page_items(Context::Ptr, _pdfPage->obj);
	};
	~Page() {
		this->!Page();
	}
	property pdf_page* Ptr {
		pdf_page* get() { return _pdfPage; }
	}
	property pdf_obj* PagePtr {
		pdf_obj* get() { return _pdfPage->obj; }
	}
protected:
	!Page();

private:
	fz_page* _page;
	pdf_page* _pdfPage;
	int _pageNumber;
	MuPDF::TextPage^ _textPage;
	array<Byte>^ _contents;
};
};

#endif // !__PAGE
