#include "Pixmap.h"

#pragma unmanaged
DLLEXP fz_pixmap* NewPixmapWithBBox(fz_context* ctx, fz_colorspace* colorspace, fz_irect bbox, fz_separations* seps, int alpha) {
	fz_pixmap* r;
	MuTryReturn(ctx, fz_new_pixmap_with_bbox(ctx, colorspace, bbox, seps, alpha), r);
}

DLLEXP int TintPixmap(fz_context* ctx, fz_pixmap* pixmap, int black, int white) {
	MuTry(ctx, fz_tint_pixmap(ctx, pixmap, black, white));
}

#pragma managed
bool MuPDF::Pixmap::Tint(int black, int white) {
	return TintPixmap(Context::Ptr, _pixmap, black, white);
}