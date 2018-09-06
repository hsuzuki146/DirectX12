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

#define NEW(p) new p;
#define NEW_ARRAY(p, size) new p[size];
#define SAFE_DELETE(p) if(p) { delete p; }
#define SAFE_DELETE_ARRAY(p) if(p) { delete[] p; }
