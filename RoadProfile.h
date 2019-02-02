//RoadProfile.h
//
#ifndef _ROADPROFILE_H
#define _ROADPROFILE_H

#include "Structures.h"

////////////////////////////////////////////////////////////////////////////////////////////////

class CRoadProfile : public CObject
{
public:
	CRoadProfile(const BOOL&);
	CRoadProfile(const CRoadProfile&);
	~CRoadProfile();
	
	BOOL	ImportRec(const ROADPROFILE_CHG_Z&);
	BOOL	ImportGradRec(const GRADDETAIL&);
	void	SetStartLevOfDsgnPfl(const double& dVal) { m_dDsgnPflStartElev = dVal;}
	
	BOOL	HasData() const;
	BOOL	Draw(const RDPFL_CHG_Z_DWG_PARAM&, const AcGePoint3d&, const AcGePoint3dArray* pPtArrayExistingDesignProfile = 0L, double* pfStartChgOnExistingDesignProfile = 0L);
	double	GetStartChainage() const;
private:
	static BOOL	_GetDsgnLevel(const double&, const double&, const CGradDataRecs&, double&, double* = 0L);
	static BOOL	_GetExistingDsgnLevel(const double& dCumDistParam, const AcGePoint3dArray& arrayPtsExistingDesignProfile, double& dExistingDsgnLev, const double* pfStartChgOnExistingDesignProfile = 0L);
	static BOOL	_GetElevOfStartGradChg(const double&, const double&, const CGradDataRecs&, double&);
	static BOOL	_GetGradElevs(const double&, const CGradDataRecs&);
	static BOOL	_GetGradElevsWRTXistPfl(const double&, const double&, const double&, const CGradDataRecs&, CGradDataRecs&);

	static void	_SortByChainage(CRPflChgZDataRecs&);
	static int	_GetMinCumDistIndex(const CRPflChgZDataRecs&);
	
	static BOOL	_FindMinMaxCumDistElev(const CRPflChgZDataRecs&, double&, double&, double&, double&);
	static BOOL	_GenOrdinatesAtEqInterval(const CRPflChgZDataRecs&, CRPflChgZDataRecs&, const double&);
	static BOOL	_CalcNInsertPointAtDist(const double&, CRPflChgZDataRecs&, const double fPrecision = 1.0E-4);
	
	static double SlopeToDistMF(const double& fParam)
	{
		return ((fParam == 0.0) ? fParam:(1.0 / fParam));
	}
	
	static double _GetMaxElev(const CRPflChgZDataRecs&);
private:
	static BOOL IsChgDefinedAsDsgnLevData(const CGradDataRecs&, const double&);
	
	BOOL _GetExtents(double&, double&, AXISDETAILXLSPFL*);
	void _DrawAxis(AcGePoint2d&, const AXISDETAILXLSPFL&);
	void _DrawOrdinates(const CRPflChgZDataRecs&, const CGradDataRecs&, const AcGePoint2d&, const AXISDETAILXLSPFL&);
	BOOL _CalculateDsgnLevels();
private:
	CRPflChgZDataRecs		m_RPflChgZDataRecs;
	CRPflChgZDataRecs		m_RPflChgZDataRecsAfterCalc;
	CGradDataRecs			m_GradDataRecs;
	double					m_dDsgnPflStartElev;
	CGradDataRecs			m_ArrayGradCompact;
	
	//
	AcGePoint3dArray*				m_pPtArrayExistingDesignProfile;
	double							m_fStartChgOnExistingDesignProfile;		//To store the start chainage of existing design profile
	const RDPFL_CHG_Z_DWG_PARAM*	m_pDwgParam;
	const AcGePoint3d*				m_pPtLLCParam;	
	CMapStringToString				m_mapRowTagNIndex;		//To store the Data Row name & corresponding index
private:
	const BOOL				m_bConstHasNEZData;
private:
	static const double			m_dAllowebleErr;
	static const double			m_fTxtWidMF;
	static const char			pszGenLyrName[];
};
#endif /*  _ROADPROFILE_H */
