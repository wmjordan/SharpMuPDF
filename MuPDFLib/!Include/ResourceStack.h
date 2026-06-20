#ifndef __RESOURCE_STACK
#define __RESOURCE_STACK
#pragma once

#include "MuPDF.h"

using namespace System;

namespace MuPDF {

ref class PdfDictionary;

public ref class ResourceStack sealed : IDisposable {

public:
	ResourceStack() : _head(nullptr), _count(0) {}

	void Push(PdfDictionary^ resources);

	PdfDictionary^ Pop();

	property int Count {
		int get() { return _count; }
	}

	property bool IsEmpty {
		bool get() { return _head == nullptr; }
	}

	~ResourceStack();

internal:
	property pdf_resource_stack* Ptr {
		pdf_resource_stack* get() { return _head; }
	}

private:
	pdf_resource_stack* _head;
	int _count;

	!ResourceStack() {
		Clear();
	}

	void Clear();
};

}
#endif