#include <utils.cpp>
#include <list.cpp>
#include <memory.cpp>
#include <link_list.cpp>
#include <arena.cpp>
#include <virtual_memory.cpp>
#include <virtual_arena.cpp>

int main(int ac, const cstrp argv[]) {
	auto list = List { std_allocator.alloc(1028), 0 };
	defer { std_allocator.dealloc(list.capacity); };
	auto allocated = alloc_array<u64>(as_stack(list), 5);

	u64 buffer[99];
	auto buffer_backed = as_arena(literal(buffer));
	auto from_pool = alloc_array<u64>(as_stack(buffer_backed), 20);

	return allocated.size();
}
