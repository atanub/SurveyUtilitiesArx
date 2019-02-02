// ParameterDataCache.h : header file
////////////////////////////////////////////////////////////////
#ifndef _PARAMETERDATACACHE_H
#define _PARAMETERDATACACHE_H

#include "Structures.h"

class CParameterDataCache  
{
public:
	CParameterDataCache();
	~CParameterDataCache();
	//
	const RDPFLNEZDATA*	GetRdPflNEZDataRec();
	void				SetRdPflNEZDataRec(const RDPFLNEZDATA*);
	//
	const PFLOFFZDWGPARAM*	GetPflOffZDWGParam();
	void					SetPflOffZDWGParam(const PFLOFFZDWGPARAM*);
	//
	const RDXSECOFFELEVDATA*	GetRdXSecOffElevDataParam();
	void						SetRdXSecOffElevDataParam(const RDXSECOFFELEVDATA*);
	//
	const CSDDWGPARAM*			GetCSDDwgParam();
	void						SetCSDDwgParam(const CSDDWGPARAM*);
	//
	const CSDDWGPARAMEX*		GetCSDDwgParamEx();
	void						SetCSDDwgParamEx(const CSDDWGPARAMEX*);
	//
	const DRAWPARAMRDPLANFRTXTFILE*		GetDrawParamRDPlanFRTxtFile();
	void								SetDrawParamRDPlanFRTxtFile(const DRAWPARAMRDPLANFRTXTFILE*);
	//
	const INSBLKFRFILEPARAMS*			GetInsBlkFrFileParam();
	void								SetInsBlkFrFileParam(const INSBLKFRFILEPARAMS*);
	//
	const FBDWGPARAM*					GetFBookDwgParam();
	void								SetFBookDwgParam(const FBDWGPARAM*);
	//
	const XLSXSECDWGPARAM*				GetXlsXSecDwgParam();
	void								SetXlsXSecDwgParam(const XLSXSECDWGPARAM*);
	//
	const RDPFL_CHG_Z_DWG_PARAM*		GetRDPflChgZDwgParam();
	void								SetRDPflChgZDwgParam(const RDPFL_CHG_Z_DWG_PARAM*);
	//
	const TRAVDWGPARAM*					GetTravDwgParam();
	void								SetTravDwgParam(const TRAVDWGPARAM*);

protected:
	//
	RDPFLNEZDATA*					m_pRdPflNEZDataRec;
	PFLOFFZDWGPARAM*				m_pPflOffZDWGParam;
	RDXSECOFFELEVDATA*				m_pRdXSecOffElevData;
	CSDDWGPARAM*					m_pCSDDwgParam;
	CSDDWGPARAMEX*					m_pCSDDwgParamEx;
	DRAWPARAMRDPLANFRTXTFILE*		m_pDrawParamRDPlanFRTxtFile;
	INSBLKFRFILEPARAMS*				m_pInsBlkFrFileParams;
	FBDWGPARAM*						m_pFBookDwgParam;
	XLSXSECDWGPARAM*				m_pXlsXSecDwgParam;
	RDPFL_CHG_Z_DWG_PARAM*			m_pRDPflChgZDwgParam;
	TRAVDWGPARAM*					m_pTravDwgParam;
};

/////////////////////////////////////////////////////////////////////////////
#endif //_PARAMETERDATACACHE_H

