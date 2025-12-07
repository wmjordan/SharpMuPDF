#include "PdfObject.h"
#include "MuException.h"
#include "Vcclr.h"

#pragma region pdf-object.c

typedef enum pdf_objkind_e {
	PDF_INT = 'i',
	PDF_REAL = 'f',
	PDF_STRING = 's',
	PDF_NAME = 'n',
	PDF_ARRAY = 'a',
	PDF_DICT = 'd',
	PDF_INDIRECT = 'r'
} pdf_objkind;

#pragma endregion

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace MuPDF;

// hack: this method hacks into mupdf/pdf-object.c and provides direct type kind info of a PdfObject
Kind PdfObject::TypeKind::get() {
	if (_obj == PDF_NULL) {
		return Kind::Null;
	}
	if (_obj == PDF_TRUE || _obj == PDF_FALSE) {
		return Kind::Boolean;
	}
	if (_obj < PDF_LIMIT) {
		return Kind::Name;
	}
	switch (_obj->kind) {
	case PDF_INT: return Kind::Integer;
	case PDF_REAL: return Kind::Float;
	case PDF_STRING: return Kind::String;
	case PDF_NAME: return Kind::Name;
	case PDF_ARRAY: return Kind::Array;
	case PDF_DICT: return pdf_is_stream(_ctx, _obj) ? Kind::Stream : Kind::Dictionary;
	case PDF_INDIRECT: return Kind::Reference;
	}
	return Kind::Unknown;
};

PdfObject^ PdfObject::Wrap(pdf_obj* obj, bool resolve) {
	START:
	if (obj == PDF_NULL) {
		return (PdfObject^)PdfNull::Instance;
	}
	if (obj == PDF_TRUE) {
		return (PdfObject^)PdfBoolean::True;
	}
	if (obj == PDF_FALSE) {
		return (PdfObject^)PdfBoolean::False;
	}
	if (obj < PDF_LIMIT) {
		return gcnew PdfName(obj);
	}
	switch (obj->kind) {
	case PDF_INT: return gcnew PdfInteger(obj);
	case PDF_REAL: return gcnew PdfFloat(obj);
	case PDF_STRING: return gcnew PdfString(obj);
	case PDF_NAME: return gcnew PdfName(obj);
	case PDF_ARRAY: return gcnew PdfArray(obj);
	case PDF_DICT: return gcnew PdfDictionary(obj);
	case PDF_INDIRECT:
		if (resolve) {
			if (pdf_is_stream(Context::Ptr, obj)) {
				return gcnew PdfStream(obj);
			}
			obj = pdf_resolve_indirect_chain(Context::Ptr, obj);
			goto START;
		}
		return gcnew PdfReference(obj);
	}
	throw gcnew MuException("Unexpected object kind: " + obj->kind.ToString());
}

bool PdfObject::Equals(PdfObject^ other) {
	return other && pdf_objcmp(_ctx, Ptr, other->Ptr) == 0;
}

pdf_obj* PdfContainer::NewPdfString(String^ text) {
	array<unsigned char>^ b;
	pin_ptr<unsigned char> pb;
	for each (auto ch in text) {
		if (ch > 127) {
			int l = (text->Length + 1) << 1;
			b = gcnew array<unsigned char>(l);
			b[0] = 0xFF;
			b[1] = 0xFE;
			pin_ptr<const wchar_t> ps = PtrToStringChars(text);
			pb = &b[2];
			memcpy(pb, ps, l - 2);
			goto MAKE_STRING;
		}
	}
	b = AsciiEncoding->GetBytes(text);
MAKE_STRING:
	pb = &b[0];
	return pdf_new_string(Ctx, (const char*)(void*)pb, b->Length);
}

PdfObject^ MuPDF::PdfDictionary::GetValue(String^ key) {
	EncodeUTF8(key, p)
	pdf_obj* v = pdf_dict_gets(Context::Ptr, Ptr, (const char*)p);
	return Wrap(v);
}

PdfObject^ MuPDF::PdfDictionary::GetObject(String^ key) {
	EncodeUTF8(key, p)
	pdf_obj* v = pdf_dict_gets(Context::Ptr, Ptr, (const char*)p);
	return Wrap(v, true);
}

PdfObject^ PdfDictionary::Locate(...array<PdfNames>^ names) {
	auto ctx = Ctx;
	auto obj = Ptr;
	int length = names->Length;
	if (length == 0) {
		goto RETURN_NULL;
	}
	int i;
	for (i = 0; i < length - 1; i++) {
		obj = pdf_dict_get(ctx, obj, (pdf_obj*)names[i]);
		if (!obj) {
			goto RETURN_NULL;
		}
		obj = pdf_resolve_indirect_chain(ctx, obj);
		if (pdf_is_dict(ctx, obj) == false) {
			goto RETURN_NULL;
		}
	}
	return Wrap(pdf_dict_get(ctx, obj, (pdf_obj*)names[i]), true);
RETURN_NULL:
	return (PdfObject^)PdfNull::Instance;
}

array<Byte>^ PdfString::GetBytes() {
	size_t l;
	auto b = pdf_to_string(Ctx, Ptr, &l);
	if (l > INT_MAX) {
		throw gcnew System::InsufficientMemoryException("String length larger than INT_MAX");
	}
	array<Byte>^ r = gcnew array<Byte>((int)(l));
	if (l) {
		pin_ptr<unsigned char> pb = &r[0];
		memcpy(pb, b, l);
	}
	return r;
}

String^ PdfString::DecodePdfString() {
	size_t l;
	auto b = pdf_to_string(Ctx, Ptr, &l);
	if (l > INT_MAX) {
		throw gcnew System::InsufficientMemoryException("String length larger than INT_MAX");
	}
	if (b[0] == (char)254 && b[1] == (char)255) {
		return gcnew String(b, 2, (int)(l - 2), Encoding::BigEndianUnicode);
	}
	if (b[0] == (char)255 && b[1] == (char)254) {
		return gcnew String(b, 2, (int)(l - 2), Encoding::Unicode);
	}
	// PDFENCODE
	auto chars = new Char[l];
	for (size_t i = 0; i < l; i++) {
		chars[i] = fz_unicode_from_pdf_doc_encoding[b[i]];
	}
	return gcnew String(chars, 0, (int)(l));
}
String^ PdfString::Value::get() {
	return _string ? _string : (_string = DecodePdfString());
}

void PdfStream::SetBytes(array<Byte>^ data, bool isCompressed) {
	auto ctx = Ctx;
	pin_ptr<Byte> d = &data[0];
	fz_buffer* b = fz_new_buffer_from_copied_data(ctx, d, data->Length);
	pdf_update_stream(ctx, pdf_pin_document(ctx, Ptr), Ptr, b, isCompressed);
	fz_free(ctx, b);
}

String^ MuPDF::PdfName::GetText() {
	auto b = pdf_to_name(Ctx, Ptr);
	return DecodeUTF8(b);
}
