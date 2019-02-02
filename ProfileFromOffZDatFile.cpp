// ProfileFromOffZDatFile.cpp : implementation file
//
#include "stdafx.h"
#include "SurvUtilApp.h"
#include "Util.h"
#include "DrawUtil.h"
#include "ProfileFromOffZDatFile.h"
#include "XData.h"
#include "PflFromChgZDlg.h"
#include "XSecFrCSDEx.h"
#include "ParameterDataCache.h"

static BOOL ReadDataFile(const char*, CPflOffZRecArray&);
static void DrawAxis(const PFLOFFZDWGPARAM&, const CPflOffZRecArray&, AcGePoint3d&);
static void _DrawOrdinates(const PFLOFFZDWGPARAM&, const AcGePoint3d&, const CPflOffZRecArray&);
static BOOL CalcNInsertPointAtDist(double, CPflOffZRecArray&, const double fPrecision = 1.0E-4);
static BOOL GenPtsInEqInterval(CPflOffZRecArray&, const double&, const double fPrecision = 1.0E-4);

static const char* pszGenLyrName = "GENERATED_ORDINATES";

void DrawProfileFrOffZDatFile(const CPflOffZRecArray& arrayParam, const PFLOFFZDWGPARAM& RecParam)
{
	AcGePoint3d ptTmp;

	LAYER(RecParam.szLyrAnnotation, 1);
	LAYER(RecParam.szLyrTopSurface, 2);
	LAYER(RecParam.szLyrOrdinate, 3);
	LAYER(RecParam.szLyrAxis, 1);

	DrawAxis(RecParam, arrayParam, ptTmp);
	if(ads_usrbrk() == 1)
	{
		ads_printf("\nUser break encountered...\n");
		return;
	}
	DRAWPOINT(ptTmp, "0");
	_DrawOrdinates(RecParam, ptTmp, arrayParam);
}

void DrawProfileFrOffZDatFile()
{
	PFLOFFZDWGPARAM RecParam;
	CPflOffZRecArray arrayOfRecs;
	
	RecParam = *(CSurvUtilApp::GetDataCache()->GetPflOffZDWGParam());
	if(RecParam.fOrdAnnInterval >= 0.0)
		RecParam.fOrdAnnInterval = -1.0;

	if(!GetPflFromChgZParam(RecParam))
		return;
	CSurvUtilApp::GetDataCache()->SetPflOffZDWGParam(&RecParam);

	if(!ReadDataFile(RecParam.szDataFile, arrayOfRecs))
	{
		DeallocatePtrArray(arrayOfRecs);
		return;
	}
	if(RecParam.fOrdAnnInterval > 0.0)
	{
		GenPtsInEqInterval(arrayOfRecs, RecParam.fOrdAnnInterval);
	}

	//Draw.....
	AcGePoint3d ptGraphOrg;
	
	if(RTNORM == ads_getpoint(0L, "\nPick lower left corner of profile :", asDblArray(ptGraphOrg)))
	{
		RecParam.PtOrg = ptGraphOrg;
		DrawProfileFrOffZDatFile(arrayOfRecs, RecParam);
	}
	else
	{
		ads_printf("\nProfile drawing operation cancelled!");
	}
	//....
	DeallocatePtrArray(arrayOfRecs);
}
void DrawProfileFrOffZDatFileEx()
{
	PFLOFFZDWGPARAM RecParam;
	CPflOffZRecArray arrayOfRecs;
	
	RecParam = *(CSurvUtilApp::GetDataCache()->GetPflOffZDWGParam());
	if(RecParam.fOrdAnnInterval <= 0.0)
		RecParam.fOrdAnnInterval = 5.0;

	if(!GetPflFromChgZParam(RecParam))
		return;
	CSurvUtilApp::GetDataCache()->SetPflOffZDWGParam(&RecParam);

	if(!ReadDataFile(RecParam.szDataFile, arrayOfRecs))
	{
		DeallocatePtrArray(arrayOfRecs);
		return;
	}
	if(RecParam.fOrdAnnInterval > 0.0)
	{
		GenPtsInEqInterval(arrayOfRecs, RecParam.fOrdAnnInterval);
	}

	//Draw.....
	AcGePoint3d ptGraphOrg;
	
	if(RTNORM == ads_getpoint(0L, "\nPick lower left corner of profile :", asDblArray(ptGraphOrg)))
	{
		RecParam.PtOrg = ptGraphOrg;
		DrawProfileFrOffZDatFile(arrayOfRecs, RecParam);
	}
	else
	{
		ads_printf("\nProfile drawing operation cancelled!");
	}
	//....
	DeallocatePtrArray(arrayOfRecs);
}
static void _DrawOrdinates(const PFLOFFZDWGPARAM& RecParam, const AcGePoint3d& ptGrphOrg, const CPflOffZRecArray& arrayOfRecs)
{
	const double fPrecision = 1.0E-4;
	AcGePoint3dArray ProfTopPtsArrayOrg;
	AcGePoint3d PtStart, PtEnd, PtTxtOrg;
	double fOrdDist, fTmpVal, fLastAnnChainage;
	int i, j;
	CString strTmp;
	ads_name entLast;
	PFLOFFZREC* pRec;

	if(arrayOfRecs.GetSize() <= 0)
		return;

	//Draw Ords
	const double fMinChg = (arrayOfRecs[0])->fChg;
	for(i = 0, fLastAnnChainage = 0.0; i < arrayOfRecs.GetSize(); i++)
	{
		BOOL bSuppressBaseAnn;

		pRec = arrayOfRecs[i];
		const double fRem = fabs((RecParam.fOrdAnnInterval > 0.0) ? fmod(pRec->fChg, RecParam.fOrdAnnInterval) : 0.0);
		const BOOL bAnnotateThisOrd = (((i + 1) == arrayOfRecs.GetSize()) || (i == 0) || (RecParam.fOrdAnnInterval <= 0.0) || ((RecParam.fOrdAnnInterval > 0.0) && ((fRem <= fPrecision) || (fabs(fRem - RecParam.fOrdAnnInterval) <= fPrecision))));

		if(RecParam.fOrdAnnInterval > 0.0)
		{
			bSuppressBaseAnn = !bAnnotateThisOrd;
		}
		else
		{
			bSuppressBaseAnn = (pRec->iBitCode & 8);
		}
		//
		fOrdDist = (pRec->fChg - fMinChg) * RecParam.dXScale;
		ads_printf("\rPlease wait...Drawing ordinate at chainage %f (#%d/%d)...", pRec->fChg, (i+1), arrayOfRecs.GetSize());
		//
		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break encountered...Total %d# of ordinates drawn!\n", i);
			return;
		}

		//Elev(Existing)
		PtTxtOrg.x = ptGrphOrg.x; PtTxtOrg.y = ptGrphOrg.y;
		PtTxtOrg.x  += fOrdDist;
		PtTxtOrg.y  -= (0.5 * RecParam.dRowHt);
		if(!bSuppressBaseAnn)
		{
			TEXTMID(PtTxtOrg, RTOS(pRec->fElev), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
			if(RTNORM != ads_entlast (entLast))
			{
				ads_printf("\nERROR: Fatal error!");
				return;
			}
			if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszElevTag, pRec->fElev))
			{
				ads_printf("\nERROR: Fatal error!");
				return;
			}
		}
		//Chainage
		PtTxtOrg.x = ptGrphOrg.x; PtTxtOrg.y = ptGrphOrg.y;
		PtTxtOrg.x  += fOrdDist;
		PtTxtOrg.y  -= (1.5 * RecParam.dRowHt);
		if(!bSuppressBaseAnn)
		{
			TEXTMID(PtTxtOrg, RTOS(pRec->fChg), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
			if(RTNORM != ads_entlast (entLast))
			{
				ads_printf("\nERROR: Fatal error!");
				return;
			}
			if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszChgTag, pRec->fChg))
			{
				ads_printf("\nERROR: Fatal error!");
				return;
			}
		}
		//Dist from Last Ordinate
		PtTxtOrg.x = ptGrphOrg.x; PtTxtOrg.y = ptGrphOrg.y;
		PtTxtOrg.x  += fOrdDist;
		PtTxtOrg.y  -= (2.5 * RecParam.dRowHt);
		if(i == 0)
			fTmpVal = 0.0;
		else
			fTmpVal = (pRec->fChg - fLastAnnChainage);
		
		fTmpVal = fabs(fTmpVal);
		if(!bSuppressBaseAnn)
		{
			TEXTMID(PtTxtOrg, RTOS(fTmpVal), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
		}
		{//Ord
			const double fElev = ((pRec->fElev - RecParam.dDatum) * RecParam.dYScale);
			PtStart.x = ptGrphOrg.x;
			PtStart.y = ptGrphOrg.y;
			PtStart.x  += fOrdDist;

			PtEnd = PtStart;
			PtEnd.y  += fElev;
			
			if(RecParam.fOrdAnnInterval > 0.0)
			{
				if(!bSuppressBaseAnn)
					LINE(PtStart, PtEnd, RecParam.szLyrOrdinate);//Vertical Ordinate Line 
			}
			else if(pRec->iBitCode & 1)
			{
				LINE(PtStart, PtEnd, RecParam.szLyrOrdinate);//Vertical Ordinate Line 
			}
			{//Add to Vertex Array...
				PtEnd = PtStart;
				PtEnd.y += fElev;
				ProfTopPtsArrayOrg.append(PtEnd);
			}
			//Text Block Annotation
			//******************************************************
			//*Bit Code Specification for each Line of Station data
			//*	0 = No Text/Block Annotation & Ordinate Line
			//*	1 = Only Draws Ordinate Line
			//*	2 = Only Draws Text Annotation
			//*	4 = Only Inserts the specified Blockname
			//*	8 = No Label Annotation (Off/Elev etc.)
			//******************************************************
			if(pRec->iBitCode & 2)
			{
				PtStart = PtEnd;
				PtStart.y += RecParam.dNumAnnSize;
				TEXTLEFT(PtStart, pRec->szTxt, DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
			}
			if(pRec->iBitCode & 4)
			{
				PtStart = PtEnd;
				PtStart.y += RecParam.dNumAnnSize;
				if(!IsBlockExists(pRec->szBlock))
				{
					ads_printf("\nERROR: Block \"%s\" does not exists...Insertion failed\n", pRec->szBlock);
				}
				else
				{
					BLOCK(pRec->szBlock, PtStart, RecParam.szLyrAnnotation, 0.0, RecParam.dNumAnnSize, RecParam.dNumAnnSize);
				}
			}

			if(!bSuppressBaseAnn)
			{//Draw Ticks
				for(j = 0; j <= 3; j++)
				{
					switch(j)
					{
					case 0:
						PtStart.x = ptGrphOrg.x;
						PtStart.y = ptGrphOrg.y;
						PtStart.x  += fOrdDist;
						PtStart.y  -= ((j * RecParam.dRowHt)  + (RecParam.dNumAnnSize * 0.0));

						PtEnd = PtStart;
						PtEnd.y  -= (RecParam.dNumAnnSize * 1.0);
						break;
					case 3:
						PtStart.x = ptGrphOrg.x;
						PtStart.y = ptGrphOrg.y;
						PtStart.x  += fOrdDist;
						PtStart.y  -= ((j * RecParam.dRowHt)  + (RecParam.dNumAnnSize * 0.0));

						PtEnd = PtStart;
						PtEnd.y  += (RecParam.dNumAnnSize * 1.0);
						break;
					default:
						PtStart.x = ptGrphOrg.x;
						PtStart.y = ptGrphOrg.y;
						PtStart.x  += fOrdDist;
						PtStart.y  -= ((j * RecParam.dRowHt) - (RecParam.dNumAnnSize * 0.5));

						PtEnd = PtStart;
						PtEnd.y  -= (RecParam.dNumAnnSize * 1.0);

						break;
					}
					LINE(PtStart, PtEnd, RecParam.szLyrAnnotation);
				}
			}
		}
		if(!bSuppressBaseAnn)
		{
			fLastAnnChainage = pRec->fChg;
		}
	}
	POLYLINE(ProfTopPtsArrayOrg, RecParam.szLyrTopSurface, FALSE);
	if(strlen(RecParam.szXistTopSurfAnnBlk) > 0)
	{
		CXSecStn::AnnotateTopSurface(ProfTopPtsArrayOrg, RecParam.dXistTopSurfAnnBlkScale, (RecParam.dXistTopSurfAnnBlkSpaceMin * RecParam.dXScale), RecParam.szXistTopSurfAnnBlk, RecParam.szLyrTopSurface);
	}
}
static void DrawAxis(const PFLOFFZDWGPARAM& RecParam, const CPflOffZRecArray& arrayOfRecs, AcGePoint3d& ptGraphOrg)
{
	int i, iMaxLenOfTagStrs;
	double	fLeftOfXAxisWid, dTotalXAxisLen;
	const double fTitleXtn = RecParam.dRowAnnSize * 1.5;
	const double fTxtWidMF = 1.0;
	const int iMaxDatumDigits = 7;
	AcGePoint3d PtStart, PtEnd;


	if(arrayOfRecs.GetSize() <= 0)
		return;

	fLeftOfXAxisWid = 0.0;
	iMaxLenOfTagStrs = __max (strlen(RecParam.szElevTag), strlen(RecParam.szCumDistTag));
	iMaxLenOfTagStrs = __max ((int)strlen(RecParam.szDatumTag) + iMaxDatumDigits, iMaxLenOfTagStrs);
	iMaxLenOfTagStrs = __max ((int)strlen(RecParam.szDistTag), iMaxLenOfTagStrs);

	fLeftOfXAxisWid = iMaxLenOfTagStrs * RecParam.dRowAnnSize * fTxtWidMF;
	dTotalXAxisLen = RecParam.dXScale * (arrayOfRecs[arrayOfRecs.GetSize() - 1]->fChg - arrayOfRecs[0]->fChg) + fLeftOfXAxisWid;

	//Draw Axis..Horz Lines
	for(i = 0; i <= 3; i++)
	{
		if((i == 0) || (i == 3))
		{
			PtStart.x	= RecParam.PtOrg.x;					PtStart.y	= RecParam.PtOrg.y + (i * RecParam.dRowHt);	PtStart.z	= 0.0;
			PtEnd.x		= RecParam.PtOrg.x + dTotalXAxisLen;	PtEnd.y		= RecParam.PtOrg.y + (i * RecParam.dRowHt);	PtEnd.z		= 0.0;
			PtStart.y += fTitleXtn;
			PtEnd.y += fTitleXtn;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
		}
		switch(i)
		{
		case 0:
			PtStart.x	= RecParam.PtOrg.x;					PtStart.y	= RecParam.PtOrg.y + (i * RecParam.dRowHt) + RecParam.dNumAnnSize;	PtStart.z	= 0.0;
			PtEnd.x		= RecParam.PtOrg.x + dTotalXAxisLen;	PtEnd.y		= RecParam.PtOrg.y + (i * RecParam.dRowHt) + RecParam.dNumAnnSize;	PtEnd.z		= 0.0;
			PtStart.y += fTitleXtn;
			PtEnd.y += fTitleXtn;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			break;
		case 3:
			PtStart.x	= RecParam.PtOrg.x;					PtStart.y  = RecParam.PtOrg.y	+ (i * RecParam.dRowHt) - RecParam.dNumAnnSize;	PtStart.z	= 0.0;
			PtEnd.x		= RecParam.PtOrg.x + dTotalXAxisLen;	PtEnd.y		= RecParam.PtOrg.y	+ (i * RecParam.dRowHt) - RecParam.dNumAnnSize;	PtEnd.z		= 0.0;
			PtStart.y += fTitleXtn;
			PtEnd.y += fTitleXtn;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			break;
		default:
			PtStart.x	= RecParam.PtOrg.x;					PtStart.y	= RecParam.PtOrg.y + (i * RecParam.dRowHt) - (RecParam.dNumAnnSize / 2.0);	PtStart.z = 0.0;
			PtEnd.x		= RecParam.PtOrg.x + dTotalXAxisLen;	PtEnd.y		= RecParam.PtOrg.y + (i * RecParam.dRowHt) - (RecParam.dNumAnnSize / 2.0);	PtEnd.z = 0.0;
			PtStart.y += fTitleXtn;
			PtEnd.y += fTitleXtn;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			PtStart[X]	= RecParam.PtOrg.x;					PtStart.y	= RecParam.PtOrg.y + (i * RecParam.dRowHt) + (RecParam.dNumAnnSize / 2.0);	PtStart.z = 0.0;
			PtEnd[X]	= RecParam.PtOrg.x + dTotalXAxisLen;	PtEnd.y		= RecParam.PtOrg.y + (i * RecParam.dRowHt) + (RecParam.dNumAnnSize / 2.0);	PtEnd.z = 0.0;
			PtStart.y += fTitleXtn;
			PtEnd.y += fTitleXtn;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			break;
		}

	}
	//Draw Axis..Vert Lines
	PtStart.x	= RecParam.PtOrg.x;	PtStart.y	= RecParam.PtOrg.y + (0.0 * RecParam.dRowHt);	PtStart.z = 0.0;
	PtEnd.x		= RecParam.PtOrg.x;	PtEnd.y		= RecParam.PtOrg.y + (3.0 * RecParam.dRowHt);	PtEnd.z = 0.0;
	PtStart.y += fTitleXtn;
	PtEnd.y += fTitleXtn;
	LINE(PtStart, PtEnd, RecParam.szLyrAxis);

	//Text Labels...
	//Elev 
	PtStart.x	= RecParam.PtOrg.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = RecParam.PtOrg.y	+ (2.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	PtStart.y += fTitleXtn;
	TEXTLEFT(PtStart, RecParam.szElevTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	//Chainage 
	PtStart.x	= RecParam.PtOrg.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = RecParam.PtOrg.y	+ (1.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	PtStart.y += fTitleXtn;
	TEXTLEFT(PtStart, RecParam.szDistTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	//Cum Chainage 
	PtStart.x	= RecParam.PtOrg.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = RecParam.PtOrg.y	+ (0.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	PtStart.y += fTitleXtn;
	TEXTLEFT(PtStart, RecParam.szCumDistTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	
	{//Datum 
		CString strTmp;
		
		PtStart.x	= RecParam.PtOrg.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = RecParam.PtOrg.y	+ (3.0 * RecParam.dRowHt) + (1.25 * RecParam.dRowAnnSize);	PtStart.z = 0.0;
		PtStart.y += fTitleXtn;

		strTmp.Format("%s %s", RecParam.szDatumTag, RTOS(RecParam.dDatum));
		TEXTLEFT(PtStart, strTmp, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	}
	ptGraphOrg = RecParam.PtOrg;
	ptGraphOrg.x += fLeftOfXAxisWid;
	ptGraphOrg.y += (3.0 * RecParam.dRowHt);
	ptGraphOrg.y += fTitleXtn;
}

static BOOL ReadDataFile(const char* pszFile, CPflOffZRecArray& arrayParam)
{
	if(arrayParam.GetSize() > 0)
	{
		ads_printf("\nERROR: Control should not reach here! [ProfileFromOffZDatFile.cpp->->ReadDataFile()]\n\n");
		return FALSE;
	}
	CStdioFile objFile;
	long iLineNo;
	
	if(!objFile.Open(pszFile, CFile::modeRead|CFile::typeText))
	{
		ads_printf("\nERROR: Unable to open input file \"%s\"", pszFile);
		return FALSE;
	}
	for(iLineNo = 0;;)
	{
		CStringArray strWordArray;

		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break encountered...Total %d# of data lines read!\n", iLineNo);
			return FALSE;
		}
		
		if(!ReadNextValidLine(objFile, strWordArray, iLineNo))
		{//...EOF reached 
			if(arrayParam.GetSize() <= 0)
			{
				ads_printf("\nData file '%s' doesn't contain valid data\n", pszFile);
				return FALSE;
			}
			return TRUE;
		}
		{//other lines should contain only the [station name], [Distance], [Inc. Angle]
			PFLOFFZREC Rec;
			PFLOFFZREC* pRec;
			int iNoCols;

			memset(&Rec, 0, sizeof(PFLOFFZREC));
			if(strWordArray.GetSize() < 3)
			{
				ads_printf("\nERROR: Invalid Data found at line #%d in file '%s'\nThis Line should contain following fields <<BitCode>><<Chainage>><<Elevation>>", iLineNo, pszFile);
				return FALSE;
			}
			
			Rec.iBitCode = atoi((LPCSTR)strWordArray[0]);
			if((Rec.iBitCode < 0) || (Rec.iBitCode > 15))
			{
				ads_printf("\nERROR: Invalid Bit-Code found at line #%d in file '%s'\n", iLineNo, pszFile);
				return FALSE;
			}
			//Calc #of elements required for this line of data
			if((Rec.iBitCode & 2) && (Rec.iBitCode & 4))
			{
				iNoCols = 5;
			}
			else if((Rec.iBitCode & 2) || (Rec.iBitCode & 4))
			{
				iNoCols = 4;
			}
			else
			{
				iNoCols = 3;
			}
			if(strWordArray.GetSize() < iNoCols)
			{
				ads_printf("\nERROR: Invalid Data found at line #%d in file '%s'\nInvalid # of parameters specified", iLineNo, pszFile);
				return FALSE;
			}
			//
			Rec.fChg = atof((LPCSTR)strWordArray[1]);
			Rec.fElev = atof((LPCSTR)strWordArray[2]);
			if((Rec.iBitCode & 2) && (Rec.iBitCode & 4))
			{
				if((strlen(strWordArray[3]) <= 0) || (strlen(strWordArray[3]) >= 64))
				{
					ads_printf("\nERROR: Invalid Text String found at line #%d in file '%s'\n", iLineNo, pszFile);
					return FALSE;
				}
				strcpy(Rec.szTxt, strWordArray[3]);
				if((strlen(strWordArray[4]) <= 0) || (strlen(strWordArray[4]) >= 64))
				{
					ads_printf("\nERROR: Invalid Block Name found at line #%d in file '%s'\n", iLineNo, pszFile);
					return FALSE;
				}
				strcpy(Rec.szBlock, strWordArray[4]);
			}
			else
			{
				if(Rec.iBitCode & 2)
				{
					if((strlen(strWordArray[3]) <= 0) || (strlen(strWordArray[3]) >= 64))
					{
						ads_printf("\nERROR: Invalid Text String found at line #%d in file '%s'\n", iLineNo, pszFile);
						return FALSE;
					}
					strcpy(Rec.szTxt, strWordArray[3]);
				}
				else if(Rec.iBitCode & 4)
				{
					if((strlen(strWordArray[3]) <= 0) || (strlen(strWordArray[3]) >= 64))
					{
						ads_printf("\nERROR: Invalid Block Name found at line #%d in file '%s'\n", iLineNo, pszFile);
						return FALSE;
					}
					strcpy(Rec.szBlock, strWordArray[3]);
				}
			}
			//Check chainage value with the last inserted rec...to avoid searching...
			if(arrayParam.GetSize() > 0)
			{
				if((arrayParam[arrayParam.GetSize() - 1])->fChg >= Rec.fChg)
				{
					ads_printf("\nERROR: Invalid Chainage found at line #%d in file '%s'\n", iLineNo, pszFile);
					return FALSE;
				}
			}
			//
			pRec = new PFLOFFZREC;
			*pRec = Rec;
			arrayParam.Add(pRec);
		}
	}
	return (arrayParam.GetSize() > 0);
}
//Text Block Annotation
//******************************************************
//*Bit Code Specification for each Line of Station data
//*	0 = No Text/Block Annotation & Ordinate Line
//*	1 = Only Draws Ordinate Line
//*	2 = Only Draws Text Annotation
//*	4 = Only Inserts the specified Blockname
//*	8 = No Label Annotation (Off/Elev etc.)
//******************************************************
static BOOL CalcNInsertPointAtDist(double dDistParam, CPflOffZRecArray& arrayRecs, const double fPrecision /*= 1.0E-4*/)
{
	int i;

	for(i = 1; i < arrayRecs.GetSize(); i++)
	{
		const PFLOFFZREC* pRecCur = arrayRecs[i];
		const PFLOFFZREC* pRecPrev = arrayRecs[i - 1];

		if((fabs(pRecCur->fChg - dDistParam) <= fPrecision) || (fabs(pRecPrev->fChg - dDistParam) <= fPrecision))
		{
			return TRUE;//No need to insert 
		}
		if((dDistParam > pRecPrev->fChg) && (dDistParam < pRecCur->fChg))
		{
			PFLOFFZREC* pNewRec;
			
			pNewRec = new PFLOFFZREC;
			memset(pNewRec, 0, sizeof(PFLOFFZREC));

			pNewRec->iBitCode = 1;//Draw Ordinate....
			pNewRec->fChg = dDistParam;
			pNewRec->fElev = pRecPrev->fElev + (dDistParam - pRecPrev->fChg) * (pRecCur->fElev - pRecPrev->fElev) / (pRecCur->fChg - pRecPrev->fChg);
			strcpy(pNewRec->szTxt, pszGenLyrName);
			strcpy(pNewRec->szBlock, "");
			#ifdef _DEBUG
				pNewRec->iBitCode = 3;//Draw Ordinate + Text....
				strcpy(pNewRec->szTxt, pszGenLyrName);
			#endif//_DEBUG

			arrayRecs.InsertAt(i, pNewRec);
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL GenPtsInEqInterval(CPflOffZRecArray& arrayRecs, const double& fInterval, const double fPrecision/*= 1.0E-4*/)
{
	double dCumDistEqInterval;

	if(arrayRecs.GetSize() <= 0)
		return FALSE;

	const double fTotalLength = arrayRecs[arrayRecs.GetSize() - 1]->fChg;//Last Ordinate's Chainage....
	
	dCumDistEqInterval = 0.0;
	while(dCumDistEqInterval < fTotalLength)
	{
		dCumDistEqInterval += fInterval;
		if(!CalcNInsertPointAtDist(dCumDistEqInterval, arrayRecs, fPrecision))
		{
			break;
		}
	}
	return TRUE;
}

