//OTravData.h
//
#ifndef _OTRAVDATA_H
#define _OTRAVDATA_H

#include "stdafx.h"
#include "Util.h"
#include "DrawUtil.h"
#include "XData.h"

#include "TraverseAdj.h"


////////////////////////////////////////////////////////////////////////////////////////////////
class COTravData: public CObject
{
public:
	COTravData();
	~COTravData();
	
	BOOL	ImportRec(const TRAVVERTEX&, const BOOL& bIsLastRec = FALSE);
	BOOL	HasData() const;
	void	Adjust(const TRAVDWGPARAM&);

	void	Dump() const;
private:
	CTravVertexArray				m_arrayRecs;
	static const CString			m_strLyrDebug;
private:
	void _CalcAdjustmentReqd(double&, double&) const;
	void _WriteOutput(CStdioFile& fileOut, const TRAVDWGPARAM& recParam) const;

	static void _CalculateCorrIncAngle(const CTravVertexArray&, const int&, double&);
	static void _DrawTraverse(const CTravVertexArray&, const TRAVDWGPARAM&);
};

#endif /*  _OTRAVDATA_H */
