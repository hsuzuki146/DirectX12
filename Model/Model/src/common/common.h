#pragma once
#include <windows.h>
#include <tchar.h>
#include <DirectXMath.h>
#include <string>

#if defined (_DEBUG)
#include <assert.h>
#define ASSERT(flag) assert(flag);
#else	// #if defined (_DEBUG)
#define ASSERT(flag) 
#endif	// #if defined (_DEBUG)

#include "types.h"
#include "singleton.h"
#include "setup_param.h"
#include "window.h"