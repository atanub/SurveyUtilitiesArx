//Util.h
//////////////////////////////////////////////////////////////////////////////
#ifndef _UTIL_H
#define _UTIL_H

#include <acgi.h>
#include <AdsCodes.h>
#include <GEAssign.h>
#include <Math.h>

#define PI 3.1415926535897932384626433832795


inline int XtractStrFrEntData(struct resbuf* pEntData, const int& iDXFCode, char* pszText)
{
	struct resbuf* pTemp;
	for(pTemp = pEntData; pTemp != 0L; pTemp = pTemp->rbnext)
	{
		if(pTemp->restype == iDXFCode)
		{
			strcpy(pszText, pTemp->resval.rstring);
			return 1;
		}
	}
	return 0;
}
inline int XtractPoint3DFrEntData(struct resbuf* pEntData, const int& iDXFCode, AcGePoint3d& Point3D)
{
	struct resbuf* pTemp;
	for(pTemp = pEntData; pTemp != 0L; pTemp = pTemp->rbnext)
	{
		if(pTemp->restype == iDXFCode)
		{
			Point3D.set(pTemp->resval.rpoint[X], pTemp->resval.rpoint[Y], pTemp->resval.rpoint[Z]);
			return 1;
		}
	}
	return 0;
}
inline double RTOD(const double& dVal)
{
	return (180.0 * dVal / PI);
}
inline double DTOR(const double& dVal)
{
	return (PI * dVal / 180.0);
}
inline double Angle2D(const AcGePoint3d& Pt01, const AcGePoint3d& Pt02)
{
	ads_point Point01, Point02;

	Point01[X] = Pt01.x;	Point01[Y] = Pt01.y;	Point01[Z] = 0.0;
	Point02[X] = Pt02.x;	Point02[Y] = Pt02.y;	Point02[Z] = 0.0;

	return ads_angle (Point01, Point02);
}
inline double Dist3D(const AcGePoint3d* pPt01, const AcGePoint3d* pPt02)
{
	return pPt01->distanceTo(*pPt02);
}
inline double Dist3D(const AcGePoint3d& Pt01, const AcGePoint3d& Pt02)
{
	return Pt01.distanceTo(Pt02);
}

inline double Dist2D(const AcGePoint3d& Pt01, const AcGePoint3d& Pt02)
{
	return AcGePoint3d(Pt01.x, Pt01.y, 0.0).distanceTo(AcGePoint3d(Pt02.x, Pt02.y, 0.0));
}
inline BOOL GetIntersPt2D(const AcGePoint3d* pPt01A, const AcGePoint3d* pPt02A, const AcGePoint3d* pPt01B, const AcGePoint3d* pPt02B, AcGePoint3d& Result)
{
	AcGePoint3d Pt01A, Pt02A, Pt01B, Pt02B;
	ads_point ptResult;

	Pt01A = *pPt01A;	Pt01A.z = 0.0;
	Pt02A = *pPt02A;	Pt02A.z = 0.0;

	Pt01B = *pPt01B;	Pt01B.z = 0.0;
	Pt02B = *pPt02B;	Pt02B.z = 0.0;

	if(RTNORM == ads_inters(asDblArray(Pt01A), asDblArray(Pt02A), asDblArray(Pt01B), asDblArray(Pt02B), 1, ptResult))
	{
		Result = asPnt3d(ptResult);
		Result.z = pPt01A->z;
		return TRUE;
	}
	return FALSE;
}

inline BOOL XTractWordsFromLine(const CString& strDataLine, CStringArray& strArray, const CString& strWhite)
{
	if((strArray.GetSize() != 0) || (strlen(strWhite) <= 0))
	{
		ads_printf("\nERROR: Invalid array parameter passed!! Array is not empty\n");
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

#ifdef _DEBUG
//	ads_printf("\n_XTractWords >>Dump >> ");
//	for(i = 0; i < strArray.GetSize(); i++)
//		ads_printf("<%s>", (LPCSTR)strArray[i]);
#endif //_DEBUG
	return (strArray.GetSize() > 0);
}
inline BOOL XTractWordsFromLine(const CString& strDataLine, CStringArray& strArray)
{
	const CString strWhite(" \t");

	return XTractWordsFromLine(strDataLine, strArray, strWhite);
}
inline BOOL ReadNextValidLine(CStdioFile& File, CStringArray& strWordArray, long& iLineNo)
{
	const char chComment = '*';
	CString strLine;
	BOOL bIsCommentLine;

	for(;;)
	{
		if(!File.ReadString(strLine))
		{
			return FALSE;
		}
		iLineNo++;

		strLine.TrimLeft();
		strLine.TrimRight();
		{//Chk whether comment line...
			bIsCommentLine = (strlen(strLine) == 0);
			if((strlen(strLine) > 0) && !bIsCommentLine)
			{
				bIsCommentLine = (strLine[0] == chComment);
			}
		}
		if(!bIsCommentLine)
		{
			if(!XTractWordsFromLine(strLine, strWordArray))
				return FALSE;//ERROR

			return TRUE;
		}
	}
	return FALSE;
}
inline void DeallocatePtrArray(CPtrArray& Array)
{
	int i;
	void* pVoid;
	for(i = 0; i < Array.GetSize(); i++)
	{
		pVoid = Array[i];
		delete pVoid;
	}
	if(Array.GetSize() > 0)
		Array.RemoveAll();
}
inline int FindInPtrArray(void* pPtr, CPtrArray& Array)
{
	int i;

	for(i = 0; i < Array.GetSize(); i++)
	{
		if(pPtr == Array[i])
			return i;
	}
	return -1;
}
inline void RemoveTrailingZeros(CString& strTmp)
{
	int i;

	//Remove Trailing Zeros....
	for(i = strlen(strTmp) - 1; i >= 0; i--)
	{
		if(strTmp[i] == '.')
		{
			strTmp = strTmp.Mid(0, i);
			break;
		}
		if(strTmp[i] != '0')
		{
			strTmp = strTmp.Mid(0, i+1);
			break;
		}
	}
	if(i < 0)
	{
		strTmp = "0";
	}
}
/*****************************************************************************
   Determines which side of a line the point (x,y) lies.
   The line goes from (x1,y1) to (x2,y2)
 
   Return codes are  -1 for points to the left
          0 for points on the line
          +1 for points to the right
*/
inline int PtInWhichSide(const AcGePoint3d& ptToTest, const AcGePoint3d& ptStart, const AcGePoint3d& ptEnd)
{
    double fDist;
 
    fDist = (ptToTest.y - ptStart.y) * (ptEnd.x - ptStart.x) - (ptEnd.y - ptStart.y) * (ptToTest.x - ptStart.x);
    if(fDist > 0.0)
       return -1;//Left
    else if(fDist < 0.0)
       return 1;//Rite

	return 0;//On the line
}
/*****************************************************************************
   Returns TRUE if the point (xp,yp) lies inside the projection triangle
   on the x-y plane with the vertices p1,p2,p3
 
   A point is in the centre if it is on the same side of all the edges
   or if it lies on one of the edges.
*/
inline BOOL IsPointInTriangle(const AcGePoint3d& ptToTest, const AcGePoint3d& p1, const AcGePoint3d& p2, const AcGePoint3d& p3)
{
   int iSide1, iSide2, iSide3;
 
   iSide1 = PtInWhichSide(ptToTest, p1, p2);
   iSide2 = PtInWhichSide(ptToTest, p2, p3);
   iSide3 = PtInWhichSide(ptToTest, p3, p1);
 
   if (iSide1 == 0 && iSide2 == 0)
      return (TRUE);
   if (iSide1 == 0 && iSide3 == 0)
      return (TRUE);
   if (iSide2 == 0 && iSide3 == 0)
      return (TRUE);
 
   if (iSide1 == 0 && (iSide2 == iSide3))
      return (TRUE);
   if (iSide2 == 0 && (iSide1 == iSide3))
      return (TRUE);
   if (iSide3 == 0 && (iSide1 == iSide2))
      return (TRUE);
 
   if ((iSide1 == iSide2) && (iSide2 == iSide3))
      return (TRUE);
  
   return (FALSE);
}

inline BOOL IsPointOnLine(const AcGePoint3d& ptParam, const AcGePoint3d& ptStart, const AcGePoint3d& ptEnd, const double fErr = 1.0E-6)
{
	double fVal;
	fVal = (Dist2D(ptStart, ptParam) + Dist2D(ptParam, ptEnd)) - Dist2D(ptStart, ptEnd);
	fVal = fabs(fVal);
	return (fVal <= fErr);
}
//
//Returns -VE value when the point doesn't lie on the profile
inline double CalculateChainage(const AcGePoint3d& ptToTest, const AcGePoint3dArray& arrayPoints, const double fPrec = 1.0E-6)
{
	int i;
	double fResult;

	fResult = 0.0;
	for(i = 1; i < arrayPoints.length(); i++)
	{
		if(IsPointOnLine(ptToTest, arrayPoints[i - 1], arrayPoints[i], fPrec))
		{
			fResult += Dist2D(arrayPoints[i - 1], ptToTest);
			return fResult;
		}
		fResult += Dist2D(arrayPoints[i - 1], arrayPoints[i]);
	}
	return -1.0;//Not found!!
}
inline double CalculateAutoDatum(const double& fMinElev, const double& fYScale, const double& fMinBufferInDWGUnits, const double& fDatumIsMultipleOf)
{
	double fDatum, fCumVal;

	fDatum = fMinElev - fMinBufferInDWGUnits / fYScale;
	if(fDatum >= fDatumIsMultipleOf)
	{
		fCumVal = fDatumIsMultipleOf;
	}
	else
	{
		const int iMF = 1 + (int)(fabs(fDatum) / fDatumIsMultipleOf);

		fCumVal = 0.0 - iMF * fDatumIsMultipleOf;
	}
	while(TRUE)
	{
		if(fCumVal >= fDatum)
		{
			if(fCumVal == fDatum)
				return fCumVal;
			return (fCumVal - fDatumIsMultipleOf);
		}
		fCumVal += fDatumIsMultipleOf;
	}
	return fCumVal;
}
inline double CalculateLength(const AcGePoint3dArray& arrayVertices)
{
	int i;
	double dCumDist;
	AcGePoint3d ptStart, ptEnd;
	
	dCumDist = 0.0;
	for(i = 1; i < arrayVertices.length(); i++)
	{
		ptStart = arrayVertices[i - 1];
		ptEnd = arrayVertices[i];

		dCumDist += Dist2D(ptStart, ptEnd);
	}
	return dCumDist;
}
inline BOOL FindPerpPointOnLine(const AcGePoint3d& ptParam, const AcGePoint3d& ptStartOfLine, const AcGePoint3d& ptEndOfLine, AcGePoint3d& ptResult, bool bFindTrueIntersection = false)
{
	double fAngle;
	AcGePoint3d ptTmp, ptParam2D(ptParam), ptStartOfLine2D(ptStartOfLine), ptEndOfLine2D(ptEndOfLine);
	fAngle = Angle2D(ptStartOfLine, ptEndOfLine);
	fAngle += (PI / 2.0);
	//
	ptParam2D.z = 0.0;
	ptStartOfLine2D.z = 0.0;
	ptEndOfLine2D.z = 0.0;
	//
	ads_polar(asDblArray(ptParam2D), fAngle, 1E3, asDblArray(ptTmp));

	if(RTNORM != ads_inters(asDblArray(ptParam2D), asDblArray(ptTmp), asDblArray(ptStartOfLine2D), asDblArray(ptEndOfLine2D), 0, asDblArray(ptResult)))
	{
		return FALSE;
	}
	if(bFindTrueIntersection)
	{
		if(!IsPointOnLine(ptResult, ptStartOfLine, ptEndOfLine))
			return FALSE;
	}
	return TRUE;
}

#endif // _UTIL_H
