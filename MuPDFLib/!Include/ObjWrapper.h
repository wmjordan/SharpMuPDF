#ifndef __OBJWRAPPER
#define __OBJWRAPPER

#pragma once

#define GcnewArray(TManage, var, Length) array<TManage>^ var = gcnew array<TManage>(Length);

#define GcWrap(TManage, ptr) (ptr ? gcnew TManage(ptr) : nullptr)

#define Unwrap(TManage) (TManage ? TManage->Ptr : NULL)

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

#define FreeHandle(gchandle) if(gchandle.IsAllocated) gchandle.Free();

#endif // !__OBJWRAPPER
