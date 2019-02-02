//NEZProfileBookReader.h
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef _NEZPROFILEBOOKREADER_H
#define _NEZPROFILEBOOKREADER_H

#include "ExcelReaderGen.h"
#include "excel8.h"
#include "RoadProfile.h"

class CNEZProfileBookReader : public CExcelReaderGen
{
public:
	CNEZProfileBookReader();
	virtual ~CNEZProfileBookReader();

public:
	BOOL ReadAllSheets(CStringArray*);
	
	BOOL HasRoadProfileData() const;//Road Profile
	BOOL ReadChgZPflSheet(CStringArray*);//Profile Sheet having chainage-Elevation data
	
	CRoadProfile*	GetRoadPflDataObjPtr() { return m_pRoadProfileData;}
	
	void UnInitialize(const BOOL& = FALSE);
private:	
	CRoadProfile*	m_pRoadProfileData;

private:
	static const CString	m_strConstPflXistTagStart;
	static const CString	m_strConstPflXistTagEnd;
	static const CString	m_strConstPflDsgnTagStart;
	static const CString	m_strConstPflDsgnTagEnd;

	static const CString	m_strConstRoadPflChgZSheet;//Road Profile Chainage-Z

	static const char		m_chConstMaxColChgZPfl;

};
#endif //_NEZPROFILEBOOKREADER_H
