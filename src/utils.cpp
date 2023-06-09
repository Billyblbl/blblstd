#ifndef G_UTILS
# define G_UTILS

#include <stdint.h>
#include <cstddef>
#include <span>
#include <string_view>
#include <assert.h>
#include <tuple>
#include <stdio.h>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

//might not be guaranted language wise
using f32 = float;
using f64 = double;

using usize = size_t;
using isize = ptrdiff_t;

//byte as defined by the platform, unsigned char is always supposed to be
//smallest size possible on a platform AFAIK
using byte = unsigned char;

using cstr = char[];
using mutcstrp = char*;
using cstrp = const char*;

using string = std::string_view;
using wstring = std::wstring_view;

using utf8 = std::u8string_view;
using utf16 = std::u16string_view;
using utf32 = std::u32string_view;

using any = void;

template<typename... T> using tuple = std::tuple<T...>;

template<typename T> using Array = std::span<T>;
template<typename T> using LiteralArray = std::initializer_list<T>;
template<typename T, typename U> inline auto cast(Array<U> arr) {
	return Array<T>((T*)arr.data(), (arr.size() * sizeof(U)) / sizeof(T));
}

template<typename T, u64... indices> auto to_tuple_helper(std::integer_sequence<u64, indices...> int_seq, Array<T> arr) {
	return tuple(arr[indices]...);
}

template<usize S, typename T> auto to_tuple(Array<T> arr) {
	assert(arr.size() >= S);
	return to_tuple_helper(std::make_integer_sequence<u64, S>{}, arr);
}

template<typename T> i64 linear_search(Array<T> arr, const T& obj, i64 start = 0) {
	return linear_search(arr, [&](const T& it) {return it == obj;}, start);
}

template<typename T, usize S> inline consteval usize array_size(const T(&)[S]) { return S; }
template<typename T, usize S> inline auto larray(T(&arr)[S]) { return Array<T>(arr, S); }
template<typename T> inline auto carray(T* arr, usize s) { return Array<T>(arr, s); }
template<typename T> inline auto larray(const LiteralArray<T>& arr) { return carray(arr.begin(), arr.size()); }
template<usize S> inline auto lstr(const char(&arr)[S]) { return string(&arr[0], S); }
template<usize S> inline auto lstr(const wchar_t(&arr)[S]) { return wstring(&arr[0], S); }
template<usize S> inline auto lutf(const char8_t(&arr)[S]) { return utf8(arr, S); }
template<usize S> inline auto lutf(const char16_t(&arr)[S]) { return utf16(arr, S); }
template<usize S> inline auto lutf(const char32_t(&arr)[S]) { return utf32(arr, S); }
template<usize S> inline auto lutf(const char(&arr)[S]) { return utf8((char8_t*)&arr[0], S); }
template<usize S> inline auto lutf(const u8(&arr)[S]) { return utf8((char8_t*)&arr[0], S); }
template<usize S> inline auto lutf(const u16(&arr)[S]) { return utf16((char16_t*)&arr[0], S); }
template<usize S> inline auto lutf(const u32(&arr)[S]) { return utf32((char32_t*)&arr[0], S); }

template<typename T> inline T bit(auto index) { return (T)1 << index; }
template<typename T> inline T mask(auto... index) { return (bit<T>(index) | ...); }
inline bool has_all(auto flags, auto mask) { return (flags & mask) == mask; }
inline bool has_one(auto flags, auto mask) { return flags & mask; }

constexpr auto null = nullptr;

inline auto min(auto a, auto b) { return a < b ? a : b; }
inline auto max(auto a, auto b) { return a > b ? a : b; }
inline auto min(auto... v, auto a) { return min(min(v...), a); }
inline auto max(auto... v, auto a) { return max(max(v...), a); }

// System guard
static_assert(sizeof(u8) == 1);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(u64) == 8);
static_assert(sizeof(i8) == 1);
static_assert(sizeof(i16) == 2);
static_assert(sizeof(i32) == 4);
static_assert(sizeof(i64) == 8);
static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);

template<typename I> struct it_range {
	I b, e;
	constexpr I begin() const noexcept { return b; }
	constexpr I end() const noexcept { return e; }
};

template<typename I> struct idx_iterator {
	I value;
	auto& operator++() { value++; return *this; }
	auto operator*() { return value; };
	idx_iterator(I _value) noexcept : value(_value) {}
	bool operator!=(idx_iterator<I> rhs) { return value != rhs.value; }
};

template<typename I> using idx_range = it_range<idx_iterator<I>>;

using u64xrange = idx_range<u64>;
using u32xrange = idx_range<u32>;
using u16xrange = idx_range<u16>;
using u8xrange = idx_range<u8>;

using i64xrange = idx_range<i64>;
using i32xrange = idx_range<i32>;
using i16xrange = idx_range<i16>;
using i8xrange = idx_range<i8>;

template<typename N> struct num_range { N min, max; N size() { return max - min; } };

template<typename N> idx_range<N> iter_inc(num_range<N> range) { return { range.min, range.max + 1 }; }
template<typename N> idx_range<N> iter_ex(num_range<N> range) { return { range.min, range.max }; }

using u64range = num_range<u64>;
using u32range = num_range<u32>;
using u16range = num_range<u16>;
using u8range = num_range<u8>;

using i64range = num_range<i64>;
using i32range = num_range<i32>;
using i16range = num_range<i16>;
using i8range = num_range<i8>;

using f64range = num_range<f64>;
using f32range = num_range<f32>;

template<typename T> num_range<usize> array_indices(Array<T> arr) { return { 0, arr.size() - 1 }; }

template<typename Ia, typename Ib> struct parallel_it {
	Ia it_a; Ib it_b;
	auto& operator++() { it_a++; it_b++; return *this; }
	auto operator*() { return tuple(&(*it_a), &(*it_b)); };
	bool operator!=(parallel_it<Ia, Ib> rhs) { return it_a != rhs.it_a || it_b != rhs.it_b; }
};

template<typename Ta, typename Tb> auto parallel_iter(Array<Ta> arra, Array<Tb> arrb) {
	return it_range{
		parallel_it {arra.begin(), arrb.begin()},
		parallel_it {arra.end(), arrb.end()},
	};
}

template<typename N> struct self_combinatronic_it {
	N i;
	N size;
	auto& operator++() { i++; return *this; }
	auto operator*() {
		auto i1 = i / size;
		auto i2 = (i + 1 + i1) % size;
		assert(i1 < size);
		assert(i2 < size);
		return tuple(i1, i2);
	};
	bool operator!=(self_combinatronic_it<N> rhs) { return i != rhs.i || size != rhs.size; }
};

template<typename N> it_range<self_combinatronic_it<N>> self_combinatronic_idx(N count) {
	return {
		self_combinatronic_it<N> { 0, count },
		self_combinatronic_it<N> { count* (count - 1) / 2, count }
	};
}

template<typename T, typename P> i64 linear_search(Array<T> arr, P predicate, i64 start = 0) {
	for (auto i : iter_inc(array_indices(arr))) {
		auto index = (i + start) % arr.size();
		if (predicate(arr[index]))
			return index;
	}
	return -1;
}

template<typename Callable> struct DeferedCall {
	Callable call;
	DeferedCall(Callable&& _call) : call(std::move(_call)) {}
	~DeferedCall() { call(); }
};

#define deferVarName(line) DeferedCall defered_call_##line = [&]()
#define deferVarNameHelper(line) deferVarName(line)
#define defer deferVarNameHelper(__LINE__)

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define PLATFORM_WINDOWS
// avoids having too many #define collisions
#define WIN32_LEAN_AND_MEAN
#endif
#if defined(unix) || defined(__unix) || defined(__unix__)
#define PLATFORM_UNIX
#endif
#if defined(__APPLE__) || defined(__MACH__)
#define PLATFORM_OSX
#endif
#if defined(__linux__) || defined(linux) || defined(__linux)
#define PLATFORM_LINUX
#endif
#if defined(FreeBSD) || defined(__FreeBSD__)
#define PLATFORM_BSD
#endif
#if defined(__ANDROID__)
#define PLATFORM_ANDROID
#endif

#define fail_msg(msg) fprintf(stderr, "%s:%u %s failed : %s\n", __FILE__, __LINE__, __FUNCTION__, msg)
#define fail_ret(m, x) (fail_msg(m), x)
#define expect(x) (x ? x : fail_ret(#x, x))

#endif
