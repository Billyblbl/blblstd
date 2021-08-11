#ifndef GARENA
# define GARENA

#include <Array.cpp>
#include <Memory.cpp>

namespace Memory {

	struct Arena : public Generics::ArrayList<Byte> {

		inline Buffer	alloc(USize size) {
			auto newCount = count + size;
			if (newCount >= buffer.count) return Buffer{};
			else {
				auto buff = subArray(count, newCount);
				count = newCount;
				return Buffer::fromArray(buff);
			}
		}

		template<typename T>
		inline static auto fromArray(Generics::Array<T> array) {
			return Arena { Buffer::fromArray(array), 0 };
		}

	};

	Buffer ArenaAlloc(Any* context, USize size) {
		auto arena = (Arena*)context;
		return arena->alloc(size);
	}

	Buffer ArenaRealloc(Any* context, USize size, Buffer buffer) {
		auto arena = (Arena*)context;
		if (size <= buffer.count) return buffer;
		return arena->alloc(size);
	}

	void ArenaClear(Any* context) {
		auto arena = (Arena*)context;
		arena->clear();
	}

	Allocator	MakeArenaAllocator(Arena *arena) {
		return Allocator {
			arena, {
				&ArenaAlloc,
				&ArenaRealloc,
				null,
				&ArenaClear
			}
		};
	}

} // namespace Memory


#endif