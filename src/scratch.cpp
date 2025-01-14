#ifndef GSCRATCH
# define GSCRATCH

#include <arena.cpp>

#include <list.cpp>
Array<Arena> scratch_preallocate(u64 size, u64 channels = 1);
void scratch_clear(bool root = true);
tuple<Arena&, u64> scratch_push_scope(u64 size = {}, Array<const Arena* const> collisions = {});
tuple<Arena&, u64> scratch_push_scope(u64 size, LiteralArray<const Arena*> collision);
tuple<Arena&, u64> scratch_push_scope(u64 size, const Arena* const collision);
Arena& scratch_pop_scope(Arena& arena, u64 scope);

// #define BLBLSTD_IMPL
#ifdef BLBLSTD_IMPL

static List<Arena> &get_scratches() {
	constexpr auto MAX_SCRATCHES = 16;
	static thread_local Arena buffer[MAX_SCRATCHES];
	static thread_local List<Arena> scratches = { larray(buffer), 0 };
	return scratches;
}

Array<Arena> scratch_preallocate(u64 size, u64 channels) {
	auto& scratches = get_scratches();
	auto begin = scratches.current;
	for (auto i = 0u; i < channels; i++)
		scratches.push(Arena::from_vmem(size, Arena::COMMIT_ON_PUSH | Arena::DECOMMIT_ON_EMPTY | Arena::ALLOW_CHAIN_GROWTH));
	return scratches.used().subspan(begin, channels);
}

void scratch_clear(bool root) {
	for (auto& s : get_scratches().used()) if (root)
		s.vmem_release();
	else
		s.reset();
	if (root)
		get_scratches().current = 0;
}

//* would be better to directly provide the tip, but would make collision check expensive
//* would it be more expensive than the recusrive push it causes ? it should be if root scratch is too small
tuple<Arena&, u64> scratch_push_scope(u64 size, Array<const Arena* const> collisions) {
	auto& scratches = get_scratches();

	auto collide = [&](const Arena& s) { return linear_search(collisions, [&](const Arena* const c) { return c == &s; }) >= 0; };
	auto i = linear_search(scratches.used(), [&](Arena& s) { return (s.current == 0 || s.free_tip().size() >= size) && !collide(s); });
	Arena* scratch = null;
	size = round_up_bit(size + sizeof(Arena));

	if (i < 0) {
		scratch = &scratches.push(Arena::from_vmem(size, Arena::COMMIT_ON_PUSH | Arena::DECOMMIT_ON_EMPTY | Arena::ALLOW_CHAIN_GROWTH));
		if (scratches.current > 5)
			fprintf(stderr, "Warning %llu scratches, potential bug or incorrect usage\n", scratches.current);
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
