#include "FontDescriptor.h"

#pragma unmanaged
DLLEXP pdf_font_desc* LoadFontDescriptor(fz_context* ctx, pdf_document* doc, pdf_resource_stack* res, pdf_obj* font) {
	pdf_font_desc* f;
	MuTryReturn(ctx, pdf_load_font(ctx, doc, res, font), f)
}

#pragma managed
using namespace MuPDF;

FontDescriptor^ MuPDF::FontDescriptor::Load(Document^ doc, PdfDictionary^ resources, PdfDictionary^ font) {
	pdf_resource_stack* res = new pdf_resource_stack();
	res->resources = resources->Ptr;
	auto fd = LoadFontDescriptor(Context::Ptr, doc->Ptr, res, font->Ptr);
	delete res;
	if (fd) {
		return gcnew FontDescriptor(fd);
	}
	throw MuException::FromContext();
}
