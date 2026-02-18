#include "Device.h"

#pragma unmanaged
int CloseDevice(fz_context* ctx, fz_device* dev) {
	MuTry(ctx, fz_close_device(ctx, dev));
}

#pragma managed
using namespace MuPDF;

Device^ Device::NewDraw(Pixmap^ pixmap) {
	return TryCreateDevice(fz_new_draw_device(Context::Ptr, fz_identity, pixmap->Ptr));
}

Device^ Device::NewDraw(Pixmap^ pixmap, Matrix matrix) {
	return TryCreateDevice(fz_new_draw_device(Context::Ptr, matrix, pixmap->Ptr));
}

Device^ Device::NewBox(Pixmap^ pixmap, Box box) {
	pin_ptr<Box> p = &box;
	return TryCreateDevice(fz_new_bbox_device(Context::Ptr, (fz_rect*)p));
}

Device^ Device::NewStructureText(TextPage^ textPage) {
	return TryCreateDevice(fz_new_stext_device(Context::Ptr, textPage->Ptr, NULL));
}

Device^ Device::NewStructureText(TextPage^ textPage, TextOptions^ options) {
	auto o = (fz_stext_options)options;
	return TryCreateDevice(fz_new_stext_device(Context::Ptr, textPage->Ptr, &o));
}

void Device::Close() {
	if (!CloseDevice(Context::Ptr, _device)) {
		throw MuException::FromContext();
	}
}

Device::~Device() {
	CloseDevice(Context::Ptr, _device);
	DropHandle(_device, fz_drop_device);
}

Device^ Device::TryCreateDevice(fz_device* device) {
	if (device) {
		return gcnew Device(device);
	}
	throw MuException::FromContext();
}
