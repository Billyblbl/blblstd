#include <blblstd.hpp>

#include <cstdio>

i32 main(i32 ac, const cstrp argv[]) {

	struct Test {
		i32 score;
	} test[] = {
		//  {64}, {614}, {97}, {31}, {82}, {71}, {93}, {46}, {52}, {85}, {13}, {19},
		//  {64}, {614}, {97}, {31}, {82}, {71}, {93}, {46}, {52}, {85}, {13}, {19},
		//  {64}, {614}, {97}, {31}, {82}, {71}, {93}, {46}, {52}, {85}, {13}, {19},
		//  {64}, {614}, {97}, {31}, {82}, {71}, {93}, {46}, {52}, {85}, {13}, {19},
		//  {64}, {614}, {97}, {31}, {82}, {71}, {93}, {46}, {52}, {85}, {13}, {19},
		//  {64}, {614}, {97}, {31}, {82}, {71}, {93}, {46}, {52}, {85}, {13}, {19},
		//  {64}, {614}, {97}, {31}, {82}, {71}, {93}, {46}, {52}, {85}, {13}, {19},
		//  {64}, {614}, {97}, {31}, {82}, {71}, {93}, {46}, {52}, {85}, {13}, {19},
		 {64}, {614}, {97}, {31}, {82}, {71}, {93}, {46}, {52}, {85}, {13}, {19}
	};

	auto end_result = (
		[&]() {
			// auto v_arena = create_virtual_arena(1024); defer{ destroy_virtual_arena(v_arena); };
			auto [v_arena, scope] = scratch_push_scope(1324); defer { scratch_pop_scope(v_arena, scope); };
			auto input = larray(test);
			printf("v_arena memory used : %llu/%llu\n", v_arena.current, v_arena.bytes.size());

			printf("input : ");
			for (auto&& i : input)
				printf("%i, ", i.score);
			printf("\n");

			printf("v_arena memory used : %llu/%llu\n", v_arena.current, v_arena.bytes.size());
			auto sorted = sort(v_arena, input, [](Test lhs, Test rhs) { return lhs.score - rhs.score; });
			printf("sorted : ");
			for (auto&& i : sorted)
				printf("%i, ", i.score);
			printf("\n");

			printf("v_arena memory used : %llu/%llu\n", v_arena.current, v_arena.bytes.size());
			auto filtered = filter(v_arena, sorted, [](Test t) { return (t.score % 2) == 1; });
			printf("filtered : ");
			for (auto&& i : filtered)
				printf("%i, ", i.score);
			printf("\n");

			printf("v_arena memory used : %llu/%llu\n", v_arena.current, v_arena.bytes.size());
			auto mapped = map(v_arena, filtered, [](Test t) { return f32(t.score) *.51247937f; });
			printf("mapped : ");
			for (auto&& i : mapped)
				printf("%f, ", i);
			printf("\n");

			printf("v_arena memory used : %llu/%llu\n", v_arena.current, v_arena.bytes.size());
			auto folded = fold(f32(0), mapped, [](f32 lhs, f32 rhs) { return lhs + rhs; });
			printf("folded : %f\n", folded);

			printf("v_arena memory used : %llu/%llu\n", v_arena.current, v_arena.bytes.size());

			return folded;
		}
	());

	return 0;
}
