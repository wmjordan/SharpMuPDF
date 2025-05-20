#include "mupdf/fitz.h"
#include "mupdf/pdf.h"
#include "MuPDF.h"

#ifndef __DOCUMENT
#define __DOCUMENT

#pragma once
using namespace System;
using namespace System::Collections::Generic;

namespace MuPDF {

ref class WriterOptions;
enum class PageLabelStyle;

public ref class Document sealed : IDisposable, IEquatable<Document^> {
public:
	property int PageCount {
		int get() {
			return _pageCount < 0
				? (_pageCount = pdf_count_pages(Context::Ptr, _pdf))
				: _pageCount;
		}
	}
	property int Version {
		int get() { return _pdf->version; }
	}
	property int ObjectCount {
		int get() { return pdf_count_objects(Context::Ptr, _pdf); }
	}
	property long FileSize {
		long get() { return _pdf->file_size; }
	}
	property long StartXref {
		long get() { return _pdf->startxref; }
	}
	property long XrefBase {
		long get() { return _pdf->xref_base; }
	}
	property int AssociatedFileCount {
		int get() { return pdf_count_document_associated_files(Context::Ptr, _pdf); }
	}
	property int IncrementalSectionCount {
		int get() { return _pdf->num_incremental_sections; }
	}
	property int XrefSectionCount {
		int get() { return _pdf->num_xref_sections; }
	}
	property bool NeedsPassword {
		bool get() { return fz_needs_password(Context::Ptr, _document); }
	}
	property bool WasRepaired {
		bool get() { return pdf_was_repaired(Context::Ptr, _pdf); }
	}
	property bool RepairAttempted {
		bool get() { return _pdf->repair_attempted; }
	}
	property bool PageTreeBroken {
		bool get() { return _pdf->page_tree_broken; }
	}
	property bool HasUnsavedChanges {
		bool get() { return pdf_has_unsaved_changes(Context::Ptr, _pdf); }
	}
	property bool Redacted {
		bool get() { return _pdf->redacted; }
	}
	property bool ResyncRequired {
		bool get() { return _pdf->resynth_required; }
	}
	property int OrphansCount {
		int get() { return _pdf->orphans_count; }
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

	static MuPDF::Document^ Open(String^ filePath);
	static MuPDF::Document^ Open(array<Byte>^ memoryFile);

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
		RefreshPageCount();
	}
	void AppendPage(Page^ page) {
		pdf_insert_page(Context::Ptr, _pdf, INT_MAX, page->PagePtr);
		RefreshPageCount();
	}
	/// <summary>
	/// Removes specific page from document (page contents and resources are still preserved).
	/// </summary>
	/// <param name="pageNumber">The page number (starts from 0).</param>
	void DeletePage(int pageNumber) {
		pdf_delete_page(Context::Ptr, _pdf, pageNumber);
		RefreshPageCount();
	}
	/// <summary>
	/// Removes specific pages [start, end) from document. This does not remove the page contents or resources from the file.
	/// </summary>
	/// <param name="start">The first page number to be removed (starts from 0).</param>
	/// <param name="end">The last page number remained (starts from 0). If end is negative or greater than the number of pages in the document, it will be taken to be the end of the document.</param>
	void DeletePage(int start, int end) {
		pdf_delete_page_range(Context::Ptr, _pdf, start, end);
		RefreshPageCount();
	}
	/// <summary>
	/// Rearrange pages with the given order and numbers in <paramref name="pageNumbers"/>.
	/// </summary>
	/// <param name="pageNumbers">The page numbers of the rearranged document.</param>
	void RearrangePages(array<int>^ pageNumbers) {
		pin_ptr<int> n = &pageNumbers[0];
		pdf_rearrange_pages(Context::Ptr, _pdf, pageNumbers->Length, n, pdf_clean_options_structure::PDF_CLEAN_STRUCTURE_DROP);
	}
	/// <summary>
	/// Graft a page (and its resources) from the <paramref name="src"/> document to the destination document of the graft. This involves a deep copy of the objects in question.
	/// </summary>
	/// <param name="src">The source document to copy from.</param>
	/// <param name="pageFrom">The source page number (pages numbered from 0, with -1 meaning "at the end").</param>
	/// <param name="pageTo">The position within the destination document at which the page should be inserted (pages numbered from 0, with - 1 meaning "at the end").</param>
	void GraftPageFrom(Document^ src, int pageFrom, int pageTo) {
		pdf_graft_page(Context::Ptr, _pdf, pageTo, src->_pdf, pageFrom);
		RefreshPageCount();
	}
	void GraftPagesFrom(Document^ src, int pageFrom, int numberOfPages, int pageTo);
	void GraftPagesFrom(Document^ src, System::Collections::Generic::IEnumerable<int>^ srcPages, int pageTo);

	/// <summary>
	/// Sets page label from page <paramref name="index"/>.
	/// </summary>
	/// <param name="index">The page index (numbered from 0).</param>
	/// <param name="style">The label style.</param>
	/// <param name="prefix">The prefix of the page label.</param>
	/// <param name="start">The start number of the label.</param>
	void SetPageLabel(int index, PageLabelStyle style, String^ prefix, int start);
	/// <summary>
	/// Deletes page label from page <paramref name="index"/>.
	/// </summary>
	/// <param name="index">The page index (numbered from 0).</param>
	void DeletePageLabel(int index) {
		pdf_delete_page_labels(Context::Ptr, _pdf, index);
	}

	MuPDF::PdfObject^ GetAssociatedFile(int index) {
		return MuPDF::PdfObject::Wrap(pdf_document_associated_file(Context::Ptr, _pdf, index));
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

	void RefreshPageCount() {
		_pageCount = -1;
	}
};

public enum class CompressionMode
{
	None,
	ZLib,
	Broti
};
public enum class GarbageCollectionMode
{
	None,
	CollectGarbage,
	Renumber,
	DeDuplicate
};
public enum class EncryptionMode
{
	Keep,
	Remove,
	Rc4_40,
	Rc4_128,
	Aes_128,
	Aes_256,
	Unknown
};
[FlagsAttribute]
public enum class Permissions
{
	None,
	Print = 1 << 2,
	Modify = 1 << 3,
	Copy = 1 << 4,
	Annotate = 1 << 5,
	Form = 1 << 8,
	Accessibility = 1 << 9, /* Deprecated In Pdf 2.0 (This Permission Is Always Granted) */
	Assemble = 1 << 10,
	PrintHq = 1 << 11,
};
public enum class PageLabelStyle
{
	None,
	Decimal = 'D',
	UpperRoman = 'R',
	LowerRoman = 'r',
	UpperAlpha = 'A',
	LowerAlpha = 'a'
};

public ref class WriterOptions sealed {
public:
	/// <summary>
	/// Write just the changed objects.
	/// </summary>
	bool Incremental;
	/// <summary>
	/// Pretty-print dictionaries and arrays.
	/// </summary>
	bool Pretty;
	/// <summary>
	/// ASCII hex encode binary streams.
	/// </summary>
	bool Ascii;
	/// <summary>
	/// Compress streams.
	/// </summary>
	CompressionMode CompressionMode;
	/// <summary>
	/// Compress (or leave compressed) image streams.
	/// </summary>
	bool CompressImages;
	/// <summary>
	/// Compress (or leave compressed) font streams.
	/// </summary>
	bool CompressFonts;
	/// <summary>
	/// Decompress streams (except when compressing images/fonts).
	/// </summary>
	bool Decompress;
	/// <summary>
	/// Garbage collect objects before saving.
	/// </summary>
	GarbageCollectionMode Garbage;
	bool Linear;
	bool Clean;
	bool Sanitize;
	bool Appearance;
	EncryptionMode Encrypt;
	bool DoNotRegenerateId;
	/// <summary>
	/// Document encryption permissions.
	/// </summary>
	Permissions Permissions;
	array<Byte>^ OwnerPassword;
	array<Byte>^ UserPassword;
	bool Snapshot;
	bool PreserveMetadata;
	bool UseObjectStreams;
	/// <summary>
	/// 0: Default, 1: min, 100: max
	/// </summary>
	int CompressionEffort;
	/// <summary>
	/// Add labels to each object showing how it can be reached from the Root.
	/// </summary>
	bool AddLabels;
internal:
	pdf_write_options ToNative();
};

}

#endif // !__DOCUMENT
