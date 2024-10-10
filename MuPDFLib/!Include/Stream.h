#include "mupdf/fitz.h"
#include "mupdf/pdf.h"
#include "MuPDF.h"

#ifndef __STREAM
#define __STREAM

#pragma once
using namespace System;
using namespace System::Runtime::InteropServices;

namespace MuPDF {

/// <summary>
/// Encapsulates the file stream used by MuPDF
/// </summary>
public ref class Stream sealed : IDisposable {
public:
	Stream(array<Byte>^ data);
	Stream(String^ filePath);
	array<Byte>^ ReadAll(int maxSize);
	array<Byte>^ ReadAll() {
		return ReadAll(0x6400000);
	}
	Stream^ DecodeTiffFax(int width, int height, int k, bool endOfLine, bool encodeByteAlign, bool endOfBlock, bool blackIs1);
internal:
	Stream(fz_stream* stream) : _stream(stream), _initDataLength(-1) {
		fz_keep_stream(Context::Ptr, stream);
	}
	~Stream() {
		ReleaseHandle();
	}
	property fz_stream* Ptr {
		fz_stream* get() { return _stream; }
	}
private:
	fz_stream* _stream;
	GCHandle _data;
	int _initDataLength;

	void ReleaseHandle();
};

};

#endif