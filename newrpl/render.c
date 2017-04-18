/*
 * Copyright (c) 2017 Claudio Lapilli and the newRPL Team
 * All rights reserved,
 * This file is released under the 3-clause BSD license,
 * See the file LICENSE,txt that shipped with this distribution,
 */

// THIS FILE CONTAINS A GENERIC SCAN-LINE RENDERER
// ALL MATH USES fastmath,c 40,24 FIXED POINT

#include "newrpl.h"
#include "render.h"


// EVALUATE A CUBIC BEZIER FOR THE PARAMETER t (0 TO 1)
void rndEvalCurve(FPINT t,CURVEPT * bezier,CURVEPT *result)
{
// B(t)=
//    (1-t)^3 * P0 +
//   3*(1-t)^2 * t * P1 +
//   3*(1-t) * t^2 * P2
//   t^3 * P3

    // SINCE t IS GUARANTEED TO BE 0-1, t<=2^24 THEREFORE T*T<=2^48 AND NO OVERFLOW CAN OCCUR

    // THIS COULD BE REPLACED WITH TABLES FOR t BETWEEN 0-1024 AND DO LINEAR INTERPOLATION IN BETWEEN

    FPINT oneminust=((1<<24)-t);
    FPINT tsq=(t*t)>>24;
    FPINT oneminustsq=(oneminust*oneminust)>>24;

    FPINT w0=(oneminust*oneminustsq)>>24;
    FPINT w1=(oneminustsq*t*3)>>24;
    FPINT w2=(tsq*oneminust*3)>>24;
    FPINT w3=(t*tsq)>>24;

    result->x=mulFPINT(w0,bezier[0].x)+mulFPINT(w1,bezier[1].x)+mulFPINT(w2,bezier[2].x)+mulFPINT(w3,bezier[3].x);
    result->y=mulFPINT(w0,bezier[0].y)+mulFPINT(w1,bezier[1].y)+mulFPINT(w2,bezier[2].y)+mulFPINT(w3,bezier[3].y);

}

// SAME AS THE OTHER FUNCTION BUT USES INTERPOLATION AND RETURNS THE FIRST DERIVATIVE

void rndEvalCurve2(FPINT t,CURVEPT * bezier,CURVEPT *result,CURVEPT *deriv)
{
CURVEPT temp[3+2+1];

// UNROLLED AND NOT RECURSIVE
temp[0].x=mulFPINT(t,bezier[0].x)+mulFPINT((1<<24)-t,bezier[1].x);
temp[0].y=mulFPINT(t,bezier[0].y)+mulFPINT((1<<24)-t,bezier[1].y);

temp[1].x=mulFPINT(t,bezier[1].x)+mulFPINT((1<<24)-t,bezier[2].x);
temp[1].y=mulFPINT(t,bezier[1].y)+mulFPINT((1<<24)-t,bezier[2].y);

temp[2].x=mulFPINT(t,bezier[2].x)+mulFPINT((1<<24)-t,bezier[3].x);
temp[2].y=mulFPINT(t,bezier[2].y)+mulFPINT((1<<24)-t,bezier[3].y);

temp[3].x=mulFPINT(t,temp[0].x)+mulFPINT((1<<24)-t,temp[1].x);
temp[3].y=mulFPINT(t,temp[0].y)+mulFPINT((1<<24)-t,temp[1].y);

temp[4].x=mulFPINT(t,temp[1].x)+mulFPINT((1<<24)-t,temp[2].x);
temp[4].y=mulFPINT(t,temp[1].y)+mulFPINT((1<<24)-t,temp[2].y);

result->x=mulFPINT(t,temp[3].x)+mulFPINT((1<<24)-t,temp[4].x);
result->y=mulFPINT(t,temp[3].y)+mulFPINT((1<<24)-t,temp[4].y);

deriv->x=temp[4].x-temp[3].x;
deriv->y=temp[4].y-temp[3].y;

}

// SCANS A POLYGON INTO A BUFFER. THE BUFFER HAS PAIRS OF FPINT NUMBERS
// THE FUNCTION WILL ALLOCATE A BUFFER AS LARGE AS NEEDED AND RETURN ITS POINTER
// THE BUFFER WILL BE INSIDE A TEMPORARY OBJECT IN TEMPOB (GC MIGHT BE TRIGGERED)
// THE BUFFER CONTAINS PAIRS OF SCANS STARTX+LENGTH. WHEN LENGTH<0 THEN Y COORDINATE
// MUST BE INCREMENTED BY ONE PIXEL AFTER PAINTING SCAN WITH (-LENGTH),
// OTHERWISE NEXT SCAN IS ON THE SAME LINE

// ARGUMENTS: poly = (CURVEPT *)POINTER TO THE CURVE DATA
//            starty = FIRST SCAN LINE TO GENERATE
//            endy = LAST SCAN LINE TO GENERATE

void rndScanPolygon(BINT npoints,CURVEPT *poly, FPINT starty, FPINT endy, FPINT **result)
{
BINT k;

// SCAN CONVERT ALL LINES
for(k=0;k<npoints;++k)
{
    switch(poly[k].type)
    {
    case TYPE_STARTPOINT:
    case TYPE_CTLPT:
    case TYPE_CLOSEEND:
        break;  // NOTHING TO DO!
    case TYPE_LINE:
        // SCAN THE LINE AT [k-1] TO [k]
    {
        FPINT incx=divFPINT((poly[k].x-poly[k-1].x),poly[k].y-poly[k-1].y);
        FPINT dy=0;




    }

    case TYPE_CURVE:
    {
        // SCAN A CUBIC BEZIER CURVE



    }


        break;

    }
}


}
