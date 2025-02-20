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
		ALLOW_MOVE_MORPH = 1ull << 4,
		ALLOW_CHAIN_GROWTH = 1ull << 5,
		ALLOW_VMEM_REPLACE_GROWTH = 1ull << 6,//! Pointer unstable
		ALLOW_SCOPE_UNSTABLE = 1ull << 7,
		FORCE_NONE = 1ull << 63,
	};

	inline bool is_stable() { return !(flags & (ALLOW_VMEM_REPLACE_GROWTH | ALLOW_SCOPE_UNSTABLE | ALLOW_FAILURE)); }

	static inline Arena from_buffer(Buffer buffer, u64 flags = 0) {
		assert(flags & (FULL_COMMIT | COMMIT_ON_PUSH));//* if none of these flags is present, we never have committed memory
		Arena new_arena = {
			.bytes = buffer,
			.current = 0,
			.commit = (flags & FULL_COMMIT) ? buffer.size() : 0,
			.next = null,
			.flags = flags
		};
		if (flags & ALLOW_CHAIN_GROWTH)//* preallocates the next arena' slot, avoids all the fuckery when doing growth by self containing the next arenas
			new_arena.next = &new_arena.push(Arena{});
		return new_arena;
	}

	template<typename T> static inline Arena from_array(Array<T> arr, u64 flags = 0) { return from_buffer(cast<byte>(arr), flags | FULL_COMMIT); }
	template<typename T, usize S> static inline Arena from_array(const T(&arr)[S], u64 flags = 0) { return from_array(larray(arr), flags | FULL_COMMIT); }

	static constexpr u64 DEFAULT_VMEM_FLAGS = COMMIT_ON_PUSH | DECOMMIT_ON_EMPTY | ALLOW_CHAIN_GROWTH | ALLOW_MOVE_MORPH;

	static inline Arena from_vmem(u64 size, u64 flags = DEFAULT_VMEM_FLAGS) {
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
		if (bytes.size() > 0) {
			virtual_release(bytes);
			*this = {};
		}
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

	inline Arena& push_sub_arena(u64 size) {
		assert(flags & ALLOW_CHAIN_GROWTH);
		assert(next);
		*next = from_vmem(size, flags);
		return *next;
	}

	u64 scope(bool local_only = false) const {
		if (local_only) return current;
		u64 scope = current;
		if (next && next->bytes.size() > 0) scope += next->scope();
		return scope;
	}

	u64 tip(bool local_only = false) const {
		if (local_only) return bytes.size();
		u64 tip = current;
		if (next && next->bytes.size() > 0) tip += next->tip();
		else tip = bytes.size();
		return tip;
	}

	Buffer free_tip() const {
		const Arena* it;
		for (it = this; it->next && it->next->bytes.size() > 0; it = it->next);
		return it->free();
	}

	static constexpr u64 PAGE_SIZE_HEURISTIC = 4096;
	static constexpr u64 COMMIT_CHUNK_SIZE = PAGE_SIZE_HEURISTIC * 4;

	inline u64 align_padding(u64 align) { return -uintptr_t(free().data()) & (align - 1); }//* based on https://nullprogram.com/blog/2023/09/27/

	inline Buffer push_local(u64 size, u64 padding, bool zero_mem = false) {
		auto extent = size + padding;
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

	inline Buffer push_bytes(u64 size, u64 align, bool zero_mem = false) {
		u64 padding = align_padding(align);
		u64 extent = padding + size;

		//* growth strategies
		if ((flags & ALLOW_VMEM_REPLACE_GROWTH) && extent > free().size())
			bytes = virtual_remake(bytes, round_up_bit(bytes.size() + extent), current, flags & FULL_COMMIT ? round_up_bit(bytes.size() + extent) : commit);
		else if (
			(flags & ALLOW_CHAIN_GROWTH) &&
			(
				extent > free().size() || //* local doesn't have enough space or
				((next && next->current > 0) && !(flags & ALLOW_SCOPE_UNSTABLE))//* next has data which forbids local to change scope() result
				)
			) {
			if (next->bytes.size() == 0)
				push_sub_arena(2 * (sizeof(Arena) + max(extent, u64(bytes.size()))));
			return next->push_bytes(size, align, zero_mem);
		}

		return push_local(size, padding, zero_mem);
	}

	inline u64 pop_local(u64 size, u64 flags_override = 0) {
		auto popped = min(current, size);
		if (popped == 0)
			return popped;
		current -= popped;
		auto used_flags = flags_override ? flags_override : flags;
		poison(free().subspan(0, popped));
		if (current == 0 && (used_flags & DECOMMIT_ON_EMPTY) && !(used_flags & FULL_COMMIT)) {
			commit = 0;
			virtual_decommit(bytes);
		}
		return popped;
	}

	inline u64 pop_to(u64 scope) {
		if (scope > current) {
			assert(next);
			return next->pop_to(scope - current);
		} else {
			auto popped = 0;
			if (next && next->bytes.size() > 0) {
				popped += next->scope();
				next->vmem_release();
			}
			return popped + pop_local(current - scope);
		}
	}

	inline Arena& reset() {
		pop_to((flags & ALLOW_CHAIN_GROWTH) ? sizeof(Arena) : 0);
		return *this;
	}

	inline Buffer morph(Buffer buffer, u64 size, u64 align) {
		if (buffer.size() == size) return buffer;//* untouched

		bool growing = size > buffer.size();
		bool shrinking = size < buffer.size();
		u64 diff = growing ? size - buffer.size() : buffer.size() - size;

		auto local_tip = buffer.end() == used().end();
		auto chain_tip = !next || next->current == 0;
		auto enough_space = (growing && (diff <= free().size())) || shrinking;

		if (local_tip && (chain_tip || (flags & ALLOW_SCOPE_UNSTABLE)) && enough_space) {//* tip morph
			if (growing) { //*grow tip
				return Buffer(buffer.begin(), push_local(diff, 0).end());
			} else { //* shrink tip
				pop_local(diff);
				return buffer.subspan(0, size);
			}
		} else if (shrinking) {//* shrink
			return buffer.subspan(0, size);
		} else if (flags & ALLOW_MOVE_MORPH) {//* move morph
			auto new_buffer = push_bytes(size, align);
			memcpy(new_buffer.data(), buffer.data(), min(buffer.size(), new_buffer.size()));
			return new_buffer;
		} else {//* failure
			assert((fprintf(stderr, "Failed memory morph : initial=%llu, available=%llu, requested=%llu\n", buffer.size(), free().size(), size), flags & ALLOW_FAILURE));
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

	template<typename... Args> string format(const cstr fmt, Args&&... args) {
		auto size = snprintf(null, 0, fmt, args...);
		auto str = push_array<char>(size + 1);
		snprintf(str.data(), size + 1, fmt, args...);
		return string(str.data(), size);
	}

};

#endif
