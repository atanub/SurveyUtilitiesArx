// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//


// Check if the build is DEBUG version and it's intended
// to be used with Non-DEBUG AutoCAD.
// In this case, for MFC header files, we need to undefine
// _DEBUG symbol
// Read this project readme.txt for more detail
#if defined( _DEBUG) && !defined(DEBUG_AUTOCAD)
//    #pragma message("Building debug version of SurvUtilArx.ARX to be used with non-debug/Prod AutoCAD")
    #define _DEBUG_WAS_DEFINED
    #undef _DEBUG
#endif

#include <afxwin.h>

/*
#ifndef _ATANU_BANIK
    #pragma message("Building SurvUtilArx.ARX for Self use")
	#define _ATANU_BANIK
#endif
*/

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxtempl.h>
#include <afxdisp.h>        // MFC OLE automation classes

#include <Math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <io.h>
#include <assert.h>

#include <ATLBase.h>
#include <ATLConv.h>
// Turn on the _DEBUG symbol if it was defined, before including
// non-MFC header files.
//
#ifdef _DEBUG_WAS_DEFINED
    #define _DEBUG
    #undef _DEBUG_WAS_DEFINED
#endif

#include "RXMFCapi.h"         // ACAD MFC stuff
#include "acdb.h"             // acdb definitions
#include "adslib.h"           // ads defs
#include "aced.h"             // aced stuff
#include "dbmain.h"           // object and entity reactors
#include "dbdict.h"           // dictionaries
#include "actrans.h"          // transaction manager
#include "appinfo.h"		  // ACAD stuff
#include "rxregsvc.h"		  // ACAD stuff
#include "Accmd.h"			  // ACAD stuff
#include <dbents.h>
#include <Dbsymtb.h>
#include "acdb.h"             // acdb definitions
#include "actrans.h"          // transaction manager
#include "RXMFCapi.h"         // ACAD MFC stuff
#include <acgi.h>
#include <AcGiUtil.h>
#include <dbelipse.h>
#include <AdsCodes.h>

#include <LicenseAll.h>