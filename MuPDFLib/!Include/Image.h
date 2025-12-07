#include "mupdf/fitz.h"
#include "MuPDF.h"

#ifndef __IMAGE
#define __IMAGE
#pragma once

using namespace System;

namespace MuPDF {

public ref class Image sealed : IDisposable {

public:
	property int Width { int get() { return _img->w; } }
	property int Height { int get() { return _img->h; } }
	property Byte N { Byte get() { return (Byte)_img->n; } }
	property Byte BitsPerComponent { Byte get() { return (Byte)_img->bpc; } }
	property bool Mask { bool get() { return _img->imagemask; } }
	property bool IsInterpolate { bool get() { return _img->interpolate; } }
	property bool UseColorKey { bool get() { return _img->use_colorkey; } }
	property bool UseDecode { bool get() { return _img->use_decode; } }
	property bool IsDecoded { bool get() { return _img->decoded; } }
	property bool IsScalable { bool get() { return _img->scalable; } }
	property bool HasIntent { bool get() { return _img->has_intent; } }
	property Byte Orientation { Byte get() { return (Byte)_img->orientation; } }
	property MuPDF::Colorspace^ Colorspace {
		MuPDF::Colorspace^ get();
	}
	virtual String^ ToString() override { return String::Concat("Image: ", Width, "*", Height); }
internal:
	Image(fz_image* img) : _img(img) {
		fz_keep_image(Context::Ptr, img);
	}
	~Image() {
		ReleaseHandle();
	}

private:
	fz_image* _img;

	void ReleaseHandle() {
		fz_drop_image(Context::Ptr, _img);
		_img = NULL;
	}
};

}

#endif