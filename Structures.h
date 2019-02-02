//Structures.h
//Define General structures
#ifndef _STRUCTURES_H
#define _STRUCTURES_H

#include <acgi.h>

typedef struct tagPointPair3D
{
	AcGePoint3d	pt01,
				pt02;
} POINTPAIR3D;
typedef CTypedPtrArray<CPtrArray, POINTPAIR3D*> CPointPair3DPtrArray;

typedef struct tagRdPflNEZData
{
	double	dXScale;
	double	dYScale;
	double	dDatum;
	double	dNumAnnSize;
	double	dRowAnnSize;
	double	dRowHt;
	double	dOrdInterval;

	char	szProfileTag[_MAX_PATH];
	char	szElevTag[_MAX_PATH];
	char	szNorthingTag[_MAX_PATH];
	char	szEastingTag[_MAX_PATH];
	char	szCumDistTag[_MAX_PATH];
	char	szDistTag[_MAX_PATH];
	char	szDatumTag[_MAX_PATH];
	
	char	szLyrAnnotation[_MAX_PATH];
	char	szLyrTopSurface[_MAX_PATH];
	char	szLyrOrdinate[_MAX_PATH];
	char	szLyrAxis[_MAX_PATH];

	char	szOutFile[_MAX_PATH];
	//Top Surface Related.......
	char	szXistTopSurfAnnBlk[_MAX_PATH];
	double	dXistTopSurfAnnBlkScale;
	double	dXistTopSurfAnnBlkSpaceMin;
	
	BOOL	bConsider3DPts;
	BOOL	bDrawGeneratedPoints;

	AcGePoint3d	PtOrg;
} RDPFLNEZDATA;

typedef struct tagPflOffZDwgParam
{
	double	dXScale;
	double	dYScale;
	double	dDatum;
	double	dNumAnnSize;
	double	dRowAnnSize;
	double	dRowHt;
	double	fOrdAnnInterval;

	char	szDataFile[_MAX_PATH];
	char	szOutDataFile[_MAX_PATH];
	
	char	szProfileTag[_MAX_PATH];
	char	szElevTag[_MAX_PATH];
	char	szCumDistTag[_MAX_PATH];
	char	szDistTag[_MAX_PATH];
	char	szDatumTag[_MAX_PATH];
	
	char	szLyrAnnotation[_MAX_PATH];
	char	szLyrTopSurface[_MAX_PATH];
	char	szLyrOrdinate[_MAX_PATH];
	char	szLyrAxis[_MAX_PATH];
	//Top Surface Related.......
	char	szXistTopSurfAnnBlk[_MAX_PATH];
	double	dXistTopSurfAnnBlkScale;
	double	dXistTopSurfAnnBlkSpaceMin;
	
	AcGePoint3d	PtOrg;
} PFLOFFZDWGPARAM;

typedef struct tagPflOffZRec
{
	int		iBitCode;
	double	fChg;
	double	fElev;
	
	char	szTxt[_MAX_PATH];
	char	szBlock[_MAX_PATH];
} PFLOFFZREC;
typedef CTypedPtrArray<CPtrArray, PFLOFFZREC*> CPflOffZRecArray;

typedef struct tagRdXSecOffElevData
{
	double	dXScale;
	double	dYScale;
	double	dNumAnnSize;
	double	dRowAnnSize;
	double	dRowHt;
	double	dCSLabelAnnSize;

	double	dSheetWid;
	double	dSheetHeight;
	double	dNPlateWid;
	double	dNPlateHeight;
	
	double	dStartChg;
	int		iDrawPathWIthMinMaxElev;//0 = Don't draw, Ask user for PID
									//1 = Draw PID having Min Elevs
									//2 = Calculate PID having Min Elevs and Draw C/S 
									//4 = Draw PID having Max Elevs
									//8 = Calculate PID having Max Elevs and Draw C/S 
	BOOL	bDrawNorthingEastingAnnSection;
	double	fOrdInterval;
	BOOL	bDrawGeneratedPoints;

	char	szOffsetTag[_MAX_PATH];
	char	szElevTag[_MAX_PATH];
	char	szNorthingTag[_MAX_PATH];
	char	szEastingTag[_MAX_PATH];
	char	szDatumTagPrefix[_MAX_PATH];
	char	szDatumTagSuffix[_MAX_PATH];
	char	szXSecLabelTag[_MAX_PATH];
	
	char	szLyrAnnotation[_MAX_PATH];
	char	szLyrTopSurface[_MAX_PATH];
	char	szLyrOrdinate[_MAX_PATH];
	char	szLyrAxis[_MAX_PATH];
	char	szLyrMinMaxPIDPath[_MAX_PATH];

	//Surface Marker...
	char	szXistTopSurfAnnBlk[_MAX_PATH];
	double	dXistTopSurfAnnBlkScale;
	double	dXistTopSurfAnnBlkSpaceMin;

	char	szOutFileCSD[_MAX_PATH];
	char	szOutFilePID[_MAX_PATH];

	AcGePoint3d	PtOrg;
	
	AcGePoint3dArray arrayDatumRanges;
	double			 fDatumDenominator;
	double			 fMinOrdinateHt;

} RDXSECOFFELEVDATA;

typedef struct tagCSDDwgData
{
	double	dXScale;
	double	dYScale;
	
	double	dNumAnnSize;
	double	dRowAnnSize;
	double	dCSLabelAnnSize;
	double	dRowHt;
	double	fOrdInterval;

	double	dStartChg;
	double	dEndChg;

	double	dSheetWid;
	double	dSheetHeight;
	double	dNPlateWid;
	double	dNPlateHeight;

	char	szOffsetTag[_MAX_PATH];
	char	szElevTag[_MAX_PATH];
	char	szDatumTag[_MAX_PATH];
	char	szXSecLabelTag[_MAX_PATH];
	
	char	szLyrAnnotation[_MAX_PATH];
	char	szLyrTopSurface[_MAX_PATH];
	char	szLyrOrdinate[_MAX_PATH];
	char	szLyrAxis[_MAX_PATH];

	//Top Surface Related.......
	char	szXistTopSurfAnnBlk[_MAX_PATH];
	double	dXistTopSurfAnnBlkSpaceMin;
	double	dXistTopSurfAnnBlkScale;

	char	szDataFile[_MAX_PATH];

	double	fDatumDenominator;
	double	fMinOrdinateHt;
} CSDDWGPARAM;

typedef struct tagCSDDwgDataEx
{
	double	dXScale;
	double	dYScale;
	
	double	dNumAnnSize;
	double	dRowAnnSize;
	double	dCSLabelAnnSize;
	double	dRowHt;

	double	dStartChg;
	double	dEndChg;

	double	dSheetWid;
	double	dSheetHeight;
	double	dNPlateWid;
	double	dNPlateHeight;

	char	szOffsetTag[_MAX_PATH];
	char	szElevTag[_MAX_PATH];
	char	szElevTagDsgn[_MAX_PATH];	//New
	char	szDatumTag[_MAX_PATH];
	char	szXSecLabelTag[_MAX_PATH];
	

	char	szLyrAnnotation[_MAX_PATH];
	char	szLyrTopSurface[_MAX_PATH];
	char	szLyrOrdinate[_MAX_PATH];
	char	szLyrAxis[_MAX_PATH];

	char	szLyrAnnotationDsgn[_MAX_PATH];	//New
	char	szLyrTopSurfaceDsgn[_MAX_PATH];	//New
	char	szLyrOrdinateDsgn[_MAX_PATH];	//New

	char	szDwgSheet[_MAX_PATH];	//New

	char	szDataFile[_MAX_PATH];
	char	szDataFileDsgn[_MAX_PATH];
	char	szDataFileOut[_MAX_PATH];

	//Existing Top Surface Related.......
	char	szXistTopSurfAnnBlk[_MAX_PATH];
	double	dXistTopSurfAnnBlkSpaceMin;
	double	dXistTopSurfAnnBlkScale;
	//Proposed Top Surface Related.......UNUSED...KEPT FOR FUTURE.......
	char	szPropTopSurfAnnBlk[_MAX_PATH];
	double	dPropTopSurfAnnBlkScale;
	double	dPropTopSurfAnnBlkScaleMin;
}
CSDDWGPARAMEX;


typedef struct tagDrawParamRDPlanFrTxtFile
{
	double	dBlkScaleX;
	double	dBlkScaleY;
	double	dNumAnnSize;

	BOOL	bConsiderZ;

	char	szBlkName[_MAX_PATH];

	char	szLyrAnnotation[_MAX_PATH];
	char	szLyrPID[_MAX_PATH];

	char	szDataFileOut[_MAX_PATH];
	char	szDataFilePID[_MAX_PATH];
	char	szDataFileSTN[_MAX_PATH];
}
DRAWPARAMRDPLANFRTXTFILE;

typedef struct tagTravVertex
{
	char			szStn[_MAX_PATH];
	double			fIncAngle; //Of this station
	double			fDistance; //Prev Station to this Station <<AntiClock wise>>
	//Calculation
	double			fBearing;	//Prev Station to this Station <<AntiClock wise in AutoCAD System>>
	double			fIncAngleAdj;
	double			fNorthing;
	double			fEasting;
	double			fNorthingAdj;
	double			fEastingAdj;

} TRAVVERTEX;
typedef CTypedPtrArray<CPtrArray, TRAVVERTEX*> CTravVertexArray;

typedef struct tagTravDwgParam
{
	double			fStnMarkTxtSize;
	AcGePoint3d		ptTravBase;
	//Table Output
	double			fTableTxtSize;
	double			fTableMargin;
	AcGePoint3d		ptTableBase;
	//
	char			szInFile[_MAX_PATH];
	char			szOutFile[_MAX_PATH];
	//Layers.......Hardcoded...for Internal use only
	char			szLyrCalcTable[64];
	char			szLyrTraverseOrg[64];
	char			szLyrTraverseAdj[64];
} TRAVDWGPARAM;

typedef struct tagCSDRec
{
	long	lBit;
	double	dOffset;
	double	dElev;
	
	CString strTxt;
	CString strBlk;

} CSDREC;
typedef CTypedPtrArray<CPtrArray, CSDREC*> CCSDataRecArray;

typedef struct tagCSStnData
{
	double			dDatum;
	double			dChainage;
	CCSDataRecArray	Array;
	
} CSSTNDATA;
typedef CTypedPtrArray<CPtrArray, CSSTNDATA*> CCSStnRecArray;

typedef struct tagROADPROFILE_CHG_Z
{
	double		dChainage,
				dElevXisting,
				dElevXistingDsgn,
				dElevDsgn;
	char		szDesc[_MAX_PATH],
				szBlock[_MAX_PATH];
	long		lBit;
} ROADPROFILE_CHG_Z;
typedef CTypedPtrArray<CPtrArray, ROADPROFILE_CHG_Z*> CRPflChgZDataRecs;

typedef struct tagGRADDETAIL
{
	double		dDist,
				dSlope1inValue,
				dElev;
} GRADDETAIL;
typedef CTypedPtrArray<CPtrArray, GRADDETAIL*> CGradDataRecs;

typedef struct tagRDPFL_CHG_Z_DWG_PARAM
{
	char		szXElevTag[_MAX_PATH],
				szDsgnElevTag[_MAX_PATH], 
				szExistingDsgnElevTag[_MAX_PATH],	//24-Sep-2006
				szDsgnElevDifferenceTag[_MAX_PATH], //24-Sep-2006
				szCutFillTag[_MAX_PATH],
				szDatum[_MAX_PATH],
				szChgTag[_MAX_PATH];
	
	char		szLyrAxis[_MAX_PATH],
				szLyrOrdinate[_MAX_PATH],
				szLyrAnnotation[_MAX_PATH],
				szLyrTopSurfaceOrg[_MAX_PATH],
				szLyrTopSurfaceDsgn[_MAX_PATH],
				szLyrTopSurfaceDsgnExisting[_MAX_PATH], //24-Sep-2006
				szLyrAnnotationEx[_MAX_PATH];
	
	double		dXScale,
				dYScale,
				dLabTxtSize,
				dNumAnnTxtSize,
				dRowHt,
				dDatum;
	double		fOrdInterval;
	double		fTxtAnnotationAngleInDeg,
				fTxtAnnotationMargin;

	//Top Surface Related.......
	char		szXistTopSurfAnnBlk[_MAX_PATH];
	double		dXistTopSurfAnnBlkSpaceMin,
				dXistTopSurfAnnBlkScale;

} RDPFL_CHG_Z_DWG_PARAM;


typedef struct tagAXISDETAILXLSPFL
{
	double	fLeftOfYAxis;
	double	fRiteOfYAxis;
	double	fTopOfXAxis;
	double	fBotOfXAxis;

	double fTxtWidMF;
	double fTitleXtn;
	double fMargin;
	double iMaxDatumDigits;
} AXISDETAILXLSPFL;

typedef struct tag_XLSPROFILEORDINATE
{
	long	lBit;
	double	dCumDist;
	
	double	dNorth;
	double	dEast;
	double	dElev;

	CString strTxt;
	CString strBlk;

} XLSPROFILEORDINATE;
typedef CTypedPtrArray<CPtrArray, XLSPROFILEORDINATE*> CXLSProfileOrdinateArray;


typedef struct tagXLSXSECDWGPARAM
{
	double	dXScale;
	double	dYScale;
	
	double	dNumAnnSize;
	double	dRowAnnSize;
	double	dCSLabelAnnSize;
	double	dRowHt;

	double	dDatum;

	double	dSheetWid;
	double	dSheetHeight;
	double	dNPlateWid;
	double	dNPlateHeight;


	char	szElevTag[_MAX_PATH];
	char	szNorthingTag[_MAX_PATH];
	char	szEastingTag[_MAX_PATH];
	char	szOffsetTag[_MAX_PATH];
	
	char	szDatumTag[_MAX_PATH];
	char	szXSecLabelTag[_MAX_PATH];
	
	char	szLyrDwgSheet[_MAX_PATH];
	char	szLyrAnnotation[_MAX_PATH];
	char	szLyrTopSurface[_MAX_PATH];
	char	szLyrOrdinate[_MAX_PATH];
	char	szLyrAxis[_MAX_PATH];

	//Top Surface Related.......
	char	szXistTopSurfAnnBlk[_MAX_PATH];
	double	dXistTopSurfAnnBlkSpaceMin;
	double	dXistTopSurfAnnBlkScale;

} XLSXSECDWGPARAM;

typedef struct tagFBAlignment
{
	double		fAngleInDeg,
				fStartChg,
				fEndChg,
				fChainage;
} FBALIGNMENT;
typedef CTypedPtrArray<CPtrArray, FBALIGNMENT*> CFBAlignmentRecArray;

typedef struct tagFBDwgParam
{
	double		fTxtSize,
				fBlkScale;
	char		szLyr[_MAX_PATH];
} FBDWGPARAM;

typedef struct tagFBSurveyObj
{
	long		iPointNo,
				iBitCode;

	double		fChainage,
				fOffset,
				fElev;
	char		szLyr[_MAX_PATH],
				szText[_MAX_PATH],
				szBlock[_MAX_PATH];
} FBSURVEYOBJ;
typedef CTypedPtrArray<CPtrArray, FBSURVEYOBJ*> CFBSurveyObjRecArray;

typedef struct tagFBObjDef
{
	char		szEntity[_MAX_PATH];
	CUIntArray	arrayPtNos;
} FBOBJDEF;
typedef CTypedPtrArray<CPtrArray, FBOBJDEF*>	CFBObjDefsArray;

typedef struct tagAlgnScale
{
	double		fStartChg;
	double		fEndChg;
	double		fScale;
} FBALGNSCALE;
typedef CTypedPtrArray<CPtrArray, FBALGNSCALE*> CFBAlgnScalesArray;

typedef struct tagInsBlkFrFileParams
{
	double	dBlkScaleX;
	double	dBlkScaleY;
	double	dBlkRotAngle;
	BOOL	bInsConsideringZValue;
	
	double	dTxtSize;
	double	dTxtRotation;

	BOOL	bHasDataZValue;

	char	szBlkName[_MAX_PATH];
	
	char	szLyrAnnotation[_MAX_PATH];
	char	szDataFile[_MAX_PATH];
} INSBLKFRFILEPARAMS;

typedef struct tagAXISDETAIL
{
	double	fLeftOfYAxis;
	double	fRiteOfYAxisA;
	double	fRiteOfYAxisB;
	double	fTopOfXAxis;
	double	fBotOfXAxis;

	double fTxtWidMF;
	double fTitleXtn;
	double fMargin;
	double iMaxDatumDigits;
} AXISDETAIL;

typedef struct tagCSDRecEx
{
	long	lBit;
	double	dOffset;
	double	dSlope;
	double	dCalcElev;
	
	CString strTxt;
	CString strBlk;

} CSDRECEX;
typedef CTypedPtrArray<CPtrArray, CSDRECEX*> CCSDataRecArrayEx;

typedef struct tag_NEZXSECORDINATE
{
	long	lBit;
	double	dOffset;
	
	double	dElev;
	double	dNorth;
	double	dEast;

	CString strTxt;
	CString strBlk;
} NEZXSECORDINATE;
typedef CTypedPtrArray<CPtrArray, NEZXSECORDINATE*> CNEZXSecOrdinateArray;

#endif /*  _STRUCTURES_H */