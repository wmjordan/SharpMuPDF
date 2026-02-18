#include "CompressedBuffer.h"
#include "Stream.h"

using namespace MuPDF;

Stream^ CompressedBuffer::Open() { return gcnew Stream(fz_open_compressed_buffer(Context::Ptr, _buffer)); }

CompressedBuffer::CompressedBuffer(fz_compressed_buffer* buffer) {
	if (buffer) {
		_buffer = buffer;
		fz_keep_compressed_buffer(Context::Ptr, buffer);
	}
}

MuPDF::CompressedBuffer::!CompressedBuffer() {
	DropHandle(_buffer, fz_drop_compressed_buffer);
}
