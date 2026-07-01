#include "Colorspace.h"
using namespace MuPDF;

Colorspace::Colorspace(ColorspaceKind kind, ColorspaceFlags flags, int n, String^ name) {
    EncodeUTF8(name, csn)
    _colorspace = fz_new_colorspace(Context::Ptr, static_cast<enum fz_colorspace_type>(kind), (int)flags, n, (const char*)csn);
    fz_keep_colorspace(Context::Ptr, _colorspace);
}


fz_colorspace* Colorspace::ToNativeColorspace(ColorspaceKind kind) {
    switch (kind) {
    case ColorspaceKind::RGB: return fz_device_rgb(Context::Ptr);
    case ColorspaceKind::CMYK: return fz_device_cmyk(Context::Ptr);
    case ColorspaceKind::Gray: return fz_device_gray(Context::Ptr);
    case ColorspaceKind::BGR: return fz_device_bgr(Context::Ptr);
    case ColorspaceKind::LAB: return fz_device_lab(Context::Ptr);
    default: throw gcnew MuException("ColorspaceKind is not supported. Valid kind: Gray, RGB, CMYK, BGR, LAB.");
    }
}

bool Colorspace::IsValidBlend::get() {
    return _colorspace == NULL
        || _colorspace->type == FZ_COLORSPACE_GRAY
        || _colorspace->type == FZ_COLORSPACE_RGB
        || _colorspace->type == FZ_COLORSPACE_CMYK;
}

bool Colorspace::IsSubtractive::get() {
    return fz_colorspace_is_subtractive(Context::Ptr, _colorspace);
}

bool MuPDF::Colorspace::IsDeviceNHasOnlyCmyk::get() {
    return fz_colorspace_device_n_has_only_cmyk(Context::Ptr, _colorspace);
}

bool MuPDF::Colorspace::IsDeviceNHasCmyk::get() {
    return fz_colorspace_device_n_has_cmyk(Context::Ptr, _colorspace);
}

Colorspace^ Colorspace::Base::get() {
    auto b = fz_base_colorspace(Context::Ptr, _colorspace);
    return b == _colorspace ? this : gcnew Colorspace(b);
}

String^ Colorspace::Name::get() {
    auto name = fz_colorspace_name(Context::Ptr, _colorspace);
    return name ? gcnew String(name) : "?";
}
