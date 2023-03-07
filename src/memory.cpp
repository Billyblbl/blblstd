#ifndef G_MEMORY
# define G_MEMORY

#include <utils.cpp>
#include <stdlib.h>

using Buffer = Array<byte>;
using AllocStrat = Buffer(*)(any*, Buffer, usize);

struct Alloc {
	any* context;
	AllocStrat strategy;
	inline Buffer alloc(usize size) { return strategy(context, {}, size); }
	inline Buffer realloc(Buffer buffer, usize size) { return strategy(context, buffer, size); }
	inline void dealloc(Buffer buffer) { strategy(context, buffer, 0); }
};

template<typename T> inline Array<T> alloc_array(Alloc allocator, usize count) {
	return cast<T>(allocator.alloc(sizeof(T) * count));
}

template<typename T> inline Array<T> realloc_array(Alloc allocator, Array<T> arr, usize count) {
	return cast<T>(allocator.realloc(cast<byte>(arr), sizeof(T) * count));
}

template<typename T> inline void dealloc_array(Alloc allocator, Array<T> arr) {
	return allocator.dealloc(cast<byte>(arr));
}

template<typename T> inline Array<T> duplicate_array(Alloc allocator, Array<T> arr) {
	auto other = alloc_array<T>(allocator, arr.size());
	memcpy(other.data(), arr.data(), arr.size());
	return other;
}

extern Alloc std_allocator;

#ifdef BLBLSTD_IMPL

Buffer std_alloc_strategy(any*, Buffer buffer, usize size) {
	auto ptr = realloc(buffer.data(), size);
	return Buffer((byte*)ptr, ptr != null ? size : 0);
}
Alloc std_allocator = { null, &std_alloc_strategy };
#endif

#endif
