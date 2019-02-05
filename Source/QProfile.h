// QProfile.h : header file
//

#ifndef _QPROFILE_H
#define _QPROFILE_H

#include "RdPflNEZDataDlg.h"

void DrawQProfile(AcGePoint3dArray&, const RDPFLNEZDATA&, const AcGePoint3dArray* = 0L, const CStringArray* = 0L);
void AnnotateQProfile(const double&, const AcGePoint3dArray&, const AcGePoint3dArray&, const RDPFLNEZDATA&);


#endif //_QPROFILE_H

