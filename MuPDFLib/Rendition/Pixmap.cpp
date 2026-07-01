#include "Pixmap.h"
#include "Colorspace.h"

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

DLLEXP int InvertPixmapLuminance(fz_context* ctx, fz_pixmap* pixmap) {
	MuTry(ctx, fz_invert_pixmap_luminance(ctx, pixmap));
}

DLLEXP int LoadTiffSubImageCount(fz_context* ctx, const unsigned char* data, size_t length) {
	int p;
	MuTryReturn(ctx, fz_load_tiff_subimage_count(ctx, data, length), p);
}

DLLEXP fz_pixmap* LoadTiffSubImage(fz_context* ctx, const unsigned char* data, size_t length, int index) {
	fz_pixmap* p;
	MuTryReturn(ctx, fz_load_tiff_subimage(ctx, data, length, index), p);
}

DLLEXP fz_pixmap* Mask(fz_context* ctx, fz_pixmap* color, fz_pixmap* mask) {
	fz_pixmap* p;
	MuTryReturn(ctx, fz_new_pixmap_from_color_and_mask(ctx, color, mask), p);
}

DLLEXP fz_pixmap* ConvertColorspace(fz_context* ctx, fz_pixmap* original, fz_colorspace* target, fz_color_params params) {
	fz_pixmap* p;
	MuTryReturn(ctx, fz_convert_pixmap(ctx, original, target, NULL, NULL, params, 0), p);
}
#pragma managed

using namespace MuPDF;

MuPDF::Colorspace^ Pixmap::Colorspace::get() {
	return _colorspace ? _colorspace : (_colorspace = gcnew MuPDF::Colorspace(fz_pixmap_colorspace(Context::Ptr, _pixmap)));
}

Pixmap^ Pixmap::Create(ColorspaceKind colorspace, int width, int height) {
	fz_pixmap* pixmap = GetPixmap(Context::Ptr, MuPDF::Colorspace::ToNativeColorspace(colorspace), width, height);
	if (pixmap) {
		return gcnew Pixmap(pixmap);
	}
	throw MuException::FromContext();
}

Pixmap^ Pixmap::Create(ColorspaceKind colorspace, MuPDF::BBox box) {
	fz_pixmap* pixmap = ::GetPixmap(Context::Ptr, MuPDF::Colorspace::ToNativeColorspace(colorspace), box);
	if (pixmap) {
		return gcnew Pixmap(pixmap);
	}
	throw MuException::FromContext();
}

void Pixmap::InvertLuminance() {
	if (!::InvertPixmapLuminance(Context::Ptr, _pixmap)) {
		throw MuException::FromContext();
	}
}

int Pixmap::LoadTiffSubImageCount(IntPtr data, int length) {
	return ::LoadTiffSubImageCount(Context::Ptr, (const unsigned char*)data.ToPointer(), (size_t)length);
}

Pixmap^ Pixmap::LoadTiffSubImage(IntPtr data, int length, int index) {
	return gcnew Pixmap(::LoadTiffSubImage(Context::Ptr, (const unsigned char*)data.ToPointer(), (size_t)length, index));
}

void Pixmap::Tint(int black, int white) {
	if (!::TintPixmap(Context::Ptr, _pixmap, black, white)) {
		throw MuException::FromContext();
	}
}

void Pixmap::Gamma(float gamma) {
	if (gamma == 1.0) {
		return;
	}
	fz_gamma_pixmap(Context::Ptr, _pixmap, gamma);
}

array<Byte>^ Pixmap::GetSampleBytes() {
	if (_samples == IntPtr::Zero) {
		return nullptr;
	}
	GcnewArray(Byte, d, _width * _height * _components);
	System::Runtime::InteropServices::Marshal::Copy(_samples, d, 0, d->Length);
	return d;
}

Pixmap^ Pixmap::Mask(Pixmap^ mask) {
	fz_pixmap* pixmap = ::Mask(Context::Ptr, _pixmap, mask->_pixmap);
	if (pixmap) {
		return gcnew Pixmap(pixmap);
	}
	throw MuException::FromContext();
}

Pixmap^ Pixmap::ConvertColorspace(ColorspaceKind colorspace, ColorParams^ params) {
	auto p = params ? params->ToNative() : fz_default_color_params;
	auto c = ::ConvertColorspace(Context::Ptr, _pixmap, MuPDF::Colorspace::ToNativeColorspace(colorspace), p);
	if (c) {
		return gcnew Pixmap(c);
	}
	throw MuException::FromContext();
}