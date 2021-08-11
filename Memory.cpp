#ifndef G_MEMORY
# define G_MEMORY

#include <BaseTypes.cpp>
#include <Array.cpp>
#include <stdlib.h>

namespace Memory {

	struct Buffer : public Generics::Array<Byte> {
		operator bool() const { return (data && count > 0); }

		template<typename T>
		static auto	fromArray(Generics::Array<T> array) {
			auto asByteArray = Generics::arrayCast<Byte>(array);
			return Buffer{
				asByteArray.data,
				asByteArray.count
			};
		}

		template<typename T>
		static auto fromObj(T &obj) {
			return fromArray(Generics::arrayFrom(&obj, &obj ? 1 : 0));
		}
	};

	struct AllocationStrategy {
		Buffer	(*alloc)(Any* memoryContext, USize size) = null;
		Buffer	(*realloc)(Any* memoryContext, USize size, Buffer buffer) = null;
		void	(*dealloc)(Any* memoryContext, Buffer buffer) = null;
		void	(*clear)(Any* memoryContext) = null;
	};

	struct Allocator {
		Any*				context;
		AllocationStrategy	strategy;

		inline Buffer	alloc(USize size) {
			if (strategy.alloc) return strategy.alloc(context, size);
			else return Buffer{};
		}

		inline Buffer	realloc(USize size, Buffer buffer) {
			if (strategy.realloc) return strategy.realloc(context, size, buffer);
			else return Buffer{};
		}

		inline void	dealloc(Buffer buffer) {
			if (strategy.dealloc) strategy.dealloc(context, buffer);
		}

		inline void	clear() {
			if (strategy.clear) strategy.clear(context);
		}

	};

	Buffer	standardCAlloc(Any* memoryContext, USize size) {
		auto ptr = malloc(size);
		return Buffer::fromArray(Generics::arrayFrom((Byte*)ptr, ptr ? size : 0));
	}

	Buffer	standardCRealloc(Any* memoryContext, USize size, Buffer buffer) {
		auto ptr = realloc(buffer.data, size);
		return Buffer::fromArray(Generics::arrayFrom((Byte*)ptr, ptr ? size : 0));
	}

	void	standardCDealloc(Any* memoryContext, Buffer buffer) {
		free(buffer.data);
	}

	Allocator StandardCAllocator {
		null, {
			&standardCAlloc,
			&standardCRealloc,
			&standardCDealloc,
			null
		}
	};

	template<typename T>
	Generics::Array<T>	AllocArray(Allocator& allocator, USize count) {
		return Generics::arrayCast<T>(allocator.alloc(sizeof(T) * count));
	}

	template<typename T, typename... Args>
	T*	New(Allocator& allocator, Args&&... args) {
		auto obj = &Generics::arrayCast<T>(allocator.alloc(sizeof(T)))[0];
		if (obj) return new (obj) T(std::forward<Args>(args)...);
		else return null;
	}

	template<typename T>
	void	Delete(Allocator &allocator, T *obj) {
		obj.~T();
		allocator.dealloc(Buffer::fromObj(*obj));
	}

} // namespace Memory


#endif