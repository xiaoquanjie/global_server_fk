#pragma once

template<typename T, int MaxAlloc>
class ZkObjPool {
public:
	ZkObjPool() {
		numalloc = 0;
		listhead = 0;
		elemsize = sizeof(T*) + sizeof(T);
	}

	~ZkObjPool() {
		while (listhead) {
			char* c = (char*)listhead;
			listhead = *(reinterpret_cast<T**>(listhead));
			T* obj = (T*)(c + sizeof(T*));
			obj->~T();
			free(c);
		}
	}

	T* alloc() {
		T* obj = 0;
		if (listhead == 0) {
			char* c = (char*)malloc(elemsize);
			obj = (T*)(c + sizeof(T*));
			new(obj)T();
			numalloc++;
		}
		else {
			char* c = (char*)listhead;
			listhead = *(reinterpret_cast<T**>(listhead));
			obj = (T*)(c + sizeof(T*));
		}
		return obj;
	}

	void dealloc(T* item) {
		if (numalloc > MaxAlloc) {
			char* c = (char*)item - sizeof(T*);
			item->~T();
			free(c);
			numalloc--;
		}
		else {
			char* c = (char*)item - sizeof(T*);
			*(reinterpret_cast<T**>(c)) = listhead;
			listhead = (T*)c;
		}
	}

private:
	T* listhead;
	size_t elemsize;
	size_t numalloc;
};

