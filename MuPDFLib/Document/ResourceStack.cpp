#include "ResourceStack.h"

using namespace MuPDF;

ResourceStack::ResourceStack(PdfDictionary^ resource) {
	_ptr->resources = resource->Ptr;
	_stack->Push(resource);
}

PdfDictionary^ ResourceStack::Pop() {
	if (_stack->Count) {
		auto next = _ptr->next;
		fz_free(Context::Ptr, _ptr);
		_ptr = next;
		return _stack->Pop();
	}
	return nullptr;
}

void ResourceStack::Push(PdfDictionary^ res) {
	_stack->Push(res);
	auto p = _ptr;
	_ptr = fz_malloc_struct(Context::Ptr, pdf_resource_stack);
	_ptr->resources = res->Ptr;
	_ptr->next = p;
}

PdfObject^ ResourceStack::LookupResource(PdfNames type, String^ name) {
	EncodeUTF8(name, n);
	return PdfObject::Wrap(pdf_lookup_resource(Context::Ptr, _ptr, (pdf_obj*)type, (const char*)n));
}

ResourceStack::!ResourceStack() {
	while (_ptr) {
		pdf_resource_stack* stk = _ptr;
		_ptr = stk->next;
		fz_free(Context::Ptr, stk);
	}
	_ptr = NULL;
}
