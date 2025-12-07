#include "MuPDF.h"

#ifndef __FONT_DESC
#define __FONT_DESC
#pragma once

using namespace System;

namespace MuPDF {

[Flags]
public enum class FontDescriptorFlags {
	None,
	FixedPitch = 1 << 0,
	Serif = 1 << 1,
	Symbolic = 1 << 2,
	Script = 1 << 3,
	Nonsymbolic = 1 << 5,
	Italic = 1 << 6,
	AllCap = 1 << 16,
	SmallCap = 1 << 17,
	ForceBold = 1 << 18
};

public ref class FontDescriptor sealed : IDisposable {

public:
	FontDescriptor() : FontDescriptor(pdf_new_font_desc(Context::Ptr)) {
	}
	property TextFont^ Font {
		TextFont^ get() {
			return _textFont ? _textFont : (_textFont = gcnew TextFont(_font->font));
		}
	}
	property FontDescriptorFlags Flags {
		FontDescriptorFlags get() { return (FontDescriptorFlags)_font->flags; }
	}
	property float ItalicAngle {
		float get() { return _font->italic_angle; }
	}
	property float Ascent {
		float get() { return _font->ascent; }
	}
	property float Descent {
		float get() { return _font->descent; }
	}
	property float CapHeight {
		float get() { return _font->cap_height; }
	}
	property float XHeight {
		float get() { return _font->x_height; }
	}
	property float MissingWidth {
		float get() { return _font->missing_width; }
	}
	property bool IsEmbedded {
		bool get() { return _font->is_embedded; }
	}
	property int CidToGidLength {
		int get() { return _font->cid_to_gid_len; }
	}
	property int CidToUcsLength {
		int get() { return _font->cid_to_ucs_len; }
	}
	int CidToGid(int cid) {
		return pdf_font_cid_to_gid(Context::Ptr, _font, cid);
	}
	static FontDescriptor^ Load(Document^ doc, PdfDictionary^ resources, PdfDictionary^ font);

internal:
	FontDescriptor(pdf_font_desc* font) : _font(font) {
		pdf_keep_font(Context::Ptr, font);
	}
	~FontDescriptor() {
		ReleaseHandle();
	}

private:
	pdf_font_desc* _font;
	TextFont^ _textFont;

	void ReleaseHandle() {
		pdf_drop_font(Context::Ptr, _font);
		_font = NULL;
	}
};

}

#endif