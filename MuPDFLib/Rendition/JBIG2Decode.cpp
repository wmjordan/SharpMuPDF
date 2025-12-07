#include "JBIG2Codec.h"
#include <stdint.h>
#include "jbig2.h"

array<Byte>^ MuPDF::JBig2Codec::Decode(array<Byte>^ data, array<Byte>^ globals) {
	auto ctx = jbig2_ctx_new(NULL, JBIG2_OPTIONS_EMBEDDED, NULL, NULL, NULL);
	int c;
	Jbig2GlobalCtx* globalCtx = NULL;
	Jbig2Image* image;
	array<Byte>^ decodedData = nullptr;
	try {
		if (globals != nullptr && globals->Length != 0) {
			pin_ptr<unsigned char> pg = &globals[0];
			c = jbig2_data_in(ctx, pg, globals->Length);
			globalCtx = jbig2_make_global_ctx(ctx);
			ctx = jbig2_ctx_new(NULL, JBIG2_OPTIONS_EMBEDDED, globalCtx, NULL, NULL);
		}
		pin_ptr<unsigned char> pd = &data[0];
		c = jbig2_data_in(ctx, pd, data->Length);
		c = jbig2_complete_page(ctx);
		if (image = jbig2_page_out(ctx)) {
			decodedData = gcnew array<Byte>(image->height * image->stride);
			Marshal::Copy((IntPtr)image->data, decodedData, 0, decodedData->Length);
			jbig2_release_page(ctx, image);
		}
	}
	finally {
		if (globalCtx) {
			jbig2_global_ctx_free(globalCtx);
		}
		jbig2_ctx_free(ctx);
	}
	return decodedData;
}
