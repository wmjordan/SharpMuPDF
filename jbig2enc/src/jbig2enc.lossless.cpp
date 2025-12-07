#include "jbig2enc.h"
#include "jbig2enc.lossless.h"
#include <allheaders.h>
#if (LIBLEPT_MAJOR_VERSION == 1 && LIBLEPT_MINOR_VERSION >= 83) || LIBLEPT_MAJOR_VERSION > 1
#include "pix_internal.h"
#endif

u8* jbig2_lossless_encode(int width, int height, int stride, bool zeroIsWhite, u8* const source, int* const length) {
	u8* ret;
	u32 mask = zeroIsWhite ? 0x0 : 0xffffffff;
	u8* pl = source;
	u8* p;
	u32* pw;
	char a, b;
	for (int h = 0; h < height; h++) {
		p = pl;
		for (int w = 0; w < stride; w += 4) {
			pw = (u32*)p;
			*pw = *pw ^ mask;
			a = *p;
			*p = p[3];
			p++;
			b = *p;
			*p = p[1];
			p++;
			*p = b;
			p++;
			*p = a;
			p++;
		}
		pl += stride;
	}

	Pix* pix = (Pix*)malloc(sizeof(Pix));
	if (pix == NULL) {
		return 0;
	}
	pix->w = width;
	pix->h = height;
	pix->d = 1;
	pix->wpl = (width + 31) / 32;
	pix->spp = 1;
	pix->refcount = 1;
	pix->data = (u32*)source;
	pix->informat = IFF_UNKNOWN;
	pix->xres = pix->yres = 0;
	pix->special = 0;
	pix->text = NULL;
	pix->colormap = NULL;
	ret = jbig2_encode_generic(pix, false, 0, 0, false, length);
	free(pix);
	return ret;
}
