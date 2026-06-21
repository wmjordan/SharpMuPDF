#include "../Context.h"
#include "Buffer.h"
#include "Colorspace.h"
#include "CompressedBuffer.h"
#include "Geometry.h"
#include "Pixmap.h"

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
	static Image^ FromPixmap(Pixmap^ pixmap) {
		return FromPixmap(pixmap, nullptr);
	}

	property int Width { int get() { return _img->w; } }
	property int Height { int get() { return _img->h; } }
	property int ResX { int get() { return _img->xres; } }
	property int ResY { int get() { return _img->yres; } }
	property Byte N { Byte get() { return (Byte)_img->n; } }
	property Byte BitsPerComponent { Byte get() { return (Byte)_img->bpc; } }
	property bool HasMask { bool get() { return _img->imagemask; } }
	property bool IsInterpolate { bool get() { return _img->interpolate; } }
	property bool UseColorKey { bool get() { return _img->use_colorkey; } }
	property bool UseDecode { bool get() { return _img->use_decode; } }
	property bool IsDecoded { bool get() { return _img->decoded; } }
	property bool IsScalable { bool get() { return _img->scalable; } }
	property bool HasIntent { bool get() { return _img->has_intent; } }
	property bool IsLossy { bool get() { return fz_is_lossy_image(Context::Ptr, _img); } }
	property Byte Orientation { Byte get() { return (Byte)_img->orientation; } }
	property ImageType CompressedType { ImageType get() { return (ImageType)fz_compressed_image_type(Context::Ptr, _img); } }
	property MuPDF::Colorspace^ Colorspace {
		MuPDF::Colorspace^ get();
	}
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
	property fz_image* Ptr { fz_image* get() { return _img; } }
	Image(fz_image* img) : _img(img) {
		fz_keep_image(Context::Ptr, img);
	}
	~Image() {
		ReleaseHandle();
	}

private:
	fz_image* _img;
	Buffer^ _privateBuffer;

	void ReleaseHandle() {
		fz_drop_image(Context::Ptr, _img);
		_img = NULL;
		if (_privateBuffer) {
			delete _privateBuffer;
		}
		_privateBuffer = nullptr;
	}
};

}

#endif