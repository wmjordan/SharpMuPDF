#include "Stream.h"
#include "MuException.h"
#include "ObjWrapper.h"
#include "Vcclr.h"

using namespace IO;
using namespace Runtime::InteropServices;

#pragma unmanaged
static fz_stream* OpenStream(fz_context* ctx, const wchar_t* filePath) {
	fz_stream* s;
	MuTryReturn(ctx, fz_open_file_w(ctx, filePath), s)
}

static fz_stream* OpenStream(fz_context* ctx, const unsigned char* buffer, int length) {
	fz_stream* s;
	MuTryReturn(ctx, fz_open_memory(ctx, buffer, length), s)
}

static fz_stream* DecodeTiffFax(fz_context* ctx, fz_stream* s, int width, int height, int k, int endOfLine, int encodedByteAlign, int endOfBlock, int blackIs1) {
	fz_stream* r;
	MuTryReturn(ctx, fz_open_faxd(ctx, s, k, endOfLine, encodedByteAlign, width, height, endOfBlock, blackIs1), r);
}

#pragma managed
MuPDF::Stream::Stream(String^ filePath) {
	pin_ptr<const wchar_t> p = PtrToStringChars(filePath);
	fz_stream* s = OpenStream(Context::Ptr, (const wchar_t*)p);
	if (s) {
		_stream = s;
		_initDataLength = -1;
		return;
	}
	throw MuException::FromContext();
}

array<Byte>^ MuPDF::Stream::ReadAll(int maxSize) {
	if (_data.IsAllocated) {
		GcnewArray(Byte, b, maxSize > _initDataLength ? _initDataLength : maxSize);
		pin_ptr<Byte> p = &b[0];
		fz_read(Context::Ptr, _stream, p, b->Length);
		return b;
	}

	MemoryStream^ ms = gcnew MemoryStream(4096);
	BinaryWriter^ bw = gcnew BinaryWriter(ms);
	size_t l;
	try {
		GcnewArray(Byte, b, 4096);
		pin_ptr<Byte> p = &b[0];
		while ((l = fz_read(Context::Ptr, _stream, p, 4096)) > 0) {
			bw->Write(b, 0, (int)(l));
			if (ms->Length >= 0x06400000 && ms->Length > maxSize) {
				throw gcnew IOException("Compression bomb detected.");
			}
		}
		ms->Flush();
		return ms->ToArray();
	}
	catch (const Exception^) {
		throw;
	}
	finally {
		if (ms) { delete ms; }
		if (bw) { delete bw; }
	}
}

MuPDF::Stream^ MuPDF::Stream::DecodeTiffFax(int width, int height, int k, bool endOfLine, bool encodeByteAlign, bool endOfBlock, bool blackIs1) {
	auto s = ::DecodeTiffFax(Context::Ptr, _stream, width, height, k, endOfLine, encodeByteAlign, endOfBlock, blackIs1);
	if (s != NULL) {
		return gcnew MuPDF::Stream(s);
	}
	throw MuException::FromContext();
}

MuPDF::Stream::Stream(array<Byte>^ data) {
	_data = GCHandle::Alloc(data, GCHandleType::Pinned);
	fz_stream* s = OpenStream(Context::Ptr, (const unsigned char*)(void*)(_data.AddrOfPinnedObject()), data->Length);
	if (s) {
		_stream = s;
		_initDataLength = data->Length;
		return;
	}
	throw MuException::FromContext();
}

void MuPDF::Stream::ReleaseHandle() {
	fz_drop_stream(Context::Ptr, _stream);
	if (_data.IsAllocated) {
		_data.Free();
	}
	_stream = NULL;
}
