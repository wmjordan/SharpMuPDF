#pragma once
#include "mupdf/fitz.h"
#include "mupdf/pdf.h"
#include "MuPDF.h"

#ifndef __COLORSPACE
#define __COLORSPACE

namespace MuPDF {

public enum class ColorspaceKind {
	None,
	Gray,
	Rgb,
	Bgr,
	Cmyk,
	Lab,
};

public ref class Colorspace sealed {
public:
	Colorspace(fz_colorspace* colorspace) : _colorspace(colorspace) {};

	property bool IsIndexed {
		bool get() {
			return fz_colorspace_is_indexed(Context::Ptr, _colorspace);
		}
	}
	property bool IsDevice {
		bool get() {
			return fz_colorspace_is_device(Context::Ptr, _colorspace);
		}
	}
	property bool IsGray {
		bool get() {
			return fz_colorspace_is_gray(Context::Ptr, _colorspace);
		}
	}
	property bool IsRgb {
		bool get() {
			return fz_colorspace_is_rgb(Context::Ptr, _colorspace);
		}
	}
	property bool IsCmyk {
		bool get() {
			return fz_colorspace_is_cmyk(Context::Ptr, _colorspace);
		}
	}
	property ColorspaceKind Kind {
		ColorspaceKind get() {
			return static_cast<ColorspaceKind>(fz_colorspace_type(Context::Ptr, _colorspace));
		}
	}
	property int NumberOfColorant {
		int get() {
			return fz_colorspace_n(Context::Ptr, _colorspace);
		}
	}
internal:
	property fz_colorspace* Pointer {
		fz_colorspace* get() {
			return _colorspace;
		}
	}
private:
	Colorspace() : Colorspace(NULL) {};
	fz_colorspace* _colorspace;
};
};

#endif // __COLORSPACE
