#include "Pixmap.h"

#pragma unmanaged
static fz_pixmap* GetPixmap(fz_context* ctx, fz_colorspace* cs, int width, int height) {
	fz_pixmap* p;
	MuTryReturn(ctx, fz_new_pixmap(ctx, cs, width, height, NULL, 0), p);
}
static fz_pixmap* GetPixmap(fz_context* ctx, fz_colorspace* cs, fz_irect rect) {
	fz_pixmap* p;
	MuTryReturn(ctx, fz_new_pixmap_with_bbox(ctx, cs, rect, NULL, 0), p);
}

DLLEXP fz_pixmap* NewPixmapWithBBox(fz_context* ctx, fz_colorspace* colorspace, fz_irect bbox, fz_separations* seps, int alpha) {
	fz_pixmap* r;
	MuTryReturn(ctx, fz_new_pixmap_with_bbox(ctx, colorspace, bbox, seps, alpha), r);
}

DLLEXP int TintPixmap(fz_context* ctx, fz_pixmap* pixmap, int black, int white) {
	MuTry(ctx, fz_tint_pixmap(ctx, pixmap, black, white));
}

#pragma managed

MuPDF::Pixmap^ MuPDF::Pixmap::Create(ColorspaceKind colorspace, int width, int height) {
	fz_pixmap* pixmap = GetPixmap(Context::Ptr, Context::GetFzColorspace(colorspace), width, height);
	if (pixmap) {
		return gcnew Pixmap(pixmap);
	}
	throw MuException::FromContext();
}

MuPDF::Pixmap^ MuPDF::Pixmap::Create(ColorspaceKind colorspace, MuPDF::BBox box) {
	fz_pixmap* pixmap = GetPixmap(Context::Ptr, Context::GetFzColorspace(colorspace), box);
	if (pixmap) {
		return gcnew Pixmap(pixmap);
	}
	throw MuException::FromContext();
}

bool MuPDF::Pixmap::Tint(int black, int white) {
	return TintPixmap(Context::Ptr, _pixmap, black, white);
}