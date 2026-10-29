#ifndef __OBJWRAPPER
#define __OBJWRAPPER

#pragma once

#define GcnewArray(TManage, var, Length) array<TManage>^ var = gcnew array<TManage>(Length);

#define PinArray(TManage, var, ptr) pin_ptr<TManage> ptr = (var && var->Length != 0) ? &var[0] : nullptr

#define GcWrap(TManage, ptr) (ptr ? gcnew TManage(ptr) : nullptr)

#define Unwrap(TManage) (TManage ? TManage->Ptr : NULL)

#define PropEmptyGet(TProp, propName) property TProp propName { TProp get(); }

#define PropGet(TProp, propName, func) property TProp propName { TProp get() { return func; } }

#define PropGetSet(TProp, propName, getter, setter)\
property TProp propName {\
	TProp get() { return getter; }\
	void set(TProp value) { setter; }\
}

#define ReadonlyIndexer(TValue, TParam, paramName, getter)\
	property TValue default[TParam] {\
		virtual TValue get(TParam paramName) {\
			return getter;\
		};\
	}

#define Indexer(TParam, paramName, TValue, valueName, getter, setter)\
	property TValue default[TParam] {\
		virtual TValue get(TParam paramName) {\
			return getter;\
		};\
		void set(TParam paramName, TValue valueName) {\
			setter;\
		};\
	}

#define Equatable(TManage, OPtr) \
	public: \
		static bool operator == (TManage^ x, TManage^ y) { \
			return Object::ReferenceEquals(x, y) || x && y && x->OPtr == y->OPtr; \
		} \
		static bool operator != (TManage ^ x, TManage ^ y) { \
			return !(x == y); \
		} \
		virtual bool Equals(TManage ^ other) { \
			return other && OPtr == other->OPtr; \
		} \
		virtual bool Equals(Object^ obj) override { \
			TManage ^ p; \
			return (p = dynamic_cast<TManage^>(obj)) && OPtr == p->OPtr; \
		} \
		virtual int GetHashCode() override { \
			return (int)(IntPtr)OPtr; \
		}

#define StaticMethodToFunctionPointer(method, TDelegate, TFuncPointer, staticHandle, funcPointer) \
    { \
        auto d = gcnew TDelegate(&method); \
        staticHandle = System::Runtime::InteropServices::GCHandle::Alloc(d); \
        funcPointer = static_cast<TFuncPointer*>(System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(d).ToPointer()); \
    };

#define InstanceMethodToFunctionPointer(instance, methodName, TDelegate, TFuncPointer, handle, funcPointer) \
    { \
        auto d = gcnew TDelegate(instance, &methodName); \
        handle = System::Runtime::InteropServices::GCHandle::Alloc(d); \
        funcPointer = static_cast<TFuncPointer*>(System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(d).ToPointer()); \
    };

#define DecodeUTF8(chars) gcnew String(chars, 0, strlen(chars), System::Text::Encoding::UTF8)

#define EncodeUTF8(text, ptr) pin_ptr<unsigned char> ptr = &System::Text::Encoding::UTF8->GetBytes(text)[0];

#define FreeGCHandle(gchandle) if(gchandle.IsAllocated) gchandle.Free();

#define DropHandle(var, drop) if (var) {\
	drop(Context::Ptr, var);\
	var = NULL;\
}

#define DisposeObject(var) if (var) {\
	delete var;\
	var = nullptr;\
}

#endif // !__OBJWRAPPER
