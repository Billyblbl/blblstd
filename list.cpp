#ifndef G_LIST
# define G_LIST

#include <utils.cpp>
#include <memory.cpp>

template<typename T> struct List {
	Array<T> capacity;
	usize count;

	auto& push(T&& element) {
		assert(count < capacity.size());
		return capacity[count++] = element;
	}

	auto pop() {
		assert(count > 0);
		return std::move(capacity[--count]);
	}

	auto allocate(usize count) {
		auto start = count;
		count += count;
		return capacity.subspan(start, count);
	}

	auto pop_range(usize count) {
		count -= count;
		return capacity.subspan(count, count);
	}

	auto& swap_in(usize index, T&& element) {
		assert(count < capacity.size());
		capacity[count++] = std::move(capacity[index]);
		return capacity[index] = element;
	}

	auto swap_out(usize index) {
		assert(count > 0);
		auto tmp = std::move(capacity[index]);
		capacity[index] = std::move(capacity[--count]);
		return tmp;
	}

	auto allocated() const {
		return capacity.subspan(0, count);
	}

	bool grow(Alloc& alloc) {
		if (count >= capacity.size()) {
			capacity = realloc_array(alloc, capacity, capacity.size() * 2);
			return true;
		} else return false;
	}

	bool reduce(Alloc& alloc) {
		if (count < capacity.size() / 2) {
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
