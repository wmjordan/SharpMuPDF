#include "FontDescriptor.h"

#pragma unmanaged
DLLEXP pdf_font_desc* LoadFontDescriptor(fz_context* ctx, pdf_document* doc, pdf_resource_stack* res, pdf_obj* font) {
	pdf_font_desc* f;
	MuTryReturn(ctx, pdf_load_font(ctx, doc, res, font), f)
}

#pragma managed
using namespace MuPDF;
using namespace System::Text;

static int Unhex(unsigned char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return 0;
}

static int ExtractLiteral(unsigned char* p, unsigned char* end, unsigned char* buf) {
    int len = 0;
    int depth = 1;

    while (p < end && depth > 0) {
        if (*p == '\\') {
            p++;
            if (p >= end) break;
            switch (*p) {
            case 'n':  buf[len++] = '\n'; p++; break;
            case 'r':  buf[len++] = '\r'; p++; break;
            case 't':  buf[len++] = '\t'; p++; break;
            case 'b':  buf[len++] = '\b'; p++; break;
            case 'f':  buf[len++] = '\f'; p++; break;
            case '(':  buf[len++] = '(';  p++; break;
            case ')':  buf[len++] = ')';  p++; break;
            case '\\': buf[len++] = '\\'; p++; break;
            case '\n': p++; break;
            case '\r': p++; if (p < end && *p == '\n') p++; break;
            default:
                if (*p >= '0' && *p <= '7') {
                    int val = *p - '0';
                    p++;
                    if (p < end && *p >= '0' && *p <= '7') {
                        val = val * 8 + (*p - '0'); p++;
                        if (p < end && *p >= '0' && *p <= '7') {
                            val = val * 8 + (*p - '0'); p++;
                        }
                    }
                    buf[len++] = (unsigned char)val;
                }
                else { buf[len++] = *p; p++; }
                break;
            }
        }
        else if (*p == '(') { depth++; buf[len++] = '('; p++; }
        else if (*p == ')') { depth--; if (depth > 0) buf[len++] = ')'; p++; }
        else { buf[len++] = *p; p++; }
    }
    return len;
}

static int ExtractHex(unsigned char* p, unsigned char* end, unsigned char* buf) {
    int len = 0;
    while (p < end && *p != '>') {
        if (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
            p++; continue;
        }
        int hi = Unhex(*p); p++;
        int lo = 0;
        if (p < end && *p != '>') { lo = Unhex(*p); p++; }
        buf[len++] = (unsigned char)((hi << 4) | lo);
    }
    return len;
}

TextFont^ FontDescriptor::Font::get() {
	return _textFont ? _textFont : (_textFont = gcnew TextFont(_font->font));
}

void FontDescriptor::DecodeText(array<unsigned char>^ bytes, int offset, int length, StringBuilder^ sb) {
    if (bytes == nullptr)
        throw gcnew ArgumentNullException("bytes");
    if (sb == nullptr)
        throw gcnew ArgumentNullException("sb");
    if (offset < 0 || length < 0 || offset + length > bytes->Length)
        throw gcnew ArgumentOutOfRangeException();
    if (length < 2)
        return;

    pin_ptr<unsigned char> pinned = &bytes[offset];
    unsigned char* start = pinned;
    unsigned char* end = start + length;

    if (*start == '(') {
        // literal string
        // detect escape character
        unsigned char* innerStart = start + 1;
        unsigned char* innerEnd = end - 1; // 跳过尾部 ')'
        bool hasEscapeOrNest = false;

        for (unsigned char* p = innerStart; p < innerEnd; p++) {
            if (*p == '\\') {
                hasEscapeOrNest = true;
                break;
            }
        }

        if (!hasEscapeOrNest) {
            DecodeRawBytes(innerStart, innerEnd, sb);
        }
        else {
            unsigned char* buf = new unsigned char[length];
            try {
                int bufLen = ExtractLiteral(innerStart, end, buf);
                DecodeRawBytes(buf, buf + bufLen, sb);
            }
            finally {
                delete[] buf;
            }
        }
    }
    else if (*start == '<') {
        // HEX string
        // two characters to one byte: (length-2+1)/2, allowing odd number of chars
        int bufCap = (length - 1) / 2;
        unsigned char* buf = new unsigned char[bufCap];
        try {
            int bufLen = ExtractHex(start + 1, end, buf);
            DecodeRawBytes(buf, buf + bufLen, sb);
        }
        finally {
            delete[] buf;
        }
    }
    else {
        throw gcnew ArgumentException("Input must be a PDF literal string (...) or hex string <...>.");
    }
}

void FontDescriptor::DecodeRawBytes(unsigned char* data, unsigned char* end, StringBuilder^ sb) {
    while (data < end) {
        unsigned int cpt;
        int consumed = pdf_decode_cmap(_font->encoding, data, end, &cpt);
        if (consumed == 0) break;
        data += consumed;

        int cid = pdf_lookup_cmap(_font->encoding, cpt);
        if (cid < 0) { sb->Append(L'\uFFFD'); continue; }

        int ucsbuf[256];
        int ucslen = 0;
        if (_font->to_unicode)
            ucslen = pdf_lookup_cmap_full(_font->to_unicode, cid, ucsbuf);
        if (ucslen == 0 && (size_t)cid < _font->cid_to_ucs_len) {
            ucsbuf[0] = _font->cid_to_ucs[cid]; ucslen = 1;
        }
        if (ucslen == 0 || (ucslen == 1 && ucsbuf[0] == 0)) {
            sb->Append(L'\uFFFD'); continue;
        }
        for (int i = 0; i < ucslen; i++)
            sb->Append((wchar_t)ucsbuf[i]);
    }
}

FontDescriptor^ FontDescriptor::Load(Document^ doc, PdfDictionary^ resources, PdfDictionary^ font) {
	pdf_resource_stack* res = fz_malloc_struct(Context::Ptr, pdf_resource_stack);
	res->resources = resources->Ptr;
	auto fd = LoadFontDescriptor(Context::Ptr, doc->Ptr, res, font->Ptr);
    fz_free(Context::Ptr, res);
	if (fd) {
		return gcnew FontDescriptor(fd);
	}
	throw MuException::FromContext();
}

FontDescriptor^ MuPDF::FontDescriptor::Load(Document^ doc, ResourceStack^ resources, PdfDictionary^ font) {
    auto fd = LoadFontDescriptor(Context::Ptr, doc->Ptr, resources->Ptr, font->Ptr);
    if (fd) {
        return gcnew FontDescriptor(fd);
    }
    throw MuException::FromContext();
}

FontDescriptor::~FontDescriptor() {
	DisposeObject(_textFont);
	this->!FontDescriptor();
}
