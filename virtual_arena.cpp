#ifndef GVIRTUAL_ARENA
# define GVIRTUAL_ARENA

#include <virtual_memory.cpp>
#include <arena.cpp>

Arena virtual_arena_create(usize size) { return { virtual_alloc(size), 0 }; }

void virtual_arena_destroy(Arena& arena) {
	arena.count = 0;
	auto to_dealloc = arena.capacity;
	arena.capacity = {};
	virtual_dealloc(to_dealloc);
}

Arena& virtual_arena_reset(Arena& arena) {
	arena.count = 0;
	virtual_decommit(arena.capacity);
	return arena;
}

Buffer virtual_arena_alloc(Arena& arena, Buffer buffer, usize size) {
	if (size == 0) {
		virtual_decommit(buffer);
		return {};
	}

	if (buffer.end() == arena.allocated().end() && arena.capacity.end() >= buffer.begin() + size) { // can change buffer in place

		if (buffer.size() < size) { // grow buffer
			auto to_append = arena.allocate(size - buffer.size());
			auto appended = virtual_commit(to_append);
			return Buffer(buffer.begin(), appended.end());

		} else if (buffer.size() > size) { // shrink buffer
			auto to_pop = Buffer(arena.allocated().end(), buffer.size() - size);
			arena.count -= to_pop.size();
			virtual_decommit(to_pop);
			return Buffer(buffer.begin(), size);

		} else return buffer;

	} else { // have to use new buffer

		auto new_buffer = arena.allocate(size);
		new_buffer = virtual_commit(new_buffer);
		if (buffer.data() != null) { // move if there's an old buffer
			memcpy(new_buffer.data(), buffer.data(), min(buffer.size(), size));
			virtual_decommit(buffer);
		}
		return new_buffer;

	}

}

Buffer virtual_arena_strategy(any* ctx, Buffer buffer, usize size) { return virtual_arena_alloc(*(Arena*)ctx, buffer, size); }
Alloc as_v_alloc(Arena& arena) { return { &arena, &virtual_arena_strategy }; }

Alloc self_contained_virtual_arena_alloc(usize size) {
	auto arena = virtual_arena_create(size);
	auto allocator = as_v_alloc(arena);
	auto& dest = alloc_array<Arena>(allocator, 1)[0];
	dest = arena;
	allocator.context = &dest;
	return allocator;
}

#endif
