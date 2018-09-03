#pragma once
#include <windows.h>
#include <tchar.h>

#if defined (_DEBUG)
#include <assert.h>
#define ASSERT(flag) assert(flag);
#else	// #if defined (_DEBUG)
#define ASSERT(flag) 
#endif	// #if defined (_DEBUG)

#include "singleton.h"
#include "setup_param.h"
#include "window.h"