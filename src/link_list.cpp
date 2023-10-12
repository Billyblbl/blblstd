#ifndef GLINK_LIST
# define GLINK_LIST

#include <cstddef>
#include <utils.cpp>

template<typename T> struct LinkList {
	T* first = nullptr;
	T* last = nullptr;
};

#pragma region Single link

template<typename T, T* T::* l> u64 count(T* list) {
	u64 i = 0;
	for (auto _ : traverse_by<T, l>(list)) i++;
	return i;
}

template<typename T, T* T::* l> struct ListIterator {
	T* current;
	ListIterator operator++() { return { current = (current->*l) }; }
	T& operator*() { return *current; }
};

template<typename T> T* insert_after(T* current, T* next, T* T::* link) {
	if (current != nullptr) {
		next->*link = current->*link;
		current->*link = next;
	}
	return next;
}

template<typename T> T* insert_before(T* current, T* previous, T* T::* link) {
	if (previous != nullptr) {
		previous->*link = current;
	}
	return previous;
}

template<typename T> T* list_append(LinkList<T>& parent, T* element, T* T::* link) {
	parent.last = insert_after(parent.last, element, link);
	if (parent.first == nullptr)
		parent.first = element;
	return element;
}

template<typename T> T* list_preppend(LinkList<T>& parent, T* element, T* T::* link) {
	parent.first = insert_before(parent.first, element, link);
	if (parent.last == nullptr)
		parent.last = element;
	return element;
}

template<typename T, T* T::* l> bool operator!=(ListIterator<T, l> a, ListIterator<T, l> b) {
	if (a.current == nullptr && b.current == nullptr) return false;
	return a.current != b.current;
}

template<typename T, T* T::* l> auto traverse_by(T* start) {
	return it_range{
		ListIterator<T, l> { start },
		ListIterator<T, l> { nullptr }
	};
};

template<typename T, T* T::* l> auto traverse_by(const LinkList<T>& list) {
	return it_range{
		ListIterator<T, l> { list.first },
		ListIterator<T, l> { list.last != null ? list.last->*l : null }
	};
};

template<typename T, T* T::* l> auto link_range_incl(T* first, T* last) { return traverse_by<T, l>(LinkList{ first, last }); }
template<typename T, T* T::* l> auto link_range_excl(T* begin, T* end) {
	return it_range{
		ListIterator<T, l> { begin },
		ListIterator<T, l> { end }
	};
}

#pragma endregion Single link

#pragma region Double link

template<typename T> struct DoubleLink {
	T* previous = nullptr;
	T* next = nullptr;
};

template<typename T, DoubleLink<T> T::* l> u64 count(T* list) {
	u64 i = 0;
	for (auto _ : traverse_by<l>(list)) i++;
	return i;
}

template<typename T, DoubleLink<T> T::* l> struct DoubleListIterator {
	T* current;
	DoubleListIterator operator++() { return { current = (current->*l).next }; }
	DoubleListIterator operator--() { return { current = (current->*l).previous }; }
	T& operator*() { return *current; }
};

template<typename T> T* insert_after(T* current, T* next, DoubleLink<T> T::* link) {
	if (current != nullptr) {
		if ((current->*link).next != nullptr)
			(next->*link).next = (current->*link).next;
		(current->*link).next = next;
	}
	if (next != nullptr)
		(next->*link).previous = current;
	return next;
}

template<typename T> T* insert_before(T* current, T* previous, DoubleLink<T> T::* link) {
	if (current != nullptr) {
		if ((current->*link).previous != nullptr)
			(previous->*link).previous = (current->*link).previous;
		(current->*link).previous = previous;
	}
	if (previous != nullptr)
		(previous->*link).next = current;
	return previous;
}


template<typename T> T* list_append(LinkList<T>& parent, T* element, DoubleLink<T> T::* link) {
	parent.last = insert_after(parent.last, element, link);
	if (parent.first == nullptr)
		parent.first = element;
	return element;
}

template<typename T> T* list_preppend(LinkList<T>& parent, T* element, DoubleLink<T> T::* link) {
	parent.first = insert_before(parent.first, element, link);
	if (parent.last == nullptr)
		parent.last = element;
	return element;
}

template<typename T, DoubleLink<T> T::* l> bool operator!=(DoubleListIterator<T, l> a, DoubleListIterator<T, l> b) {
	if (a.current == nullptr && b.current == nullptr) return false;
	return a.current != b.current;
}

template<typename T, DoubleLink<T> T::* l> auto traverse_by(T* start) {
	return it_range{
		DoubleListIterator<T, l> { start },
		DoubleListIterator<T, l> { nullptr }
	};
};

template<typename T, DoubleLink<T> T::* l> auto traverse_by(const LinkList<T>& list) {
	return it_range{
		DoubleListIterator<T, l> { list.first },
		DoubleListIterator<T, l> { (list.last->*l).next }
	};
};

template<typename T, DoubleLink<T> T::* l> auto link_range_incl(T* first, T* last) { return traverse_by<T, l>(LinkList{ first, last }); }
template<typename T, DoubleLink<T> T::* l> auto link_range_excl(T* begin, T* end) {
	return it_range{
		DoubleListIterator<T, l> { begin },
		DoubleListIterator<T, l> { end }
	};
}

#pragma endregion Double link

#endif
