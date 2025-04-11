#pragma once

#pragma warning(disable: 4100 4201 4244 4389 5054)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <stdint.h>
#include <cmath>
#include <DirectXMath.h>

#include "SafeWrite/SafeWrite.hpp"
#include "Utils/Bitfield.hpp"
#include "Utils/DirectXUtils.hpp"

#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

#ifdef NDEBUG
#define ASSUME_ASSERT(x) __assume(x)
#else
#define ASSUME_ASSERT(x) assert(x)
#endif

#define ASSERT_SIZE(a, b) static_assert(sizeof(a) == b, "Wrong structure size!");
#define ASSERT_OFFSET(a, b, c) static_assert(offsetof(a, b) == c, "Wrong member offset!");
#define CREATE_OBJECT(CLASS, ADDRESS) static CLASS* CreateObject() { return CdeclCall<CLASS*>(ADDRESS); };

#ifdef GAME 
#define FUNCTION_UNKOWN static_assert(false, "Find \"" __FUNCSIG__ "\" in the game" );
#define FUNCTION_NOT_IMPLEMENTED static_assert(false, "Implement \"" __FUNCSIG__ "\" in the game" );
#else
#define FUNCTION_UNKOWN static_assert(false, "Find \"" __FUNCSIG__ "\" in GECK" );
#define FUNCTION_NOT_IMPLEMENTED static_assert(false, "Implement \"" __FUNCSIG__ "\" in GECK" );
#endif

template <typename T_Ret = uint32_t, typename ...Args>
__forceinline T_Ret ThisCall(uint32_t _addr, const void* _this, Args ...args) {
	return ((T_Ret(__thiscall*)(const void*, Args...))_addr)(_this, std::forward<Args>(args)...);
}

template <typename T_Ret = void, typename ...Args>
__forceinline T_Ret StdCall(uint32_t _addr, Args ...args) {
	return ((T_Ret(__stdcall*)(Args...))_addr)(std::forward<Args>(args)...);
}

template <typename T_Ret = void, typename ...Args>
__forceinline T_Ret CdeclCall(uint32_t _addr, Args ...args) {
	return ((T_Ret(__cdecl*)(Args...))_addr)(std::forward<Args>(args)...);
}

template <typename T_Ret = void, typename ...Args>
__forceinline T_Ret FastCall(uint32_t _addr, Args ...args) {
	return ((T_Ret(__fastcall*)(Args...))_addr)(std::forward<Args>(args)...);
}