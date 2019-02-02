// XSecFrCSD.cpp : implementation file
//
#include "stdafx.h"
#include "SurvUtilApp.h"
#include "Util.h"
#include "DrawUtil.h"
#include "XSecFrCSD.h"
#include "XSecFrCSDEx.h"
#include "XData.h"
#include "DrawCSFrCSDDlg.h"
#include "ParameterDataCache.h"

static BOOL ReadCSDFile(const CSDDWGPARAM&, CCSStnRecArray&, long&);
static BOOL ReadNextCSData(CStdioFile&, CCSDataRecArray&, double&, double&, BOOL&, long&);


static void DeallocateStnRecArray(CCSStnRecArray&);
static void DeallocateCSDataRecArray(CCSDataRecArray&);
#ifdef _DEBUG
	static void PrintStnRecArray(CCSStnRecArray&);
#endif //_DEBUG
static void FindMinMaxOffElev(const CCSDataRecArray&, const CSDDWGPARAM&, const double&, double&, double&, double&);
static BOOL GetNextSectionRange(const double&, const int&, const CCSStnRecArray&, const CSDDWGPARAM&, int&, double&, double&);


static BOOL DrawAllSections(const AcGePoint3d&, const CCSStnRecArray&, const CSDDWGPARAM&);
static BOOL DrawCSRow(const AcGePoint2d&, const int&, const int&, const CCSStnRecArray&, const CSDDWGPARAM&, const double&, const double&, const CString&, const AcGePoint3d&, int&);

static BOOL IsCSDwgPossible(const AcGePoint3d&, const double&, const double&, const AcGePoint3d&, const double&, const double&);
static BOOL IsPointWithinRect(const AcGePoint3d&, const AcGePoint3d&, const double&, const double&);

static void DrawAxis(const CSDDWGPARAM&, const AcGePoint2d&, const double&, const double&, const double&, const double&, AcGePoint2d&);
static void DrawOrdinates(const AcGePoint2d&, const CSSTNDATA*, const double&, const double&, const CSDDWGPARAM&);
//
static BOOL GenOrdsInEqInterval(const CCSStnRecArray&, const double&);
static BOOL GenOrdsInEqInterval(CCSDataRecArray&, const double&);
static BOOL SortCSDData(CCSDataRecArray&);
static BOOL InsertOrdinateIn(const double&, CCSDataRecArray&, const double fPrecision = 1.0E-4);

static const char* pszGenLyrName = "GENERATED_POINTS";

void XSecFrCSD()
{
	CSDDWGPARAM Rec;
	BOOL bFlag;
	CCSStnRecArray StnRecArray;
	long iLineNo;

	Rec = *(CSurvUtilApp::GetDataCache()->GetCSDDwgParam());
	if(!GetCSDDwgParam(Rec))
	{
		ads_printf("\nUser Termination\n");
		return;
	}
	CSurvUtilApp::GetDataCache()->SetCSDDwgParam(&Rec);
	iLineNo = 0;
	bFlag = ReadCSDFile(Rec, StnRecArray, iLineNo);
	
	#ifdef _DEBUG /////////////////////////
	//PrintStnRecArray(StnRecArray);
	#endif //_DEBUG/////////////////////////
	
	if(!bFlag)
	{
		ads_printf("\nERROR: Failed to read file %s\nPlease Check line #%d for error\n", Rec.szDataFile, iLineNo);
	}
	else
	{//Start Drawing routine calls
		AcGePoint3d ptULCornerOfSheet;

		if(RTNORM == ads_getpoint(0L, "\nPick upper left corner of starting drawing sheet :", asDblArray(ptULCornerOfSheet)))
		{//
			if(GenOrdsInEqInterval(StnRecArray, Rec.fOrdInterval))
			{
				DrawAllSections(ptULCornerOfSheet, StnRecArray, Rec);
			}
			else
			{
				ads_printf("\nERROR: Abnormal termination\n");
			}
		}
		else
		{
			ads_printf("\nERROR: User termination\n");
		}
	}
	//
	DeallocateStnRecArray(StnRecArray);
}
static BOOL ReadCSDFile(const CSDDWGPARAM& RecParam, CCSStnRecArray& ArrayParam, long& iLineNo)
{
	CStdioFile	FileInp;
	BOOL bFlag, bIsEOFReached;

	bFlag = FileInp.Open(RecParam.szDataFile, CFile::modeRead|CFile::typeText);
	if(!bFlag)
	{
		ads_printf("\nERROR: Unable to open input file %s", RecParam.szDataFile);
		return FALSE;
	}
	ads_printf("\rPlease wait... reading data file");
	for(iLineNo = 0;;)
	{
		CSSTNDATA* pNewRec;
		
		pNewRec = new CSSTNDATA;
		ads_printf("\rPlease wait... reading line no. %d", iLineNo);
		bFlag = ReadNextCSData(FileInp, pNewRec->Array, pNewRec->dChainage, pNewRec->dDatum, bIsEOFReached, iLineNo);
		if(!bFlag)
		{
			delete pNewRec;
			return FALSE;
		}
		if(bIsEOFReached)
		{
			delete pNewRec;
			return TRUE;
		}
		if((pNewRec->dChainage >= RecParam.dStartChg) && (pNewRec->dChainage <= RecParam.dEndChg))
		{
			ArrayParam.Add(pNewRec);
		}
		else
		{
			DeallocateCSDataRecArray(pNewRec->Array);
			delete pNewRec;
		}
		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break encountered\nTotal %d# of line(s) read\n", iLineNo);
			return FALSE;
		}
	}
	return (ArrayParam.GetSize() > 0);
}
static BOOL ReadNextCSData(CStdioFile& File, CCSDataRecArray& ArrayParam, double& fChg, double& fDatum, BOOL& bIsEOF, long& iLineNo)
{
	if(ArrayParam.GetSize() != 0)
	{
		ads_printf("\nERROR: Invalid array parameter passed!! Array is not empty\n");
		return FALSE;//ERROR
	}
			
	for(bIsEOF = FALSE;;)
	{
		CStringArray strWordArray;
		
		if(!ReadNextValidLine(File, strWordArray, iLineNo))
		{//EOF reached...
			if(ArrayParam.GetSize() != 0)
			{//Chk whether this data set is OK or not ....may be an error
				CSDREC* pRecTmp;
				
				pRecTmp = ArrayParam[ArrayParam.GetSize() - 1];
				if(pRecTmp->lBit != 10)
				{
					DeallocateCSDataRecArray(ArrayParam);
					return FALSE;//ERROR
				}
				DeallocateCSDataRecArray(ArrayParam);
				return TRUE;//ERROR
			}
			bIsEOF = TRUE;
			return TRUE;
		}
		{//
			int iBitCode;

			iBitCode = atoi(strWordArray[0]);
			if(iBitCode == 10)
			{//End of this Station Data
				return (ArrayParam.GetSize() > 0);
			}
			if(iBitCode == 9)
			{//Start of this Station Data
				if(ArrayParam.GetSize() != 0)
				{
					DeallocateCSDataRecArray(ArrayParam);//for Safety 
					return FALSE;
				}
				if(!CXSecStn::ParseCSDStartLine(strWordArray, fChg, fDatum))
				{
					CString strTmp;

					strTmp.Format("Invalid no of parameters found at line #%d (the start of data segment)\nExpected format is <<Bit Code = 9>><<Chainage>><<Datum Level>>", iLineNo);
					AfxMessageBox(strTmp, MB_ICONSTOP);
					DeallocateCSDataRecArray(ArrayParam);//for Safety 
					return FALSE;
				}
			}
			else
			{//Collect lines Data 
				CSDREC* pNewRec;

				pNewRec = new CSDREC;
				if(!CXSecStn::ParseCSDLineInput(strWordArray, pNewRec))
				{
					delete pNewRec;
					DeallocateCSDataRecArray(ArrayParam);
					return FALSE;//ERROR
				}
				ArrayParam.Add(pNewRec);
			}
		}
	}
	return (ArrayParam.GetSize() > 0);
}
static void DeallocateStnRecArray(CCSStnRecArray& Array)
{
	int i;
	CSSTNDATA* pRec;

	for(i = 0; i < Array.GetSize(); i++)
	{
		pRec = Array[i];
		DeallocateCSDataRecArray(pRec->Array);
		delete pRec;
	}
	if(Array.GetSize() > 0)
		Array.RemoveAll();
}
static void DeallocateCSDataRecArray(CCSDataRecArray& Array)
{
	int i;
	CSDREC* pRec;

	for(i = 0; i < Array.GetSize(); i++)
	{
		pRec = Array[i];
		delete pRec;
	}
	if(Array.GetSize() > 0)
		Array.RemoveAll();
}
#ifdef _DEBUG
	static void PrintStnRecArray(CCSStnRecArray& Array)
	{
		int i, j;
		CSSTNDATA* pRec;
		CSDREC* pRecSub;

		for(i = 0; i < Array.GetSize(); i++)
		{
			pRec = Array[i];
			ads_printf("\nData in Chg = %f, Datum = %f", pRec->dChainage, pRec->dDatum);
			for(j = 0; j < pRec->Array.GetSize(); j++)
			{
				pRecSub = pRec->Array[j];
				ads_printf("\nBit = %d, Off = %f, Elev = %f, Txt = %s, Blk = %s", pRecSub->lBit, pRecSub->dOffset, pRecSub->dElev, pRecSub->strTxt, pRecSub->strBlk);
			}
		}
	}
#endif //_DEBUG

static void FindMinMaxOffElev(const CCSDataRecArray& Array, const CSDDWGPARAM& RecParam, const double& fDatum, double& fMinOff, double& fMaxOff, double& fMaxElevIncOrdAnnTxt)
{
	const double fTxtWidMF = 1.0;
	int i;
	CSDREC* pRec;
	double fTemp;

	fMinOff = 0.0;
	fMaxOff = 0.0;
	fMaxElevIncOrdAnnTxt = 0.0;
	if(Array.GetSize() < 1)
		return;

	pRec = Array[0];
	fMinOff = pRec->dOffset;
	fMaxOff = pRec->dOffset;

	fMaxElevIncOrdAnnTxt = (pRec->dElev - fDatum) * RecParam.dYScale + (strlen(pRec->strTxt) + 1) * RecParam.dNumAnnSize * fTxtWidMF;
	for(i = 1; i < Array.GetSize(); i++)
	{
		pRec = Array[i];

		if(pRec->dOffset < fMinOff)
			fMinOff = pRec->dOffset;
		if(pRec->dOffset > fMaxOff)
			fMaxOff = pRec->dOffset;
	
		fTemp = (pRec->dElev - fDatum) * RecParam.dYScale + (strlen(pRec->strTxt) + 1) * RecParam.dNumAnnSize * fTxtWidMF;
		if(fTemp > fMaxElevIncOrdAnnTxt)
			fMaxElevIncOrdAnnTxt = fTemp;
	}
}

static BOOL IsCSDwgPossible(const AcGePoint3d& ptLROfCS, const double& fCSWid, const double& fCSHt, const AcGePoint3d& ptULOfNPlate, const double& fNPWid, const double& fNPHt)
{
	AcGePoint3d ptLLCOfNPlate;

	ptLLCOfNPlate.x = ptULOfNPlate.x;
	ptLLCOfNPlate.y = ptULOfNPlate.y - fNPHt;
	ptLLCOfNPlate.z = 0.0;
	{//Chk whether any vertex of CS Dwg extent lies in the NPlate Rectangle or not....
		AcGePoint3d ptTmp;

		//LLC
		ptTmp = ptLROfCS;
		ptTmp.x = ptLROfCS.x - fCSWid;
		if(IsPointWithinRect(ptTmp, ptLLCOfNPlate, fNPWid, fNPHt))
			return FALSE;
		//LRC
		ptTmp = ptLROfCS;
		if(IsPointWithinRect(ptTmp, ptLLCOfNPlate, fNPWid, fNPHt))
			return FALSE;
		//URC
		ptTmp = ptLROfCS;
		ptTmp.y = ptLROfCS.y + fCSHt;
		if(IsPointWithinRect(ptTmp, ptLLCOfNPlate, fNPWid, fNPHt))
			return FALSE;
		//ULC
		ptTmp = ptLROfCS;
		ptTmp.y = ptLROfCS.y + fCSHt;
		ptTmp.x = ptLROfCS.x - fCSWid;
		if(IsPointWithinRect(ptTmp, ptLLCOfNPlate, fNPWid, fNPHt))
			return FALSE;
	}
	{//Chk whether any vertex of NPlate lies in the CS Dwg extent Rectangle or not....
		AcGePoint3d ptTmp, ptLLCOfCS;

		ptLLCOfCS = ptLROfCS;
		ptLLCOfCS.x = ptLROfCS.x - fCSWid;

		//LLC
		ptTmp = ptLLCOfNPlate;
		if(IsPointWithinRect(ptTmp, ptLLCOfCS, fCSWid, fCSHt))
			return FALSE;
		//LRC
		ptTmp = ptLLCOfNPlate;
		ptTmp.x += fNPWid;
		if(IsPointWithinRect(ptTmp, ptLLCOfCS, fCSWid, fCSHt))
			return FALSE;
		//URC
		ptTmp = ptLLCOfNPlate;
		ptTmp.x += fNPWid;
		ptTmp.y += fNPHt;
		if(IsPointWithinRect(ptTmp, ptLLCOfCS, fCSWid, fCSHt))
			return FALSE;
		//ULC
		ptTmp = ptLLCOfNPlate;
		ptTmp.y += fNPHt;
		if(IsPointWithinRect(ptTmp, ptLLCOfCS, fCSWid, fCSHt))
			return FALSE;
	}
	return TRUE;
}
static BOOL IsPointWithinRect(const AcGePoint3d& ptParam, const AcGePoint3d& ptLLCOfRect, const double& fWid, const double& fHt)
{
	BOOL bFlag;
	
	bFlag = (ptParam.x >= ptLLCOfRect.x) && (ptParam.x <= ptLLCOfRect.x + fWid);
	bFlag = bFlag && (ptParam.y >= ptLLCOfRect.y) && (ptParam.y <= ptLLCOfRect.y + fHt);
	return bFlag;
}
///////
static void DrawAxis(const CSDDWGPARAM& RecParam, const AcGePoint2d& ptLLCOfExtent, const double& fDatum, const double& fMinOff, const double& fMaxOff, const double& fLeftOfXAxisWid, AcGePoint2d& ptGraphOrg)
{
	int i;
	double	dTotalXAxisLen;
	const double fTitleXtn = RecParam.dCSLabelAnnSize * 1.5;
	AcGePoint3d PtStart, PtEnd;


	dTotalXAxisLen = RecParam.dXScale * (fabs(fMinOff) + fMaxOff) + fLeftOfXAxisWid;

	//Draw Axis..Horz Lines
	for(i = 0; i <= 2; i++)
	{
		if((i == 0) || (i == 2))
		{
			PtStart.x	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * RecParam.dRowHt);	PtStart.z	= 0.0;
			PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * RecParam.dRowHt);	PtEnd.z		= 0.0;
			PtStart.y += fTitleXtn;
			PtEnd.y += fTitleXtn;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
		}
		switch(i)
		{
		case 0:
			PtStart.x	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * RecParam.dRowHt) + RecParam.dNumAnnSize;	PtStart.z	= 0.0;
			PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * RecParam.dRowHt) + RecParam.dNumAnnSize;	PtEnd.z		= 0.0;
			PtStart.y += fTitleXtn;
			PtEnd.y += fTitleXtn;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			break;
		case 2:
			PtStart.x	= ptLLCOfExtent.x;					PtStart.y  = ptLLCOfExtent.y	+ (i * RecParam.dRowHt) - RecParam.dNumAnnSize;	PtStart.z	= 0.0;
			PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y	+ (i * RecParam.dRowHt) - RecParam.dNumAnnSize;	PtEnd.z		= 0.0;
			PtStart.y += fTitleXtn;
			PtEnd.y += fTitleXtn;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			break;
		default:
			PtStart.x	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * RecParam.dRowHt) - (RecParam.dNumAnnSize / 2.0);	PtStart.z = 0.0;
			PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * RecParam.dRowHt) - (RecParam.dNumAnnSize / 2.0);	PtEnd.z = 0.0;
			PtStart.y += fTitleXtn;
			PtEnd.y += fTitleXtn;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			PtStart[X]	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * RecParam.dRowHt) + (RecParam.dNumAnnSize / 2.0);	PtStart.z = 0.0;
			PtEnd[X]	= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * RecParam.dRowHt) + (RecParam.dNumAnnSize / 2.0);	PtEnd.z = 0.0;
			PtStart.y += fTitleXtn;
			PtEnd.y += fTitleXtn;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			break;
		}

	}
	//Draw Axis..Vert Lines
	PtStart.x	= ptLLCOfExtent.x;	PtStart.y	= ptLLCOfExtent.y + (0.0 * RecParam.dRowHt);	PtStart.z = 0.0;
	PtEnd.x		= ptLLCOfExtent.x;	PtEnd.y		= ptLLCOfExtent.y + (2.0 * RecParam.dRowHt);	PtEnd.z = 0.0;
	PtStart.y += fTitleXtn;
	PtEnd.y += fTitleXtn;
	LINE(PtStart, PtEnd, RecParam.szLyrAxis);

	//Text Labels...
	//Elev 
	PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (1.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	PtStart.y += fTitleXtn;
	TEXTLEFT(PtStart, RecParam.szElevTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	//Offset 
	PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (0.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	PtStart.y += fTitleXtn;
	TEXTLEFT(PtStart, RecParam.szOffsetTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	
	{//Datum 
		CString strTmp;
		
		PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (2.0 * RecParam.dRowHt) + (1.25 * RecParam.dRowAnnSize);	PtStart.z = 0.0;
		
		strTmp.Format("%s %s", RecParam.szDatumTag, RTOS(fDatum));

		PtStart.y += fTitleXtn;
		TEXTLEFT(PtStart, strTmp, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	}
	ptGraphOrg = ptLLCOfExtent;
	ptGraphOrg.x += fLeftOfXAxisWid;
	ptGraphOrg.y += (2.0 * RecParam.dRowHt);
	ptGraphOrg.y += fTitleXtn;
}

static void DrawOrdinates(const AcGePoint2d& ptGrphOrg, const CSSTNDATA* pStnRec, const double& fMinOff, const double& fMaxOff, const CSDDWGPARAM& RecParam)
{
	AcGePoint3dArray ProfTopPtsArray;
	AcGePoint3d PtStart, PtEnd, PtTxtOrg;
	double fOrdDist, fElev, fXAxisLen;
	int i, j;
	ads_name entLast;
	CSDREC* pRec;
	
	fXAxisLen = fabs(fMinOff) + fMaxOff;
	//Draw Ords
	for(i = 0; i < pStnRec->Array.GetSize(); i++)
	{
		pRec = pStnRec->Array[i];
		const double fPrecision = 1.0E-4;
		const double fRem = fabs((RecParam.fOrdInterval > 0.0) ? fmod(pRec->dOffset, RecParam.fOrdInterval) : 0.0);
		const BOOL bAnnotateThisOrd = (((i + 1) == pStnRec->Array.GetSize()) || (i == 0) || (RecParam.fOrdInterval <= 0.0) || ((RecParam.fOrdInterval > 0.0) && ((fRem <= fPrecision) || (fabs(fRem - RecParam.fOrdInterval) <= fPrecision))));

		fOrdDist = fXAxisLen - fMaxOff + pRec->dOffset;
		fOrdDist *= RecParam.dXScale;
		fElev = (pRec->dElev - pStnRec->dDatum) * RecParam.dYScale;

		//Elev 
		PtTxtOrg.x = ptGrphOrg.x; PtTxtOrg.y = ptGrphOrg.y;
		PtTxtOrg.x  += fOrdDist;
		PtTxtOrg.y  -= (0.5 * RecParam.dRowHt);
		if(bAnnotateThisOrd)
		{
			TEXTMID(PtTxtOrg, RTOS(pRec->dElev), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
			if(RTNORM != ads_entlast (entLast))
			{
				ads_printf("\nERROR: Fatal error!");
				return;
			}
			if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszElevTag, pRec->dElev))
			{
				ads_printf("\nERROR: Fatal error!");
				return;
			}
		}
		//Offset
		PtTxtOrg.x = ptGrphOrg.x; PtTxtOrg.y = ptGrphOrg.y;
		PtTxtOrg.x  += fOrdDist;
		PtTxtOrg.y  -= (1.5 * RecParam.dRowHt);
		if(bAnnotateThisOrd)
		{
			TEXTMID(PtTxtOrg, RTOS(pRec->dOffset), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
			if(RTNORM != ads_entlast (entLast))
			{
				ads_printf("\nERROR: Fatal error!");
				return;
			}
		}

		{//Ord
			PtStart.x = ptGrphOrg.x;
			PtStart.y = ptGrphOrg.y;
			PtStart.x  += fOrdDist;

			PtEnd = PtStart;
			PtEnd.y  += fElev;
			
			if(bAnnotateThisOrd)
			{
				if(pRec->lBit & 1)
				{
					LINE(PtStart, PtEnd, RecParam.szLyrOrdinate);
				}
			}
			//Add to Vertex Array...
			ProfTopPtsArray.append(PtEnd);
			//Text Block Annotation
			//******************************************************
			//*Bit Code Specification for each Line of Station data
			//*	0 = No Text/Block Annotation & Ordinate Line
			//*	1 = Only Draws Ordinate Line
			//*	2 = Only Draws Text Annotation
			//*	4 = Only Inserts the specified Blockname
			//******************************************************
			if(pRec->lBit & 2)
			{
				PtStart = PtEnd;
				PtStart.y += RecParam.dNumAnnSize;
				TEXTLEFT(PtStart, pRec->strTxt, DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
			}
			if(pRec->lBit & 4)
			{
				PtStart = PtEnd;
				PtStart.y += RecParam.dNumAnnSize;
				if(!IsBlockExists(pRec->strBlk))
				{
					ads_printf("\nERROR: Block \"%s\" does not exists...Insertion failed\n", pRec->strBlk);
				}
				else
				{
					BLOCK(pRec->strBlk, PtStart, RecParam.szLyrAnnotation, 0.0, RecParam.dNumAnnSize, RecParam.dNumAnnSize);
				}
			}

			//Draw Ticks
			for(j = 0; ((j <= 2) && (bAnnotateThisOrd)); j++)
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
				case 2:
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
	POLYLINE(ProfTopPtsArray, RecParam.szLyrTopSurface, FALSE);
	if(strlen(RecParam.szXistTopSurfAnnBlk) > 0)
	{
		CXSecStn::AnnotateTopSurface(ProfTopPtsArray, RecParam.dXistTopSurfAnnBlkScale, (RecParam.dXistTopSurfAnnBlkSpaceMin * RecParam.dXScale), RecParam.szXistTopSurfAnnBlk, RecParam.szLyrTopSurface);
	}
}

static BOOL DrawAllSections(const AcGePoint3d& ptULCornerOfSheet, const CCSStnRecArray& Array, const CSDDWGPARAM& RecParam)
{
	const double fSheetShiftFac = 0.1;
	const CString strSheetLyr = "DRAWING_SHEET";
	const CString strCSGridLyr = "CROSS_SEXN_GRID";
	int i, iEndIndex;
	double fSheetOrgX, fCumY, fSheetOrgY, fTotalHeight, fLeftOfXAxisWid;
	AcGePoint3d ptULOfNPlate;
	
	LAYER(strCSGridLyr, 1);
	LAYER(strSheetLyr, 2);

	LAYER(RecParam.szLyrAnnotation, 7);
	LAYER(RecParam.szLyrTopSurface, 1);
	LAYER(RecParam.szLyrOrdinate, 3);
	LAYER(RecParam.szLyrAxis, 6);

	fSheetOrgX = ptULCornerOfSheet.x;
	fSheetOrgY = ptULCornerOfSheet.y;

	//Sheet Border
	CXSecStn::RectangleUL(AcGePoint3d(fSheetOrgX, fSheetOrgY, 0.0), RecParam.dSheetWid, RecParam.dSheetHeight, strSheetLyr);
	{//Name Plate Border
		ptULOfNPlate.x = fSheetOrgX + RecParam.dSheetWid - RecParam.dNPlateWid;
		ptULOfNPlate.y = fSheetOrgY - RecParam.dSheetHeight + RecParam.dNPlateHeight;
		ptULOfNPlate.z = 0.0;
		CXSecStn::RectangleUL(ptULOfNPlate, RecParam.dNPlateWid, RecParam.dNPlateHeight, strSheetLyr);
	}
	ads_printf("\nPlease wait...Drawing Cross Section(s)...\n");
	for(i = 0, fCumY = 0.0; i < Array.GetSize(); )
	{
		int iLastDrawnCS;

		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break encountered\nTotal %d# of cross section(s) drawn\n", i);
			return FALSE;
		}

		if(!GetNextSectionRange(fCumY, i, Array, RecParam, iEndIndex, fLeftOfXAxisWid, fTotalHeight))
			return FALSE;
		if(!DrawCSRow(AcGePoint2d(fSheetOrgX, fSheetOrgY - fTotalHeight), i, iEndIndex, Array, RecParam, fLeftOfXAxisWid, fTotalHeight, strCSGridLyr, ptULOfNPlate, iLastDrawnCS))
			return FALSE;
		
		fCumY += fTotalHeight;
		fSheetOrgY -= fTotalHeight;
		if(fCumY >= RecParam.dSheetHeight)
		{
			fCumY = 0.0;
			fSheetOrgX += RecParam.dSheetWid + (RecParam.dSheetWid * fSheetShiftFac);
			fSheetOrgY = ptULCornerOfSheet.y;
			if(i < Array.GetSize())
			{
				//Sheet Border
				CXSecStn::RectangleUL(AcGePoint3d(fSheetOrgX, fSheetOrgY, 0.0), RecParam.dSheetWid, RecParam.dSheetHeight, strSheetLyr);
				{//Name Plate Border
					ptULOfNPlate.x = fSheetOrgX + RecParam.dSheetWid - RecParam.dNPlateWid;
					ptULOfNPlate.y = fSheetOrgY - RecParam.dSheetHeight + RecParam.dNPlateHeight;
					ptULOfNPlate.z = 0.0;
					CXSecStn::RectangleUL(ptULOfNPlate, RecParam.dNPlateWid, RecParam.dNPlateHeight, strSheetLyr);
				}
			}
		}
		i = iLastDrawnCS;
	}
	ads_printf("\nDone !\n");
	return TRUE;
}
static BOOL GetNextSectionRange(const double& fCurRowDepth, const int& iStartIndex, const CCSStnRecArray& Array, const CSDDWGPARAM& RecParam, int& iEndIndex, double& fLeftOfXAxisWid, double& fMaxHeight)
{
	const double fTitleXtn = RecParam.dCSLabelAnnSize * 1.5;
	const double fTxtWidMF = 1.0;
	const double fCell2CellDist = RecParam.dNumAnnSize * 4.0;
	const int iMaxDatumDigits = 7;
	int i, iMaxLenOfTagStrs;
	double fBotOfXAxisLen, fCSWid, fCSHt, fCumWid, fMaxElevIncOrdAnnTxtTmp, fMinOff, fMaxOff, fCurShtWid, fPrevMaxHt;
	
	if(iStartIndex >= Array.GetSize())
		return FALSE;
	
	iMaxLenOfTagStrs = (int)__max (strlen(RecParam.szElevTag), strlen(RecParam.szOffsetTag));
	iMaxLenOfTagStrs = (int)__max ((int)strlen(RecParam.szDatumTag) + iMaxDatumDigits, iMaxLenOfTagStrs);
	fLeftOfXAxisWid = iMaxLenOfTagStrs * RecParam.dRowAnnSize * fTxtWidMF;
	fBotOfXAxisLen = RecParam.dRowHt * 2.0 + fTitleXtn;

	fCumWid = fPrevMaxHt = fMaxHeight = 0.0;
	for(i = iStartIndex, iEndIndex = iStartIndex; i < Array.GetSize(); i++)
	{
		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break encountered\n");
			return FALSE;
		}
		FindMinMaxOffElev(Array[i]->Array, RecParam, Array[i]->dDatum, fMinOff, fMaxOff, fMaxElevIncOrdAnnTxtTmp);
		
		fCSWid = (fabs(fMinOff) + fMaxOff) * RecParam.dXScale + fLeftOfXAxisWid + fCell2CellDist;
		fCumWid += fCSWid;
		
		if(fMaxElevIncOrdAnnTxtTmp >= fMaxHeight)
		{
			fPrevMaxHt = fMaxHeight;
			fMaxHeight = fMaxElevIncOrdAnnTxtTmp;
		}

		fCSHt = fMaxHeight + fBotOfXAxisLen + fCell2CellDist;
		
		//Check sheet width & height ......
		if(fCSWid >= RecParam.dSheetWid)
		{
			ads_printf("\nERROR: Failed to draw cross section\nPossible cause: Sheet width is too small");
			return FALSE;
		}
		if(fCSHt >= RecParam.dSheetHeight)
		{
			ads_printf("\nERROR: Failed to draw cross section\nPossible cause: Sheet height is too small");
			return FALSE;
		}

		//
		if((RecParam.dSheetHeight - (fCurRowDepth + fCSHt)) <= RecParam.dNPlateHeight)
		{
			fCurShtWid = RecParam.dSheetWid - RecParam.dNPlateWid;
		}
		else
		{
			fCurShtWid = RecParam.dSheetWid;
		}
		if((RecParam.dSheetHeight - (fCurRowDepth + fCSHt)) < 0.0)
		{//Out of Range
			iEndIndex = i - 1;
			if((fMaxElevIncOrdAnnTxtTmp == fMaxHeight) && (fPrevMaxHt > 0.0))
				fMaxHeight = fPrevMaxHt;
			fMaxHeight += fBotOfXAxisLen;
			fMaxHeight += fCell2CellDist;
			return TRUE;
		}

		if(fCumWid > fCurShtWid)
		{
			iEndIndex = i - 1;
			if((fMaxElevIncOrdAnnTxtTmp == fMaxHeight) && (fPrevMaxHt > 0.0))
				fMaxHeight = fPrevMaxHt;
			fMaxHeight += fBotOfXAxisLen;
			fMaxHeight += fCell2CellDist;
			return TRUE;
		}
		else
		{
			if(fCumWid == fCurShtWid)
			{
				iEndIndex = i;
				fMaxHeight += fBotOfXAxisLen;
				fMaxHeight += fCell2CellDist;
				return TRUE;
			}
		}
	}
	iEndIndex = Array.GetSize() - 1;
	fMaxHeight += fBotOfXAxisLen;
	fMaxHeight += fCell2CellDist;
	return TRUE;
}
static BOOL DrawCSRow(const AcGePoint2d& ptLLCStart, const int& iStartIndex, const int& iEndIndex, const CCSStnRecArray& Array, const CSDDWGPARAM& RecParam, const double& fLeftOfXAxisWid, const double& fCSHeight, const CString& strCSGridLyr, const AcGePoint3d& ptULOfNPlate, int& iLastDrawnCS)
{
	const double fTitleXtn = RecParam.dCSLabelAnnSize * 1.5;
	const double fTxtWidMF = 1.0;
	const double fCell2CellDist = RecParam.dNumAnnSize * 4.0;
	int i;
	double fCSWid, fMaxElevIncOrdAnnTxtTmp, fMinOff, fMaxOff;
	AcGePoint2d ptGrphOrg, ptLLCOfCS;
	AcGePoint3d ptLROfCS;
	
	if((iStartIndex >= Array.GetSize()) || (iEndIndex >= Array.GetSize()))
		return FALSE;

	//Drawing .....
	for(i = iStartIndex, iLastDrawnCS = iStartIndex, ptLLCOfCS = ptLLCStart; i <= iEndIndex; i++)
	{
		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break encountered\n");
			return FALSE;
		}
		FindMinMaxOffElev(Array[i]->Array, RecParam, Array[i]->dDatum, fMinOff, fMaxOff, fMaxElevIncOrdAnnTxtTmp);
		fCSWid = (fabs(fMinOff) + fMaxOff) * RecParam.dXScale + fLeftOfXAxisWid + fCell2CellDist;

		ptLROfCS.x = ptLLCOfCS.x;
		ptLROfCS.y = ptLLCOfCS.y;
		ptLROfCS.x += fCSWid;
		
		if(fCSWid >= RecParam.dSheetWid)
		{
			ads_printf("\nERROR: Failed to draw cross section\nPossible cause: Sheet width is too small");
			return FALSE;
		}
		if(fCSHeight >= RecParam.dSheetHeight)
		{
			ads_printf("\nERROR: Failed to draw cross section\nPossible cause: Sheet height is too small");
			return FALSE;
		}
		if(!IsCSDwgPossible(ptLROfCS, fCSWid, fCSHeight, ptULOfNPlate, RecParam.dNPlateWid, RecParam.dNPlateHeight))
		{
			ads_printf("\nFatal error : DrawCSRow\n");
			return TRUE;
		}

		ads_printf("\rPlease wait...Drawing Cross Section at Chainage %f (#%d/%d)", Array[i]->dChainage, i+1, Array.GetSize());
		DrawAxis(RecParam, ptLLCOfCS, Array[i]->dDatum, fMinOff, fMaxOff, fLeftOfXAxisWid, ptGrphOrg);
		DrawOrdinates(ptGrphOrg, Array[i], fMinOff, fMaxOff, RecParam);
		{//XSec Label
			AcGePoint3d ptTmp;
			CString strTmp;
			const double fTitleXtn = RecParam.dCSLabelAnnSize * 1.5;

			strTmp.Format("%s %s", RecParam.szXSecLabelTag, RTOS(Array[i]->dChainage));

			ptTmp.x = ptLLCOfCS.x;
			ptTmp.y = ptLLCOfCS.y;
			ptTmp.x += fCSWid / 2.0;
			ptTmp.y += (fTitleXtn / 2.0);
			TEXTMID(ptTmp, strTmp, 0.0, RecParam.dCSLabelAnnSize, RecParam.szLyrAxis);
		}
		CXSecStn::RectangleLR(AcGePoint3d(ptLLCOfCS.x+fCSWid, ptLLCOfCS.y, 0.0), fCSWid, fCSHeight, strCSGridLyr);
		ptLLCOfCS.x += fCSWid;
		iLastDrawnCS++;
	}
	return TRUE;
}

static BOOL SortCSDData(CCSDataRecArray& arrayCSD)
{
	CCSDataRecArray arraySorted;

	while(arrayCSD.GetSize() > 0)
	{
		int i, iIndex;
		const CSDREC* pRecMin;

		for(i = 0, iIndex = -1; i < arrayCSD.GetSize(); i++)
		{
			const CSDREC* pRec = arrayCSD[i];
			if(ads_usrbrk() == 1)
			{
				ads_printf("\nUser break encountered !\n");
				return FALSE;
			}
			if(i == 0)
			{
				pRecMin = arrayCSD[i];
				iIndex = i;
				continue;
			}
			if(pRec->dOffset < pRecMin->dOffset)
			{
				pRecMin = pRec;
				iIndex = i;
			}
		}
		if(iIndex >= 0)
		{
			assert(0L != pRecMin);
			arrayCSD.RemoveAt(iIndex);
			arraySorted.Add((CSDREC*)pRecMin);
		}
	}
	assert(arrayCSD.GetSize() == 0);
	arrayCSD.Append(arraySorted);
	return TRUE;
}
static BOOL GenOrdsInEqInterval(const CCSStnRecArray& XSecArray, const double& fOrdInterval)
{
	int i;

	if(fOrdInterval <= 0.0)
		return TRUE;//generation not required!!

	for(i = 0; i < XSecArray.GetSize(); i++)
	{
		CSSTNDATA* pRec;
		
		pRec = XSecArray[i];

		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break encountered !\n");
			return FALSE;
		}
		if(!SortCSDData(pRec->Array))
		{
			return FALSE;
		}
		if(!GenOrdsInEqInterval(pRec->Array, fOrdInterval))
		{
			ads_printf("\nFATAL ERROR: Ordinate generation failed at CSD#%d\n", (i+1));
			return FALSE;
		}
	}
	return TRUE;
}

static BOOL GenOrdsInEqInterval(CCSDataRecArray& arrayCSD, const double& fOrdInterval)
{
	double fCumDist, fOffsetLeftMost;

	if(fOrdInterval <= 0.0)
	{
		return FALSE;
	}
	if(arrayCSD.GetSize() < 2)
	{
		return FALSE;
	}
	{//Refine Extreme left ordinate offset...
		double fCumDist;

		fCumDist = 0.0;
		while(TRUE)
		{
			if(fCumDist <= arrayCSD[0]->dOffset)
			{
				if(fCumDist == arrayCSD[0]->dOffset)
					fOffsetLeftMost = fCumDist;
				else
					fOffsetLeftMost = fCumDist + fOrdInterval;
				
				break;
			}
			fCumDist -= fOrdInterval;
		}
	}
	const double fOffsetRiteMost = arrayCSD[arrayCSD.GetSize() - 1]->dOffset;
	fCumDist = fOffsetLeftMost;
	while(fCumDist <= fOffsetRiteMost)
	{
		if(!InsertOrdinateIn(fCumDist, arrayCSD))
		{
			return FALSE;
		}
		fCumDist += fOrdInterval;
	}
	return TRUE;
}
static BOOL InsertOrdinateIn(const double& fOffset, CCSDataRecArray& arrayCSD, const double fPrecision/* = 1.0E-4*/)
{
	int i;

	for(i = 1; i < arrayCSD.GetSize(); i++)
	{
		const CSDREC* pRecStart = arrayCSD[i - 1];
		const CSDREC* pRecEnd = arrayCSD[i];

		if((fabs(pRecStart->dOffset - fOffset) <= fPrecision) || (fabs(pRecEnd->dOffset - fOffset) <= fPrecision))
		{
			return TRUE;//No need to insert 
		}
		if((fOffset > pRecStart->dOffset) && (fOffset < pRecEnd->dOffset))
		{
			const double fElev = pRecStart->dElev + (fOffset - pRecStart->dOffset) * (pRecEnd->dElev - pRecStart->dElev) / (pRecEnd->dOffset - pRecStart->dOffset);
			CSDREC* pRecNew;
			
			pRecNew = new CSDREC;
			//Text Block Annotation
			//******************************************************
			//*Bit Code Specification for each Line of Station data
			//*	0 = No Text/Block Annotation & Ordinate Line
			//*	1 = Only Draws Ordinate Line
			//*	2 = Only Draws Text Annotation
			//*	4 = Only Inserts the specified Blockname
			//******************************************************
			pRecNew->lBit = 1;
			pRecNew->dOffset = fOffset;
			pRecNew->dElev = fElev;
			pRecNew->strTxt = "";
			pRecNew->strBlk = "";
			#ifdef _DEBUG
				pRecNew->lBit = 3;//Draws Ordinate Line + Text Annotation
				pRecNew->strTxt = pszGenLyrName;
			#endif//_DEBUG
			
			arrayCSD.InsertAt(i, pRecNew);
			return TRUE;
		}
	}
	return FALSE;
}
