#include "mupdf/fitz.h"
#include "mupdf/pdf.h"
#include "MuPDF.h"

#ifndef __COOKIE
#define __COOKIE

#pragma once
namespace MuPDF {

public ref class Cookie : IEquatable<Cookie^> {
public:
	Cookie() {
		_cookie = new fz_cookie();
	}
	property bool IsCancellationPending {
		bool get() { return _cookie->abort; }
	}
	property int Progress {
		int get() { return _cookie->progress; }
	}
	property int ProgressMax {
		int get() { return _cookie->progress_max; }
	}
	property int Errors {
		int get() { return _cookie->errors; }
	}
	void Cancel() {
		_cookie->abort = 1;
	}
	~Cookie() {
		delete _cookie;
	}

	Equatable(Cookie, _cookie)

internal:
	Cookie(fz_cookie* cookie) {
		_cookie = cookie;
	};

	property fz_cookie* Ptr {
		fz_cookie* get() { return _cookie; }
	}
private:
	fz_cookie* _cookie;
};

};

#endif
