#include <blblstd.hpp>

#include <cstdio>

i32 main(i32 ac, const cstrp argv[]) {

	struct Test {
		i32 score;
	} test[] = { {64}, {614}, {97}, {31}, {82}, {71}, {93}, {46}, {52}, {85}, {13}, {19} };

	auto input = larray(test);
	printf("input : ");
	for (auto &&i : input)
		printf("%i, ", i.score);
	printf("\n");

	auto sorted = sort(std_allocator, input, [](Test lhs, Test rhs) { return lhs.score - rhs.score; });
	printf("sorted : ");
	for (auto &&i : sorted)
		printf("%i, ", i.score);
	printf("\n");

	auto filtered = filter(std_allocator, sorted, [](Test t) { return (t.score % 2) == 0; });
	printf("filtered : ");
	for (auto &&i : filtered)
		printf("%i, ", i.score);
	printf("\n");

	auto mapped = map(std_allocator, filtered, [](Test t) { return f32(t.score); });
	printf("mapped : ");
	for (auto &&i : mapped)
		printf("%f, ", i);
	printf("\n");

	auto folded = fold(f32(0), mapped, [](f32 lhs, f32 rhs) { return lhs + rhs; });
	printf("folded : %f\n", folded);

	return 0;
}
