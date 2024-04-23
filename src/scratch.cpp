#ifndef GSCRATCH
# define GSCRATCH

#include <arena.cpp>

tuple<Arena&, u64> scratch_push_scope(u64 size, Array<const Arena* const> collisions = {});
tuple<Arena&, u64> scratch_push_scope(u64 size, LiteralArray<const Arena*> collision);
tuple<Arena&, u64> scratch_push_scope(u64 size, const Arena* const collision);
Arena& scratch_pop_scope(Arena& arena, u64 scope);

#ifdef BLBLSTD_IMPL

static List<Arena> &get_scratches() {
	constexpr auto MAX_SCRATCHES = 16;
	static thread_local Arena buffer[MAX_SCRATCHES];
	static thread_local List<Arena> scratches = { larray(buffer), 0 };
	return scratches;
}

tuple<Arena&, u64> scratch_push_scope(u64 size, Array<const Arena* const> collisions) {
	auto& scratches = get_scratches();

	auto collide = [&](const Arena& s) { return linear_search(collisions, [&](const Arena* const c) { return c == &s; }) >= 0; };
	auto i = linear_search(scratches.used(), [&](Arena& s) { return (s.current == 0 || s.free().size() >= size) && !collide(s); });
	Arena* scratch = null;
	size = round_up_bit(size);

	if (i < 0) {
		scratch = &scratches.push(Arena::from_vmem(size, Arena::COMMIT_ON_PUSH | Arena::DECOMMIT_ON_EMPTY | Arena::ALLOW_CHAIN_GROWTH));
	} else {
		if (scratches[i].current == 0 && scratches[i].bytes.size() < size)
			scratches[i].vmem_resize(size);
		scratch = &scratches[i];
	}

	return { *scratch, scratch->scope() };
}

tuple<Arena&, u64> scratch_push_scope(u64 size, LiteralArray<const Arena*> collision) { return scratch_push_scope(size, larray(collision)); }
tuple<Arena&, u64> scratch_push_scope(u64 size, const Arena* const collision) { return scratch_push_scope(size, carray(&collision, 1)); }
Arena& scratch_pop_scope(Arena& arena, u64 scope) { return (arena.pop_to(scope), arena); }

#endif

#endif
