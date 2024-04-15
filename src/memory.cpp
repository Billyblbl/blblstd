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
	return cast<T>(allocator.alloc(sizeof(T) * count, flags));
}

template<typename T> inline Array<T> realloc_array(Alloc allocator, Array<T> arr, usize count, u64 flags = 0) {
	return cast<T>(allocator.realloc(cast<byte>(arr), sizeof(T) * count, flags));
}

template<typename T> inline void dealloc_array(Alloc allocator, Array<T> arr, u64 flags = 0) {
	return allocator.dealloc(cast<byte>(arr), flags);
}

template<typename T> inline Array<T> duplicate_array(Alloc allocator, Array<T> arr, u64 flags = 0) {
	auto other = alloc_array<T>(allocator, arr.size(), flags);
	memcpy(other.data(), arr.data(), arr.size_bytes());
	return other;
}

template<typename T> inline Array<T> duplicate_array(Alloc allocator, Array<const T> arr, u64 flags = 0) {
	auto other = alloc_array<T>(allocator, arr.size(), flags);
	memcpy(other.data(), arr.data(), arr.size_bytes());
	return other;
}

template<typename T> inline Array<T> push_array(Alloc allocator, LiteralArray<T> arr, u64 flags = 0) { return duplicate_array(allocator, larray(arr), flags); }

inline string push_string(Alloc allocator, string str, u64 flags = 0) {
	auto other = alloc_array<char>(allocator, str.size() + 1, flags);
	memcpy(other.data(), str.data(), str.size());
	other[str.size()] = 0;
	return other.data();
}

template<typename T> inline T* alloc(Alloc allocator, u64 flags = 0) {
	return &alloc_array<T>(allocator, 1, flags)[0];
}

template<typename T> inline void dealloc(Alloc allocator, T& t, u64 flags = 0) {
	dealloc_array(allocator, carray(&t, 1), flags);
}

template<typename T> inline T* realloc(Alloc allocator, T& t, u64 flags = 0) {
	return &realloc_array(allocator, carray(&t, 1), flags)[0];
}

template<typename T> inline T* duplicate(Alloc allocator, T& t, u64 flags = 0) {
	return &duplicate_array(allocator, carray(&t, 1), flags)[0];
}

extern Alloc std_allocator;

u64 round_up_bit(u64 value);

#ifdef BLBLSTD_IMPL

Buffer std_alloc_strategy(any*, Buffer buffer, usize size, u64) {
	auto ptr = realloc(buffer.data(), size);
	return Buffer((byte*)ptr, ptr != null ? size : 0);
}
Alloc std_allocator = { null, &std_alloc_strategy };

u64 round_up_bit(u64 value) {
	value--;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value |= value >> 32;
	value++;
	return value;
}

#endif

#endif
