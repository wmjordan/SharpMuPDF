#include "MuPDF.h"
#include "ResourceStack.h"

using namespace MuPDF;

void ResourceStack::Push(PdfDictionary^ resources) {
	if (resources == nullptr)
		throw gcnew ArgumentNullException("resources");

	pdf_obj* obj = resources->Ptr;
	if (obj == nullptr)
		throw gcnew InvalidOperationException("Resources object is null.");

	pdf_keep_obj(Context::Ptr, obj);

	pdf_resource_stack* node = fz_malloc_struct(Context::Ptr, pdf_resource_stack);
	node->resources = obj;
	node->next = _head;
	_head = node;
	_count++;
}

PdfDictionary^ ResourceStack::Pop() {
	if (_head == nullptr)
		throw gcnew InvalidOperationException("Resource stack is empty.");

	pdf_resource_stack* node = _head;
	_head = _head->next;
	_count--;

	PdfDictionary^ result = gcnew PdfDictionary(node->resources);

	pdf_drop_obj(Context::Ptr, node->resources);
	fz_free(Context::Ptr, node);

	return result;
}

void ResourceStack::Clear() {
	while (_head != nullptr) {
		pdf_resource_stack* node = _head;
		_head = _head->next;
		pdf_drop_obj(Context::Ptr, node->resources);
		fz_free(Context::Ptr, node);
	}
	_count = 0;
}

ResourceStack::~ResourceStack() {
	Clear();
}

