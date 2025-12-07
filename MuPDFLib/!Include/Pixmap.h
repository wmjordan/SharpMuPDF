#include "mupdf/fitz.h"
#include "MuPDF.h"

#ifndef __PIXMAP
#define __PIXMAP

using namespace System;

#pragma once

namespace MuPDF {
public ref class Pixmap sealed : IDisposable {
public:
	property IntPtr Samples {
		IntPtr get() { return _samples; }
	}
	property int Stride {
		int get() { return _stride; }
	}
	property int Width {
		int get() { return _width; }
	}
	property int Height {
		int get() { return _height; }
	}
	property int Components {
		int get() { return _components; }
	}
	property int Colorants {
		int get() { return fz_pixmap_colorants(Context::Ptr, _pixmap); }
	}
	property MuPDF::BBox BBox {
		MuPDF::BBox get() { return fz_pixmap_bbox(Context::Ptr, _pixmap); }
	}
	/// <summary>
	/// Return the number of alpha planes in a Pixmap. Does not throw exceptions.
	/// </summary>
	property int Alpha {
		int get() { return fz_pixmap_alpha(Context::Ptr, _pixmap); }
	}

	static Pixmap^ Create(ColorspaceKind colorspace, int width, int height);

	static Pixmap^ Create(ColorspaceKind colorspace, MuPDF::BBox box);

	void SetBackgroundWhite() {
		Clear(0xFF);
	}
	void Clear(int value) {
		fz_clear_pixmap_with_value(Context::Ptr, _pixmap, value);
	}
	void Invert() {
		fz_invert_pixmap(Context::Ptr, _pixmap);
	}
	bool Tint(int black, int white);
	bool Tint(int color) {
		return Tint(0, color);
	}
	void Gamma(float gamma) {
		if (gamma == 1.0) {
			return;
		}
		fz_gamma_pixmap(Context::Ptr, _pixmap, gamma);
	}
	array<Byte>^ GetSampleBytes() {
		if (_samples == IntPtr::Zero) {
			return nullptr;
		}
		GcnewArray(Byte, d, _width * _height * _components);
		System::Runtime::InteropServices::Marshal::Copy(_samples, d, 0, d->Length);
		return d;
	}
internal:
	Pixmap(fz_pixmap* pixmap) : _pixmap(pixmap) {
		auto ctx = Context::Ptr;
		_width = fz_pixmap_width(ctx, pixmap);
		_height = fz_pixmap_height(ctx, pixmap);
		_components = fz_pixmap_components(ctx, pixmap);
		_stride = fz_pixmap_stride(ctx, pixmap);
		_samples = (IntPtr)(void*)fz_pixmap_samples(ctx, pixmap);
	};
	~Pixmap() {
		ReleaseHandle();
	}
	property fz_pixmap* Ptr {
		fz_pixmap* get() { return _pixmap; }
	}
private:
	fz_pixmap* _pixmap;
	int _width, _height, _stride, _components;
	IntPtr _samples;

	void ReleaseHandle() {
		fz_drop_pixmap(Context::Ptr, _pixmap);
		_pixmap = NULL;
	}
};

};

#endif
