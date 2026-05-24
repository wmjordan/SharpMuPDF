#include "FontDescriptor.h"

#pragma unmanaged
DLLEXP pdf_font_desc* LoadFontDescriptor(fz_context* ctx, pdf_document* doc, pdf_resource_stack* res, pdf_obj* font) {
	pdf_font_desc* f;
	MuTryReturn(ctx, pdf_load_font(ctx, doc, res, font), f)
}

#pragma managed
using namespace MuPDF;

TextFont^ FontDescriptor::Font::get() {
	return _textFont ? _textFont : (_textFont = gcnew TextFont(_font->font));
}

String^ FontDescriptor::DecodeText(array<unsigned char>^ bytes, int offset, int length) {
    if (bytes == nullptr)
        throw gcnew ArgumentNullException("bytes");
    if (offset < 0 || length < 0 || offset + length > bytes->Length)
        throw gcnew ArgumentOutOfRangeException();

    pin_ptr<unsigned char> pinned = &bytes[offset];
    unsigned char* data = pinned;
    unsigned char* end = data + length;

    auto sb = gcnew System::Text::StringBuilder();

    while (data < end) {
        unsigned int cpt;
        int consumed = pdf_decode_cmap(_font->encoding, data, end, &cpt);
        if (consumed == 0) break;
        data += consumed;

        // remap cpt to CID via encoding CMap
        int cid = pdf_lookup_cmap(_font->encoding, cpt);
        if (cid < 0) {
            sb->Append(L'\uFFFD');
            continue;
        }

        // lookup ToUnicode with CID
        int ucsbuf[256];
        int ucslen = 0;
        if (_font->to_unicode)
            ucslen = pdf_lookup_cmap_full(_font->to_unicode, cid, ucsbuf);

        // fallback to cid_to_ucs
        if (ucslen == 0 && (size_t)cid < _font->cid_to_ucs_len) {
            ucsbuf[0] = _font->cid_to_ucs[cid];
            ucslen = 1;
        }

        // fallback with FFFD
        if (ucslen == 0 || (ucslen == 1 && ucsbuf[0] == 0)) {
            sb->Append(L'\uFFFD');
            continue;
        }

        for (int i = 0; i < ucslen; i++)
            sb->Append((wchar_t)ucsbuf[i]);
    }

    return sb->ToString();
}

FontDescriptor^ FontDescriptor::Load(Document^ doc, PdfDictionary^ resources, PdfDictionary^ font) {
	pdf_resource_stack* res = new pdf_resource_stack();
	res->resources = resources->Ptr;
	auto fd = LoadFontDescriptor(Context::Ptr, doc->Ptr, res, font->Ptr);
	delete res;
	if (fd) {
		return gcnew FontDescriptor(fd);
	}
	throw MuException::FromContext();
}

FontDescriptor::~FontDescriptor() {
	DisposeObject(_textFont);
	this->!FontDescriptor();
}
