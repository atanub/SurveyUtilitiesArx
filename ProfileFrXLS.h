// ProfileFrXLS.h : header file
//

#ifndef _PROFILEFRXLS_H
#define _PROFILEFRXLS_H

#include "Structures.h"

class CProfileFrXLS
{
public:
	CProfileFrXLS();
	~CProfileFrXLS();

	BOOL AddOrdinate(const XLSPROFILEORDINATE&);

	const CXLSProfileOrdinateArray& GetOrdinates() const {return m_RecArray;}
	double GetMaxCumDist();
	double GetMinCumDist();
	double GetMinElev();

	void Reset();
	BOOL Draw(const AcGePoint3d&, const RDPFLNEZDATA&);

	BOOL GetExtents(const RDPFLNEZDATA&, double&, double&, AXISDETAILXLSPFL*);
	BOOL FindMinMaxCumDistElev(double& fMinOff, double& fMaxOff, double& fMaxElev);
	void DumpVars();
private:
	void _DrawAxis(const RDPFLNEZDATA&, const AcGePoint3d&, AcGePoint2d&, const AXISDETAILXLSPFL&);
	void _DrawOrdinates(const RDPFLNEZDATA&, const AcGePoint2d&, const AXISDETAILXLSPFL&);
	void _DrawLabel(const RDPFLNEZDATA&, const AcGePoint3d&, const AXISDETAILXLSPFL&);

	void _SortByCumDist(CXLSProfileOrdinateArray&);
	int	 _GetMinCumDistIndex(const CXLSProfileOrdinateArray& RecArray);
public:
	const static double m_fTxtWidMF;
private:
	CXLSProfileOrdinateArray	m_RecArray;
};
#endif //_PROFILEFRXLS_H

