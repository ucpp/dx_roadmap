#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <shellapi.h>

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#include <wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <algorithm>
#include <cassert>
#include <chrono>

#include <common/d3dx12.h>
#include <common/helpers.h>
#include <common/types.h>
#include <common/log.h>