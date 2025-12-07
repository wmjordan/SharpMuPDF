#include "JBIG2Codec.h"
#include "jbig2enc.lossless.h"
#include <malloc.h>

array<Byte>^ MuPDF::JBig2Codec::LosslessEncode(int width, int height, int stride, bool zeroIsWhite, IntPtr src) {
	int length = 0;
	auto encoded = jbig2_lossless_encode(width, height, stride, zeroIsWhite, (uint8_t*)src.ToPointer(), &length);
	if (encoded) {
		array<Byte>^ managedArray = gcnew array<Byte>(length);
		System::Runtime::InteropServices::Marshal::Copy(IntPtr(encoded), managedArray, 0, length);
		free(encoded);
		return managedArray;
	}
	return nullptr;
}
