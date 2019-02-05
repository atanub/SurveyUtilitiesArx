// SurveyedStation.h : header file
//

#ifndef _SURVEYEDSTATION_H
#define _SURVEYEDSTATION_H

#include "DrawRDPlanFrTxtFileDlg.h"

void DrawRdPlanOffZFromTxtFile();//PID & Stn data file
void ModifyRdPlanOffZTxtFile();

void DrawRdPlanNEZFromTxtFile();//PID & Stn data file
void ModifyRdPlanNEZTxtFile();

//for Stn data having Co-ordinate instead of Chainage
void DrawRdPlanOffZFromTxtFileEx();//PID & Stn data file

class CSurveyedStation
{
public:
	CSurveyedStation();
	~CSurveyedStation();

	BOOL DrawFromOffElevData(const DRAWPARAMRDPLANFRTXTFILE&, CStringArray&);
	BOOL DrawFromNEZData(const DRAWPARAMRDPLANFRTXTFILE&, CStringArray&);

	BOOL DrawFromOffElevDataEx(const DRAWPARAMRDPLANFRTXTFILE&, CStringArray&);

	BOOL ReadOffElevData(const char*, const char*, CStringArray&);
	BOOL ReadNEZData(const char*, const char*, CStringArray&);
	//Special Case
	//for Stn data having Co-ordinate instead of Chainage
	BOOL ReadOffElevDataEx(const char*, const char*, CStringArray&);
	//BOOL ReadNEZDataEx(const char*, const char*, CStringArray&);

	BOOL ModifyStnNEZFile(const char*, const double&, CStringArray&) const;
	BOOL ModifyStnOffZFile(const char*, const double&, CStringArray&) const;

	static void InsCoOrdMarker(const double&, const double&, const double&, const short&, const char*, const AcGePoint3d&);
private:
	BOOL _ReadPID(const char*, CStringArray&, BOOL&);
	BOOL _ReadStnNEZ(CStdioFile&, double&, AcGePoint3dArray*, CStringArray*, CStringArray&, long&, BOOL&) const;
	BOOL _ReadStnOffElev(CStdioFile&, double&, AcGePoint3dArray*, CStringArray*, CStringArray&, long&, BOOL&) const;
	//Special Case
	//for Stn data having Co-ordinate instead of Chainage
	//BOOL _ReadStnNEZEx(CStdioFile&, AcGePoint3d&, AcGePoint3dArray*, CStringArray*, CStringArray&, long&, BOOL&) const;
	BOOL _ReadStnOffElevEx(CStdioFile&, AcGePoint3d&, AcGePoint3dArray*, CStringArray*, CStringArray&, long&, BOOL&) const;

	BOOL _FindCoOrdAtZeroOffset(const double&, CStringArray&, AcGePoint3d&, double&) const;
	AcGePoint3d _CalcPoint(const AcGePoint3d&, const double&, const double&) const;
	AcGePoint3d _CalcPointAtOffset(const AcGePoint3d&, const double&, const double&) const;
	
	BOOL _FindLeftSideAngle(const AcGePoint3d&, double&) const;

	void _Reset();

	static BOOL _FindChg(const AcGePoint3dArray&, const AcGePoint3d&, const AcGePoint3d&, double&, double&);
	void _DrawNEZStnPts(const AcGePoint3dArray&, const CStringArray*, const DRAWPARAMRDPLANFRTXTFILE&, const double&) const;
	
	static void _FindEdgePts(const AcGePoint3dArray&, int&, int&);
	static void _FindMaxDistantPt(const AcGePoint3dArray&, const int&, int&, double&);

private:
	AcGePoint3dArray	m_arrayPID;				//For both NEZ/Off-Elev Data 
	
	//
	AcDbVoidPtrArray	m_stnDataArray;			//For both NEZ/Off-Elev Data each element is 'AcGePoint3dArray'
	AcGeDoubleArray		m_fChgArray;			//For both NEZ/Off-Elev Data 
	AcDbVoidPtrArray	m_arrayLyrGroups;		//Layer names For both NEZ/Off-Elev Data each element is 'CStringArray'
	
	
	//Special Case
	//for Stn data having Co-ordinate instead of Chainage
	AcGePoint3dArray	m_arrayStnCoOrds;		//For both NEZ/Off-Elev Data <<.stn file>>

public:
	const static char* m_pszDataSegStartTag;
	const static char* m_pszDataSegEndTag;
	const static int   m_iMaxNoOfErrsReporting;
};

#endif //_SURVEYEDSTATION_H
