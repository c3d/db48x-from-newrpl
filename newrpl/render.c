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


}
