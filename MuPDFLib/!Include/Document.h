#include "mupdf/fitz.h"
#include "mupdf/pdf.h"
#include "MuPDF.h"

#ifndef __DOCUMENT
#define __DOCUMENT

#pragma once
using namespace System;

namespace MuPDF {

ref class WriterOptions;

public ref class Document sealed : IDisposable, IEquatable<Document^> {
public:
	property int PageCount {
		int get() { return _pageCount; }
	}
	property int ObjectCount {
		int get() { return pdf_count_objects(Context::Ptr, _pdf); }
	}
	property int AssociatedFileCount {
		int get() { return pdf_count_document_associated_files(Context::Ptr, _pdf); }
	}
	property bool NeedsPassword {
		bool get() { return fz_needs_password(Context::Ptr, _document); }
	}
	property bool WasRepaired {
		bool get() { return pdf_was_repaired(Context::Ptr, _pdf); }
	}
	property bool HasUnsavedChanges {
		bool get() { return pdf_has_unsaved_changes(Context::Ptr, _pdf); }
	}
	property String^ FilePath {
		String^ get() { return _path; }
		internal: void set(String^ value) { _path = value; }
	}
	property PdfDictionary^ Trailer {
		PdfDictionary^ get() {
			return gcnew PdfDictionary(_trailer);
		}
	}
	property PdfDictionary^ Root {
		PdfDictionary^ get() {
			return gcnew PdfDictionary(pdf_dict_get(Context::Ptr, _trailer, PDF_NAME(Root)));
		}
	}
	property PdfDocumentInfo^ Info {
		PdfDocumentInfo^ get() {
			return gcnew PdfDocumentInfo(pdf_dict_get(Context::Ptr, _trailer, PDF_NAME(Info)));
		}
	}
	property bool CanUndo {
		bool get() { return pdf_can_undo(Context::Ptr, _pdf); }
	}
	property bool CanRedo {
		bool get() { return pdf_can_redo(Context::Ptr, _pdf); }
	}
	property bool CanBeSavedIncrementally {
		bool get() { return pdf_can_be_saved_incrementally(Context::Ptr, _pdf); }
	}
	property bool IsDisposed {
		bool get() { return _document == NULL; }
	}

	/// <summary>
	/// Loads specific page from document.
	/// </summary>
	/// <param name="pageNumber">The page number (starts from 0).</param>
	/// <returns>The loaded page.</returns>
	Page^ LoadPage(int pageNumber);
	int GetPageNumber(Page^ page) {
		return pdf_lookup_page_number(Context::Ptr, _pdf, page->PagePtr);
	}
	/// <summary>
	/// Insert a page previously created by NewPage into the pages tree of the document.
	/// </summary>
	/// <param name="page">The page to be inserted.</param>
	/// <param name="beforePageNumber">The page number to insert at (pages numbered from 0). 0 <= n <= page_count inserts before page n. Negative numbers or INT_MAX are treated as page count, and insert at the end. 0 inserts at the start. All existing pages are after the insertion point are shuffled up.</param>
	void InsertPage(Page^ page, int beforePageNumber) {
		pdf_insert_page(Context::Ptr, _pdf, beforePageNumber, page->PagePtr);
	}
	void AppendPage(Page^ page) {
		pdf_insert_page(Context::Ptr, _pdf, INT_MAX, page->PagePtr);
	}
	/// <summary>
	/// Removes specific page from document.
	/// </summary>
	/// <param name="pageNumber">The page number (starts from 0).</param>
	void DeletePage(int pageNumber) {
		pdf_delete_page(Context::Ptr, _pdf, pageNumber);
	}
	/// <summary>
	/// Removes specific pages [start, end) from document. This does not remove the page contents or resources from the file.
	/// </summary>
	/// <param name="start">The first page number to be removed (starts from 0).</param>
	/// <param name="end">The last page number remained (starts from 0). If end is negative or greater than the number of pages in the document, it will be taken to be the end of the document.</param>
	void DeletePage(int start, int end) {
		pdf_delete_page_range(Context::Ptr, _pdf, start, end);
	}
	void RearrangePages(array<int>^ pageNumbers) {
		pin_ptr<int> n = &pageNumbers[0];
		pdf_rearrange_pages(Context::Ptr, _pdf, pageNumbers->Length, n);
	}
	void SyncOpenPages() {
		pdf_sync_open_pages(Context::Ptr, _pdf);
	}
	void Save(String^ filePath, WriterOptions^ options);
	void SaveSnapshot(String^ filePath);
	bool CheckPassword(String^ password);
	void CloseFile() {
		ReleaseHandle();
	}
	void Reopen();

#pragma region Object creation
	PdfDictionary^ NewPage(Box mediaBox, int rotate, PdfDictionary^ resources, array<Byte>^ contents);
	PdfDictionary^ NewDictionary(int capacity) {
		return gcnew PdfDictionary(pdf_new_dict(Context::Ptr, _pdf, capacity));
	}
	PdfArray^ NewArray(int capacity) {
		return gcnew PdfArray(pdf_new_array(Context::Ptr, _pdf, capacity));
	}
	PdfArray^ NewBox(Box box) {
		return gcnew PdfArray(pdf_new_rect(Context::Ptr, _pdf, box));
	}
	PdfArray^ NewMatrix(Matrix matrix) {
		return gcnew PdfArray(pdf_new_matrix(Context::Ptr, _pdf, matrix));
	}
#pragma endregion

#pragma region Undo-redo
	void EnableJournal() {
		pdf_enable_journal(Context::Ptr, _pdf);
	}
	void BeginOperation() {
		pdf_begin_implicit_operation(Context::Ptr, _pdf);
	}
	void CancelOperation() {
		pdf_abandon_operation(Context::Ptr, _pdf);
	}
	void EndOperation() {
		pdf_end_operation(Context::Ptr, _pdf);
	}
	void Undo() {
		pdf_undo(Context::Ptr, _pdf);
	}
	void Redo() {
		pdf_redo(Context::Ptr, _pdf);
	}
#pragma endregion

	Equatable(Document, _document)

internal:
	Document() {
		auto d = pdf_create_document(Context::Ptr);
		_document = (fz_document*)d;
		InitTrailer();
	}
	Document(fz_document* document) : _document(document) {
		InitTrailer();
	};
	Document(fz_stream* stream);
	~Document() {
		ReleaseHandle();
	}
	property pdf_document* Ptr {
		pdf_document* get() { return _pdf; }
	}
private:
	fz_document* _document;
	fz_stream* _stream;
	pdf_document* _pdf;
	pdf_obj* _trailer;
	String^ _path;
	int _pageCount;

	void InitTrailer();

	void ReleaseHandle() {
		fz_context* ctx = Context::Ptr;
		fz_drop_document(ctx, _document);
		fz_drop_stream(ctx, _stream);
		_document = NULL;
		_pdf = NULL;
		_trailer = NULL;
		_stream = NULL;
	}
	void OpenStream(fz_stream* stream);
};

public ref class WriterOptions sealed {
public:
	bool Incremental;
	bool Pretty;
	bool Ascii;
	bool Compress;
	bool CompressImages;
	bool CompressFonts;
	bool Decompress;
	bool Garbage;
	bool Linear;
	bool Clean;
	bool Sanitize;
	bool Appearance;
	bool Decrypt;
	bool DoNotRegenerateId;
	bool Permissions;
	array<Byte>^ OwnerPassword;
	array<Byte>^ UserPassword;
	bool Snapshot;
	bool PreserveMetadata;
	bool UseObjectStreams;
	int CompressionEffort;
internal:
	pdf_write_options ToNative();
};
}

#endif // !__DOCUMENT
