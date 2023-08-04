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
		return capacity.subspan(start, count);
	}

	auto allocate_growing(Alloc allocator, usize count) {
		grow(allocator, count);
		return allocate(count);
	}

	auto push_range(Array<const T> elements) {
		assert(current + elements.size() < capacity.size());
		auto dest = allocate(elements.size());
		for (auto i : u64xrange{ 0, elements.size() })
			dest[i] = elements[i];
		return dest;
	}

	auto push_range_growing(Alloc allocator, Array<const T> elements) {
		auto dest = allocate_growing(allocator, elements.size());
		for (auto i : u64xrange{ 0, elements.size() })
			dest[i] = elements[i];
		return dest;
	}

	auto pop_range(usize count) {
		current -= count;
		return capacity.subspan(current, count);
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
			allocated()[i] = allocated()[i + 1];
		current--;
	}

	auto& insert_ordered(usize index, const T& element) {
		assert(current < capacity.size());
		if (index < current) for (auto i : u64xrange{ 0, current - index })
			allocated()[current - i] = allocated()[current - i - 1];
		current++;
		return allocated()[index] = element;
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

	auto allocated() const {
		return capacity.subspan(0, current);
	}

	bool grow(Alloc alloc, u32 intended_pushes = 1) {
		if (current + intended_pushes > capacity.size()) {
			capacity = realloc_array(alloc, capacity, max(max(1, capacity.size()) * 2, max(1, capacity.size()) + intended_pushes));
			//TODO error handling upon realloc failure
			return true;
		} else return false;
	}

	bool reduce(Alloc alloc) {
		if (current < capacity.size() / 2) {
			capacity = realloc_array(alloc, capacity, capacity.size() / 2);
			return true;
		} else return false;
	}

	Array<T> shrink_to_content(Alloc alloc) {
		return capacity = realloc_array(alloc, capacity, current);
	}

	auto& push_growing(Alloc alloc, const T& element) {
		grow(alloc);
		return push(element);
	}

	auto pop_reducing(Alloc& alloc) {
		auto&& tmp = pop();
		reduce(alloc);
		return tmp;
	}

	auto& swap_in_growing(Alloc& alloc, usize index, const T& element) {
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
