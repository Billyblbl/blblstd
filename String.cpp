#ifndef G_STRING
# define G_STRING

#include <string.h>
#include <Array.cpp>

using StringView = Generics::Array<const u8>;

inline StringView	StringViewFrom(rcstring cstr, USize size) {
	using namespace Generics;
	return arrayCast<const u8>(arrayFrom(cstr, size));
}

template<size_t size>
inline StringView	StringViewFrom(const char (&literal)[size]) {
	return StringViewFrom(literal, size - 1);
}

using String = Generics::Array<u8>;

inline String	StringFrom(cstring cstr, USize size) {
	using namespace Generics;
	return arrayCast<u8>(arrayFrom(cstr, size));
}

template<size_t size>
inline String	StringFrom(char (&literal)[size]) {
	return StringFrom(literal, size - 1);
}

template<size_t size>
inline String	StringFrom(u8 (&literal)[size]) {
	return StringFrom(literal, size - 1);
}

inline auto viewOf(const String str) {
	using namespace Generics;
	return arrayCast<const u8>(str);
}

inline bool	operator==(StringView lhs, StringView rhs) {
	return (lhs.count == rhs.count) && (
		lhs.data == rhs.data ||
		strncmp((rcstring)lhs.data, (rcstring)rhs.data, lhs.count) == 0
	);
}
inline bool	operator==(const String lhs, const String rhs) { return viewOf(lhs) == viewOf(rhs); }
inline bool	operator==(const String lhs, StringView rhs) { return viewOf(lhs) == rhs; }
inline bool	operator==(StringView lhs, const String rhs) { return lhs == viewOf(rhs); }

template<USize size>
inline auto	writeToBuffer(StringView string, u8 (&buffer)[size]) {
	auto writeSize = (string.count < size - 1) ? string.count : size - 1;
	strncpy(buffer, string.data, writeSize);
	return String
}

template<USize size>
inline auto	writeToBuffer(StringView string, char (&buffer)[size]) {
	auto writeSize = (string.count < size - 1) ? string.count : size - 1;
	strncpy(buffer, string.data, writeSize);
	return StringFrom(buffer, writeSize);
}

inline auto	writeToBuffer(StringView string, String buffer) {
	auto writeSize = (string.count < buffer.count) ? string.count : buffer.count;
	strncpy((cstring)buffer.data, (rcstring)string.data, writeSize);
	return String{ buffer.data, writeSize } ;
}

#endif