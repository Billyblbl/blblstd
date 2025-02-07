#ifndef G_VIRTUAL_MEMORY
# define G_VIRTUAL_MEMORY

#include <memory.cpp>

Buffer virtual_reserve(usize size, bool commit = false);
Buffer virtual_commit(Buffer buffer);
Buffer virtual_remake(Buffer buffer, u64 size, u64 content, u64 commit);
//! decommit whole pages, not just the buffer
void virtual_decommit(Buffer buffer);
void virtual_release(Buffer buffer);

#ifdef BLBLSTD_IMPL

Buffer virtual_remake(Buffer buffer, u64 size, u64 content, u64 commit) {
	if (content > commit)
		commit = content;
	auto new_buffer = virtual_reserve(size, commit == size);
	if (commit > 0)
		virtual_commit(new_buffer.subspan(0, min(commit, new_buffer.size())));
	memcpy(new_buffer.data(), buffer.data(), min(content, new_buffer.size()));
	return new_buffer;
}

#if defined(PLATFORM_WINDOWS)
#include <windows.h>

void log_error(DWORD error_code, string source = "") {
	char message_buffer[1024];
	DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
	DWORD LID = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

	if (FormatMessageA(flags, NULL, error_code, LID, message_buffer, 1024, NULL)) {
		fprintf(stderr, "%.*s : %s", i32(source.size()), source.data(), message_buffer);
	} else {
		fprintf(stderr, "%.*s : Failed to retrieve error message %lx", i32(source.size()), source.data(), error_code);
	}
}

Buffer virtual_reserve(usize size, bool commit) {
	auto ptr = VirtualAlloc(null, size, MEM_RESERVE | (commit ? MEM_COMMIT : 0), PAGE_READWRITE);
	if (!ptr) {
		log_error(GetLastError(), __PRETTY_FUNCTION__);
		panic();
		return Buffer{};
	}
	return Buffer((byte*)ptr, size);
}

Buffer virtual_commit(Buffer buffer) {
	if (buffer.size() == 0) return buffer;
	auto ptr = VirtualAlloc(buffer.data(), buffer.size(), MEM_COMMIT, PAGE_READWRITE);
	if (!ptr) {
		log_error(GetLastError(), __PRETTY_FUNCTION__);
		panic();
		return Buffer{};
	}
	return buffer;
}

void virtual_decommit(Buffer buffer) {
	auto success = VirtualFree(buffer.data(), buffer.size(), MEM_DECOMMIT);
	if (!success) {
		log_error(GetLastError(), __PRETTY_FUNCTION__);
		panic();
	}
}

void virtual_release(Buffer buffer) {
	auto success = VirtualFree(buffer.data(), 0, MEM_RELEASE);
	if (!success) {
		log_error(GetLastError(), __PRETTY_FUNCTION__);
		panic();
	}
}

#elif defined(PLATFORM_LINUX) || defined(PLATFORM_OSX) || defined(PLATFORM_ANDROID)

Buffer virtual_reserve(usize size, bool commit) {
	auto ptr = mmap(null, size, commit ? PROT_READ | PROT_WRITE : PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
	if (ptr == MAP_FAILED) {
		//TODO logs from errno
		return Buffer{};
	}
	return Buffer((byte*)ptr, size);
}

Buffer virtual_commit(Buffer buffer) {
	if (buffer.size() == 0) return buffer;
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

void virtual_release(Buffer buffer) {
	auto failure = munmap(buffer.data(), buffer.size());
	if (failure) {
		//TODO logs from errno
	}
}

#endif

#endif

#endif
