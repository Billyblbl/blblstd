#ifndef G_DYNARRAY
# define G_DYNARRAY

#include <Array.cpp>
#include <Memory.cpp>
#include <cstdio>

namespace Generics {

	template<typename T>
	struct DynArray : public Array<T> {
		Memory::Allocator*	allocator = &Memory::StandardCAllocator;

		void	growTo(u32 targetCount) {
			if (this->count >= targetCount) return;
			auto newBuff = allocator->realloc(
				targetCount * sizeof(T),
				Memory::Buffer::fromArray(*this)
			);
			if (newBuff) {
				auto newSelf =  arrayCast<T>(newBuff);
				this->data = newSelf.data;
				this->count = newSelf.count;
			} else {
				fprintf(stderr, "failed to grow buffer %p for DynArray %p\n", this->data, this);
				fprintf(stderr, "current size %lu\n", this->count * sizeof(T));
				fprintf(stderr, "desired size %lu\n", targetCount * sizeof(T));
				abort();
			}
		}

		void	shrinkTo(u32 targetCount) {
			if (this->count <= targetCount) return;
			auto newBuff = allocator->realloc(
				targetCount * sizeof(T),
				Memory::Buffer::fromArray(*this)
			);
			if (newBuff) {
				auto newSelf = arrayCast<T>(newBuff);
				this->data = newSelf.data;
				this->count = newSelf.count;
			}
		}

		void	release() {
			allocator->dealloc(Memory::Buffer::fromArray(drop()));
		}

		auto	drop() {
			auto dropped = arrayCast<T>(*this);
			this->data = null;
			this->count = 0;
			return dropped;
		}

		//crappy c++ move semantics need move constructors and of course
		// declaring one means need to declare all of them each tiime we need one ffs
		DynArray() = default;
		DynArray(DynArray &&other) {
			this->data = other.data;
			this->count = other.count;
			other.drop();
		}

		DynArray	&operator=(DynArray&& rhs) {
			data = rhs.data;
			count = rhs.count;
			rhs.drop();
		}


		~DynArray() {
			release();
		}

	};

	template<typename T, u32 minCapacity = 1>
	struct DynArrayList {
		DynArray<T>	buffer;
		USize		count = 0;

		auto&		add(T &&element) {

			if (capacity() <= count) grow();

			buffer[count] = element;
			count++;
			return buffer[count - 1];
		}

		template<typename... Args>
		auto&		emplace(Args&&... args) {

			if (capacity() <= count) grow();

			new(&buffer[count]) T(std::forward<Args>(args)...);
			count++;
			return buffer[count - 1];
		}

		void	shrink() {
			auto targetCap = capacity();
			while (targetCap / 2 > count)
				targetCap /= 2;
			if (count > targetCap) return;
			buffer.shrinkTo(targetCap);
		}

		void	grow() {
			auto cap = capacity();
			auto newCapacity = (cap > 0 ? cap * 2 : minCapacity);
			buffer.growTo(newCapacity);
		}

		//@Note @TODO how do we want to handle this kind of error? 2021/03/05
		void		removeNonOrdered(T &element) {
			auto position = find(element);
			if (position) removeNonOrdered(position);
		}

		//@Note element must be part of the collection, should it be checked in here?
		void		removeNonOrdered(T *element) {
			*element.~T();
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

		void	removeRange(u32 start, u32 finish) {
			removeRange(IndexRange{start, finish});
		}


		T*			find(T &element) {
			return buffer.find(element);
		}

		const T*	find(T &element) const {
			return buffer.find(element);
		}

		auto&		operator[](u32 index) {
			return buffer[index];
		}

		const auto&	operator[](u32 index) const {
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

		auto		subArray(u32 start, u32 end) {
			return Array<T> {
				&buffer[start],
				end - start
			};
		}

		auto		subArray(IndexRange indexes) {
			return subArray(indexes.start, indexes.finish);
		}

		auto		indexes() const {
			return IndexRange{0, count};
		}

		auto		drop() {
			auto list = listFrom(buffer.drop());
			list.count = count;
			clear();
			return list;
		}

		operator Array<T>() { return subArray(0, count); }

		auto		&first() { return buffer[0]; }
		auto		&last() { return buffer[count - 1]; }
		auto		&first() const { return buffer[0]; }
		auto		&last() const { return buffer[count - 1]; }

		auto		begin() { return buffer.begin(); }
		auto		end() { return begin() + count; }
		auto		begin() const { return buffer.begin(); }
		auto		end() const { return begin() + count; }

		//have to declare move constructor explicitly because ¯\_(ツ)_/¯
		DynArrayList() = default;
		DynArrayList(DynArray<T>&& buff) : buffer(buff), count(0)
		DynArrayList(DynArrayList&& other) : buffer(other.buffer) {
			count = other.count;
			other.clear();
		}

		DynArrayList	&operator=(DynArrayList&& rhs) {
			buffer = std::move(rhs.buffer);
			count = rhs.count;
			rhs.clear();
		}

		~DynArrayList() {
			removeAll();
		}

	};


} // namespace Generics

#endif