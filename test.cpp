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
			auto v_arena = create_virtual_arena(1024); defer{ destroy_virtual_arena(v_arena); };
			auto input = larray(test);
			printf("v_arena memory used : %llu/%llu\n", v_arena.current, v_arena.capacity.size());

			printf("input : ");
			for (auto&& i : input)
				printf("%i, ", i.score);
			printf("\n");

			printf("v_arena memory used : %llu/%llu\n", v_arena.current, v_arena.capacity.size());
			auto sorted = sort(as_v_alloc(v_arena), input, [](Test lhs, Test rhs) { return lhs.score - rhs.score; });
			printf("sorted : ");
			for (auto&& i : sorted)
				printf("%i, ", i.score);
			printf("\n");

			printf("v_arena memory used : %llu/%llu\n", v_arena.current, v_arena.capacity.size());
			auto filtered = filter(as_v_alloc(v_arena), sorted, [](Test t) { return (t.score % 2) == 1; });
			printf("filtered : ");
			for (auto&& i : filtered)
				printf("%i, ", i.score);
			printf("\n");

			printf("v_arena memory used : %llu/%llu\n", v_arena.current, v_arena.capacity.size());
			auto mapped = map(as_v_alloc(v_arena), filtered, [](Test t) { return f32(t.score) *.51247937f; });
			printf("mapped : ");
			for (auto&& i : mapped)
				printf("%f, ", i);
			printf("\n");

			printf("v_arena memory used : %llu/%llu\n", v_arena.current, v_arena.capacity.size());
			auto folded = fold(f32(0), mapped, [](f32 lhs, f32 rhs) { return lhs + rhs; });
			printf("folded : %f\n", folded);

			printf("v_arena memory used : %llu/%llu\n", v_arena.current, v_arena.capacity.size());

			return folded;
		}
	());

	return 0;
}
