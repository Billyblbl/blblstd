#ifndef G_VIRTUAL_MEMORY
# define G_VIRTUAL_MEMORY

#include <memory.cpp>

Buffer virtual_alloc(usize size, bool commit = false);
Buffer virtual_commit(Buffer buffer);
void virtual_decommit(Buffer buffer);
void virtual_dealloc(Buffer buffer);

#ifdef BLBLSTD_IMPL


	#if defined(PLATFORM_WINDOWS)
#include <windows.h>

Buffer virtual_alloc(usize size, bool commit) {
	auto ptr = VirtualAlloc(null, size, MEM_RESERVE | (commit ? MEM_COMMIT : 0), PAGE_READWRITE);
	if (!ptr) {
		auto err = GetLastError();
		//TODO logs something with FormatMessage()
		return Buffer{};
	}
	return Buffer((byte*)ptr, size);
}

Buffer virtual_commit(Buffer buffer) {
	auto ptr = VirtualAlloc(buffer.data(), buffer.size(), MEM_COMMIT, PAGE_READWRITE);
	if (!ptr) {
		auto err = GetLastError();
		//TODO logs something with FormatMessage()
		return Buffer{};
	}
	return Buffer((byte*)ptr, buffer.size());
}

void virtual_decommit(Buffer buffer) {
	auto success = VirtualFree(buffer.data(), buffer.size(), MEM_DECOMMIT);
	if (!success) {
		auto err = GetLastError();
		//TODO logs something with FormatMessage()
	}
}

void virtual_dealloc(Buffer buffer) {
	auto success = VirtualFree(buffer.data(), 0, MEM_RELEASE);
	if (!success) {
		auto err = GetLastError();
		//TODO logs something with FormatMessage()
	}
}

	#elif defined(PLATFORM_LINUX) || defined(PLATFORM_OSX) || defined(PLATFORM_ANDROID)

Buffer virtual_alloc(usize size, bool commit) {
	auto ptr = mmap(null, size, commit ? PROT_READ | PROT_WRITE : PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
	if (ptr == MAP_FAILED) {
		//TODO logs from errno
		return Buffer{};
	}
	return Buffer((byte*)ptr, size);
}

Buffer virtual_commit(Buffer buffer) {
	auto failure = mprotect(buffer.data(), buffer.size(), PROT_READ | PROT_WRITE);
	if (failure) {
		//TODO logs from errno
		return Buffer{};
	}
	return buffer;
}

void virtual_decommit(Buffer buffer) {
	auto failure = mprotect(buffer.data(), buffer.size(), PROT_NONE);
	if (failure) {
		//TODO logs from errno
		return Buffer{};
	}
}

void virtual_dealloc(Buffer buffer) {
	auto failure = munmap(buffer.data(), buffer.size());
	if (failure) {
		//TODO logs from errno
	}
}

	#endif

#endif

#endif
