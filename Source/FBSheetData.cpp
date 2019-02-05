//FBSheetData.cpp
#include "stdafx.h"
#include "FBSheetData.h"
#include "SurveyData.h"


CFBSheetData::CFBSheetData()
{
	m_fScaleAcrossAlgn = 0.0;
	
	memset(&m_RecDwgParam, 0, sizeof(m_RecDwgParam));
	memset(&m_RecDwgParam, 0, sizeof(m_RecDwgParam));
	
	m_RecDwgParam.fTxtSize = 0.5;
	m_RecDwgParam.fBlkScale = 1.0;
	strcpy(m_RecDwgParam.szLyr, "Test");
}

CFBSheetData::~CFBSheetData()
{
	DeallocatePtrArray(m_arrayAlignmentRecs);
	DeallocatePtrArray(m_arraySurveyObjRecs);
	DeallocatePtrArray(m_arrayAlgnScale);
	DeallocatePtrArray(m_arrayObjDefsRecs);
}
BOOL CFBSheetData::ImportRec(const FBSURVEYOBJ& Rec)
{
	if(IsPointNoExists(Rec.iPointNo))
		return FALSE;

	if(m_arraySurveyObjRecs.GetSize() > 0)
	{//Chg should be sequential (Ascending)!!
		FBSURVEYOBJ* pRecLast;
		
		pRecLast = m_arraySurveyObjRecs[m_arraySurveyObjRecs.GetSize() - 1];
		if(pRecLast->fChainage > Rec.fChainage)
		{
			return FALSE;
		}
	}
	{//Add now ...
		FBSURVEYOBJ* pRec;
		
		pRec = new FBSURVEYOBJ;
		*pRec = Rec;

		m_arraySurveyObjRecs.Add(pRec);
	}
	return TRUE;
}
BOOL CFBSheetData::ImportRec(const FBOBJDEF& Rec)
{
	FBOBJDEF* pRec;
	
	pRec = new FBOBJDEF;
	strcpy(pRec->szEntity, Rec.szEntity);
	pRec->arrayPtNos.Append(Rec.arrayPtNos);
	m_arrayObjDefsRecs.Add(pRec);
	return TRUE;
}
BOOL CFBSheetData::ImportRec(const FBALIGNMENT& Rec)
{
	FBALIGNMENT* pRec;
	
	pRec = new FBALIGNMENT;
	*pRec = Rec;
	
	if(m_arrayAlignmentRecs.GetSize() > 0)
	{
		FBALIGNMENT* pRecLast;

		pRecLast = m_arrayAlignmentRecs[m_arrayAlignmentRecs.GetSize() - 1];
		{//Check Start/End Chainage!!
			if(pRecLast->fEndChg != pRec->fStartChg)
			{
				delete pRec;
				return FALSE;
			}
		}
		pRec->fChainage += pRecLast->fChainage;
	}
	
	m_arrayAlignmentRecs.Add(pRec);
	return TRUE;
}
BOOL CFBSheetData::_ImportAlgnRecToAlgnScaleArray(const double& fChg, CFBAlgnScalesArray& arrayParam) const
{
	int i;
	FBALGNSCALE* pRec;
	FBALGNSCALE* pRecNew;

	for(i = 0; i < arrayParam.GetSize(); i++)
	{
		pRec = arrayParam[i];
		if((fChg > pRec->fStartChg) && (fChg < pRec->fEndChg))
		{
			pRecNew = new FBALGNSCALE;
			//
			pRecNew->fStartChg = fChg;
			pRecNew->fEndChg = pRec->fEndChg;
			pRecNew->fScale = pRec->fScale;
			//
			//Same pRec->fStartChg = pRec->fStartChg;
			pRec->fEndChg = fChg;
			//Same pRec->fScale = pRec->fScale;
			arrayParam.InsertAt(i+1, pRecNew);
			return TRUE;
		}
	}
	if(arrayParam.GetSize() <= 0)
	{//
		pRecNew = new FBALGNSCALE;
		//
		pRecNew->fStartChg = 0.0;
		pRecNew->fEndChg = fChg;
		pRecNew->fScale = 1.0;//Default Scale
		//
		arrayParam.Add(pRecNew);
		return TRUE;
	}
	//
	{//
		pRec = arrayParam[arrayParam.GetSize() - 1];
		if(fChg <= pRec->fEndChg)
			return TRUE;//Not Required to Add.......
		
		pRecNew = new FBALGNSCALE;
		//
		pRecNew->fStartChg = pRec->fEndChg;
		pRecNew->fEndChg = fChg;
		pRecNew->fScale = 1.0;//Default Scale
		//
		arrayParam.Add(pRecNew);
		return TRUE;
	}
}
BOOL CFBSheetData::ImportRec(const FBALGNSCALE& Rec)
{
	FBALGNSCALE* pRec;

	if((Rec.fStartChg >= Rec.fEndChg) || (Rec.fScale <= 0.0))
		return FALSE;

	if(m_arrayAlgnScale.GetSize() > 0)
	{
		const FBALGNSCALE* pLastRec = m_arrayAlgnScale[m_arrayAlgnScale.GetSize() - 1];
		if(Rec.fStartChg >= pLastRec->fEndChg)
			return FALSE;//Overlapping not allowed
	}
	{//Add 
		pRec = new FBALGNSCALE;
		*pRec = Rec;
		m_arrayAlgnScale.Add(pRec);
	}
	return TRUE;
}
BOOL CFBSheetData::HasData() const
{
	if((m_arrayAlignmentRecs.GetSize() <= 0) || (m_arraySurveyObjRecs.GetSize() <= 0))
	{
		return FALSE;
	}
	//m_arrayObjDefsRecs can be empty
	return TRUE;
}

BOOL CFBSheetData::XTractWordsFromLine(const CString& strWhite, const CString& strDataLine, CStringArray& strArray)
{
	if(strArray.GetSize() != 0)
	{
		TRACE("\nERROR: Invalid array parameter passed!! Array is not empty\n");
		return FALSE;
	}
	CString strTemp;
	BOOL bFlag;
	int iIndex;
	
	strTemp = strDataLine;
	do
	{
		iIndex = strTemp.FindOneOf(strWhite);
		bFlag = (iIndex >= 0);
		if(!bFlag)
		{
			if(strlen(strTemp) > 0)
				strArray.Add(strTemp);
			break;
		}
		{//Add only valid words.........
			CString strWord;

			strWord = strTemp.Mid(0, iIndex);
			strWord.TrimLeft();
			strWord.TrimRight();

			if(strlen(strWord) > 0)
				strArray.Add(strWord);
		}
		strTemp = strTemp.Mid(++iIndex);
	}
	while(bFlag);
	return (strArray.GetSize() > 0);
}
BOOL CFBSheetData::GetPointNosFromString(const CString& strWhite, const CString& strDataLine, CUIntArray& array)
{
	CStringArray strArray;
	int i;

	if(array.GetSize() != 0)
	{
		TRACE("\nERROR: Invalid array parameter passed!! Array is not empty\n");
		return FALSE;
	}

	if(!CFBSheetData::XTractWordsFromLine(strWhite, strDataLine, strArray))
		return FALSE;

	for(i = 0; i < strArray.GetSize(); i++)
	{
		array.Add(atoi(strArray[i]));
	}
	return (array.GetSize() > 0);
}
BOOL CFBSheetData::ParseAngleString(const CString& strWhite, const CString& strDataLine, double& fAngleInDeg)
{
	CStringArray strArray;
	int i;
	double fDeg, fMin, fSec;

	if(!CFBSheetData::XTractWordsFromLine(strWhite, strDataLine, strArray))
		return FALSE;

	if((strArray.GetSize() > 3) || (strArray.GetSize() < 1))
		return FALSE;


	fAngleInDeg = fDeg = fMin = fSec = 0.0;
	for(i = 0; i < strArray.GetSize(); i++)
	{
		switch(i)
		{
		case 0:
			fDeg = atof(strArray[i]);
			break;
		case 1:
			fMin = atof(strArray[i]);
			if(fMin >= 60.0)
				return FALSE;
			break;
		case 2:
			fSec = atof(strArray[i]);
			if(fSec >= 60.0)
				return FALSE;
			break;
		default:
			ASSERT(FALSE);
			break;
		}
	}
	fAngleInDeg = fDeg + fMin / 60.0 + fSec / 3600;
	return TRUE;
}
BOOL CFBSheetData::IsAllPointNosExists(const CUIntArray& array) const
{
	int i, iPtNo;
	
	for(i = 0; i < array.GetSize(); i++)
	{
		iPtNo = array[i];
		if(!IsPointNoExists(iPtNo))
			return FALSE;//Found
	}
	return TRUE;
}
BOOL CFBSheetData::IsPointNoExists(const int& iPtNo) const 
{
	int i;

	for(i = 0; i < m_arraySurveyObjRecs.GetSize(); i++)
	{
		const FBSURVEYOBJ* pRecTmp = m_arraySurveyObjRecs[i];

		if(pRecTmp->iPointNo == iPtNo)
			return TRUE;//Found
	}
	return FALSE;//not Found
}
//
//Drawing/Calculation Related ////////////////////////////////////////////////////////////////////////////////////
//
BOOL CFBSheetData::Draw(const AcGePoint3d& ptBase, const FBDWGPARAM& recDwgParam)
{
	double fBaseChg;
	int i;
	
	m_RecDwgParam = recDwgParam;

	fBaseChg = 0.0;
	if(m_arrayAlgnScale.GetSize() > 0)
	{//Get the Base Chainage...
		fBaseChg = m_arrayAlgnScale[0]->fStartChg;
	}
	{//Calculate & store chainages WRT base chainage
		FBALIGNMENT* pRecLast;
		
		if(m_arrayAlignmentRecs.GetSize() > 0)
			pRecLast = m_arrayAlignmentRecs[m_arrayAlignmentRecs.GetSize() - 1];
		for(i = 0; i < m_arraySurveyObjRecs.GetSize(); i++)
		{
			FBSURVEYOBJ* pRec;
			
			pRec = m_arraySurveyObjRecs[i];
			pRec->fChainage = pRec->fChainage - fBaseChg;
			if((pRec->fChainage < 0.0) || (pRec->fChainage > pRecLast->fChainage))
			{
				ads_printf("\nERROR: Chainage %.3f is out of bound\n", (pRec->fChainage + fBaseChg));
				return FALSE;
			}
		}
	}

	_SmoothAlgnScaleArray(m_arrayAlgnScale);

	AcGePoint3d ptTmp;

	LAYER(m_RecDwgParam.szLyr, 1);
	{//Draw Alignment
		FBALIGNMENT* pRec;
		AcGePoint3dArray arrayTmp;
		
		arrayTmp.append(ptBase);
		for(i = 0; i < m_arrayAlignmentRecs.GetSize(); i++)
		{
			pRec = m_arrayAlignmentRecs[i];
			if(!_GetPointAtChg(pRec->fChainage, ptBase, ptTmp))
			{
				ads_printf("\nFATAL ERROR: Control should not reach here! @ CFBSheetData::Draw()\n");
				return FALSE;
			}
			arrayTmp.append(ptTmp);    
		}
		POLYLINE(arrayTmp, m_RecDwgParam.szLyr, FALSE);
	}
	_DrawPoints(ptBase, fBaseChg);
	_DrawObjects(ptBase, fBaseChg);
	//
#ifdef _DEBUG
#endif //_DEBUG
	{//Draw Alignment portions where scale is other than 1.0
		const char* pszLyrDebug = "SURVUTIL_INSPECTION_LYR";
		double fAngle;
		FBALGNSCALE* pRecTmp;

		LAYER(pszLyrDebug, 3);
		for(i = 0; i < m_arrayAlgnScale.GetSize(); i++)
		{
			AcGePoint3dArray arrayTmp;
			AcGePoint3d pt01, pt02;

			pRecTmp = m_arrayAlgnScale[i];
			if(pRecTmp->fScale > 1.0)
			{
				fAngle = _FindFwdAngle(pRecTmp->fStartChg);
				
				_GetPointAtChg(pRecTmp->fStartChg, ptBase, pt01);
				ads_polar(asDblArray(pt01), (fAngle + (PI / 2.0)), m_RecDwgParam.fTxtSize, asDblArray(ptTmp));
				arrayTmp.append(ptTmp);
				
				_GetPointAtChg(pRecTmp->fEndChg, ptBase, pt02);
				ads_polar(asDblArray(pt02), (fAngle + (PI / 2.0)), m_RecDwgParam.fTxtSize, asDblArray(ptTmp));
				arrayTmp.append(ptTmp);
				//
				ads_polar(asDblArray(pt02), (fAngle - (PI / 2.0)), m_RecDwgParam.fTxtSize, asDblArray(ptTmp));
				arrayTmp.append(ptTmp);
				//
				ads_polar(asDblArray(pt01), (fAngle - (PI / 2.0)), m_RecDwgParam.fTxtSize, asDblArray(ptTmp));
				arrayTmp.append(ptTmp);
				//
				ads_polar(asDblArray(pt01), (fAngle + (PI / 2.0)), m_RecDwgParam.fTxtSize, asDblArray(ptTmp));
				arrayTmp.append(ptTmp);

				POLYLINE(arrayTmp, pszLyrDebug, FALSE);
			}
		}
	}
	return TRUE;
}
void CFBSheetData::_DrawObjects(const AcGePoint3d& ptBase, const double& fBaseChg) const
{
	int i;
	FBOBJDEF* pRec;

	for(i = 0; i < m_arrayObjDefsRecs.GetSize(); i++)
	{
		int j;
		AcGePoint3dArray arrayTmp;
		
		pRec = m_arrayObjDefsRecs[i];
		//Collect all the points
		for(j = 0; j < pRec->arrayPtNos.GetSize(); j++)
		{
			AcGePoint3d ptTmp;
			_ConvertPtNoToPt3D((int)pRec->arrayPtNos[j], ptBase, fBaseChg, ptTmp);
			arrayTmp.append(ptTmp);
		}
		//Draw...
		if((_stricmp("pline", pRec->szEntity) == 0) || (_stricmp("polyline", pRec->szEntity) == 0))
		{
			POLYLINE(arrayTmp, m_RecDwgParam.szLyr, FALSE);
		}
		else if(_stricmp("line", pRec->szEntity) == 0)
		{
			LINES(arrayTmp, m_RecDwgParam.szLyr);
		}
		else
		{
			ads_printf("\nERROR: Unrecognized Entity specified\"%s\" \nDrawing not possible!\n", pRec->szEntity);
		}
	}
}
BOOL CFBSheetData::_ConvertPtNoToPt3D(const int iPtNo, const AcGePoint3d& ptBase, const double& fBaseChg, AcGePoint3d& ptParam) const
{
	int i;
	FBSURVEYOBJ* pRec;

	for(i = 0; i < m_arraySurveyObjRecs.GetSize(); i++)
	{
		pRec = m_arraySurveyObjRecs[i];
		if(pRec->iPointNo == (long)iPtNo)
		{
			double fAngle;
			AcGePoint3d ptOnAlgn;
			
			fAngle = _FindFwdAngle(pRec->fChainage);
			if(!_GetPointAtChg(pRec->fChainage, ptBase, ptOnAlgn))
			{
				ads_printf("\nERROR: Chainage %.3f is out of bound\n", (pRec->fChainage + fBaseChg));
				return FALSE;
			}
			ads_polar(asDblArray(ptOnAlgn), (fAngle - (PI / 2.0)), (m_fScaleAcrossAlgn * pRec->fOffset), asDblArray(ptParam));
			return TRUE;
		}
	}
	return FALSE;
}
void CFBSheetData::_DrawPoints(const AcGePoint3d& ptBase, const double& fBaseChg) const
{
	int i, iColor;
	FBSURVEYOBJ* pRec;
	double fAngle;
	AcGePoint3d ptOnAlgn, ptSurveyed;

	for(i = 0, iColor = 1; i < m_arraySurveyObjRecs.GetSize(); i++)
	{
		pRec = m_arraySurveyObjRecs[i];
		fAngle = _FindFwdAngle(pRec->fChainage);
		//{//DEBUG
		//	double fTmp;
		//	ads_printf("\nTRACE: @ chg %f>> Angle = %f", pRec->fChainage, RTOD(fAngle));
		//	fTmp = RTOD(fAngle);
		//	fTmp = RTOD(fAngle);
		//}
		if(!_GetPointAtChg(pRec->fChainage, ptBase, ptOnAlgn))
		{
			ads_printf("\nERROR: Chainage %.3f is out of bound\n", (pRec->fChainage + fBaseChg));
			return;
		}
		ads_polar(asDblArray(ptOnAlgn), (fAngle - (PI / 2.0)), (m_fScaleAcrossAlgn * pRec->fOffset), asDblArray(ptSurveyed));
		ptSurveyed.z = pRec->fElev;
		//
		if(iColor >= 255)
			iColor = 0;
		iColor++;
		LAYER(pRec->szLyr, iColor);
		//
		DRAWPOINT(ptSurveyed, pRec->szLyr);
		TEXTMID(ptSurveyed, pRec->szLyr, fAngle, m_RecDwgParam.fTxtSize, pRec->szLyr);
#ifdef _DEBUG
		LINE(ptOnAlgn, ptSurveyed, pRec->szLyr);
		TEXTLEFT(ptSurveyed, RTOS(pRec->fChainage + fBaseChg), Angle2D(ptOnAlgn, ptSurveyed), m_RecDwgParam.fTxtSize, pRec->szLyr);
#endif//_DEBUG
		if(pRec->iBitCode & 4)
		{//
			if(IsBlockExists(pRec->szBlock))
			{
				BLOCK(pRec->szBlock, ptSurveyed, pRec->szLyr, fAngle, m_RecDwgParam.fBlkScale, m_RecDwgParam.fBlkScale);
			}
			else
			{
				ads_printf("\nERROR: Block \"%s\" does not exists...Insertion failed\n", pRec->szBlock);
			}
		}
		if(pRec->iBitCode & 2)
		{//
			TEXTMID(ptSurveyed, pRec->szText, fAngle, m_RecDwgParam.fTxtSize, pRec->szLyr);
		}
	}
}
double CFBSheetData::_FindFwdAngle(const double& fChg) const
{
	double fAngleToRet;
	int i;

	if(m_arrayAlignmentRecs.GetSize() <= 0)
		return 0.0;
	if(m_arrayAlignmentRecs.GetSize() == 1)
		return _FBAngToRad(m_arrayAlignmentRecs[0]->fAngleInDeg);

	fAngleToRet = 0.0;
	for(i = 1; i < m_arrayAlignmentRecs.GetSize(); i++)
	{
		FBALIGNMENT* pRec;
		FBALIGNMENT* pRecCur;

		pRec = m_arrayAlignmentRecs[i - 1];
		pRecCur = m_arrayAlignmentRecs[i - 0];
		if((fChg >= pRec->fChainage) && (fChg <= pRecCur->fChainage))
		{
			return _FBAngToRad(pRecCur->fAngleInDeg);
		}
		if((i == 1) && (fChg < pRec->fChainage))
			return _FBAngToRad(pRec->fAngleInDeg);
	}
	//return the last rec's angle....
	fAngleToRet = (m_arrayAlignmentRecs[m_arrayAlignmentRecs.GetSize() - 1])->fAngleInDeg;
	return _FBAngToRad(fAngleToRet);
}
BOOL CFBSheetData::_GetPointAtChg(const double& fChg, const AcGePoint3d& ptBase, AcGePoint3d& ptParam) const
{
	int i;
	double fDist;
	FBALIGNMENT* pRec;
	AcGePoint3d ptTmp;
	
	ptParam = ptBase;
	for(i = 0; i < m_arrayAlignmentRecs.GetSize(); i++)
	{
		pRec = m_arrayAlignmentRecs[i];
		if(fChg >= pRec->fChainage)
		{
			if(i == 0)
			{
				fDist = _GetDistance(&(pRec->fChainage));
			}
			else
			{
				FBALIGNMENT* pRecPrev;
				
				pRecPrev = m_arrayAlignmentRecs[i - 1];
				fDist = _GetDistance(&(pRecPrev->fChainage), &(pRec->fChainage));
			}

			ads_polar(asDblArray(ptParam), _FBAngToRad(pRec->fAngleInDeg), fDist, asDblArray(ptTmp));
			ptParam = ptTmp;
			if(fChg == pRec->fChainage)
			{
				return TRUE;
			}
		}
		else
		{//fChg < pRec->fChainage
			if(i == 0)
			{
				fDist = _GetDistance(&fChg);
				ads_polar(asDblArray(ptBase), _FBAngToRad(pRec->fAngleInDeg), fDist, asDblArray(ptTmp));
				ptParam = ptTmp;
			}
			else
			{
				FBALIGNMENT* pRecPrev;
				
				pRecPrev = m_arrayAlignmentRecs[i - 1];
				fDist = _GetDistance(&(pRecPrev->fChainage), &fChg);
				ads_polar(asDblArray(ptParam), _FBAngToRad(pRec->fAngleInDeg), fDist, asDblArray(ptTmp));
				ptParam = ptTmp;
			}
			return TRUE;
		}
	}
	return FALSE;
}
double CFBSheetData::_GetDistance(const double* pfChgStart, const double* pfChgEnd/* = 0L*/) const
{
	int i;
	FBALGNSCALE* pRec;
	double fCumDist;
	BOOL bStartChgPassed;

	if(pfChgEnd == 0L)
	{//
		fCumDist = 0.0;
		for(i = 0; i < m_arrayAlgnScale.GetSize(); i++)
		{
			pRec = m_arrayAlgnScale[i];
			if(*pfChgStart == pRec->fStartChg)
				return fCumDist;
			if((*pfChgStart > pRec->fStartChg) && (*pfChgStart < pRec->fEndChg))
			{
				fCumDist += ((*pfChgStart - pRec->fStartChg) * pRec->fScale);
				return fCumDist;
			}
			else
			{
				fCumDist += ((pRec->fEndChg - pRec->fStartChg) * pRec->fScale);
			}
		}
		return fCumDist;
	}
	fCumDist = 0.0;
	bStartChgPassed = FALSE;
	for(i = 0; i < m_arrayAlgnScale.GetSize(); i++)
	{
		pRec = m_arrayAlgnScale[i];
		if(*pfChgStart == pRec->fStartChg)
		{
			if(*pfChgEnd <= pRec->fEndChg)
			{
				fCumDist += ((*pfChgEnd - pRec->fStartChg) * pRec->fScale);
				return fCumDist;
			}
			bStartChgPassed = TRUE;
		}
		if(bStartChgPassed)
		{
			if((*pfChgEnd >= pRec->fStartChg) && (*pfChgEnd <= pRec->fEndChg))
			{
				fCumDist += ((*pfChgEnd - pRec->fStartChg) * pRec->fScale);
				return fCumDist;
			}
			else
			{
				fCumDist += ((pRec->fEndChg - pRec->fStartChg) * pRec->fScale);
			}
		}
	}
	if(!bStartChgPassed)
	{
		fCumDist = 0.0;
		ads_printf("\n\nERROR: Control should not reach here!!");
	}
	return fCumDist;

}
void CFBSheetData::_SmoothAlgnScaleArray(CFBAlgnScalesArray& arrayParams) const
{
	int i;
	FBALGNSCALE* pRec;
	FBALGNSCALE* pRecNew;
	CFBAlgnScalesArray arrayTmp;

	for(i = 0; i < arrayParams.GetSize(); i++)
	{
		pRec = arrayParams[i];
		if(i == 0)
		{
			if(pRec->fStartChg > 0.0)
			{
				pRecNew = new FBALGNSCALE;
				pRecNew->fStartChg = 0.0;
				pRecNew->fEndChg = pRec->fStartChg;
				pRecNew->fScale = 1.0;
				//
				arrayTmp.Add(pRecNew);
			}
		}
		else
		{
			FBALGNSCALE* pRecLast;
			if(arrayTmp.GetSize() <= 0)
			{
				ads_printf("\n\nERROR: Control should not reach here!!");
				ads_printf("\n\nERROR: Memory leak could occur!!");
				return;
			}
			pRecLast = arrayTmp[arrayTmp.GetSize() - 1];
			if(pRecLast->fEndChg < pRec->fStartChg)
			{
				pRecNew = new FBALGNSCALE;
				pRecNew->fStartChg = pRecLast->fEndChg;
				pRecNew->fEndChg = pRec->fStartChg;
				pRecNew->fScale = 1.0;
				//
				arrayTmp.Add(pRecNew);
			}
		}
		//
		pRecNew = new FBALGNSCALE;
		*pRecNew = *pRec;
		arrayTmp.Add(pRecNew);
	}
#ifdef _DEBUG
	ads_printf("\n#Original Array ###########################");
	for(i = 0; i < arrayParams.GetSize(); i++)
	{
		pRec = arrayParams[i];
		ads_printf("\n#%d>> fStartChg=%f, fEndChg=%f, fScale=%f", i, pRec->fStartChg, pRec->fEndChg, pRec->fScale);
	}
	ads_printf("\n#Processed Array ###########################");
	for(i = 0; i < arrayTmp.GetSize(); i++)
	{
		pRec = arrayTmp[i];
		ads_printf("\n#%d>> fStartChg=%f, fEndChg=%f, fScale=%f", i, pRec->fStartChg, pRec->fEndChg, pRec->fScale);
	}
#endif //_DEBUG
	//
	DeallocatePtrArray(arrayParams);
	arrayParams.Append(arrayTmp);
	
	{//Add Alignment vertices as well........
		FBALIGNMENT* pRecAlgn;

		_ImportAlgnRecToAlgnScaleArray(0.0, arrayParams);
		for(i = 0; i < m_arrayAlignmentRecs.GetSize(); i++)
		{
			pRecAlgn = m_arrayAlignmentRecs[i];
			_ImportAlgnRecToAlgnScaleArray(pRecAlgn->fChainage, arrayParams);
		}
	}
#ifdef _DEBUG
	ads_printf("\n#Processed Array (after inserting algn vertices) ###########################");
	for(i = 0; i < arrayParams.GetSize(); i++)
	{
		pRec = arrayParams[i];
		ads_printf("\n#%d>> fStartChg=%f, fEndChg=%f, fScale=%f", i, pRec->fStartChg, pRec->fEndChg, pRec->fScale);
	}
#endif //_DEBUG
}
double CFBSheetData::_FBAngToRad(const double& fVal)
{	
	double fValTmp;
	
	fValTmp = 360.0 - fVal;
	fValTmp += 90.0;
	
	if(fValTmp > 360.0)
		fValTmp -= 360.0;
	
	return DTOR(fValTmp);
}
