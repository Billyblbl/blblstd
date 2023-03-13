#ifndef G_LIST
# define G_LIST

#include <utils.cpp>
#include <memory.cpp>

template<typename T> struct List {
	Array<T> capacity;
	usize current;

	auto& push(T&& element) {
		assert(current < capacity.size());
		return capacity[current++] = element;
	}

	auto& push(const T& element) {
		assert(current < capacity.size());
		return capacity[current++] = element;
	}

	auto pop() {
		assert(current > 0);
		return std::move(capacity[--current]);
	}

	auto allocate(usize count) {
		auto start = current;
		current += count;
		return capacity.subspan(start, current);
	}

	auto pop_range(usize count) {
		current -= count;
		return capacity.subspan(current, count);
	}

	auto& swap_in(usize index, T&& element) {
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

	auto allocated() const {
		return capacity.subspan(0, current);
	}

	bool grow(Alloc& alloc) {
		if (current >= capacity.size()) {
			capacity = realloc_array(alloc, capacity, capacity.size() * 2);
			return true;
		} else return false;
	}

	bool reduce(Alloc& alloc) {
		if (current < capacity.size() / 2) {
			capacity = realloc_array(alloc, capacity, capacity.size() / 2);
			return true;
		} else return false;
	}

	auto& push_growing(Alloc& alloc, T&& element) {
		grow(alloc);
		return push(element);
	}

	auto pop_reducing(Alloc& alloc) {
		auto&& tmp = pop();
		reduce(alloc);
		return tmp;
	}

	auto& swap_in_growing(Alloc& alloc, usize index, T&& element) {
		grow(alloc);
		return swap_in(index, element);
	}

	auto swap_out_reducing(Alloc& alloc, usize index) {
		auto tmp = swap_out(index);
		reduce(alloc);
		return tmp;
	}
};


#endif
