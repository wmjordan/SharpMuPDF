#include "Image.h"

MuPDF::Colorspace^ MuPDF::Image::Colorspace::get() {
	return gcnew MuPDF::Colorspace(_img->colorspace);
}
