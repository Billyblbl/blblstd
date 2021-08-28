#ifndef GMEMORYDYN
# define GMEMORYDYN

#include <Memory.cpp>
#include <DynArray.cpp>

namespace Memory {

	template<typename T>
	Generics::DynArray<T>	AllocDynArray(Allocator& allocator, USize count) {
		return Generics::DynArray<T>{ AllocArray<T>(allocator, count), allocator };
	}

}


#endif