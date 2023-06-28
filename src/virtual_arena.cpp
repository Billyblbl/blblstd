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
// #if 1

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
	//? removed the decommits as it seems to decommit the whole memory page even when just giving a small range inside it
	//? even decommiting multiple pages when if the range crosses them
	//TODO research a bit more of how virtual allocation works on these points, both for windows & linux
	return virtual_commit(arena_set_buffer(arena, buffer, size));
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
