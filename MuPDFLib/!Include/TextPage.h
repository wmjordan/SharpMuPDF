#include "mupdf/fitz.h"
#include "mupdf/pdf.h"
#include "MuPDF.h"

#ifndef __TEXTPAGE
#define __TEXTPAGE

using namespace System;
using namespace System::Collections;
using namespace System::Text;

#pragma once
namespace MuPDF {

[FlagsAttribute()]
public enum class TextOption {
	None,
	PreserveLigatures = 1,
	PreserveWhitespace = 2,
	PreserveImages = 4,
	InhibitSpaces = 8,
	Dehyphenate = 16,
	PreserveSpans = 32,
	MediaBoxClip = 64,
	UseCidForUnknownUnicode = 128,
};

[FlagsAttribute]
public enum class FontFlags {
	None,
	Mono,
	Serif = 1 << 1,
	Bold = 1 << 2,
	Italic = 1 << 3,
	UseSubstituteMetrics = 1 << 4,
	StretchToMatchPDFMetrics = 1 << 5,
	FakeBold = 1 << 6,
	FakeItalic = 1 << 7,
	HasOpenType = 1 << 8,
	InvalidBBox = 1 << 9,
	CJK = 1 << 10,
	Lang0 = 1 << 11,
	Lang1 = 1 << 12,
	Embed = 1 << 13,
	NeverEmbed = 1 << 14,
};

public ref class TextOptions {
public:
	TextOption Flags;
	/// <summary>
	/// Defines scale ratio for text rendition. Base resolution is 96 DPI.
	/// </summary>
	float Scale = 1;

	static operator fz_stext_options(TextOptions^ options);
};

public ref class TextFont : IEquatable<TextFont^> {
public:
	property String^ Name {
		String^ get() {
			return _name ? _name : (_name = gcnew String(_font->name));
		}
	}
	property int GlyphCount {
		int get() { return _font->glyph_count; }
	}
	property int WidthCount {
		int get() { return _font->width_count; }
	}
	property short WidthDefault {
		short get() { return _font->width_default; }
	}
	property FontFlags Flags {
		FontFlags get() { return (FontFlags)*(int*)&(_font->flags); }
	}
	array<short>^ GetWidths() {
		GcnewArray(short, a, _font->width_count);
		System::Runtime::InteropServices::Marshal::Copy((System::IntPtr)(void*)_font->width_table, a, 0, _font->width_count);
		return a;
	}
	int GetCharacter(int cid) {
		return ft_char_index(_font->ft_face, cid);
	}
	int Encode(int unicode) {
		return fz_encode_character(Context::Ptr, _font, unicode);
	}
	float Advance(int glyph, bool vertical) {
		return fz_advance_glyph(Context::Ptr, _font, glyph, vertical);
	}
	Equatable(TextFont, _font)

internal:
	TextFont(fz_font* font) : _font(font) {};

private:
	fz_font* _font;
	String^ _name;
};

public ref class TextChar : Generic::IEnumerable<TextChar^>, IEquatable<TextChar^> {
public:
	/// <summary>
	/// Gets the Unicode code point for this character.
	/// </summary>
	property int Character {
		int get() { return _ch->c; }
	}
	/// <summary>
	/// Gets the sRGB Hex color.
	/// </summary>
	property int Color {
		int get() { return _ch->color; }
	}
	property float Size {
		float get() { return _ch->size; }
	}
	property TextFont^ Font {
		TextFont^ get() { return gcnew TextFont(_ch->font); }
	}
	property TextChar^ Next {
		TextChar^ get() { return _ch->next ? gcnew TextChar(_ch->next) : nullptr; }
	}
	property Point Origin {
		Point get() { return _ch->origin; }
	}
	property MuPDF::Quad Quad {
		MuPDF::Quad get() { return _ch->quad; }
	}
	static operator Char(TextChar^ ch) {
		return ch->_ch->c;
	}
internal:
	TextChar(fz_stext_char* ch) : _ch(ch) {}
	property fz_stext_char* Ptr {
		fz_stext_char* get() { return _ch; }
	}
private:
	fz_stext_char* _ch;

#pragma region IEnumerator
public:
	virtual Generic::IEnumerator<MuPDF::TextChar^>^ GetEnumerator() sealed = Generic::IEnumerable<MuPDF::TextChar^>::GetEnumerator {
		return gcnew Enumerator<TextChar, fz_stext_char>(_ch, _ch->next);
	}

	virtual System::Collections::IEnumerator^ GetEnumeratorBase() sealed = System::Collections::IEnumerable::GetEnumerator {
		return GetEnumerator();
	}
#pragma endregion

#pragma region IEquatable
	Equatable(TextChar, _ch)
#pragma endregion

};

public ref class TextLine : Generic::IEnumerable<TextChar^>, IEquatable<MuPDF::TextLine^> {
public:
	property bool IsVertical {
		bool get() { return _line->wmode; }
	}
	property Box Bound {
		Box get() { return _line->bbox; }
	}
	property TextChar^ FirstCharacter {
		TextChar^ get() { return gcnew TextChar(_line->first_char); }
	}
	property TextChar^ LastCharacter {
		TextChar^ get() { return gcnew TextChar(_line->last_char); }
	}
	String^ ToString() override {
		StringBuilder^ sb = gcnew StringBuilder();
		fz_stext_char* c = _line->first_char;
		do {
			sb->Append((Char)c->c);
		} while (c = c->next);
		return sb->ToString();
	}
internal:
	TextLine(fz_stext_line* line) : _line(line) {}
	property fz_stext_line* Ptr {
		fz_stext_line* get() { return _line; }
	}
private:
	fz_stext_line* _line;

#pragma region IEnumerator
public:
	virtual Generic::IEnumerator<MuPDF::TextChar^>^ GetEnumerator() sealed = Generic::IEnumerable<MuPDF::TextChar^>::GetEnumerator {
		return gcnew Enumerator<TextChar, fz_stext_char>(_line->first_char, _line->last_char);
	}

	virtual System::Collections::IEnumerator^ GetEnumeratorBase() sealed = System::Collections::IEnumerable::GetEnumerator{
		return GetEnumerator();
	}
#pragma endregion

#pragma region IEquatable
	Equatable(TextLine, _line)
#pragma endregion

};

public ref class TextBlock : Generic::IEnumerable<TextLine^>, IEquatable<TextBlock^> {
public:
	property bool IsImageBlock {
		bool get() { return _block->type; }
	}
	property Box Bound {
		Box get() { return _block->bbox; }
	}
	virtual String^ ToString() override;

internal:
	TextBlock(fz_stext_block* block) : _block(block) {
	}
	property fz_stext_block* Ptr {
		fz_stext_block* get() { return _block; }
	}
private:
	fz_stext_block* _block;

#pragma region IEnumerator
public:
	virtual Generic::IEnumerator<MuPDF::TextLine^>^ GetEnumerator() sealed = Generic::IEnumerable<MuPDF::TextLine^>::GetEnumerator{
		return _block->type == FZ_STEXT_BLOCK_TEXT ? gcnew Enumerator<TextLine, fz_stext_line>(_block->u.t.first_line, _block->u.t.last_line) : EmptyCollection<MuPDF::TextLine^>::GetEnumerator();
	}

	virtual System::Collections::IEnumerator^ GetEnumeratorBase() sealed = System::Collections::IEnumerable::GetEnumerator{
			return GetEnumerator();
	}
#pragma endregion

#pragma region IEquatable
	Equatable(TextBlock, _block)
#pragma endregion

};

public ref class TextPage : Generic::IEnumerable<TextBlock^> {
public:
	property Box Bound {
		Box get() { return _page->mediabox; }
	}
	property TextBlock^ FirstBlock {
		TextBlock^ get() { return gcnew TextBlock(_page->first_block); }
	}
	property TextBlock^ LastBlock {
		TextBlock^ get() { return gcnew TextBlock(_page->last_block); }
	}
internal:
	TextPage(fz_stext_page* page) : _page(page) {};
	~TextPage() {
		ReleaseHandle();
	}
	property fz_stext_page* Ptr {
		fz_stext_page* get() { return _page; }
	}
private:
	fz_stext_page* _page;

	void ReleaseHandle() {
		fz_drop_stext_page(Context::Ptr, _page);
		_page = NULL;
	}


public:
	virtual Generic::IEnumerator<MuPDF::TextBlock^>^ GetEnumerator() sealed = Generic::IEnumerable<MuPDF::TextBlock^>::GetEnumerator {
		return gcnew Enumerator<TextBlock, fz_stext_block>(_page->first_block, _page->last_block);
	}

	virtual System::Collections::IEnumerator^ GetEnumeratorBase() sealed = System::Collections::IEnumerable::GetEnumerator {
		return GetEnumerator();
	}

};

};

#endif // !__TEXTPAGE
