#ifndef G_MODULE
# define G_MODULE

#include <utils.cpp>
#include <cstdio>

#ifdef PLATFORM_WINDOWS
// #include <windows.h>
#include <libloaderapi.h>
#include <errhandlingapi.h>
using Module = HINSTANCE;

Module load_module(string path) {
	auto mod = LoadLibraryA((LPCSTR)path.data());
	if (mod == null)
		return fail_ret(GetLastError(), null);
	return mod;
}

void unload_module(Module& mod) {
	if (FreeLibrary(mod) == 0)
		fail_msg(GetLastError());
	mod = null;
}

any* get_symbol(Module mod, string name) {
	auto sym = GetProcAddress(mod, name.data());
	if (sym == null)
		return fail_ret(GetLastError(), null);
	return (any*)sym;
}

#else
#include <dlfcn.h>
using Module = any*;

Module load_module(string path) {
	auto mod = dlopen(path.data(), RTLD_NOW);
	if (mod == null)
		return fail_ret(dlerror(), null);
	return mod;
}

void unload_module(Module& mod) {
	if (dlclose(mod) != 0)
		fail_msg(dlerror());
	mod = null;
}

any* get_symbol(Module mod, string name) {
	auto sym = dlsym(mod, name.data());
	auto err = dlerror();
	if (err != null)
		return fail_ret(err, null);
	return sym;
}

#endif

template<typename T> auto get_symbol(Module mod, string symbol) {
	return (T*)get_symbol(mod, symbol);
}

#endif
