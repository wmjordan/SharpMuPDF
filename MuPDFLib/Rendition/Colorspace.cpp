#include "Colorspace.h"

MuPDF::Colorspace::Colorspace(ColorspaceKind kind, ColorspaceFlags flags, int n, String^ name) {
    EncodeUTF8(name, csn)
    _colorspace = fz_new_colorspace(Context::Ptr, static_cast<enum fz_colorspace_type>(kind), (int)flags, n, (const char*)csn);
    fz_keep_colorspace(Context::Ptr, _colorspace);
}
