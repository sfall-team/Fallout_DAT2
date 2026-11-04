// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Current MFC refuses a WINVER older than XP (0x0501)
#define WINVER	0x0601
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include <afx.h>
#include <afxwin.h>			// MFC core and standard components
#include <afxext.h>			// MFC extensions

#include <afxtempl.h>
