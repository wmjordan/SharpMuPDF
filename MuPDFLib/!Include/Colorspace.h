#ifndef __COLORSPACE
#define __COLORSPACE

#pragma once
#include "fitz.h"
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
	Colorspace(ColorspaceKind kind) : Colorspace(ToNativeColorspace(kind)) {};

	property ColorspaceKind Kind { ColorspaceKind get() { return static_cast<ColorspaceKind>(_colorspace ? _colorspace->type : 0); } }
	property ColorspaceFlags Flags { ColorspaceFlags get() { return static_cast<ColorspaceFlags>(_colorspace ? _colorspace->flags : 0); } }

	property int NumberOfColorant { int get() { return fz_colorspace_n(Context::Ptr, _colorspace); } }
	property bool IsValidBlend { bool get(); }
	property bool IsSubtractive { bool get(); }
	property bool IsDeviceNHasOnlyCmyk { bool get(); }
	property bool IsDeviceNHasCmyk { bool get(); }
	property Colorspace^ Base { Colorspace ^ get(); }
	property String^ Name { String ^ get(); }

internal:
	Colorspace(fz_colorspace* colorspace) : _colorspace(colorspace) {
		fz_keep_colorspace(Context::Ptr, colorspace);
	};
	property fz_colorspace* Ptr { fz_colorspace* get() { return _colorspace; } }

	static fz_colorspace* ToNativeColorspace(MuPDF::ColorspaceKind kind);

private:
	Colorspace() : Colorspace(NULL) {};
	~Colorspace() {
		DropHandle(_colorspace, fz_drop_colorspace);
	}

	fz_colorspace* _colorspace;
};
};

#endif // __COLORSPACE
