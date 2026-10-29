#include "../Context.h"
#include "Buffer.h"
#include "Colorspace.h"
#include "CompressedBuffer.h"
#include "Geometry.h"
#include "Pixmap.h"
#include "ObjWrapper.h"

#ifndef __IMAGE
#define __IMAGE
#pragma once

using namespace System;

namespace MuPDF {

// see image.h
public enum class ImageOrientation {
	Undefined,
	NoRotation,
	Right,
	Down,
	Left,
	FlipHorizontal,
	FlipHorizontalRotateRight,
	FlipHorizontalRotateDown,
	FlipHorizontalRotateLeft
};

// see compressed-buffer.h: FZ_IMAGE_*
public enum class ImageType {
	Unknown,
	// Uncompressed types
	Raw,
	// Compressed types
	Flate,
	LZW,
	RLD,
	Brotli,
	// Full image formats
	BMP,
	GIF,
	JBIG2,
	JPEG,
	JPX,
	JXR,
	PNG,
	PNM,
	TIFF,
	PSD,
};

public ref class Image sealed : IDisposable {

public:
	static ImageType RecognizeImageFormat(array<Byte>^ data);
	static Image^ Load(String^ fileName);
	static Image^ Load(Buffer^ buffer);
	static Image^ Load(array<Byte>^ data);
	static Image^ FromPixmap(Pixmap^ pixmap, Image^ mask);
	static Image^ Load(int width, int height, int bpc, MuPDF::Colorspace^ colorspace, int xres, int yres, bool interpolate, bool isMask, array<float>^ decode, array<int>^ colorkey, CompressedBuffer^ buffer, Image^ mask);
	static Image^ FromPixmap(Pixmap^ pixmap) {
		return FromPixmap(pixmap, nullptr);
	}

	PropGet(int, Width, _img->w);
	PropGet(int, Height, _img->h);
	PropGet(int, ResX, _img->xres);
	PropGet(int, ResY, _img->yres);
	PropGet(Byte, N, (Byte)_img->n);
	PropGet(Byte, BitsPerComponent, (Byte)_img->bpc);
	PropGet(bool, HasMask, _img->imagemask);
	PropGet(bool, IsInterpolate, _img->interpolate);
	PropGet(bool, UseColorKey, _img->use_colorkey);
	PropGet(bool, UseDecode, _img->use_decode);
	PropGet(bool, IsDecoded, _img->decoded);
	PropGet(bool, IsScalable, _img->scalable);
	PropGet(bool, HasIntent, _img->has_intent);
	PropGet(bool, IsLossy, fz_is_lossy_image(Context::Ptr, _img));
	PropGet(Byte, Orientation, (Byte)_img->orientation);
	PropGet(ImageType, CompressedType, (ImageType)fz_compressed_image_type(Context::Ptr, _img));
	PropEmptyGet(MuPDF::Colorspace^, Colorspace);
	ImageOrientation GetOrientation() {
		return (ImageOrientation)fz_image_orientation(Context::Ptr, _img);
	}
	Matrix GetOrientationMatrix() {
		return (Matrix)fz_image_orientation_matrix(Context::Ptr, _img);
	}
	array<Byte>^ GetCompressedBytes();
	Pixmap^ GetPixmap();
	Pixmap^ GetPixmap(BBox rect, Matrix ctm);
	Image^ GetMask() {
		return _img->mask ? gcnew Image(_img->mask) : nullptr;
	}
	CompressedBuffer^ GetCompressedBuffer();
	virtual String^ ToString() override { return String::Concat("ImageType: ", Width, "*", Height); }
internal:
	PropGet(fz_image*, Ptr, _img);
	Image(fz_image* img) : _img(img) {
		fz_keep_image(Context::Ptr, img);
	}
	~Image() {
		ReleaseHandle();
	}

private:
	fz_image* _img;
	Buffer^ _privateBuffer;

	void ReleaseHandle();
};

}

#endif