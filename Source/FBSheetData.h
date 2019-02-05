//FBSheetData.h
//
#ifndef _FBSHEETDATA_H
#define _FBSHEETDATA_H

#include "stdafx.h"
#include "Util.h"
#include "DrawUtil.h"
#include "XData.h"
#include "Structures.h"


////////////////////////////////////////////////////////////////////////////////////////////////
class CFBSheetData: public CObject
{
public:
	CFBSheetData();
	~CFBSheetData();
	
	BOOL	ImportRec(const FBSURVEYOBJ&);
	BOOL	ImportRec(const FBOBJDEF&);
	BOOL	ImportRec(const FBALIGNMENT&);
	BOOL	ImportRec(const FBALGNSCALE&);
	BOOL	SetFBScaleAcrossAlgn(const double& fVal) { if(fVal <= 0.0) return FALSE; m_fScaleAcrossAlgn = fVal; return TRUE; }

	BOOL	HasData() const;

	BOOL	IsAllPointNosExists(const CUIntArray&) const;
	BOOL	IsPointNoExists(const int&) const;
	
	BOOL	Draw(const AcGePoint3d&, const FBDWGPARAM&);
public:
	static BOOL XTractWordsFromLine(const CString&, const CString&, CStringArray&);
	static BOOL GetPointNosFromString(const CString&, const CString&, CUIntArray&);
	static BOOL ParseAngleString(const CString&, const CString&, double&);

private:
	void	_DrawPoints(const AcGePoint3d&, const double&) const;
	void	_DrawObjects(const AcGePoint3d&, const double&) const;

	BOOL	_ConvertPtNoToPt3D(const int, const AcGePoint3d&, const double&, AcGePoint3d&) const;
	BOOL	_GetPointAtChg(const double&, const AcGePoint3d&, AcGePoint3d&) const;
	double	_GetDistance(const double*, const double* = 0L) const;
	double	_FindFwdAngle(const double&) const;
	
	BOOL	_ImportAlgnRecToAlgnScaleArray(const double&, CFBAlgnScalesArray&) const;
	void	_SmoothAlgnScaleArray(CFBAlgnScalesArray&) const;
private:
	static double	_FBAngToRad(const double&);

private:
	CFBAlignmentRecArray	m_arrayAlignmentRecs;
	CFBSurveyObjRecArray	m_arraySurveyObjRecs;
	CFBObjDefsArray			m_arrayObjDefsRecs;
	CFBAlgnScalesArray		m_arrayAlgnScale;
	double					m_fScaleAcrossAlgn;
private:
	FBDWGPARAM				m_RecDwgParam;

};

#endif /*  _FBSHEETDATA_H */
