#ifndef __COLORSPACE
#define __COLORSPACE

#pragma once
#include "mupdf/fitz.h"
#include "MuPDF.h"

namespace MuPDF {

public enum class ColorspaceKind {
	None,
	Gray,
	RGB,
	BGR,
	CMYK,
	LAB,
	Indexed,
	Separation
};

[FlagsAttribute()]
public enum class ColorspaceFlags
{
	None,
	Device = 1,
	ICC = 1 << 1,
	HasCMYK = 1 << 2,
	HasSpots = 1 << 4,
	HasCMYKSpots = HasCMYK | HasSpots
};

public ref class Colorspace sealed {
public:
	Colorspace(ColorspaceKind kind, ColorspaceFlags flags, int n, String^ name);

	property ColorspaceKind Kind {
		ColorspaceKind get() {
			return static_cast<ColorspaceKind>(_colorspace->type);
		}
	}
	property ColorspaceFlags Flags {
		ColorspaceFlags get() { return static_cast<ColorspaceFlags>(_colorspace->flags); }
	}

	property int NumberOfColorant {
		int get() {
			return fz_colorspace_n(Context::Ptr, _colorspace);
		}
	}
	property bool IsValidBlend {
		bool get() {
			return _colorspace->type == FZ_COLORSPACE_GRAY
				|| _colorspace->type == FZ_COLORSPACE_RGB
				|| _colorspace->type == FZ_COLORSPACE_CMYK;
		}
	}

internal:
	Colorspace(fz_colorspace* colorspace) : _colorspace(colorspace) {
		fz_keep_colorspace(Context::Ptr, colorspace);
	};
	property fz_colorspace* Ptr {
		fz_colorspace* get() {
			return _colorspace;
		}
	}

private:
	Colorspace() : Colorspace(NULL) {};
	~Colorspace() {
		DropHandle(_colorspace, fz_drop_colorspace);
	}

	fz_colorspace* _colorspace;
};
};

#endif // __COLORSPACE
