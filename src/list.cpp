#ifndef G_LIST
# define G_LIST

#include <utils.cpp>
#include <memory.cpp>

template<typename T> struct List {
	Array<T> capacity;
	usize current;

	inline Array<T> used() const { return capacity.subspan(0, current); }
	inline Array<T> free() const { return capacity.subspan(current); }

	auto push_count(usize count) {
		assert(current + count <= capacity.size());
		auto start = current;
		current += count;
		return capacity.subspan(start, count);
	}

	auto& push(T&& element) {
		assert(current < capacity.size());
		return capacity[current++] = element;
	}

	auto& push(const T& element) {
		assert(current < capacity.size());
		return capacity[current++] = element;
	}

	auto push(Array<const T> elements) {
		auto dest = push_count(elements.size());
		for (auto i : u64xrange{ 0, elements.size() })
			dest[i] = elements[i];
		return dest;
	}

	T pop() {
		assert(current > 0);
		return std::move(capacity[--current]);
	}

	auto pop(usize count) {
		current -= count;
		return capacity.subspan(current, count);
	}

	auto push_growing(Arena& arena, usize count) {
		grow(arena, count);
		return push_count(count);
	}

	auto& push_growing(Arena& arena, const T& element) {
		grow(arena);
		return push(element);
	}

	auto push_growing(Arena& arena, Array<const T> elements) {
		auto dest = push_growing(arena, elements.size());
		for (auto i : u64xrange{ 0, elements.size() })
			dest[i] = elements[i];
		return dest;
	}

	auto& swap_in(usize index, const T& element) {
		assert(current < capacity.size());
		capacity[current++] = std::move(capacity[index]);
		return capacity[index] = element;
	}

	auto swap_out(usize index) {
		assert(current > 0);
		auto tmp = std::move(capacity[index]);
		capacity[index] = std::move(capacity[--current]);
		return tmp;
	}

	void remove_ordered(usize index) {
		for (auto i : u64xrange{ index, current - 1 })
			used()[i] = used()[i + 1];
		current--;
	}

	auto& insert_ordered(usize index, const T& element) {
		assert(current < capacity.size());
		if (index < current) for (auto i : u64xrange{ 0, current - index })
			used()[current - i] = used()[current - i - 1];
		current++;
		return used()[index] = element;
	}

	inline auto& insert(usize index, const T& element, bool ordered = false) {
		if (ordered) {
			return insert_ordered(index, element);
		} else {
			return swap_in(index, element);
		}
	}

	inline void remove(usize index, bool ordered = false) {
		if (ordered) {
			remove_ordered(index);
		} else {
			swap_out(index);
		}
	}

	bool grow(Arena& arena, u32 intended_pushes = 1) {
		if (current + intended_pushes > capacity.size()) {
			capacity = arena.morph_array(capacity, max(max(1, capacity.size()) * 2, max(1, capacity.size()) + intended_pushes));
			//TODO error handling upon realloc failure
			return true;
		} else return false;
	}

	bool reduce(Arena& arena) {
		if (current < capacity.size() / 2) {
			capacity = arena.morph_array(capacity, capacity.size() / 2);
			return true;
		} else return false;
	}

	Array<T> shrink_to_content(Arena& arena) {
		return (capacity = arena.morph_array(capacity, current));
	}

	auto pop_reducing(Arena& arena) {
		auto&& tmp = pop();
		reduce(arena);
		return tmp;
	}

	auto& swap_in_growing(Arena& arena, usize index, const T& element) {
		grow(arena);
		return swap_in(index, element);
	}

	auto swap_out_reducing(Arena& arena, usize index) {
		auto tmp = swap_out(index);
		reduce(arena);
		return tmp;
	}

	auto& operator[](u64 index) { return used()[index]; }
	const auto& operator[](u64 index) const { return used()[index]; }
};


#endif
