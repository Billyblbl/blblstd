#ifndef GSCRATCH
# define GSCRATCH

#include <arena.cpp>

tuple<Arena&, u64> scratch_push_scope(u64 size, Array<const Arena* const> collisions = {});
tuple<Arena&, u64> scratch_push_scope(u64 size, LiteralArray<const Arena*> collision);
tuple<Arena&, u64> scratch_push_scope(u64 size, const Arena* const collision);
Arena& scratch_pop_scope(Arena& arena, u64 scope);

#ifdef BLBLSTD_IMPL

static u64 round_up_bit(u64 value) {
	value--;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value |= value >> 32;
	value++;
	return value;
}

tuple<Arena&, u64> scratch_push_scope(u64 size, Array<const Arena* const> collisions) {
	constexpr u8 MAX_SCRATCHES = 128;
	static thread_local Arena buffer[MAX_SCRATCHES];
	static thread_local List<Arena> scratches = { larray(buffer), 0 };

	auto collide = [&](const Arena& s) { return linear_search(collisions, [&](const Arena* const c) { return c == &s; }) >= 0; };
	auto i = linear_search(scratches.used(), [&](Arena& s) { return (s.current == 0 || s.free().size() >= size) && !collide(s); });
	Arena* scratch = null;
	size = round_up_bit(size);

	if (i < 0) {
		scratch = &scratches.push(Arena::from_vmem(size));
	} else {
		if (scratches[i].current == 0 && scratches[i].bytes.size() < size) {
			scratches[i].vmem_release();
			scratches[i] = Arena::from_vmem(size);
		}
		scratch = &scratches[i];
	}

	return { *scratch, scratch->current };
}

tuple<Arena&, u64> scratch_push_scope(u64 size, LiteralArray<const Arena*> collision) { return scratch_push_scope(size, larray(collision)); }
tuple<Arena&, u64> scratch_push_scope(u64 size, const Arena* const collision) { return scratch_push_scope(size, carray(&collision, 1)); }
Arena& scratch_pop_scope(Arena& arena, u64 scope) { return (arena.pop_to(scope), arena); }

#endif

#endif
