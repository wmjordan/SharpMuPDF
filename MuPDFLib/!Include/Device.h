#include "mupdf/fitz.h"
#include "mupdf/pdf.h"
#include "MuPDF.h"

#ifndef __DEVICE
#define __DEVICE
#pragma once
int CloseDevice(fz_context* ctx, fz_device* dev);

using namespace System;

namespace MuPDF {

[FlagsAttribute]
public enum class DeviceHints {
	DontInterpolateImages = 1,
	NoCache = 2,
	DontDecodeImages = 4
};

public ref class Device : IDisposable {
internal:
	Device(fz_device* device) : _device(device) {};
	~Device() {
		ReleaseHandle();
	}

	property fz_device* Ptr {
		fz_device* get() {
			return _device;
		}
	}

public:
	/// <summary>
	/// Creates a new draw device to render PDF page contents.
	/// </summary>
	/// <param name="pixmap">The pixmap to render page contents.</param>
	/// <returns>A draw device which paints content to the pixmap.</returns>
	static Device^ NewDraw(Pixmap^ pixmap) {
		return TryCreateDevice(fz_new_draw_device(Context::Ptr, fz_identity, pixmap->Ptr));
	}
	static Device^ NewDraw(Pixmap^ pixmap, Matrix matrix) {
		return TryCreateDevice(fz_new_draw_device(Context::Ptr, matrix, pixmap->Ptr));
	}
	static Device^ NewBox(Pixmap^ pixmap, Box box) {
		pin_ptr<Box> p = &box;
		return TryCreateDevice(fz_new_bbox_device(Context::Ptr, (fz_rect*)p));
	}
	static Device^ NewStructureText(TextPage^ textPage) {
		return TryCreateDevice(fz_new_stext_device(Context::Ptr, textPage->Ptr, NULL));
	}
	static Device^ NewStructureText(TextPage^ textPage, TextOptions^ options) {
		auto o = (fz_stext_options)options;
		return TryCreateDevice(fz_new_stext_device(Context::Ptr, textPage->Ptr, &o));
	}
	void EnableDeviceHints(DeviceHints hints) {
		fz_enable_device_hints(Context::Ptr, _device, (int)hints);
	}
	void DisableDeviceHints(DeviceHints hints) {
		fz_enable_device_hints(Context::Ptr, _device, (int)hints);
	}
	void Close() {
		if (!CloseDevice(Context::Ptr, _device)) {
			throw MuException::FromContext();
		}
	}

private:
	fz_device* _device;

	void ReleaseHandle() {
		CloseDevice(Context::Ptr, _device);
		fz_drop_device(Context::Ptr, _device);
		_device = NULL;
	}

	static Device^ TryCreateDevice(fz_device* device) {
		if (device) {
			return gcnew Device(device);
		}
		throw MuException::FromContext();
	}
};
};

#endif