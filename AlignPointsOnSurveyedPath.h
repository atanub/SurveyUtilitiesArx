//AlignPointsOnSurveyedPath.h
//
#ifndef _ALIGNPOINTSONSURVEYEDPATH_H
#define _ALIGNPOINTSONSURVEYEDPATH_H

//
//
// CAlignPointsOnSurveyedPath
//////////////////////////////////////////////////////////////////////////////////////////////
//
class CAlignPointsOnSurveyedPath
{
public:
	static void Align();

protected:
	CAlignPointsOnSurveyedPath(const AcGePoint3d& ptStart, const AcGePoint3d& ptEnd, bool bConsider3Doint = false);
	bool Move(const AcDbObjectIdArray* pObjIDArrayPoints);

protected:
	AcGePoint3d MovePointOnAlignment(const AcGePoint3d& ptParam) const;

protected:
	const bool				m_bConsider3Doint;
	const AcGePoint3d		m_ptStart;
	const AcGePoint3d		m_ptEnd;
};

#endif //_ALIGNPOINTSONSURVEYEDPATH_H
