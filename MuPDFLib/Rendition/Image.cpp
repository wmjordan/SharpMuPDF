#include "Image.h"
#include <Buffer.h>
#include <Colorspace.h>
#include <MuException.h>
#include <ObjWrapper.h>
#include <Stream.h>
#include <mupdf/fitz/context.h>
#include <mupdf/fitz/image.h>

#pragma unmanaged
static fz_image* LoadImage(fz_context* ctx, const char* file) {
	fz_image* p;
	MuTryReturn(ctx, fz_new_image_from_file(ctx, file), p);
}

static fz_pixmap* GetPixmapFromImage(fz_context* ctx, fz_image* img, const fz_irect* subarea, fz_matrix* ctm, int* w, int* h) {
	fz_pixmap* p;
	MuTryReturn(ctx, fz_get_pixmap_from_image(ctx, img, subarea, ctm, w, h), p);
}

static fz_image* LoadImageFromPdf(fz_context* ctx, int width, int height, int bpc, fz_colorspace* colorspace, int xres, int yres, bool interpolate, bool imagemask, const float* decode, const int* colorkey, fz_compressed_buffer* buffer, fz_image* mask) {
	fz_image* p;
	MuTryReturn(ctx, fz_new_image_from_compressed_buffer(ctx, width, height, bpc, colorspace, xres, yres, interpolate, imagemask, decode, colorkey, buffer, mask), p);
}

#pragma managed

using namespace MuPDF;

Colorspace^ Image::Colorspace::get() {
	return gcnew MuPDF::Colorspace(_img->colorspace);
}

ImageType Image::RecognizeImageFormat(array<Byte>^ data) {
	if (!data || data->Length < 8) {
		return ImageType::Unknown;
	}
	pin_ptr<Byte> p = &data[0];
	int type = fz_recognize_image_format(Context::Ptr, (unsigned char*)p);
	return (ImageType)type;
}

Image^ MuPDF::Image::Load(String^ fileName) {
	EncodeUTF8(fileName, f);
	auto img = ::LoadImage(Context::Ptr, (const char*)f);
	if (img) {
		return gcnew Image(img);
	}
	throw MuException::FromContext();
}

Image^ MuPDF::Image::Load(MuPDF::Buffer^ buffer) {
	if (!buffer || buffer->IsDisposed) {
		throw gcnew ArgumentNullException("buffer");
	}
	auto img = fz_new_image_from_buffer(Context::Ptr, buffer->Ptr);
	if (img) {
		return gcnew Image(img);
	}
	throw MuException::FromContext();
}

Image^ MuPDF::Image::Load(array<Byte>^ data) {
	auto buffer = Buffer::FromArray(data);
	Image^ img;
	try {
		img = Load(buffer);
		img->_privateBuffer = buffer;
	}
	catch (Exception^) {
		delete buffer;
		throw;
	}
	return img;
}

Image^ MuPDF::Image::FromPixmap(Pixmap^ pixmap, Image^ mask) {
	if (pixmap == nullptr) {
		throw gcnew ArgumentNullException("pixmap");
	}
	auto img = fz_new_image_from_pixmap(Context::Ptr, pixmap->Ptr, mask ? mask->_img : NULL);
	return gcnew Image(img);
}

Image^ MuPDF::Image::Load(int width, int height, int bpc, MuPDF::Colorspace^ colorspace, int xres, int yres, bool interpolate, bool isMask, array<float>^ decode, array<int>^ colorkey, CompressedBuffer^ buffer, Image^ mask) {
	PinArray(float, decode, pDecode);
	PinArray(int, colorkey, pColorKey);

	auto img = ::LoadImageFromPdf(Context::Ptr, width, height, bpc, colorspace->Ptr, xres, yres, interpolate, isMask, reinterpret_cast<const float*>(pDecode), reinterpret_cast<const int*>(pColorKey), buffer->Ptr, mask->Ptr);
	if (img) {
		return gcnew Image(img);
	}
	throw MuException::FromContext();
}

CompressedBuffer^ Image::GetCompressedBuffer() {
	auto buffer = fz_compressed_image_buffer(Context::Ptr, _img);
	return buffer ? gcnew CompressedBuffer(buffer) : nullptr;
}

void MuPDF::Image::ReleaseHandle() {
	fz_drop_image(Context::Ptr, _img);
	_img = NULL;
	if (_privateBuffer) {
		delete _privateBuffer;
	}
	_privateBuffer = nullptr;
}

array<Byte>^ Image::GetCompressedBytes() {
	auto compressed = GetCompressedBuffer();
	if (!compressed) {
		return nullptr;
	}
	Stream^ s;
	try {
		s = compressed->Open();
		return s->ReadAll();
	}
	finally {
		delete s;
	}
}

Pixmap^ MuPDF::Image::GetPixmap() {
	auto p = ::GetPixmapFromImage(Context::Ptr, _img, NULL, NULL, NULL, NULL);
	if (p) {
		return gcnew Pixmap(p);
	}
	throw MuException::FromContext();
}

Pixmap^ MuPDF::Image::GetPixmap(BBox rect, Matrix ctm) {
	fz_irect r = rect;
	fz_matrix m = ctm;
	auto p = ::GetPixmapFromImage(Context::Ptr, _img, &r, &m, NULL, NULL);
	if (p) {
		return gcnew Pixmap(p);
	}
	throw MuException::FromContext();
}

