#ifndef G_UTILS
# define G_UTILS

#include <functional>
#include <stdint.h>
#include <cstddef>
#include <span>
#include <string_view>
#include <assert.h>

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
using cstrp = char*;

using utf8 = std::u8string_view;
using utf16 = std::u16string_view;
using utf32 = std::u32string_view;

using any = void;

template<typename T> using Array = std::span<T>;
template<typename T, typename U> inline auto cast(Array<U> arr) {
	return Array<T> ( (T*)arr.data(), (arr.size() * sizeof(U)) / sizeof(T) );
}
template<typename T, usize S> inline auto literal(T (&arr)[S]) { return Array<T>(arr, S); }

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

template<class I> struct range {
	I b, e;
	constexpr I begin() const noexcept { return b; }
	constexpr I end() const noexcept { return e; }
};

template<typename Callable> struct DeferedCall {
	Callable call;
	DeferedCall(Callable&& _call): call(std::move(_call)) {}
	~DeferedCall() { call(); }
};

#define deferVarName(line) DeferedCall defered_call_##line = [&]()
#define deferVarNameHelper(line) deferVarName(line)
#define defer deferVarNameHelper(__LINE__)

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
 #define PLATFORM_WINDOWS
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

#endif