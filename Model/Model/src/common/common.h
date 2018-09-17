#pragma once
#include <windows.h>
#include <tchar.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <map>

#if defined (_DEBUG)
#include <assert.h>
#define ASSERT(flag) assert(flag);
#else	// #if defined (_DEBUG)
#define ASSERT(flag) 
#endif	// #if defined (_DEBUG)

#include "types.h"
#include "math.h"
#include "singleton.h"
#include "setup_param.h"
#include "window.h"
#include "d3dx12manager.h"
using namespace DirectX;

#define NEW(p) new p;
#define NEW_ARRAY(p, size) new p[size];
#define SAFE_DELETE(p) if(p) { delete p; }
#define SAFE_DELETE_ARRAY(p) if(p) { delete[] p; }
#define SAFE_RELEASE(p) if(p) { p->Release(); }
#define SAFE_RELEASE_ARRAY(p) if(p) { const UInt32 size = ARRAYSIZE(p); for( UInt32 i = 0; i < size; ++i ) { p[i]->Release(); } }