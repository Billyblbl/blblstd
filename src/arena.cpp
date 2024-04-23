#ifndef GARENA
# define GARENA

#include <virtual_memory.cpp>
#include <sanitizer/asan_interface.h>

struct Arena {
	Buffer bytes = {};
	u64 current = 0;
	u64 commit = 0;
	Arena* next = null;
	u64 flags = 0;

	enum : u64 {
		COMMIT_ON_PUSH = 1ull << 0,
		DECOMMIT_ON_EMPTY = 1ull << 1,
		FULL_COMMIT = 1ull << 2,
		ALLOW_FAILURE = 1ull << 3,
		ALLOW_MOVE_MORPH = 1ull << 4,//! Pointer unstable (for individual allocations components, since they can be moved)
		ALLOW_CHAIN_GROWTH = 1ull << 5,//! Scope unstable
		ALLOW_VMEM_GROWTH = 1ull << 6,//! Pointer unstable
		FORCE_NONE = 1ull << 63,
	};

	inline bool is_stable() { return !(flags & (ALLOW_VMEM_GROWTH | ALLOW_MOVE_MORPH | ALLOW_CHAIN_GROWTH)); }

	static inline Arena from_buffer(Buffer buffer, u64 flags = 0) {
		Arena new_arena;
		new_arena.bytes = buffer;
		new_arena.flags = flags;
		new_arena.commit = (flags & FULL_COMMIT) ? buffer.size() : 0;
		new_arena.current = 0;
		new_arena.next = null;
		assert(flags & (FULL_COMMIT | COMMIT_ON_PUSH));//* if none of these flags is present, we're never committing memory
		return new_arena;
	}

	template<typename T> static inline Arena from_array(Array<T> arr, u64 flags = 0) { return from_buffer(cast<byte>(arr), flags | FULL_COMMIT); }
	template<typename T, usize S> static inline Arena from_array(const T(&arr)[S], u64 flags = 0) { return from_array(larray(arr), flags | FULL_COMMIT); }

	static inline Arena from_vmem(u64 size, u64 flags = COMMIT_ON_PUSH | DECOMMIT_ON_EMPTY) {
		auto buffer = virtual_reserve(size, flags & FULL_COMMIT);
		if ((flags & FULL_COMMIT))
			poison(buffer);
		return from_buffer(buffer, flags);
	}

	Arena& commit_all() {
		virtual_commit(bytes);
		commit = bytes.size();
		flags |= FULL_COMMIT;
		return *this;
	}

	static Buffer zero_buff(Buffer buff) {
		memset(buff.data(), 0, buff.size_bytes());
		return buff;
	}

	void vmem_release() {
		if (next) next->vmem_release();
		virtual_release(bytes);
		*this = {};
	}

	inline Arena& vmem_resize(u64 size) {
		bytes = virtual_remake(bytes, size, current, flags & FULL_COMMIT ? size : commit);
		return *this;
	}

	inline Buffer used() const { return bytes.subspan(0, current); }
	inline Buffer free() const { return bytes.subspan(current); }
	inline Buffer commited() const { return bytes.subspan(0, commit); }

	static inline Buffer poison(Buffer buffer) {
		ASAN_POISON_MEMORY_REGION(buffer.data(), buffer.size_bytes());
		return buffer;
	}

	static inline Buffer unpoison(Buffer buffer) {
		ASAN_UNPOISON_MEMORY_REGION(buffer.data(), buffer.size_bytes());
		return buffer;
	}

	//? this model of sub arena might cause issues with scoped pops, since we're morphing into the sub arena in place and keeping a backup,
	//? but otherwise can't use this in push_bytes & pop since otherwise we would have to somehow return the new arena struct's memory
	inline Arena& push_sub_arena(u64 size) {
		auto new_arena = from_vmem(size, flags);
		auto& backup = new_arena.push(*this);
		*this = new_arena;
		next = &backup;
		return *this;
	}

	inline Arena& pop_sub_arena() {
		assert(next);
		Arena backup = *next;
		vmem_release();
		return *this = backup;
	}

	static constexpr u64 PAGE_SIZE_HEURISTIC = 4096;
	static constexpr u64 COMMIT_CHUNK_SIZE = PAGE_SIZE_HEURISTIC * 4;

	inline Buffer push_bytes(u64 size, u64 align, bool zero_mem = false) {
		u64 padding = -uintptr_t(free().data()) & (align - 1); //* based on https://nullprogram.com/blog/2023/09/27/
		u64 extent = padding + size;

		//* growth strategies
		if ((flags & ALLOW_VMEM_GROWTH) && extent > free().size())
			bytes = virtual_remake(bytes, round_up_bit(bytes.size() + extent), current, flags & FULL_COMMIT ? round_up_bit(bytes.size() + extent) : commit);
		else if ((flags & ALLOW_CHAIN_GROWTH) && extent > free().size())
			push_sub_arena(2 * (sizeof(Arena) + max(extent, u64(bytes.size()))));

		if (extent > bytes.size() || current > bytes.size() - extent) {
			fprintf(stderr, "Failed allocation : available=%llu, requested=%llu, needed=%llu\n", free().size(), size, extent);
			assert(flags & ALLOW_FAILURE);
			return {};
		}
		u64 start = current + padding;
		current += extent;
		if (current > commit && (flags & COMMIT_ON_PUSH)) {
			u64 prev_commit = commit;
			commit = min(((current / COMMIT_CHUNK_SIZE) + 1) * COMMIT_CHUNK_SIZE, bytes.size());
			poison(virtual_commit(commited().subspan(prev_commit)));
		}
		assert(commit >= current);
		if (zero_mem)
			return zero_buff(unpoison(used().subspan(start)));
		else
			return unpoison(used().subspan(start));
	}

	inline Buffer pop(u64 size, u64 flags_override = 0) {
		current -= size;
		auto used_flags = flags_override ? flags_override : flags;
		//TODO handle chained arenas
		auto popped = free().subspan(0, size);
		if (current > 0 || !(used_flags & DECOMMIT_ON_EMPTY) || (used_flags & FULL_COMMIT))
			poison(popped);
		else {
			virtual_decommit(bytes);
			commit = 0;
		}
		return popped;
	}

	inline Buffer pop_to(u64 scope) { return pop(current - scope); }

	inline Arena& reset() {
		pop(current);
		if (next) next->vmem_release();
		return *this;
	}

	inline Buffer morph(Buffer buffer, u64 size, u64 align) {
		if (buffer.size() == size) return buffer;//* untouched
		if (buffer.end() == used().end()) {//* tip morph
			pop(buffer.size(), FORCE_NONE);
			auto new_buffer = push_bytes(size, align);
			return new_buffer.data() ? new_buffer : push_bytes(buffer.size(), align);
		} else if (size < buffer.size()) {//* shrink
			return buffer.subspan(0, size);
		} else if (flags & ALLOW_MOVE_MORPH) {//* move morph
			auto new_buffer = push_bytes(size, align);
			memcpy(new_buffer.data(), buffer.data(), min(buffer.size(), new_buffer.size()));
			return new_buffer;
		} else {//* failure
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
