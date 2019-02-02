//DrawTable.h
//
#ifndef _DRAWTABLE_H
#define _DRAWTABLE_H

#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////////////////////////

class CDrawTable
{
public:
	CDrawTable(const CPtrArray*, const double&, const double&, const BOOL&);
	~CDrawTable();
	
	BOOL Draw(const AcGePoint3d&, const CString&);
	
public:
	static BOOL FindMaxColWids(const CPtrArray*, const double&, AcGeDoubleArray&);
	static BOOL DrawGrid(const CPtrArray*, const AcGeDoubleArray&, const AcGePoint3d&, const double&, const double&, const CString&);
	static BOOL DrawText(const CPtrArray*, const AcGeDoubleArray&, const AcGePoint3d&, const double&, const double&, const CString&);

private:
	const CPtrArray*		m_pPtrArray;
	//Dwg params
	const double& m_fTxtHt;
	const double& m_fMargin;
	const BOOL&	  m_bDrawGrid;

};
#endif /*  _DRAWTABLE_H */
