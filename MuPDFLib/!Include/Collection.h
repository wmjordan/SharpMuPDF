using namespace System::Collections;

#ifndef __COLLECTION
#define __COLLECTION

namespace MuPDF {

#pragma once
generic<class T>
private ref class EmptyCollection {
public:
	static initonly array<T>^ Instance = gcnew array<T>(0);
	static Generic::IEnumerator<T>^ GetEnumerator() {
		return ((Generic::IEnumerable<T>^)Instance)->GetEnumerator();
	}
};

generic<class T>
public interface class IIndexableCollection {
	property int Count {
		int get();
	}
	property T default[int] {
		T get(int index);
	}
};

generic<class TCollection, typename TItem>
where TCollection : IIndexableCollection<TItem>
	ref class IndexableEnumerator : System::Collections::Generic::IEnumerator<TItem> {
	public:
		IndexableEnumerator(TCollection collection) : _collection(collection), _count(collection->Count), _index(-1) {}
		~IndexableEnumerator() {}
		property TItem Current {
			virtual TItem get() {
				return _current;
			}
		};

		property Object^ CurrentBase {
			virtual Object^ get() sealed = System::Collections::IEnumerator::Current::get{
				return Current;
			}
		};

		virtual bool MoveNext() {
			if (++_index < _count) {
				_current = _collection->default[_index];
				return true;
			}
			return false;
		}

		virtual void Reset() {
			_index = -1;
		}

	private:
		TCollection _collection;
		TItem _current;
		int _index;
		const int _count;
	};

template<class TManaged, typename TUnmanaged>
private value struct Enumerator : Generic::IEnumerator<TManaged^> {

public:
	Enumerator(TUnmanaged* first, TUnmanaged* last) : _first(first), _last(last) {}

	property TManaged^ Current {
		virtual TManaged^ get() {
			return gcnew TManaged(_current);
		}
	};

	property Object^ CurrentBase {
		virtual Object^ get() sealed = System::Collections::IEnumerator::Current::get{
			return Current;
		}
	};

	virtual bool MoveNext() {
		if (!_current) {
			_current = _first;
			return _first != NULL;
		}
		if (_current->next && _current != _last) {
			_current = _current->next;
			return true;
		}
		return false;
	}

	virtual void Reset() {
		_current = NULL;
	}

private:
	TUnmanaged* _first;
	TUnmanaged* _last;
	TUnmanaged* _current;
};

};

#endif // !__COLLECTION

