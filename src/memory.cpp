#ifndef G_MEMORY
# define G_MEMORY

#include <utils.cpp>
#include <stdlib.h>

using Buffer = Array<byte>;
using AllocStrat = Buffer(*)(any*, Buffer, usize, u64);

struct Alloc {
	any* context;
	AllocStrat strategy;
	inline Buffer alloc(usize size, u64 flags = 0) { return strategy(context, {}, size, flags); }
	inline Buffer realloc(Buffer buffer, usize size, u64 flags = 0) { return strategy(context, buffer, size, flags); }
	inline void dealloc(Buffer buffer, u64 flags = 0) { strategy(context, buffer, 0, flags); }
};

template<typename T> inline Array<T> alloc_array(Alloc allocator, usize count, u64 flags = 0) {
	return cast<T>(allocator.alloc(sizeof(T) * count));
}

template<typename T> inline Array<T> realloc_array(Alloc allocator, Array<T> arr, usize count, u64 flags = 0) {
	return cast<T>(allocator.realloc(cast<byte>(arr), sizeof(T) * count));
}

template<typename T> inline void dealloc_array(Alloc allocator, Array<T> arr, u64 flags = 0) {
	return allocator.dealloc(cast<byte>(arr));
}

template<typename T> inline Array<T> duplicate_array(Alloc allocator, Array<T> arr, u64 flags = 0) {
	auto other = alloc_array<T>(allocator, arr.size());
	memcpy(other.data(), arr.data(), arr.size_bytes());
	return other;
}

extern Alloc std_allocator;

#ifdef BLBLSTD_IMPL

Buffer std_alloc_strategy(any*, Buffer buffer, usize size, u64) {
	auto ptr = realloc(buffer.data(), size);
	return Buffer((byte*)ptr, ptr != null ? size : 0);
}
Alloc std_allocator = { null, &std_alloc_strategy };
#endif

#endif
