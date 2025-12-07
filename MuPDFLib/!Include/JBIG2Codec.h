
#pragma once

#ifndef __JBIG2CODEC
#define __JBIG2CODEC

using namespace System;
using namespace System::Runtime::InteropServices;

namespace MuPDF {

public ref class JBig2Codec sealed {
public:
	static array<Byte>^ LosslessEncode(int width, int height, int stride, bool zeroIsWhite, IntPtr src);
	static array<Byte>^ Decode(array<Byte>^ data, array<Byte>^ globals);

private:

};
};

#endif // __COLORSPACE
