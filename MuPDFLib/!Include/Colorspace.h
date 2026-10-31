#ifndef __COLORSPACE
#define __COLORSPACE

#pragma once
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
	/// <summary>Creates a component-N colorspace</summary>
	Colorspace(ColorspaceKind kind, ColorspaceFlags flags, int n, String^ name);
	/// <summary>Creates an ICC colorspace</summary>
	Colorspace(ColorspaceKind kind, ColorspaceFlags flags, String^ name, Buffer^ buffer);
	/// <summary>Gets a reference to predefined device colorspace</summary>
	Colorspace(ColorspaceKind kind) : Colorspace(ToNativeColorspace(kind)) {};

	PropGet(ColorspaceKind, Kind, static_cast<ColorspaceKind>(_colorspace ? _colorspace->type : 0));
	PropGet(ColorspaceFlags, Flags, static_cast<ColorspaceFlags>(_colorspace ? _colorspace->flags : 0));

	PropGet(int, NumberOfColorant, fz_colorspace_n(Context::Ptr, _colorspace));
	PropEmptyGet(bool, IsValidBlend);
	PropEmptyGet(bool, IsSubtractive);
	PropEmptyGet(bool, IsDeviceNHasOnlyCmyk);
	PropEmptyGet(bool, IsDeviceNHasCmyk);
	PropEmptyGet(bool, IsLabIcc);
	PropEmptyGet(Colorspace^, Base);
	PropEmptyGet(String^, Name);

internal:
	Colorspace(fz_colorspace* colorspace) : _colorspace(colorspace) {
		fz_keep_colorspace(Context::Ptr, colorspace);
	};

	PropGet(fz_colorspace*, Ptr, _colorspace);

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
