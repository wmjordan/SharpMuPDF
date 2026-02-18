#ifndef __BUFFER
#define __BUFFER

#pragma once
#include "../Context.h"
#include <mupdf/fitz/buffer.h>

using namespace System;

namespace MuPDF {

	public ref class Buffer sealed : IDisposable {
	public:
		property bool IsDisposed {
			bool get() { return _buf == NULL; }
		}

		property int Capacity { int get() { return _buf ? (int)_buf->cap : 0; } }

		/// <summary>Gets current size of buffer in bytes.</summary>
		property int Length { int get() { return _buf ? (int)_buf->len : 0; } }

		/// <summary>
		/// Returns a managed copy of buffer contents.
		/// </summary>
		array<Byte>^ ToArray();

		/// <summary>
		/// Append bytes to buffer.
		/// </summary>
		void Append(array<Byte>^ data);

		/// <summary>
		/// Clear buffer contents (keeps capacity).
		/// </summary>
		void Clear() {
			if (!_buf) return;
			fz_clear_buffer(Context::Ptr, _buf);
		}

		/// <summary>
		/// Ensure buffer is zero-terminated (useful if treated as C string).
		/// </summary>
		void Terminate() {
			if (!_buf) return;
			fz_terminate_buffer(Context::Ptr, _buf);
		}

		/// <summary>
		/// Return buffer contents as a UTF8 string (uses zero-terminated data).
		/// May return empty string for binary data.
		/// </summary>
		String^ GetString();

		/// <summary>
		/// Extracts buffer contents: makes a managed copy and leaves native buffer empty.
		/// </summary>
		array<Byte>^ Extract();

		/// <summary>
		/// Create a Buffer from managed bytes (copies data).
		/// </summary>
		static Buffer^ FromArray(array<Byte>^ data);

	internal:
		property fz_buffer* Ptr { fz_buffer* get() { return _buf; } }
		Buffer(fz_buffer* buf) : _buf(buf) {
			if (_buf) {
				fz_keep_buffer(Context::Ptr, _buf);
			}
		}

		~Buffer() {
			ReleaseHandle();
		}

	private:
		fz_buffer* _buf;

		void ReleaseHandle();
	};

}

#endif // !__BUFFER