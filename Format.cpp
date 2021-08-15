#ifndef GFORMAT
# define GFORMAT

#include <String.cpp>
#include <Memory.cpp>
#include <Scope.cpp>

template<typename T>
StringView	ToStringView(const T& data, Memory::Allocator& allocator = Memory::StandardCAllocator);

template<>
StringView	ToStringView<StringView>(const StringView& data, Memory::Allocator&) {
	return data;
}

template<>
StringView	ToStringView<String>(const String& data, Memory::Allocator&) {
	return viewOf(data);
}

template<>
StringView	ToStringView<rcstring>(const rcstring& data, Memory::Allocator&) {
	return StringViewFrom(data, strlen(data));
}

template<>
StringView	ToStringView<cstring>(const cstring& data, Memory::Allocator&) {
	return StringViewFrom(data, strlen(data));
}

template<>
StringView	ToStringView<u8>(const u8& data, Memory::Allocator& allocator) {
	auto maxSize = 3; //256
	auto buff = allocator.alloc(maxSize);
	buff.count = snprintf((cstring)buff.data, buff.count, "%u", data);
	return Generics::arrayCast<const u8>(buff);
}

template<>
StringView	ToStringView<u16>(const u16& data, Memory::Allocator& allocator) {
	auto maxSize = 5; //65 535
	auto buff = allocator.alloc(maxSize);
	buff.count = snprintf((cstring)buff.data, buff.count, "%u", data);
	return Generics::arrayCast<const u8>(buff);
}

template<>
StringView	ToStringView<u32>(const u32& data, Memory::Allocator& allocator) {
	auto maxSize = 10; //4 294 967 295
	auto buff = allocator.alloc(maxSize);
	buff.count = snprintf((cstring)buff.data, buff.count, "%u", data);
	return Generics::arrayCast<const u8>(buff);
}

template<>
StringView	ToStringView<u64>(const u64& data, Memory::Allocator& allocator) {
	auto maxSize = 20; //18 446 744 073 709 551 615
	auto buff = allocator.alloc(maxSize);
	buff.count = snprintf((cstring)buff.data, buff.count, "%llu", data);
	return Generics::arrayCast<const u8>(buff);
}

template<>
StringView	ToStringView<s8>(const s8& data, Memory::Allocator& allocator) {
	auto maxSize = 3; //128
	auto buff = allocator.alloc(maxSize);
	buff.count = snprintf((cstring)buff.data, buff.count, "%d", data);
	return Generics::arrayCast<const u8>(buff);
}

template<>
StringView	ToStringView<s16>(const s16& data, Memory::Allocator& allocator) {
	auto maxSize = 5; //32 767
	auto buff = allocator.alloc(maxSize);
	buff.count = snprintf((cstring)buff.data, buff.count, "%d", data);
	return Generics::arrayCast<const u8>(buff);
}

template<>
StringView	ToStringView<s32>(const s32& data, Memory::Allocator& allocator) {
	auto maxSize = 10; //2 147 483 647
	auto buff = allocator.alloc(maxSize);
	buff.count = snprintf((cstring)buff.data, buff.count, "%d", data);
	return Generics::arrayCast<const u8>(buff);
}

template<>
StringView	ToStringView<s64>(const s64& data, Memory::Allocator& allocator) {
	auto maxSize = 19; //9 223 372 036 854 775 807
	auto buff = allocator.alloc(maxSize);
	buff.count = snprintf((cstring)buff.data, buff.count, "%lld", data);
	return Generics::arrayCast<const u8>(buff);
}

template<typename... Types>
String	Format(StringView format, Memory::Allocator& allocator, Types&&... elements) {
	StringView	parts[sizeof...(elements)];
	USize		partsTotalLength = 0;
	auto scope = Memory::Scope(allocator);

	u64 i = 0;
	(..., [&](){
		parts[i] = ToStringView(elements, scope);
		partsTotalLength += parts[i++].count;
	}());

	u64	lastWriteFromFormatString = 0;
	u64 partWrites = 0;
	LinearStringBuilder	builder = { Memory::AllocArray<u8>(allocator, format.count - sizeof...(elements) + partsTotalLength), 0 };
	for (u64 i = 0; i < format.count; i++) if (format[i] == '%') {
		builder.addString(format.subArray(lastWriteFromFormatString, i));
		lastWriteFromFormatString = i+1;
		builder.addString(parts[partWrites++]);
	}
	builder.addString(format.subArray(lastWriteFromFormatString, format.count));
	return builder;
}

template<USize size, typename... Types>
inline String	Format(const char (&literalFormat)[size], Memory::Allocator& allocator, Types&&... elements) {
	return Format(StringViewFrom(literalFormat), allocator, std::forward<Types>(elements)...);
}

#endif