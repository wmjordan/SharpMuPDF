#include "mupdf/fitz.h"
#include "../Context.h"

#ifndef __MUEXCEPTION
#define __MUEXCEPTION

#define MuTryReturn(ctx, op, var) var = NULL; fz_try(ctx) { var = op; } fz_catch(ctx) { } return var;
#define MuTry(ctx, op) fz_try(ctx) { op; } fz_catch(ctx) { return 0; } return 1;

#pragma once
using namespace System;
using namespace System::Collections::Generic;

namespace MuPDF {

public ref class MuException : ApplicationException {
public:
	MuException() : ApplicationException(), _code(-1) {}
	MuException(String^ message) : ApplicationException(message), _code(-1) {};
	MuException(String^ message, Exception^ innerException) : ApplicationException(message, innerException), _code(-1) {};
	property int Code {
		int get() { return _code; }
	internal:
		void set(int code) {
			_code = code;
		}
	}

internal:
	MuException(String^ message, int code) : ApplicationException(message), _code(code) {};
	static MuException^ FromContext() {
		int code;
		const char* msg = fz_convert_error(Context::Ptr, &code);
		return gcnew MuException(gcnew String(msg), code);
	}
	static KeyValuePair<int, String^> TryGetErrorFromContext() {
		int code;
		const char* msg = fz_convert_error(Context::Ptr, &code);
		return KeyValuePair<int, String^>(code, gcnew String(msg));
	}

private:
	int _code;
};

};

#endif // !__MUEXCEPTION
