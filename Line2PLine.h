//Line2PLine.h
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _LINE2PLINE_H
#define _LINE2PLINE_H

#include "Util.h"
#include "DrawUtil.h"



void Lines2PLine();
BOOL L2P(const CPointPair3DPtrArray&, const char*);
BOOL GetPtsFrLinesForQPfl(AcGePoint3dArray&);

inline void DeallocatePtPairArray(CPointPair3DPtrArray& Array)
{
	int i;

	for(i = 0; i < Array.GetSize(); i++)
	{
		POINTPAIR3D* pRec;
		pRec = Array[i];
		delete pRec;
	}
	if(Array.GetSize() > 0)
		Array.RemoveAll();
}
inline POINTPAIR3D* SwapPoints(POINTPAIR3D* pRec)
{
	AcGePoint3d	ptTmp;

	ptTmp = pRec->pt01;
	pRec->pt01 = pRec->pt02;
	pRec->pt02 = ptTmp;
	
	return pRec;
}

inline void RevertPtPairArray(CPointPair3DPtrArray& Array)
{
	int i;
	CPointPair3DPtrArray ArrayLocal;

	ArrayLocal.Append(Array);
	Array.RemoveAll();

	for(i = (ArrayLocal.GetSize() - 1); i >= 0; i--)
	{
		Array.Add(SwapPoints(ArrayLocal[i]));
	}
}

#endif //_LINE2PLINE_H
