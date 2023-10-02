#ifndef GHIGH_ORDER
# define GHIGH_ORDER

#include <utils.cpp>
#include <memory.cpp>
#include <list.cpp>
#include <link_list.cpp>
#include <concepts>

template<typename S> struct signature {
	using r = void;
	using t = tuple<void>;
};

template<typename R, typename... A> struct signature<R(A...)> {
	using r = R;
	using t = tuple<A...>;
};

template<typename F, typename T, typename R> concept functor_t = requires(F && f, T && args) { { std::apply(f, args) } -> std::same_as<R>; };
template<typename F, typename S> concept functor = functor_t<F, typename signature<S>::t, typename signature<S>::r>;

//TODO concepts for function parameter requirements mapper, score, comp

//! Intentionally leaks, meant to be used with the intent of clearing all memory after multiple operations
template<typename T> Array<T> filter(Alloc allocator, Array<T> collection, functor<bool(const T&)> auto predicate) {
	auto list = List{ alloc_array<T>(allocator, collection.size()), 0 };
	for (auto&& i : collection) if (predicate(i))
		list.push(i);
	list.shrink_to_content(allocator);
	return list.allocated();
}

//! Intentionally leaks, meant to be used with the intent of clearing all memory after multiple operations
template<typename T> auto map(Alloc allocator, Array<T> collection, auto mapper) {
	using R = decltype(mapper(collection[0]));
	auto list = List{ alloc_array<R>(allocator, collection.size()), 0 };
	for (auto&& i : collection)
		list.push(mapper(i));
	return list.allocated();
}

template<typename T> i64 best_fit_search(Array<T> collection, auto score) {
	if (collection.size() == 0)
		return -1;
	auto s = score(collection[0]);
	auto index = 0;
	for (auto i : i64xrange{ 0, collection.size() }) {
		auto si = score(collection[i]);
		if (si > s) {
			s = si;
			index = i;
		}
	}
	return index;
}

template<typename T> T fit_highest(const T& t) { return t;}
template<typename T> T fit_lowest(const T& t) { return -t;}

//! Intentionally leaks, meant to be used with the intent of clearing all memory after multiple operations
//! don't use this with big collections or will probably explode stack with nodes_buff size
template<typename T> Array<T> sort(Alloc allocator, Array<T> collection, auto comp) {
	struct IndexNode {
		u64 index;
		IndexNode* next;
	} nodes_buff[collection.size()];
	auto nodes = List{ carray(nodes_buff, collection.size()), 0 };
	LinkList<IndexNode> ll;

	for (auto i : u64xrange{ 0, collection.size() }) {
		IndexNode* closest = null;
		auto best = linear_search(nodes.allocated(), [&](const IndexNode& n) { return comp(collection[n.index], collection[i]) < 0 && (n.next == null || comp(collection[n.next->index], collection[i]) >= 0);});
		if (best >= 0)
			closest = &nodes.allocated()[best];
		auto& new_node = nodes.push({ i, nullptr });
		if (closest == null)
			list_preppend(ll, &new_node, &IndexNode::next);
		else {
			insert_after(closest, &new_node, &IndexNode::next);
		}
	}

	auto sorted = List{ alloc_array<T>(allocator, collection.size()), 0 };
	for (auto&& node : traverse_by<IndexNode, &IndexNode::next>(ll.first))//* using first because insert_after can't move the last in the list, so if we want to iterate on the whole thing we need to ignore the current end of ll
		sorted.push(collection[node.index]);

	return sorted.allocated();
}

template<typename R, typename T> R fold(const R& init, Array<T> collection, auto acc) {
	R result = init;
	for (auto&& e : collection)
		result = acc(result, e);
	return result;
}

#endif
