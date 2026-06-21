#ifndef __PDFOBJECT
#define __PDFOBJECT

#pragma once
#include "fitz.h"
#include "pdf.h"
#include "Collection.h"
#include "Stream.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Text;

struct pdf_obj {
	short refs;
	unsigned char kind;
	unsigned char flags;
};


namespace MuPDF {

ref class Stream;

public enum class Kind {
	Null,
	Boolean,
	Name,
	Integer,
	Float,
	String,
	Array,
	Dictionary,
	Reference,
	Stream,
	Unknown
};

/// <summary>
/// Encapsulates the PDF obj used by MuPDF
/// </summary>
public ref class PdfObject : IDisposable, IEquatable<PdfObject^> {
public:
	literal int MaxObjectNumber = PDF_MAX_OBJECT_NUMBER;
	literal int MaxGenerationNumber = PDF_MAX_GEN_NUMBER;
	/// <summary>
	/// Gets or sets whether this object is dirty (modified).
	/// </summary>
	property bool IsDirty {
		bool get() { return pdf_obj_is_dirty(_ctx, _obj); }
		void set(bool value) {
			if (value) {
				pdf_dirty_obj(_ctx, _obj);
			}
			else {
				pdf_clean_obj(_ctx, _obj);
			}
		}
	}
	property bool IsIncremental{
		bool get() { return pdf_obj_is_incremental(_ctx, _obj); }
	}
	/// <summary>
	/// Provides direct object kind info.
	/// </summary>
	property Kind TypeKind {
		virtual Kind get();
	}
	property bool IsIndirect {
		bool get() { return pdf_is_indirect(_ctx, _obj); }
	}
	property bool IsStream {
		bool get() { return pdf_is_stream(_ctx, _obj); }
	}
	property bool IsName {
		bool get() { return pdf_is_name(_ctx, _obj); }
	}
	property bool IsNull {
		bool get() { return pdf_is_null(_ctx, _obj); }
	}
	property bool IsBoolean {
		bool get() { return pdf_is_bool(_ctx, _obj); }
	}
	property bool IsInteger {
		bool get() { return pdf_is_int(_ctx, _obj); }
	}
	property bool IsFloat {
		bool get() { return pdf_is_real(_ctx, _obj); }
	}
	property bool IsNumber {
		bool get() { return pdf_is_number(_ctx, _obj); }
	}
	property bool IsString {
		bool get() { return pdf_is_string(_ctx, _obj); }
	}
	property bool IsArray {
		bool get() { return pdf_is_array(_ctx, _obj); }
	}
	property bool IsDictionary {
		bool get() { return pdf_is_dict(_ctx, _obj); }
	}
	property bool IsPredefined {
		bool get() { return _obj < PDF_LIMIT; }
	}
	property int IntegerValue {
		int get() { return pdf_to_int(_ctx, _obj); }
	}
	property long LongValue {
		long get() { return pdf_to_int64(_ctx, _obj); }
	}
	property float FloatValue {
		float get() { return pdf_to_real(_ctx, _obj); }
	}
	property PdfObject^ UnderlyingObject {
		PdfObject^ get() {
			return pdf_is_indirect(_ctx, _obj)
				? Wrap(_obj, true)
				: this;
		}
	}
	void MarkDirty() {
		pdf_dirty_obj(_ctx, _obj);
	}
	bool ToBoolDefault(bool defaultValue) {
		return pdf_to_bool_default(Context::Ptr, _obj, defaultValue);
	}
	int ToInt32Default(int defaultValue) {
		return pdf_to_int_default(Context::Ptr, _obj, defaultValue);
	}
	float ToSingleDefault(float defaultValue) {
		return pdf_to_real_default(Context::Ptr, _obj, defaultValue);
	}
	PdfObject^ DeepClone(bool deep) {
		return Wrap(pdf_deep_copy_obj(Context::Ptr, _obj));
	}

	static bool operator == (PdfObject^ x, PdfObject^ y) {
		return Object::ReferenceEquals(x, y) || x && y && pdf_objcmp(x->_ctx, x->Ptr, y->Ptr) == 0;
	}
	static bool operator != (PdfObject^ x, PdfObject^ y) {
		return !Object::ReferenceEquals(x, y)
			&& !(x && y && pdf_objcmp(x->_ctx, x->Ptr, y->Ptr) == 0);
	}
	virtual bool Equals(PdfObject^ other);
	virtual bool Equals(Object^ obj) override {
		PdfObject^ p;
		return (p = dynamic_cast<PdfObject^>(obj)) && _obj == p->_obj;
	}
	virtual int GetHashCode() override {
#pragma warning(push)
#pragma warning(disable:4302 4311)
		return (int)_obj;
#pragma warning(pop)
	}
	virtual String^ ToString() override {
		return TypeKind.ToString();
	}
internal:
	PdfObject(pdf_obj* obj) : _obj(obj) {
		if (obj >= PDF_LIMIT) {
			pdf_keep_obj(_ctx = Context::Ptr, obj);
		}
	}
	PdfObject(pdf_obj* obj, bool keep) : _obj(obj) {
		if (keep && obj >= PDF_LIMIT) {
			pdf_keep_obj(_ctx = Context::Ptr, obj);
		}
	}
	~PdfObject() {
		ReleaseHandle();
	}
	!PdfObject() {
		ReleaseHandle();
	}
	property fz_context* Ctx { fz_context* get() { return _ctx; } }
	property pdf_obj* Ptr { pdf_obj* get() { return _obj; } }
	static PdfObject^ Wrap(pdf_obj* obj, bool resolve);
	static PdfObject^ Wrap(pdf_obj* obj) {
		return Wrap(obj, false);
	}
private:
	pdf_obj* _obj;
	fz_context* _ctx;

	void ReleaseHandle();
};

public ref class PdfNull : PdfObject {
public:
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Null; }
	}
	static initonly PdfNull^ Instance = gcnew PdfNull();
	virtual String^ ToString() override { return "<null>"; }
	~PdfNull() {}
internal:
	PdfNull() : PdfObject(PDF_NULL) {}
};

public ref class PdfBoolean : PdfObject {
public:
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Boolean; }
	}
	static initonly PdfBoolean^ True = gcnew PdfBoolean(true);
	static initonly PdfBoolean^ False = gcnew PdfBoolean(false);
	property bool Value {
		bool get() { return Ptr == PDF_TRUE; }
	}
	virtual String^ ToString() override { return Ptr == PDF_TRUE ? "<true>" : "<false>"; }
internal:
	PdfBoolean(bool value) : PdfObject(value ? PDF_TRUE : PDF_FALSE) {}
};

#define PDF_MAKE_NAME(STRING,NAME) NAME,
public enum class PdfNames {
	Undefined,
	True,
	False,
#include "name-table.h"
	AllPredefinedNames
};

public ref class PdfName : PdfObject {
public:
	PdfName(String^ name) : PdfObject(New(name)) {};
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Name; }
	}
	property String^ Name {
		String^ get() { return _name ? _name : (_name = GetText()); }
	}
	property PdfNames PredefinedValue {
		PdfNames get() { return (int)Ptr < PDF_ENUM_LIMIT ? (PdfNames)(int)Ptr : PdfNames::Undefined; }
	}
	static operator PdfName^(PdfNames value) {
		return gcnew PdfName((pdf_obj*)value);
	}
	bool Equals(PdfName^ value) {
		return pdf_name_eq(Context::Ptr, Ptr, value->Ptr);
	}
	bool Equals(PdfNames value) {
		return pdf_name_eq(Context::Ptr, Ptr, (pdf_obj*)value);
	}
	virtual String^ ToString() override { return "/" + Name; }
internal:
	PdfName(pdf_obj* obj) : PdfObject(obj) {};
	static pdf_obj* New(String^ text);
	String^ GetText();
private:
	String^ _name;
};

public ref class PdfInteger : PdfObject {
public:
	PdfInteger(int value) : PdfObject(pdf_new_int(Context::Ptr, value)) {};
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Integer; }
	}
	property int Value {
		int get() { return pdf_to_int(Context::Ptr, Ptr); }
		void set(int value) { pdf_set_int(Context::Ptr, Ptr, value); }
	}
	property long LongValue {
		long get() { return pdf_to_int64(Context::Ptr, Ptr); }
		void set(long value) { pdf_set_int(Context::Ptr, Ptr, value); }
	}
	virtual String^ ToString() override { return LongValue.ToString(); }
internal:
	PdfInteger(pdf_obj* obj) : PdfObject(obj) {};
};

public ref class PdfFloat : PdfObject {
public:
	PdfFloat(float value) : PdfFloat(pdf_new_real(Context::Ptr, value)) {};
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Float; }
	}
	property float Value {
		float get() { return pdf_to_real(Context::Ptr, Ptr); }
	}
	virtual String^ ToString() override { return Value.ToString(); }
internal:
	PdfFloat(pdf_obj* obj) : PdfObject(obj) {};
};

public ref class PdfString : PdfObject {
public:
	PdfString(String^ text) : PdfObject(New(text)) {};
	property Kind TypeKind {
		virtual Kind get() override { return Kind::String; }
	}
	property String^ Value {
		String^ get();
	}
	property int Length {
		int get() { return (int)(pdf_to_str_len(Context::Ptr, Ptr)); }
	}
	/// <summary>
	/// Gets underlying bytes in a PDF string.
	/// </summary>
	array<Byte>^ GetBytes();
	virtual String^ ToString() override { return Value; }
internal:
	PdfString(pdf_obj* obj) : PdfObject(obj) {};
	static pdf_obj* New(String^ text);
private:
	static Encoding^ AsciiEncoding = Encoding::ASCII;
	String^ _string;
	String^ DecodePdfString();
};

public ref class PdfContainer abstract : PdfObject {
public:
	property int Count {
		virtual int get() abstract;
	}
protected:
	PdfContainer(pdf_obj* obj) : PdfObject(obj) {};
	PdfContainer(pdf_obj* obj, bool keep) : PdfObject(obj, keep) {};
private:
};

public ref class PdfDictionary : PdfContainer, System::Collections::Generic::IEnumerable<KeyValuePair<PdfName^, PdfObject^>>, IIndexableCollection<KeyValuePair<PdfName^, PdfObject^>> {
public:
	property int Count {
		virtual int get() override { return pdf_dict_len(Context::Ptr, Ptr); }
	}
	property PdfName^ Type {
		PdfName^ get();
	}
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Dictionary; }
	}
	bool IsType(PdfNames typeName) {
		auto o = pdf_dict_get(Context::Ptr, Ptr, PDF_NAME(Type));
		return pdf_name_eq(Context::Ptr, o, (pdf_obj*)typeName);
	}
	bool ContainsKey(PdfNames key) {
		return pdf_dict_get(Context::Ptr, Ptr, (pdf_obj*)key);
	}
	bool ContainsKey(String^ key) {
		return GetValue(key)->Ptr != nullptr;
	}
	PdfName^ GetKey(int index);
	PdfObject^ GetValue(int index) {
		return PdfObject::Wrap(pdf_dict_get_val(Context::Ptr, Ptr, index));
	}
	/// <summary>
	/// Gets value from dictionary.
	/// </summary>
	/// <param name="key">Key of entry.</param>
	/// <returns>Value of corresponding entry, or <see cref="PdfNull"/> if key does not exist.</returns>
	/// <seealso cref="GetObject(String)"/>
	PdfObject^ GetValue(String^ key);
	PdfObject^ GetValue(PdfNames key) {
		return PdfObject::Wrap(pdf_dict_get(Context::Ptr, Ptr, (pdf_obj*)key));
	}
	PdfObject^ GetValue(PdfNames key, PdfNames abbrev) {
		return PdfObject::Wrap(pdf_dict_geta(Context::Ptr, Ptr, (pdf_obj*)key, (pdf_obj*)abbrev));
	}
	/// <summary>
	/// Gets dereferenced object from dictionary.
	/// </summary>
	/// <param name="key">Key of entry.</param>
	/// <returns>Value of corresponding entry, or <see cref="PdfNull"/> if key does not exist.</returns>
	PdfObject^ GetObject(String^ key);
	/// <summary>
	/// Gets dereferenced object from dictionary.
	/// </summary>
	/// <param name="key">Key of entry.</param>
	/// <returns>Value of corresponding entry, or <see cref="PdfNull"/> if key does not exist.</returns>
	PdfObject^ GetObject(PdfNames key) {
		return PdfObject::Wrap(pdf_dict_get(Context::Ptr, Ptr, (pdf_obj*)key), true);
	}
	/// <summary>
	/// Gets dereferenced object from dictionary.
	/// </summary>
	/// <param name="key">Key of entry.</param>
	/// <param name="abbrev">Alternative key of entry.</param>
	/// <returns>Value of corresponding entry, or <see cref="PdfNull"/> if key does not exist.</returns>
	PdfObject^ GetObject(PdfNames key, PdfNames abbrev) {
		return PdfObject::Wrap(pdf_dict_geta(Context::Ptr, Ptr, (pdf_obj*)key, (pdf_obj*)abbrev), true);
	}
	Point GetPoint(PdfNames key) {
		return pdf_dict_get_point(Context::Ptr, Ptr, (pdf_obj*)key);
	}
	Box GetRect(PdfNames key) {
		return pdf_dict_get_rect(Context::Ptr, Ptr, (pdf_obj*)key);
	}
	Matrix GetMatrix(PdfNames key) {
		return pdf_dict_get_matrix(Context::Ptr, Ptr, (pdf_obj*)key);
	}
	int GetInt32(PdfNames key) {
		return pdf_dict_get_int(Context::Ptr, Ptr, (pdf_obj*)key);
	}
	float GetSingle(PdfNames key) {
		return pdf_dict_get_real(Context::Ptr, Ptr, (pdf_obj*)key);
	}
	PdfObject^ InheritableGet(PdfNames key) {
		return PdfObject::Wrap(pdf_dict_get_inheritable(Context::Ptr, Ptr, (pdf_obj*)key));
	}
	PdfObject^ Locate(... array<PdfNames>^ names);
	void Set(PdfNames key, PdfNames value) {
		pdf_dict_put_drop(Context::Ptr, Ptr, (pdf_obj*)key, (pdf_obj*)value);
	}
	void Set(PdfNames key, PdfObject^ value) {
		pdf_dict_put(Context::Ptr, Ptr, (pdf_obj*)key, value->Ptr);
	}
	void Set(PdfNames key, int value) {
		pdf_dict_put_int(Context::Ptr, Ptr, (pdf_obj*)key, value);
	}
	void Set(PdfNames key, long value) {
		pdf_dict_put_int(Context::Ptr, Ptr, (pdf_obj*)key, value);
	}
	void Set(PdfNames key, float value) {
		pdf_dict_put_real(Context::Ptr, Ptr, (pdf_obj*)key, value);
	}
	void Set(PdfNames key, double value) {
		pdf_dict_put_real(Context::Ptr, Ptr, (pdf_obj*)key, value);
	}
	void Set(PdfNames key, String^ value) {
		pdf_dict_put_drop(Context::Ptr, Ptr, (pdf_obj*)key, PdfString::New(value));
	}
	void Set(PdfNames key, DateTime dateTime);
	void SetName(PdfNames key, String^ value);
	void Sort() {
		pdf_sort_dict(Context::Ptr, Ptr);
	}
	bool Remove(PdfNames key);
	bool Remove(PdfName^ key);
	bool Remove(String^ key);
	PdfDictionary^ Clone(bool deep) {
		return gcnew PdfDictionary(deep ? pdf_deep_copy_obj(Context::Ptr, Ptr) : pdf_copy_dict(Context::Ptr, Ptr));
	}
	virtual String^ ToString() override {
		PdfName^ type = Type;
		return type ? String::Concat("{", type->ToString(), "}") : "{}";
	}
	virtual System::Collections::Generic::IEnumerator<KeyValuePair<PdfName^, PdfObject^>>^ GetEnumerator() sealed = System::Collections::Generic::IEnumerable<KeyValuePair<PdfName^, PdfObject^>>::GetEnumerator {
		return gcnew IndexableEnumerator<PdfDictionary^, KeyValuePair<PdfName^, PdfObject^>>(this);
	}
	virtual System::Collections::IEnumerator^ GetEnumeratorBase() sealed =
		System::Collections::IEnumerable::GetEnumerator {
		return GetEnumerator();
	}
internal:
	PdfDictionary(pdf_obj* obj) : PdfContainer(obj) {};
	PdfDictionary(pdf_obj* obj, bool keep) : PdfContainer(obj, keep) {};
public:
	property KeyValuePair<PdfName^, PdfObject^> default[int] {
		virtual KeyValuePair<PdfName^, PdfObject^> get(int index) {
			return KeyValuePair<PdfName^, PdfObject^>(GetKey(index), GetValue(index));
		};
	}
	/// <summary>
	/// Gets or sets entry in <see cref="PdfDictionary"/>.
	/// The return value of get will never be <see langword="null"/>.
	/// If requested entry does not exists, instance of <see cref="PdfNull"/> will be returned instead.
	/// </summary>
	property PdfObject^ default[PdfNames] {
		PdfObject^ get(PdfNames key) { return PdfObject::Wrap(pdf_dict_get(Context::Ptr, Ptr, (pdf_obj*)key)); }
		void set(PdfNames key, PdfObject^ value) { pdf_dict_put(Context::Ptr, Ptr, (pdf_obj*)key, value->Ptr); }
	}
	/// <summary>
	/// Gets or sets entry in <see cref="PdfDictionary"/>.
	/// The return value of get will never be <see langword="null"/>.
	/// If requested entry does not exists, instance of <see cref="PdfNull"/> will be returned instead.
	/// </summary>
	property PdfObject^ default[PdfName^] {
		PdfObject^ get(PdfName^ key) { return PdfObject::Wrap(pdf_dict_get(Context::Ptr, Ptr, key->Ptr)); }
		void set(PdfName^ key, PdfObject^ value) { pdf_dict_put(Context::Ptr, Ptr, key->Ptr, value->Ptr); }
	}
};

public ref class PdfStream : PdfDictionary {
public:
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Stream; }
	}
	Stream^ Open();
	Stream^ OpenRaw();
	array<Byte>^ GetBytes();
	array<Byte>^ GetRawBytes();

	/// <summary>
	/// Replaces bytes in the stream. The data must match /Filter, if <paramref name="compress"/> is true.
	/// </summary>
	/// <param name="data">The data to be placed into the stream.</param>
	/// <param name="compress">Whether the data is compressed. If not compressed, /Filter and /DecodeParms will be removed.</param>
	void SetBytes(array<Byte>^ data, bool compress);
internal:
	PdfStream(pdf_obj* obj) : PdfDictionary(obj) {
		pdf_keep_obj(Context::Ptr, obj);
	};
};

public ref class PdfDocumentInfo : PdfDictionary {
public:
	property String^ Title {
		String^ get() { return GetString(PdfNames::Title); }
	}
	property String^ Subject {
		String^ get() { return GetString(PdfNames::Subject); }
	}
	property String^ Producer {
		String^ get() { return GetString(PdfNames::Producer); }
	}
	property String^ Creator {
		String^ get() { return GetString(PdfNames::Creator); }
	}
	property String^ Author {
		String^ get() { return GetString(PdfNames::Author); }
	}
	property String^ Keywords {
		String^ get() { return GetString(PdfNames::Keywords); }
	}
	property String^ CreationDate {
		String^ get() { return GetString(PdfNames::CreationDate); }
	}
	property String^ ModificationDate {
		String^ get() { return GetString(PdfNames::ModDate); }
	}
internal:
	PdfDocumentInfo(pdf_obj* obj) : PdfDictionary(obj) {};
private:
	String^ GetString(PdfNames key) {
		return GetValue(key)->ToString();
	}
};

public ref class PdfArray : PdfContainer, System::Collections::Generic::IEnumerable<PdfObject^>, IIndexableCollection<PdfObject^> {
public:
	property int Count {
		virtual int get() override { return pdf_array_len(Context::Ptr, Ptr); }
	}
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Array; }
	}
	PdfObject^ Get(int index) {
		return Wrap(pdf_array_get(Context::Ptr, Ptr, index));
	}
	/// <summary>
	/// Gets dereferenced object from array.
	/// </summary>
	/// <param name="index">Index of entry.</param>
	/// <returns>Value of corresponding entry, or <see cref="PdfNull"/> if index does not exist.</returns>
	PdfObject^ GetObject(int index) {
		return Wrap(pdf_array_get(Context::Ptr, Ptr, index), true);
	}
	PdfName^ GetName(int index);
	bool Contains(PdfObject^ obj) {
		return pdf_array_contains(Context::Ptr, Ptr, obj->Ptr);
	}
	/// <summary>
	/// Gets position of given object from array.
	/// </summary>
	/// <param name="obj">Object to find.</param>
	/// <returns>Position of object, or -1 if object does not exist in the array.</returns>
	int IndexOf(PdfObject^ obj) {
		return pdf_array_find(Context::Ptr, Ptr, obj->Ptr);
	}
	void Append(bool value) {
		pdf_array_push_bool(Context::Ptr, Ptr, value);
	}
	void Append(long value) {
		pdf_array_push_int(Context::Ptr, Ptr, value);
	}
	void Append(double value) {
		pdf_array_push_real(Context::Ptr, Ptr, value);
	}
	void Append(PdfNames value) {
		pdf_array_push_drop(Context::Ptr, Ptr, (pdf_obj*)value);
	}
	void Append(String^ value) {
		pdf_array_push_drop(Context::Ptr, Ptr, PdfString::New(value));
	}
	void Append(PdfObject^ value) {
		pdf_array_push_drop(Context::Ptr, Ptr, value->Ptr);
	}
	void Set(int index, bool value) {
		pdf_array_put_bool(Context::Ptr, Ptr, index, value);
	}
	void Set(int index, long value) {
		pdf_array_put_int(Context::Ptr, Ptr, index, value);
	}
	void Set(int index, double value) {
		pdf_array_put_real(Context::Ptr, Ptr, index, value);
	}
	void Set(int index, PdfNames value) {
		pdf_array_put(Context::Ptr, Ptr, index, (pdf_obj*)value);
	}
	void Set(int index, String^ value) {
		pdf_array_put_drop(Context::Ptr, Ptr, index, PdfString::New(value));
	}
	void Set(int index, PdfObject^ value) {
		pdf_array_put_drop(Context::Ptr, Ptr, index, value->Ptr);
	}
	void InsertAt(int index, PdfObject^ value) {
		pdf_array_insert_drop(Context::Ptr, Ptr, value->Ptr, index);
	}
	void RemoveAt(int index) {
		pdf_array_delete(Context::Ptr, Ptr, index);
	}
	PdfArray^ Clone(bool deep) {
		return gcnew PdfArray(deep ? pdf_deep_copy_obj(Context::Ptr, Ptr) : pdf_copy_array(Context::Ptr, Ptr));
	}
	virtual String^ ToString() override { return String::Concat("[", Count.ToString(), "]"); }
	virtual System::Collections::Generic::IEnumerator<PdfObject^>^ GetEnumerator() sealed = System::Collections::Generic::IEnumerable<PdfObject^>::GetEnumerator{
		return gcnew IndexableEnumerator<PdfArray^, PdfObject^>(this);
	}
	virtual System::Collections::IEnumerator^ GetEnumeratorBase() sealed = System::Collections::IEnumerable::GetEnumerator {
		return GetEnumerator();
	}
internal:
	PdfArray(pdf_obj* obj) : PdfContainer(obj) {};
public:
	property PdfObject^ default[int] {
		virtual PdfObject^ get(int index) { return Wrap(pdf_array_get(Context::Ptr, Ptr, index)); }
		void set(int index, PdfObject^ value) { pdf_array_put_drop(Context::Ptr, Ptr, index, value->Ptr); }
	}
};

public ref class PdfReference : PdfObject {
public:
	property int Number {
		int get() { return pdf_to_num(Context::Ptr, Ptr); }
	}
	property int Generation {
		int get() { return pdf_to_gen(Context::Ptr, Ptr); }
	}
	property Kind TypeKind {
		virtual Kind get() override { return Kind::Reference; }
	}
	PdfObject^ Resolve() {
		return Wrap(Ptr, true);
	}
	virtual String^ ToString() override {
		return String::Concat(Number.ToString(), " ", Generation.ToString(), " R");
	}
internal:
	PdfReference(pdf_obj* obj) : PdfObject(obj) {};
};

};

#endif // !__PDFOBJECT
