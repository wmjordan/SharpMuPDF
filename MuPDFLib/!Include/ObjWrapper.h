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
			return (int)OPtr; \
		}
	