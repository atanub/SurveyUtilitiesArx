// TraverseAdj.h : header file
//

#ifndef _TRAVERSEADJ_H
#define _TRAVERSEADJ_H

#include "Structures.h"

BOOL ParseAngleStr(const CString&, double&);
void AdjTravUsingBowditchRule();

CString FormatAngle(const double&);
double ConvertBearing(const double&);
double ConvertAcadAngleToBearing(const double&);

#endif //_TRAVERSEADJ_H


