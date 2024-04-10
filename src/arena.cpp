#ifndef GARENA
# define GARENA

#include <virtual_memory.cpp>

struct Arena {
	Buffer bytes = {};
	Arena* next = null;
	u64 current = 0;
	u64 flags = 0;

	enum : u64 {
		COMMIT_ON_PUSH = 1 << 0,
		DECOMMIT_ON_EMPTY = 1 << 1,
		ALLOW_FAILURE = 1 << 2,
		ALLOW_MOVE_MORPH = 1 << 3,
		ALLOW_CHAIN_GROWTH = 1 << 4,
		FULL_COMMIT = 1 << 5,
		FORCE_NONE = u64(1) << 63,
	};

	static inline Arena from_buffer(Buffer buffer, u64 flags = 0) {
		Arena new_arena;
		new_arena.bytes = buffer;
		assert(!(flags & ALLOW_CHAIN_GROWTH));//? chain growth not allowed as implementation is incomplete
		new_arena.flags = flags;
		return new_arena;
	}

	template<typename T> static inline Arena from_array(Array<T> arr, u64 flags = 0) { return from_buffer(cast<byte>(arr), flags); }
	template<typename T, usize S> static inline Arena from_array(const T(&arr)[S], u64 flags = 0) { return from_array(larray(arr), flags); }

	static inline Arena from_vmem(u64 size, u64 flags = COMMIT_ON_PUSH | DECOMMIT_ON_EMPTY) { return from_buffer(virtual_reserve(size, flags & FULL_COMMIT), flags); }

	Arena& commit_all() {
		virtual_commit(bytes);
		flags |= FULL_COMMIT;
		return *this;
	}

	static Buffer zero_buff(Buffer buff) {
		memset(buff.data(), 0, buff.size_bytes());
		return buff;
	}

	void vmem_release() {
		if (flags & ALLOW_CHAIN_GROWTH) next->vmem_release();
		virtual_release(bytes);
		*this = {};
	}

	inline Buffer used() const { return bytes.subspan(0, current); }
	inline Buffer free() const { return bytes.subspan(current); }

	inline Buffer push_bytes(u64 size, u64 align, bool zero_mem = false) {
		auto padding = -uintptr_t(free().data()) & (align - 1); //* based on https://nullprogram.com/blog/2023/09/27/
		auto inprint = padding + size;
		if ((flags & ALLOW_CHAIN_GROWTH) && inprint > free().size())
			next = &Arena::from_vmem(bytes.size(), flags).self_contain();
		if (current <= bytes.size() - inprint) {
			auto start = current + padding;
			current += inprint;
			if ((flags & COMMIT_ON_PUSH) && !(flags & FULL_COMMIT))
				virtual_commit(used().subspan(start));
			if (zero_mem)
				return zero_buff(used().subspan(start));
			else
				return used().subspan(start);
		} else {
			assert((fprintf(stderr, "Failed allocation : available=%llu, requested=%llu, needed=%llu\n", free().size(), size, inprint), flags & ALLOW_FAILURE));
			return {};
		}
	}

	inline Buffer pop(u64 size, u64 flags_override = 0) {
		current -= size;
		auto used_flags = flags_override ? flags_override : flags;
		//TODO handle chained arenas
		if (current == 0 && (used_flags & DECOMMIT_ON_EMPTY) && !(used_flags & FULL_COMMIT))
			virtual_decommit(bytes);
		return free().subspan(0, size);
	}

	inline Buffer pop_to(u64 scope) { return pop(current - scope); }

	inline Arena& reset() {
		pop(current);//TODO handle chained arenas
		return *this;
	}

	inline Buffer morph(Buffer buffer, u64 size, u64 align) {
		if (buffer.size() == size) return buffer;
		if (buffer.end() == used().end()) {
			pop(buffer.size(), FORCE_NONE);
			auto new_buffer = push_bytes(size, align);
			return new_buffer.data() ? new_buffer : push_bytes(buffer.size(), align);
		} else if (size < buffer.size()) {
			return buffer.subspan(0, size);
		} else if (flags & ALLOW_MOVE_MORPH) {
			auto new_buffer = push_bytes(size, align);
			memcpy(new_buffer.data(), buffer.data(), min(buffer.size(), new_buffer.size()));
			return new_buffer;
		} else {
			assert((fprintf(stderr, "Failed morph : initial=%llu, available=%llu, requested=%llu\n", buffer.size(), free().size(), size), flags & ALLOW_FAILURE));
			return buffer;
		}
	}

	template<typename T> inline Array<T> push_array(usize count, bool zero_mem = false) { return cast<T>(push_bytes(count * sizeof(T), alignof(T), zero_mem)); }

	template<typename T> inline Array<T> push_array(Array<const T> arr) {
		auto other = push_array<T>(arr.size());
		memcpy(other.data(), arr.data(), arr.size_bytes());
		return other;
	}

	template<typename T> inline Array<T> push_array(Array<T> arr) {
		return push_array(cast<const T>(arr));
	}

	template<typename T> inline Array<T> push_array(LiteralArray<T> arr) { return push_array(larray(arr)); }

	inline string push_string(string str) {
		auto arr = push_array<char>(str.size() + 1);
		memcpy(arr.data(), str.data(), str.size());
		arr[str.size()] = 0;
		return arr.data();
	}

	template<typename T> inline T& push(bool zero_mem = false) { return cast<T>(push_bytes(sizeof(T), alignof(T), zero_mem))[0]; }
	template<typename T> inline T& push(const T& obj) { return cast<T>(push_bytes(sizeof(T), alignof(T)))[0] = obj; }

	template<typename T> inline Array<T> morph_array(Array<T> arr, u64 count) {
		return cast<T>(morph(cast<byte>(arr), count * sizeof(T), alignof(T)));
	}

	inline Arena& self_contain() { return push(*this); }

};

#endif
