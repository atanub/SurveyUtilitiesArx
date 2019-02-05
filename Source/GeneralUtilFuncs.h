// GeneralUtilFuncs.h : header file
//
#ifndef __GENERALUTILFUNCS_H_
#define __GENERALUTILFUNCS_H_

#include "SurvUtilApp.h"
#include "RdPflNEZDataDlg.h"


/////////////////////////////////////////////////////////////////////////////
// CGeneralUtilFuncs 

class CGeneralUtilFuncs
{
// Construction
private:
	CGeneralUtilFuncs();
	~CGeneralUtilFuncs();

public:
	static BOOL InsBlocksFromDatFile();
	static BOOL SetZValueOfSelEnts();
	static BOOL ModifyZVal();
	static BOOL ModifyTxtSize();
	static BOOL InsBlocks(const CString&, const CString&, const double&, const double&, const double&, const double&, const BOOL&);
	static BOOL JoinPoints(const CString&, const BOOL& = TRUE, const BOOL& = TRUE);
	static BOOL QProfilePts(const RDPFLNEZDATA&);
	static BOOL QProfileLines(const RDPFLNEZDATA&);
	static BOOL QProfilePickNAnnotate(const RDPFLNEZDATA&);

	static int IsCurSortDirOK(const AcGePoint3dArray&);
	static int ShowCurDirection(const AcGePoint3dArray&);
	static void DrawArrowHeadOnDisp(const AcGePoint3d&, const AcGePoint3d&, const BOOL=FALSE);

	static BOOL ExtractPtsNLyrsFromSelSet(ads_name&, AcGePoint3dArray&, CStringArray&);
private:
	static BOOL Select2DPLine(AcGePoint3d&, AcDbObjectId*, AcGePoint3dArray*, BOOL&);
	static BOOL SelectDatumLine(double&);
};
/////////////////////////////////////////////////////////////////////////////
#endif //__GENERALUTILFUNCS_H_
