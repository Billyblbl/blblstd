#ifndef G_ARRAY
# define G_ARRAY

#include <utility>
#include <BaseTypes.cpp>

namespace Generics {

	struct IndexIterator {
		USize	value;
		auto operator++() { return value++; }
		auto operator--() { return value--; }
		auto operator*() { return value; }
	};

	struct ReverseIndexIterator {
		USize	value;
		auto operator--() { return value++; }
		auto operator++() { return value--; }
		auto operator*() { return value; }
	};

	auto operator!=(IndexIterator a, IndexIterator b) {return a.value != b.value;}
	auto operator!=(ReverseIndexIterator a, ReverseIndexIterator b) {return a.value != b.value;}

	struct ReverseIndexRange {
		USize start;
		USize finish;

		auto contains(USize value) { return (value > finish && value <= start); }
		auto begin() { return ReverseIndexIterator{start}; }
		auto end() { return ReverseIndexIterator{finish}; }
		auto first() { return start; }
		auto last() { return finish + 1; }
		auto first() const { return start; }
		auto last() const { return finish + 1; }
		auto size() const { return start - finish; }
	};

	struct IndexRange {
		USize start;
		USize finish;

		auto contains(USize value) { return (value >= start && value < finish); }
		auto begin() { return IndexIterator{start}; }
		auto end() { return IndexIterator{finish}; }
		auto first() { return start; }
		auto last() { return finish - 1; }
		auto first() const { return start; }
		auto last() const { return finish - 1; }
		auto reverse() { return ReverseIndexRange{ finish - 1, start - 1 }; }
		auto reverse() const { return ReverseIndexRange{ finish - 1, start - 1 }; }
		auto size() const { return finish - start; }
	};

	template<typename T>
	struct Array {
		T*		data = null;
		USize	count = 0;

		operator Array<const T>() const {
			return arrayCast<const T>(*this);
		}

		auto *begin() { return data; }
		auto *end() { return data + count; }
		const auto *begin() const { return data; }
		const auto *end() const { return data + count; }

		auto&		operator[](u64 index) {
			return data[index];
		}

		const auto&	operator[](u64 index) const {
			return data[index];
		}

		T*			find(T &element) {
			for (auto &it : *this)
				if (it == element) return &it;
			return null;
		}

		const T*	find(T &element) const {
			for (auto &it : *this)
				if (it == element) return &it;
			return null;
		}

		auto		subArray(u64 start, u64 end) {
			return Array<T> {
				&data[start],
				end - start
			};
		}

		auto		subArray(IndexRange indexes) {
			return subArray(indexes.start, indexes.finish);
		}

		const auto		subArray(u64 start, u64 end) const {
			return Array<T> {
				&data[start],
				end - start
			};
		}

		auto		subArray(IndexRange indexes) const {
			return subArray(indexes.start, indexes.finish);
		}

		auto		indexes() const {
			return IndexRange{0, count};
		}


		auto		&first() { return (*this)[0]; }
		auto		&last() { return (*this)[count - 1]; }
		auto		&first() const { return (*this)[0]; }
		auto		&last() const { return (*this)[count - 1]; }

	};

	template<typename T, u64 arraySize>
	auto	arrayFrom(T (&array)[arraySize]) { return Array<T>{array, arraySize}; }

	template<typename T>
	auto	arrayFrom(T* array, USize arraySize) { return Array<T>{array, arraySize}; }

	template<typename T, typename U>
	auto	arrayCast(Array<U> view) {
		return Array<T>{
			(T*)view.begin(),
			USize((T*)view.end() - (T*)view.begin())
		};
	}

	template<typename T>
	struct ArrayList {
		Array<T>	buffer;
		USize		count = 0;

		auto&		add(T &&element) {
			buffer[count] = element;
			count++;
			return buffer[count - 1];
		}

		template<typename... Args>
		auto&		emplace(Args&&... args) {
			new(&buffer[count]) T(std::forward<Args>(args)...);
			count++;
			return buffer[count - 1];
		}

		//@Note @TODO how do we want to handle this kind of error? 2021/03/05
		void		removeNonOrdered(T &element) {
			auto position = find(element);
			if (position) removeNonOrdered(position);
		}

		//@Note element must be part of the collection, should it be checked in here?
		void		removeNonOrdered(T *element) {
			element->~T();
			count--;
			*element = buffer[count];
		}

		void		removeRange(IndexRange range) {
			for (auto i : range)
				buffer[i].~T();
			auto toMove = subArray(range.finish, count);
			for (auto i : toMove.indexes()) {
				buffer[range.start + i] = toMove[i];
			}
			count -= range.size();
		}

		auto&		last() { return buffer[count - 1]; }
		auto&		first() { return buffer[0]; }
		const auto&		last() const { return buffer[count - 1]; }
		const auto&		first() const { return buffer[0]; }

		T*			find(T &element) {
			return buffer.find(element);
		}

		const T*	find(T &element) const {
			return buffer.find(element);
		}

		auto&		operator[](u64 index) {
			return buffer[index];
		}

		const auto&	operator[](u64 index) const {
			return buffer[index];
		}

		void		clear() {
			count = 0;
		}

		void		removeAll() {
			for (auto &element : *this) element.~T();
			clear();
		}

		USize		capacity() const {
			return buffer.count;
		}

		auto		subArray(u64 start, u64 end) {
			return Array<T> {
				&buffer[start],
				end - start
			};
		}

		operator Array<T>() { return subArray(0, count); }

		auto		indexes() const {
			return IndexRange{0, count};
		}

		auto		begin() { return buffer.begin(); }
		auto		end() { return begin() + count; }
		auto		begin() const { return buffer.begin(); }
		auto		end() const { return begin() + count; }

		~ArrayList() { removeAll(); }

	};

	template<typename T>
	auto	listFrom(Array<T> array) {
		return ArrayList<T>{
			array,
			0
		};
	}

	template<typename T, u64 arraySize>
	auto	listFrom(T (&array)[arraySize]) { return listFrom(arrayFrom(array)); }

	template<typename T>
	auto	listFrom(T* array, USize arraySize) { return listFrom(arrayFrom(array, arraySize)); }

	template<typename T, u64 arraySize>
	auto	arrayCount(T (&)[arraySize]) { return arraySize; }

} // namespace Generics


#endif