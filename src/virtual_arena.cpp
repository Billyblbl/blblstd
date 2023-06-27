#ifndef GVIRTUAL_ARENA
# define GVIRTUAL_ARENA

#include <virtual_memory.cpp>
#include <arena.cpp>

Arena create_virtual_arena(usize size);
void destroy_virtual_arena(Arena& arena);
Arena& reset_virtual_arena(Arena& arena);
Buffer virtual_arena_set_buffer(Arena& arena, Buffer buffer, usize size);
Alloc as_v_alloc(Arena& arena);
Alloc self_contained_virtual_arena_alloc(usize size);

#ifdef BLBLSTD_IMPL

Arena create_virtual_arena(usize size) { return { virtual_alloc(size), 0 }; }

void destroy_virtual_arena(Arena& arena) {
	arena.current = 0;
	auto to_dealloc = arena.capacity;
	arena.capacity = {};
	virtual_dealloc(to_dealloc);
}

Arena& reset_virtual_arena(Arena& arena) {
	virtual_decommit(arena.capacity);
	reset(arena);
	return arena;
}

Buffer virtual_arena_set_buffer(Arena& arena, Buffer buffer, usize size) {
	if (size == 0) {
		virtual_decommit(buffer);
		if (buffer.end() == arena.allocated().end())
			arena.pop_range(buffer.size());
		return {};
	}

	if (buffer.end() == arena.allocated().end() && arena.capacity.end() >= buffer.begin() + size) { // can change buffer in place

		if (buffer.size() < size) { // grow buffer
			auto appended = virtual_commit(arena.allocate(size - buffer.size()));
			return Buffer(buffer.begin(), appended.end());
		} else if (buffer.size() > size) { // shrink buffer
			virtual_decommit(arena.pop_range(buffer.size() - size));
			return Buffer(buffer.begin(), size);
		} else return buffer;

	} else { // have to use new buffer

		auto new_buffer = virtual_commit(arena.allocate(size));
		if (buffer.data() != null) { // move if there's an old buffer
			memcpy(new_buffer.data(), buffer.data(), min(buffer.size(), size));
			virtual_decommit(buffer);
		}
		return new_buffer;

	}

}

Buffer virtual_arena_strategy(any* ctx, Buffer buffer, usize size, u64) { return virtual_arena_set_buffer(*(Arena*)ctx, buffer, size); }
Alloc as_v_alloc(Arena& arena) { return { &arena, &virtual_arena_strategy }; }

Alloc self_contained_virtual_arena_alloc(usize size) {
	auto arena = create_virtual_arena(size);
	auto allocator = as_v_alloc(arena);
	auto& dest = alloc_array<Arena>(allocator, 1)[0];
	dest = arena;
	allocator.context = &dest;
	return allocator;
}

#endif

#endif
