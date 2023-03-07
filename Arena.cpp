#ifndef GARENA
# define GARENA

#include <memory.cpp>
#include <list.cpp>

using Arena = List<byte>;

template<typename T> auto as_arena(Array<T> buffer) { return Arena { cast<byte>(buffer), 0 }; }

Buffer stack_alloc(Arena& arena, Buffer buffer, usize size) {
	if (size == 0) {
		if (buffer.end() == arena.allocated().end())
			arena.pop_range(buffer.size());
		return {};
	}

	if (buffer.end() == arena.allocated().end() && arena.capacity.end() >= buffer.begin() + size) { // can change buffer in place

		if (buffer.size() < size) { // grow buffer
			arena.allocate(size - buffer.size());
			return Buffer(buffer.begin(), size);
		} else if (buffer.size() > size) { // shrink buffer
			arena.pop_range(buffer.size() - size);
			return Buffer(buffer.begin(), size);
		} else return buffer;

	} else { // have to use new buffer -> very bad if moving buffers with this allocation strategy
		auto new_buffer = arena.allocate(size);
		if (new_buffer.data() != null && buffer.data() != null) // move if there's an old buffer
			memcpy(new_buffer.data(), buffer.data(), min(buffer.size(), size));
		return new_buffer;
	}
}

void reset(Arena& arena) { arena.current = 0; }
Buffer stack_alloc_strategy(any* ctx, Buffer buffer, usize size) { return stack_alloc(*(Arena*)ctx, buffer, size); }
Alloc as_stack(Arena& arena) { return Alloc { &arena, &stack_alloc_strategy }; }

Arena& self_contain(Arena&& arena) {
	return cast<Arena>(arena.allocate(sizeof(Arena)))[0] = std::move(arena);
}

#endif
