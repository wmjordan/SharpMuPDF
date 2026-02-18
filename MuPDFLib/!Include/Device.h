#ifndef __DEVICE
#define __DEVICE
#pragma once
#include "mupdf/fitz.h"
#include "MuPDF.h"

using namespace System;

namespace MuPDF {

	ref class TextPage;
	ref class TextOptions;

[FlagsAttribute]
public enum class DeviceHints {
	DontInterpolateImages = 1,
	NoCache = 2,
	DontDecodeImages = 4
};

public ref class Device : IDisposable {
public:
	/// <summary>
	/// Creates a new draw device to render PDF page contents.
	/// </summary>
	/// <param name="pixmap">The pixmap to render page contents.</param>
	/// <returns>A draw device which paints content to the pixmap.</returns>
	static Device^ NewDraw(Pixmap^ pixmap);
	static Device^ NewDraw(Pixmap^ pixmap, Matrix matrix);
	static Device^ NewBox(Pixmap^ pixmap, Box box);
	static Device^ NewStructureText(TextPage^ textPage);
	static Device^ NewStructureText(TextPage^ textPage, TextOptions^ options);
	void EnableDeviceHints(DeviceHints hints) {
		fz_enable_device_hints(Context::Ptr, _device, (int)hints);
	}
	void DisableDeviceHints(DeviceHints hints) {
		fz_disable_device_hints(Context::Ptr, _device, (int)hints);
	}
	void Close();

internal:
	Device(fz_device* device) : _device(device) {};
	~Device();

	property fz_device* Ptr { fz_device* get() { return _device; } }

private:
	fz_device* _device;

	!Device() {
		DropHandle(_device, fz_drop_device);
	}
	static Device^ TryCreateDevice(fz_device* device);
};
};

#endif