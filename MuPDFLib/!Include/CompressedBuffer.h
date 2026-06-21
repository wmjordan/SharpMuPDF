#ifndef __COMPRESSEDBUFFER
#define __COMPRESSEDBUFFER

#pragma once
#include "../Context.h"

using namespace System;

namespace MuPDF {

ref class Stream;

public ref class CompressedBuffer sealed : IDisposable {
public:
	CompressedBuffer() : CompressedBuffer(fz_new_compressed_buffer(Context::Ptr)) {};

	property bool IsDisposed { bool get() { return _buffer == NULL; } }

	/// <summary>
	/// Returns storage size used for this compressed buffer (in bytes).
	/// </summary>
	property long long Size { long long get() { return _buffer ? (long)fz_compressed_buffer_size(_buffer) : 0; } }

	/// <summary>Open a stream to read decompressed data from this compressed buffer.</summary>
	Stream^ Open();

internal:
	CompressedBuffer(fz_compressed_buffer* buffer);

	~CompressedBuffer() {
		this->!CompressedBuffer();
	}

private:
	fz_compressed_buffer* _buffer;
	!CompressedBuffer();
};

}

#endif // !__COMPRESSEDBUFFER