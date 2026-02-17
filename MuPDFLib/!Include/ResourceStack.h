#ifndef __RESOURCE_STACK
#define __RESOURCE_STACK

#pragma once
#include "MuPDF.h"

using namespace System;
using namespace System::Collections::Generic;
using ResStack = System::Collections::Generic::Stack<MuPDF::PdfDictionary^>;

namespace MuPDF {

public ref class ResourceStack sealed {

public:
	ResourceStack(PdfDictionary^ resource);
	property int Count {
		int get() { return _stack->Count; }
	}

	PdfDictionary^ Pop();

	void Push(PdfDictionary^ res);

	PdfObject^ LookupResource(PdfNames type, String^ name);

internal:
	property pdf_resource_stack* Ptr { pdf_resource_stack* get() { return _ptr; } }

private:
	pdf_resource_stack* _ptr = fz_malloc_struct(Context::Ptr, pdf_resource_stack);
	ResStack^ _stack = gcnew ResStack();
	!ResourceStack();

	~ResourceStack() {
		this->!ResourceStack();
	}
};

}

#endif