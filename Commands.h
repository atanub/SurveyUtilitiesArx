// Commands.h : header file
//

#ifndef __COMMANDS_H_
#define __COMMANDS_H_


/////////////////////////////////////////////////////////////////////////////
// CCommands 

class CCommands
{
// Construction
protected:
	CCommands();
	~CCommands();

public:
	static void InsBlocks();
	static void JoinPoints();
	static void InsBlocksFromDatFile();
	static void ModifyZValueOfSelEnts();
	static void ModifyTxtSizeOfSelEnts();
	static void SetZValueOfSelEnts();
	static void ExportPoints();
	static void ExportPointsEx();
	static void QProfileLines();
	static void QProfileLinesOrg();
	static void QProfilePts();
	static void QProfilePtsOrg();
	static void QProfilePickNAnnotate();
	static void DrawXSecFrDwg();
	static void DrawXSecFrDwgEx();
};
/////////////////////////////////////////////////////////////////////////////
#endif //__COMMANDS_H_


static void AnnotateChgCore(const double& fChg, const double& fLeftOff, const double& fRiteOff, const double& fTxtSize, const char* pszLyr);
static BOOL AnnotateChg(const AcGePoint3dArray& ptArray, const double& fChg, const double& fTxtSize, const char* pszLyr, const double* pfLeftOffset, const double* pfRiteOffset);
static BOOL XtractVertsFr2DPLine(const AcDbObjectId& objId, AcGePoint3dArray& Array, CString* pErrMsg = 0L);
