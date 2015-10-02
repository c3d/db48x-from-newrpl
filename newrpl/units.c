/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include "newrpl.h"
#include "libraries.h"


// SYSTEM UNIT NAME TABLE: CONTAINS NAMES FOR ALL SYSTEM DEFINED UNITS
const WORD const system_unit_names[]={
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('m',0,0,0),                           // [0]='m'
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','g',0,0),                                  // [2]='kg'
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('s',0,0,0),                           // [4]='s'
    MKPROLOG(DOIDENT,1),TEXT2WORD('a',0,0,0),                           // [6]='a'     (are=100 m^2)
    MKPROLOG(DOIDENT,1),TEXT2WORD(0xe2,0x84,0xab,0),                            // [8]='Å'     (1e-10 m)
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('A',0,0,0),                           // [10]='A'     (Ampere)
    MKPROLOG(DOIDENT,1),TEXT2WORD('a','c','r','e'),                             // [12]='acre'    (acre international)
    MKPROLOG(DOIDENT,2),TEXT2WORD('a','c','r','e'),TEXT2WORD('U','S',0,0),      // [14]='acreUS'    (acre US Survey)
    MKPROLOG(DOIDENT,2),TEXT2WORD('a','r','c','m'),TEXT2WORD('i','n',0,0),      // [17]='arcmin'
    MKPROLOG(DOIDENT,1),TEXT2WORD('a','r','c','s'),                              // [20]='arcs'
    MKPROLOG(DOIDENT,1),TEXT2WORD('a','t','m', 0 ),                              // [22]='atm'
    MKPROLOG(DOIDENT,1),TEXT2WORD('a','u', 0 , 0 ),                              // [24]='au'
    MKPROLOG(DOIDENT,1),TEXT2WORD('b', 0 , 0 , 0 ),                              // [26]='b'
    MKPROLOG(DOIDENT,1),TEXT2WORD('b','a','r', 0 ),                              // [28]='bar'
    MKPROLOG(DOIDENT,1),TEXT2WORD('b','b','l', 0 ),                              // [30]='bbl'
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('B','q', 0 , 0 ),                      // [32]='Bq'
    MKPROLOG(DOIDENT,1),TEXT2WORD('B','t','u', 0 ),                              // [34]='Btu'
    MKPROLOG(DOIDENT,1),TEXT2WORD('b','u', 0 , 0 ),                              // [36]='bu'
    MKPROLOG(DOIDENT,1),TEXT2WORD(0xc2,0xb0,'C', 0 ),                            // [38]='°C'
    MKPROLOG(DOIDENT,2),TEXT2WORD(0xce,0x94,0xc2,0xb0),TEXT2WORD('C',0,0,0),     // [40]='Δ°C'
    MKPROLOG(DOIDENT,1),TEXT2WORD('c', 0 , 0 , 0 ),                              // [43]='c' (speed of light)
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('C', 0 , 0 , 0 ),                      // [45]='C'
    MKPROLOG(DOIDENT,1),TEXT2WORD('c','a','l', 0 ),                              // [47]='cal'
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','c','a','l'),                              // [49]='kcal'
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('c','d', 0 , 0 ),                      // [51]='cd'
    MKPROLOG(DOIDENT,2),TEXT2WORD('c','h','a','i'),TEXT2WORD('n',0,0,0),         // [53]='chain'
    MKPROLOG(DOIDENT,1),TEXT2WORD('C','i', 0 , 0 ),                              // [56]='Ci' (Curie)
    MKPROLOG(DOIDENT,1),TEXT2WORD('c','t', 0 , 0 ),                              // [58]='ct' (carat)
    MKPROLOG(DOIDENT,1),TEXT2WORD('c','u', 0 , 0 ),                              // [60]='cu' (US cup)
    MKPROLOG(DOIDENT,1),TEXT2WORD(0xc2,0xb0, 0 , 0 ),                            // [62]='°' (angular degree)
    MKPROLOG(DOIDENT,1),TEXT2WORD('d', 0 , 0 , 0 ),                              // [64]='d' (day)
    MKPROLOG(DOIDENT,1),TEXT2WORD('d','y','n', 0 ),                              // [66]='dyn'
    MKPROLOG(DOIDENT,1),TEXT2WORD('e','r','g', 0 ),                              // [68]='erg'
    MKPROLOG(DOIDENT,1),TEXT2WORD('e','V', 0 , 0 ),                              // [70]='eV'
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('F', 0 , 0 , 0 ),                      // [72]='F'
    MKPROLOG(DOIDENT,1),TEXT2WORD(0xc2,0xb0,'F', 0 ),                            // [74]='°F'
    MKPROLOG(DOIDENT,2),TEXT2WORD(0xce,0x94,0xc2,0xb0),TEXT2WORD('F',0,0,0),     // [76]='Δ°F'
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','a','t','h'),                              // [79]='fath' (fathom)
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','b','m', 0 ),                              // [81]='fbm' (board foot)
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','c', 0 , 0 ),                              // [83]='fc' (footcandle)
    MKPROLOG(DOIDENT,1),TEXT2WORD('F','d','y', 0 ),                              // [85]='Fdy' (Faraday)
    MKPROLOG(DOIDENT,2),TEXT2WORD('f','e','r','m'),TEXT2WORD('i',0,0,0),         // [87]='fermi'
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','l','a','m'),                              // [90]='flam' (footlambert)
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','t', 0 , 0 ),                              // [92]='ft'
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','t','U','S'),                              // [94]='ftUS'
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('g', 0 , 0 , 0 ),                      // [96]='g'
    MKPROLOG(DOIDENT,1),TEXT2WORD('g','a', 0 , 0 ),                              // [98]='ga' (gravity's acceleration)
    MKPROLOG(DOIDENT,1),TEXT2WORD('g','a','l', 0 ),                              // [100]='gal' (US gallon)
    MKPROLOG(DOIDENT,1),TEXT2WORD('g','a','l','C'),                              // [102]='galC' (Canadian gallon)
    MKPROLOG(DOIDENT,2),TEXT2WORD('g','a','l','U'),TEXT2WORD('K',0,0,0),         // [104]='galUK' (UK gallon)
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('g','f', 0 , 0 ),                      // [107]='gf' (gram force)
    MKPROLOG(DOIDENT,1),TEXT2WORD('g','r','a','d'),                              // [109]='grad'
    MKPROLOG(DOIDENT,2),TEXT2WORD('g','r','a','i'),TEXT2WORD('n',0,0,0),         // [111]='grain'
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('G','y', 0 , 0 ),                      // [114]='Gy' (Gray)
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('H', 0 , 0 , 0 ),                      // [116]='H' (Henry)
    MKPROLOG(DOIDENT,1),TEXT2WORD('h', 0 , 0 , 0 ),                              // [118]='h' (hour)
    MKPROLOG(DOIDENT,1),TEXT2WORD('h','p', 0 , 0 ),                              // [120]='hp' (horsepower)
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('H','z', 0 , 0 ),                      // [122]='Hz'
    MKPROLOG(DOIDENT,1),TEXT2WORD('i','n', 0 , 0 ),                              // [124]='in'
    MKPROLOG(DOIDENT,1),TEXT2WORD('i','n','H','g'),                              // [126]='inHg'
    MKPROLOG(DOIDENT,2),TEXT2WORD('i','n','H','2'),TEXT2WORD('O',0,0,0),         // [128]='inH2O'
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('J', 0 , 0 , 0 ),                      // [131]='J'
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('K', 0 , 0 , 0 ),                      // [133]='K'
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD(0xce,0x94,'K', 0 ),                    // [135]='ΔK'
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','i','p', 0 ),                              // [137]='kip'
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','n','o','t'),                              // [139]='knot'
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','p','h', 0 ),                              // [141]='kph'  (kilometers per hour)
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('l', 0 , 0 , 0 ),                      // [143]='l'   (liter)
    MKPROLOG(DOIDENT,1),TEXT2WORD('l','a','m', 0 ),                              // [145]='lam' (Lambert)
    MKPROLOG(DOIDENT,1),TEXT2WORD('l','b', 0 , 0 ),                              // [147]='lb'
    MKPROLOG(DOIDENT,1),TEXT2WORD('l','b','f', 0 ),                              // [149]='lbf'

    MKPROLOG(DOIDENT,1),TEXT2WORD('l','b','t', 0 ),                              // [151]='lbt' (pound troy)
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('l','m', 0 , 0 ),                      // [153]='lm' (lumen)
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('l','x', 0 , 0 ),                      // [155]='lx' (lux)
    MKPROLOG(DOIDENT,1),TEXT2WORD('l','y','r', 0 ),                              // [157]='lyr' (light year)
    MKPROLOG(DOIDENT,1),TEXT2WORD(0xce,0xbc, 0 , 0 ),                            // [159]='μ'   (micron)
    MKPROLOG(DOIDENT,1),TEXT2WORD('m','h','o', 0 ),                              // [161]='mho'
    MKPROLOG(DOIDENT,1),TEXT2WORD('m','i', 0 , 0 ),                              // [163]='mi'
    MKPROLOG(DOIDENT,1),TEXT2WORD('m','i','l', 0 ),                              // [165]='mil'  (mils)
    MKPROLOG(DOIDENT,1),TEXT2WORD('m','i','n', 0 ),                              // [167]='min'
    MKPROLOG(DOIDENT,1),TEXT2WORD('m','i','U','S'),                              // [169]='miUS' (mile US)
    MKPROLOG(DOIDENT,1),TEXT2WORD('m','m','H','g'),                              // [171]='mmHg'
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('m','o','l', 0 ),                      // [173]='mol'
    MKPROLOG(DOIDENT,1),TEXT2WORD('m','p','h', 0 ),                              // [175]='mph'
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('N', 0 , 0 , 0 ),                      // [177]='N'
    MKPROLOG(DOIDENT,1),TEXT2WORD('n','m','i', 0 ),                              // [179]='nmi' (nautical mile)
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD(0xce,0xa9, 0 , 0 ),                    // [181]='Ω' (Ohm)
    MKPROLOG(DOIDENT,1),TEXT2WORD('o','z', 0 , 0 ),                              // [183]='oz' (ounce)
    MKPROLOG(DOIDENT,1),TEXT2WORD('o','z','f','l'),                              // [185]='ozfl' (US fluid ounce)
    MKPROLOG(DOIDENT,1),TEXT2WORD('o','z','t', 0 ),                              // [187]='ozt' (Troy ounce)
    MKPROLOG(DOIDENT,1),TEXT2WORD('o','z','U','K'),                              // [189]='ozUK' (UK ounce)
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('P', 0 , 0 , 0 ),                      // [191]='P' (poise)
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('P','a', 0 , 0 ),                      // [193]='Pa' (pascal)
    MKPROLOG(DOIDENT,1),TEXT2WORD('p','c', 0 , 0 ),                              // [195]='pc' (parsec)
    MKPROLOG(DOIDENT,1),TEXT2WORD('p','d','l', 0 ),                              // [197]='pdl' (poundal)
    MKPROLOG(DOIDENT,1),TEXT2WORD('p','h', 0 , 0 ),                              // [199]='ph' (phot)
    MKPROLOG(DOIDENT,1),TEXT2WORD('p','k', 0 , 0 ),                              // [201]='pk' (peck)
    MKPROLOG(DOIDENT,1),TEXT2WORD('p','s','i', 0 ),                              // [203]='psi'
    MKPROLOG(DOIDENT,1),TEXT2WORD('p','t', 0 , 0 ),                              // [205]='pt' (pint)
    MKPROLOG(DOIDENT,1),TEXT2WORD('q','t', 0 , 0 ),                              // [207]='qt' (quart)
    MKPROLOG(DOIDENT,1),TEXT2WORD('r', 0 , 0 , 0 ),                              // [209]='r' (radian)
    MKPROLOG(DOIDENT,1),TEXT2WORD('R', 0 , 0 , 0 ),                              // [211]='R' (roentgen)
    MKPROLOG(DOIDENT,1),TEXT2WORD(0xc2,0xb0,'R', 0 ),                            // [213]='°R' (Rankine)
    MKPROLOG(DOIDENT,2),TEXT2WORD(0xce,0x94,0xc2,0xb0),TEXT2WORD('R',0,0,0),     // [215]='Δ°R'
    MKPROLOG(DOIDENT,1),TEXT2WORD('r','a','d', 0 ),                              // [218]='rad' (rad)
    MKPROLOG(DOIDENT,1),TEXT2WORD('r','d', 0 , 0 ),                              // [220]='rd' (rod international)
    MKPROLOG(DOIDENT,1),TEXT2WORD('r','d','U','S'),                              // [222]='rdUS' (rod US survey)
    MKPROLOG(DOIDENT,1),TEXT2WORD('p','t','U','K'),                              // [224]='ptUK' (pint imperial)
    MKPROLOG(DOIDENT,1),TEXT2WORD('q','t','U','K'),                              // [226]='qtUK' (quart imperial)

    MKPROLOG(DOIDENT,1),TEXT2WORD('r','e','m', 0 ),                              // [228]='rem' (roentgen equivalent man)
    MKPROLOG(DOIDENT,1),TEXT2WORD('r','p','m', 0 ),                              // [230]='rpm' (revolutions per minute)
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('S', 0 , 0 , 0 ),                      // [232]='S' (Siemens)
    MKPROLOG(DOIDENT,1),TEXT2WORD('s','b', 0 , 0 ),                              // [234]='sb' (stilb)
    MKPROLOG(DOIDENT,1),TEXT2WORD('s','l','u','g'),                              // [236]='slug'
    MKPROLOG(DOIDENT,1),TEXT2WORD('s','r', 0 , 0 ),                              // [238]='sr' (steradian)
    MKPROLOG(DOIDENT,1),TEXT2WORD('s','t', 0 , 0 ),                              // [240]='st' (stere)
    MKPROLOG(DOIDENT,1),TEXT2WORD('S','t', 0 , 0 ),                              // [242]='St' (Stokes)
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('S','v', 0 , 0 ),                      // [244]='Sv' (Sievert)
    MKPROLOG(DOIDENT,1),TEXT2WORD('t', 0 , 0 , 0 ),                              // [246]='t' (metric ton)
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('T', 0 , 0 , 0 ),                      // [248]='T' (Tesla)
    MKPROLOG(DOIDENT,1),TEXT2WORD('t','b','s','p'),                              // [250]='tbsp' (tablespoon)
    MKPROLOG(DOIDENT,2),TEXT2WORD('t','h','e','r'),TEXT2WORD('m',0,0,0),         // [252]='therm' (EEC therm)
    MKPROLOG(DOIDENT,1),TEXT2WORD('t','o','n', 0 ),                              // [255]='ton' (short ton)
    MKPROLOG(DOIDENT,2),TEXT2WORD('t','o','n','U'),TEXT2WORD('K',0,0,0),         // [257]='tonUK' (long ton UK)
    MKPROLOG(DOIDENT,1),TEXT2WORD('t','o','r','r'),                              // [260]='torr' (=mmHg)
    MKPROLOG(DOIDENT,1),TEXT2WORD('t','s','p', 0 ),                              // [262]='tsp' (teaspoon)
    MKPROLOG(DOIDENT,1),TEXT2WORD('u', 0 , 0 , 0 ),                              // [264]='u' (unified atomic mass)
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('V', 0 , 0 , 0 ),                      // [266]='V' (Volt)
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('W', 0 , 0 , 0 ),                      // [268]='W' (Watt)
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('W','b', 0 , 0 ),                      // [270]='Wb' (Weber)
    MKPROLOG(DOIDENT,1),TEXT2WORD('y','d', 0 , 0 ),                              // [272]='yd' (yard)
    MKPROLOG(DOIDENT,1),TEXT2WORD('y','r', 0 , 0 ),                              // [274]='yr' (year)


    MKPROLOG(DOIDENT,1),TEXT2WORD(0xcf,0x80,0,0),          // [276]='π'   (INTERNAL CONSTANT PI AS A UNIT)
    MKPROLOG(DOIDENT,1),TEXT2WORD('?','C','D','G'),          // [278]='?CDG'  (INTERNAL CONSTANT 180 FOR DEGREE CONVERSION)
    MKPROLOG(DOIDENT,1),TEXT2WORD('?','C','F','T'),          // [280]='?CFT'  (INTERNAL CONSTANT 3937 FOR US ft DEFINITION)



};

// SYSTEM UNIT DEFINITION TABLE: CONTAINS THE VALUES OF ALL SYSTEM DEFINED UNITS
const WORD const system_unit_defs[]={
    // [0] = are
    MKPROLOG(DOUNIT,5),
    MAKESINT(100),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(2),MAKESINT(1),

    // [6] = Å
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-10,1,0),1,       // 1e-10
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(1),MAKESINT(1),

    //[14] = acre
    MKPROLOG(DOUNIT,5),
    MAKESINT(4840),
    MKPROLOG(DOIDENT,1),TEXT2WORD('y','d',0,0),MAKESINT(2),MAKESINT(1),

    //[20] = acreUS
    MKPROLOG(DOUNIT,5),
    MAKESINT(4840),
    MKPROLOG(DOIDENT,1),TEXT2WORD('y','d','U','S'),MAKESINT(2),MAKESINT(1),

    //[26] = arcmin
    MKPROLOG(DOUNIT,13),
    MAKESINT(3),
    MKPROLOG(DOIDENT,1),TEXT2WORD('?','C','D','G'),MAKESINT(-2),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD(0xcf,0x80,0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('r',0,0,0),MAKESINT(1),MAKESINT(1),

    //[40] = arcs
    MKPROLOG(DOUNIT,13),
    MAKESINT(9),
    MKPROLOG(DOIDENT,1),TEXT2WORD('?','C','D','G'),MAKESINT(-3),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD(0xcf,0x80,0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('r',0,0,0),MAKESINT(1),MAKESINT(1),

    //[54] = atm
    MKPROLOG(DOUNIT,5),
    MAKESINT(101325),
    MKPROLOG(DOIDENT,1),TEXT2WORD('P','a',0,0),MAKESINT(1),MAKESINT(1),

    //[60] = au
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DECBINT,2),3568982636,34,
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(1),MAKESINT(1),

    //[68] = b
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-28,1,0),1,       // 1e-28
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(2),MAKESINT(1),

    //[76] = bar
    MKPROLOG(DOUNIT,5),
    MAKESINT(100000),
    MKPROLOG(DOIDENT,1),TEXT2WORD('P','a',0,0),MAKESINT(1),MAKESINT(1),

    //[82] = bbl
    MKPROLOG(DOUNIT,5),
    MAKESINT(42),
    MKPROLOG(DOIDENT,1),TEXT2WORD('g','a','l',0),MAKESINT(1),MAKESINT(1),

    //[88] = Bq
    MKPROLOG(DOUNIT,5),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-1),MAKESINT(1),

    //[94] = Btu
    MKPROLOG(DOUNIT,8),
    MKPROLOG(DOREAL,3),MAKEREALFLAGS(-8,2,0),5585262,1055,       // 1055.05585262
    MKPROLOG(DOIDENT,1),TEXT2WORD('J',0,0,0),MAKESINT(1),MAKESINT(1),

    //[103] = bu
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-2,1,0),215042,       // 2150.42
    MKPROLOG(DOIDENT,1),TEXT2WORD('i','n',0,0),MAKESINT(3),MAKESINT(1),

    //[111] = Δ°C
    MKPROLOG(DOUNIT,5),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD(0xce,0x94,'K',0),MAKESINT(1),MAKESINT(1),

    //[117] = c
    MKPROLOG(DOUNIT,11),
    MKPROLOG(DECBINT,2),299792458,0,
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-1),MAKESINT(1),

    //[129] = C
    MKPROLOG(DOUNIT,9),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('A',0,0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(1),MAKESINT(1),

    //[139] = cal
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-4,1,0),41868,       // 4.1868
    MKPROLOG(DOIDENT,1),TEXT2WORD('J', 0 ,0,0),MAKESINT(1),MAKESINT(1),

    //[147] = kcal
    MKPROLOG(DOUNIT,5),
    MAKESINT(1000),
    MKPROLOG(DOIDENT,1),TEXT2WORD('c','a','l',0),MAKESINT(1),MAKESINT(1),

    //[153] = chain
    MKPROLOG(DOUNIT,5),
    MAKESINT(66),
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','t','U','S'),MAKESINT(1),MAKESINT(1),

    //[159] = Ci
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DECBINT,2),2640261632,8,
    MKPROLOG(DOIDENT,1),TEXT2WORD('B','q',0,0),MAKESINT(1),MAKESINT(1),

    //[167] = ct
    MKPROLOG(DOUNIT,5),
    MAKESINT(200),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m','g',0,0),MAKESINT(1),MAKESINT(1),

    //[173] = cu
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-4,1,0),625,       // 1/16
    MKPROLOG(DOIDENT,1),TEXT2WORD('g','a','l',0),MAKESINT(1),MAKESINT(1),

    //[181] = °
    MKPROLOG(DOUNIT,13),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD(0xcf,0x80,0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('?','C','D','G'),MAKESINT(-1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('r',0,0,0),MAKESINT(1),MAKESINT(1),

    //[195] = d
    MKPROLOG(DOUNIT,5),
    MAKESINT(86400),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(1),MAKESINT(1),

    //[201] = dyn
    MKPROLOG(DOUNIT,13),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('g',0,0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('c','m',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-2),MAKESINT(1),

    //[215] = erg
    MKPROLOG(DOUNIT,13),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('g',0,0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('c','m',0,0),MAKESINT(2),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-2),MAKESINT(1),

    //[229] = eV
    MKPROLOG(DOUNIT,8),
    MKPROLOG(DOREAL,3),MAKEREALFLAGS(-29,2,0),21766208,160,       // 1.6021766208e-19 per CODATA 2014
    MKPROLOG(DOIDENT,1),TEXT2WORD('J',0,0,0),MAKESINT(1),MAKESINT(1),

    //[238] = F
    MKPROLOG(DOUNIT,9),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('C',0,0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('V',0,0,0),MAKESINT(-1),MAKESINT(1),

    //[248] = Δ°F
    MKPROLOG(DOUNIT,6),
    MAKESINT(1),
    MKPROLOG(DOIDENT,2),TEXT2WORD(0xce,0x94,0xc2,0xb0),TEXT2WORD('R',0,0,0),MAKESINT(1),MAKESINT(1),     // 'Δ°R'

    //[255] = fath
    MKPROLOG(DOUNIT,5),
    MAKESINT(6),
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','t','U','S'),MAKESINT(1),MAKESINT(1),

    //[261] = fbm
    MKPROLOG(DOUNIT,5),
    MAKESINT(144),
    MKPROLOG(DOIDENT,1),TEXT2WORD('i','n',0,0),MAKESINT(3),MAKESINT(1),

    //[267] = fc
    MKPROLOG(DOUNIT,9),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('l','m',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','t',0,0),MAKESINT(-2),MAKESINT(1),

    //[277] = Fdy
    MKPROLOG(DOUNIT,9),
    MKPROLOG(DOREAL,4),MAKEREALFLAGS(-16,3,0),98760256,53328824,9648,       // 6.022140857e23 * 1.6021766208e-19 per CODATA 2014
    MKPROLOG(DOIDENT,1),TEXT2WORD('C',0,0,0),MAKESINT(1),MAKESINT(1),

    //[287] = zero Celsius
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-2,1,0),27315,       // 273.15
    MKPROLOG(DOIDENT,1),TEXT2WORD('K',0,0,0),MAKESINT(1),MAKESINT(1),

    //[295] = zero Farenheit
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-2,1,0),45967,       // 459.67
    MKPROLOG(DOIDENT,1),TEXT2WORD(0xc2,0xb0,'R', 0 ),MAKESINT(1),MAKESINT(1),

    //[303] = fermi
    MKPROLOG(DOUNIT,5),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','m',0,0),MAKESINT(1),MAKESINT(1),

    //[309] = flam
    MKPROLOG(DOUNIT,13),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD(0xcf,0x80,0,0),MAKESINT(-1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('c','d',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','t',0,0),MAKESINT(-2),MAKESINT(1),

    //[323] = ft
    MKPROLOG(DOUNIT,5),
    MAKESINT(12),
    MKPROLOG(DOIDENT,1),TEXT2WORD('i','n',0,0),MAKESINT(1),MAKESINT(1),

    //[329] = ftUS
    MKPROLOG(DOUNIT,9),
    MAKESINT(1200),
    MKPROLOG(DOIDENT,1),TEXT2WORD('?','C','F','T'),MAKESINT(-1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(1),MAKESINT(1),

    //[339] = g
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-3,1,0),1,       // 0.001
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','g',0,0),MAKESINT(1),MAKESINT(1),

    //[347] = ga
    MKPROLOG(DOUNIT,11),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-5,1,0),980665,       // 9.80665
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-2),MAKESINT(1),

    //[359] = gal
    MKPROLOG(DOUNIT,5),
    MAKESINT(231),
    MKPROLOG(DOIDENT,1),TEXT2WORD('i','n',0,0),MAKESINT(3),MAKESINT(1),

    //[365] = galC = galUK
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-8,1,0),454609,       // 0.00454609
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(3),MAKESINT(1),

    //[373] = gf
    MKPROLOG(DOUNIT,9),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('g',0,0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('g','a',0,0),MAKESINT(1),MAKESINT(1),

    //[383] = grad
    MKPROLOG(DOUNIT,11),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-3,1,0),5,       // 1/200
    MKPROLOG(DOIDENT,1),TEXT2WORD(0xcf,0x80,0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('r',0,0,0),MAKESINT(1),MAKESINT(1),

    //[395] = grain
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-11,1,0),6479891,       // 0.00006479891
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','g',0,0),MAKESINT(1),MAKESINT(1),

    //[403] = Gy
    MKPROLOG(DOUNIT,9),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('J',0,0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','g',0,0),MAKESINT(-1),MAKESINT(1),

    //[413] = H
    MKPROLOG(DOUNIT,9),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('W','b',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('A',0,0,0),MAKESINT(-1),MAKESINT(1),

    //[423] = h
    MKPROLOG(DOUNIT,5),
    MAKESINT(3600),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(1),MAKESINT(1),

    //[429] = hp
    MKPROLOG(DOUNIT,13),
    MAKESINT(550),
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','t',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('l','b','f',0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-1),MAKESINT(1),

    //[443] = Hz
    MKPROLOG(DOUNIT,5),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-1),MAKESINT(1),

    //[449] = in
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-4,1,0),254,       // 0.0254
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(1),MAKESINT(1),

    //[457] = inHg
    MKPROLOG(DOUNIT,19),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-1,1,0),135951,       // 13595.1
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','g',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(-3),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('i','n',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('g','a',0,0),MAKESINT(1),MAKESINT(1),

    //[477] = inH2O
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-2,1,0),24884,       // 248.84
    MKPROLOG(DOIDENT,1),TEXT2WORD('P','a',0,0),MAKESINT(1),MAKESINT(1),

    //[485] = kip
    MKPROLOG(DOUNIT,5),
    MAKESINT(1000),
    MKPROLOG(DOIDENT,1),TEXT2WORD('l','b','f',0),MAKESINT(1),MAKESINT(1),

    //[491] = knot
    MKPROLOG(DOUNIT,9),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('n','m','i',0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('h',0,0,0),MAKESINT(-1),MAKESINT(1),

    //[501] = kph
    MKPROLOG(DOUNIT,9),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','m',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('h',0,0,0),MAKESINT(-1),MAKESINT(1),

    //[511] = l
    MKPROLOG(DOUNIT,5),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('d','m',0,0),MAKESINT(3),MAKESINT(1),

    //[517] = lam
    MKPROLOG(DOUNIT,13),
    MAKESINT(10000),
    MKPROLOG(DOIDENT,1),TEXT2WORD(0xcf,0x80,0,0),MAKESINT(-1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('c','d',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(-2),MAKESINT(1),

    //[531] = lb
    MKPROLOG(DOUNIT,6),
    MAKESINT(7000),
    MKPROLOG(DOIDENT,2),TEXT2WORD('g','r','a','i'),TEXT2WORD('n',0,0,0),MAKESINT(1),MAKESINT(1),

    //[538] = lbf
    MKPROLOG(DOUNIT,9),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('l','b',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('g','a',0,0),MAKESINT(1),MAKESINT(1),

    //[548] = 'lbt'
    MKPROLOG(DOUNIT,6),
    MAKESINT(5760),
    MKPROLOG(DOIDENT,2),TEXT2WORD('g','r','a','i'),TEXT2WORD('n',0,0,0),MAKESINT(1),MAKESINT(1),

    //[555]='lm' (lumen)
    MKPROLOG(DOUNIT,9),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('c','d',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s','r',0,0),MAKESINT(1),MAKESINT(1),

    //[565]='lx' (lux)
    MKPROLOG(DOUNIT,9),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('l','m',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(-2),MAKESINT(1),


     //[575]='lyr' (light year)
    MKPROLOG(DOUNIT,11),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-2,1,0),36525,       // 365.25
    MKPROLOG(DOIDENT,1),TEXT2WORD('d',0,0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('c',0,0,0),MAKESINT(1),MAKESINT(1),

    //[587]='pi'   (INTERNAL CONSTANT PI)
    CMD_PINUM,
    //[588]='?CDG' (constant 180 for degree and other conversions)
    MAKESINT(180),


    //[589]='?CFT'  (INTERNAL CONSTANT 3937 FOR US ft DEFINITION)
    MAKESINT(3937),

    0,0,0,


    //[593]='μ'   (micron)
    MKPROLOG(DOUNIT,5),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD(0xce,0xbc,'m',0),MAKESINT(1),MAKESINT(1),

    //[599]='mho' = 'S'
    MKPROLOG(DOUNIT,9),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('A',0,0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('V',0,0,0),MAKESINT(-1),MAKESINT(1),

    //[609]='mi'
    MKPROLOG(DOUNIT,5),
    MAKESINT(5280),
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','t',0,0),MAKESINT(1),MAKESINT(1),

    //[615]='mil'  (mils)
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-3,1,0),1,       // 0.001
    MKPROLOG(DOIDENT,1),TEXT2WORD('i','n',0,0),MAKESINT(1),MAKESINT(1),


    //[623]='min'
    MKPROLOG(DOUNIT,5),
    MAKESINT(60),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(1),MAKESINT(1),

    //[629]='miUS' (mile US)
    MKPROLOG(DOUNIT,5),
    MAKESINT(5280),
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','t','U','S'),MAKESINT(1),MAKESINT(1),

    //[635]='mmHg'
    MKPROLOG(DOUNIT,19),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-1,1,0),135951,       // 13595.1
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','g',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(-3),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m','m',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('g','a',0,0),MAKESINT(1),MAKESINT(1),

    //[655]='mph'
    MKPROLOG(DOUNIT,9),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m','i',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('h',0,0,0),MAKESINT(-1),MAKESINT(1),

    // [665]='N'
    MKPROLOG(DOUNIT,13),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','g',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-2),MAKESINT(1),

    //[679]='nmi' (nautical mile)
    MKPROLOG(DOUNIT,5),
    MAKESINT(1852),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(1),MAKESINT(1),

    //[685]='Ω' (Ohm)
    MKPROLOG(DOUNIT,9),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('V',0,0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('A',0,0,0),MAKESINT(-1),MAKESINT(1),

    //[695]='oz' (ounce)
    MKPROLOG(DOUNIT,8),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-1,1,0),4375,       // 437.5
    MKPROLOG(DOIDENT,2),TEXT2WORD('g','r','a','i'),TEXT2WORD('n',0,0,0),MAKESINT(1),MAKESINT(1),

    //[704]='ozfl' (US fluid ounce)
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-7,1,0),78125,       // 0.0078125 = 1/128
    MKPROLOG(DOIDENT,1),TEXT2WORD('g','a','l',0),MAKESINT(1),MAKESINT(1),

    //[712]='ozt' (Troy ounce)
    MKPROLOG(DOUNIT,6),
    MAKESINT(480),
    MKPROLOG(DOIDENT,2),TEXT2WORD('g','r','a','i'),TEXT2WORD('n',0,0,0),MAKESINT(1),MAKESINT(1),

    //[719]='ozUK' (UK ounce)
    MKPROLOG(DOUNIT,8),
    MKPROLOG(DOREAL,3),MAKEREALFLAGS(-13,2,0),84130625,2,       // 2.84130625e-5
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(3),MAKESINT(1),

    //[728]='P' (poise)
    MKPROLOG(DOUNIT,11),
    MKPROLOG(DOREAL,1),MAKEREALFLAGS(-1,1,0),1,       // 0.1
    MKPROLOG(DOIDENT,1),TEXT2WORD('P','a',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(1),MAKESINT(1),

    //[740]='Pa' (pascal)
    MKPROLOG(DOUNIT,13),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','g',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(-1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-2),MAKESINT(1),

    //[754]='pc' (parsec)
    MKPROLOG(DOUNIT,11),
    MKPROLOG(DECBINT,2),648000,0,
    MKPROLOG(DOIDENT,1),TEXT2WORD(0xcf,0x80,0,0),MAKESINT(-1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('a','u',0,0),MAKESINT(1),MAKESINT(1),


    //[766]='pdl' (poundal)
    MKPROLOG(DOUNIT,13),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('l','b',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','t',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-2),MAKESINT(1),

    //[780]='ph' (phot)
    MKPROLOG(DOUNIT,9),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('l','m',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('c','m',0,0),MAKESINT(-2),MAKESINT(1),

    //[790]='pk' (peck)
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-3,1,0),537605,       // 537.605
    MKPROLOG(DOIDENT,1),TEXT2WORD('i','n',0,0),MAKESINT(3),MAKESINT(1),

    //[798]='psi'
    MKPROLOG(DOUNIT,9),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('l','b','f',0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('i','n',0,0),MAKESINT(-2),MAKESINT(1),

    //[808]='pt' (pint)
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-3,1,0),125,       // 0.125 = 1/8
    MKPROLOG(DOIDENT,1),TEXT2WORD('g','a','l',0),MAKESINT(1),MAKESINT(1),

    //[816]='qt' (quart)
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-2,1,0),25,       // 0.25 = 1/4
    MKPROLOG(DOIDENT,1),TEXT2WORD('g','a','l',0),MAKESINT(1),MAKESINT(1),

    //[824]='R' (roentgen)
    MKPROLOG(DOUNIT,11),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-6,1,0),258,       // 2.58e-4
    MKPROLOG(DOIDENT,1),TEXT2WORD('C',0,0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','g',0,0),MAKESINT(-1),MAKESINT(1),


    // [836]='°R' (Rankine)
    MKPROLOG(DOUNIT,9),
    MAKESINT(100),
    MKPROLOG(DOIDENT,1),TEXT2WORD('?','C','D','G'),MAKESINT(-1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('K',0,0,0),MAKESINT(1),MAKESINT(1),

    // [846]='Δ°R'
    MKPROLOG(DOUNIT,9),
    MAKESINT(100),
    MKPROLOG(DOIDENT,1),TEXT2WORD('?','C','D','G'),MAKESINT(-1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD(0xce,0x94,'K',0),MAKESINT(1),MAKESINT(1),


    //[856]='rad' (rad)
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-2,1,0),1,       // 0.01
    MKPROLOG(DOIDENT,1),TEXT2WORD('G','y',0,0),MAKESINT(1),MAKESINT(1),

    //[864]='rd' (rod)
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-1,1,0),165,       // 16.5
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','t',0,0),MAKESINT(1),MAKESINT(1),

    //[872]='rdUS' (rod)
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-1,1,0),165,       // 16.5
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','t','U','S'),MAKESINT(1),MAKESINT(1),

    //[880]='ptUK' (pint)
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-3,1,0),125,       // 0.125 = 1/8
    MKPROLOG(DOIDENT,1),TEXT2WORD('g','a','l','C'),MAKESINT(1),MAKESINT(1),

    //[888]='qtUK' (quart)
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-2,1,0),25,       // 0.25 = 1/4
    MKPROLOG(DOIDENT,1),TEXT2WORD('g','a','l','C'),MAKESINT(1),MAKESINT(1),

    // [896]='J'
    MKPROLOG(DOUNIT,13),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','g',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(2),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-2),MAKESINT(1),

    // [910]='rem' (roentgen equivalent man)
    MKPROLOG(DOUNIT,11),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-2,1,0),1,       // 0.01
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(2),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-2),MAKESINT(1),


    // [922]='rpm' (revolutions per minute)
    MKPROLOG(DOUNIT,5),
    MAKESINT(60),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-1),MAKESINT(1),

    // [928]='sb' (stilb)
    MKPROLOG(DOUNIT,9),
    MAKESINT(10000),
    MKPROLOG(DOIDENT,1),TEXT2WORD('c','d',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(-2),MAKESINT(1),


    // [938]='slug'
    MKPROLOG(DOUNIT,17),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('l','b',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(2),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','t',0,0),MAKESINT(-1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('g','a',0,0),MAKESINT(1),MAKESINT(1),

    // [956]='st' (stere)
    MKPROLOG(DOUNIT,5),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(3),MAKESINT(1),


    // [962]='St' (Stokes)
    MKPROLOG(DOUNIT,11),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-4,1,0),1,       // 0.0001
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(2),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-1),MAKESINT(1),

    // [974]='Sv' (Sievert)
    MKPROLOG(DOUNIT,9),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(2),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-2),MAKESINT(1),

    // [984]='t' (metric ton)
    MKPROLOG(DOUNIT,5),
    MAKESINT(1000),
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','g',0,0),MAKESINT(1),MAKESINT(1),


    // [990]='T' (Tesla)
    MKPROLOG(DOUNIT,13),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','g',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('A',0,0,0),MAKESINT(-1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-2),MAKESINT(1),

    // [1004]='tbsp' (tablespoon)
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-1,1,0),5,       // 0.5
    MKPROLOG(DOIDENT,1),TEXT2WORD('o','z','f','l'),MAKESINT(1),MAKESINT(1),

    // [1012]='therm' (EEC therm)
    MKPROLOG(DOUNIT,5),
    MAKESINT(100000),
    MKPROLOG(DOIDENT,1),TEXT2WORD('B','t','u',0),MAKESINT(1),MAKESINT(1),

    // [1018]='ton' (short ton)
    MKPROLOG(DOUNIT,5),
    MAKESINT(2000),
    MKPROLOG(DOIDENT,1),TEXT2WORD('l','b',0,0),MAKESINT(1),MAKESINT(1),



    // [1024]='tonUK' (long ton UK)
    MKPROLOG(DOUNIT,5),
    MAKESINT(2240),
    MKPROLOG(DOIDENT,1),TEXT2WORD('l','b',0,0),MAKESINT(1),MAKESINT(1),

    // [____]='torr' (=mmHg)
    // NO NEED FOR SEPARATE DEFINITION, USE THE mmHg DEFINITION

    // [1030]='tsp' (teaspoon)
    MKPROLOG(DOUNIT,9),
    MAKESINT(30),
    MKPROLOG(DOIDENT,1),TEXT2WORD('?','C','D','G'),MAKESINT(-1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('o','z','f','l'),MAKESINT(1),MAKESINT(1),


    // [1040]='u' (unified atomic mass)
    MKPROLOG(DOUNIT,8),
    MKPROLOG(DOREAL,3),MAKEREALFLAGS(-36,2,0),60539040,16,       // 1.660 539 040 x 10-27 kg
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','g',0,0),MAKESINT(1),MAKESINT(1),

    // [1049]='V' (Volt)
    MKPROLOG(DOUNIT,17),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','g',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(2),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('A',0,0,0),MAKESINT(-1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-3),MAKESINT(1),

    // [1067]='W' (Watt)
    MKPROLOG(DOUNIT,13),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','g',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(2),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-3),MAKESINT(1),


    // [1081]='Wb' (Weber)
    MKPROLOG(DOUNIT,17),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('k','g',0,0),MAKESINT(1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(2),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('A',0,0,0),MAKESINT(-1),MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(-2),MAKESINT(1),


    // [1099]='yd' (yard)
    MKPROLOG(DOUNIT,5),
    MAKESINT(3),
    MKPROLOG(DOIDENT,1),TEXT2WORD('f','t',0,0),MAKESINT(1),MAKESINT(1),

    // [1105]='yr' (year)
    MKPROLOG(DOUNIT,8),
    MKPROLOG(DOREAL,3),MAKEREALFLAGS(-4,2,0),69259747,3155,       // 31556925.9747
    MKPROLOG(DOIDENT,1),TEXT2WORD('s',0,0,0),MAKESINT(1),MAKESINT(1),

    // [1114]='ΔK'
    MKPROLOG(DOUNIT,5),
    MAKESINT(1),
    MKPROLOG(DOIDENT,1),TEXT2WORD('K',0,0,0),MAKESINT(1),MAKESINT(1)


};

// SYSTEM UNIT DEFINITION DIRECTORY: CONTAINS PONTERS TO NAME/VALUE PAIRS FOR ALL SYSTEM UNITS
const WORDPTR const system_unit_dir[]={
    (WORDPTR)&system_unit_names[0],(WORDPTR)&one_bint,            // 'm'=1
    (WORDPTR)&system_unit_names[2],(WORDPTR)&one_bint,            // 'kg'=1
    (WORDPTR)&system_unit_names[4],(WORDPTR)&one_bint,            // 's'=1
    (WORDPTR)&system_unit_names[6],(WORDPTR)&system_unit_defs[0], // 'a'=1_m^2
    (WORDPTR)&system_unit_names[8],(WORDPTR)&system_unit_defs[6], // 'Å'=1e-10_m
    (WORDPTR)&system_unit_names[10],(WORDPTR)&one_bint,            // 'A'=1
    (WORDPTR)&system_unit_names[12],(WORDPTR)&system_unit_defs[14], // 'acre'=4840_yd
    (WORDPTR)&system_unit_names[14],(WORDPTR)&system_unit_defs[20], // 'acreUS'=4840_ydUS
    (WORDPTR)&system_unit_names[17],(WORDPTR)&system_unit_defs[26], // 'arcmin'=3*(180^-2)*pi_r
    (WORDPTR)&system_unit_names[20],(WORDPTR)&system_unit_defs[40], // 'arcs'=9*(180^-3)*pi_r
    (WORDPTR)&system_unit_names[22],(WORDPTR)&system_unit_defs[54], // 'atm'=101325_Pa
    (WORDPTR)&system_unit_names[24],(WORDPTR)&system_unit_defs[60], // 'au'=149597870700_m
    (WORDPTR)&system_unit_names[26],(WORDPTR)&system_unit_defs[68], // 'b'=1e-28_m
    (WORDPTR)&system_unit_names[28],(WORDPTR)&system_unit_defs[76], // 'bar'=1e5_Pa
    (WORDPTR)&system_unit_names[30],(WORDPTR)&system_unit_defs[82], // 'bbl'=42_gal
    (WORDPTR)&system_unit_names[32],(WORDPTR)&system_unit_defs[88], // 'Bq'=1_1/s
    (WORDPTR)&system_unit_names[34],(WORDPTR)&system_unit_defs[94], // 'Btu'=1055.05585262_J
    (WORDPTR)&system_unit_names[36],(WORDPTR)&system_unit_defs[103],// 'bu'=2150.42_in^3
    (WORDPTR)&system_unit_names[38],(WORDPTR)&one_bint,             // '°C' IS A BASE UNIT BECAUSE IT'S INCONSISTENT UNLESS SPECIAL CASES
    (WORDPTR)&system_unit_names[40],(WORDPTR)&system_unit_defs[111], // 'Δ°C'=1_ΔK
    (WORDPTR)&system_unit_names[43],(WORDPTR)&system_unit_defs[117], // 'c'=299792458_m/s
    (WORDPTR)&system_unit_names[45],(WORDPTR)&system_unit_defs[129], // 'C'=1_A*s
    (WORDPTR)&system_unit_names[47],(WORDPTR)&system_unit_defs[139], // 'cal'=4.1868_J
    (WORDPTR)&system_unit_names[49],(WORDPTR)&system_unit_defs[147], // 'kcal'=1000_cal
    (WORDPTR)&system_unit_names[51],(WORDPTR)&one_bint,              // 'cd'=1
    (WORDPTR)&system_unit_names[53],(WORDPTR)&system_unit_defs[153], // 'chain'=66_ftUS
    (WORDPTR)&system_unit_names[56],(WORDPTR)&system_unit_defs[159], // 'Ci'=3.7e10_Bq
    (WORDPTR)&system_unit_names[58],(WORDPTR)&system_unit_defs[167], // 'ct'=200_mg
    (WORDPTR)&system_unit_names[60],(WORDPTR)&system_unit_defs[173], // 'cu'=1/16_gal (US cup)
    (WORDPTR)&system_unit_names[62],(WORDPTR)&system_unit_defs[181], // '°'=pi/180_r
    (WORDPTR)&system_unit_names[64],(WORDPTR)&system_unit_defs[195], // 'd'=86400_s
    (WORDPTR)&system_unit_names[66],(WORDPTR)&system_unit_defs[201], // 'dyn'=1_g*cm/s^2
    (WORDPTR)&system_unit_names[68],(WORDPTR)&system_unit_defs[215], // 'erg'=1_g*cm^2/s^2
    (WORDPTR)&system_unit_names[70],(WORDPTR)&system_unit_defs[229], // 'eV'=1.6021766208e-19_J
    (WORDPTR)&system_unit_names[72],(WORDPTR)&system_unit_defs[238], // 'F'=1_C/V
    (WORDPTR)&system_unit_names[74],(WORDPTR)&one_bint,              // '°F' IS A BASE UNIT BECAUSE IT'S INCONSISTENT UNLESS SPECIAL CASES
    (WORDPTR)&system_unit_names[76],(WORDPTR)&system_unit_defs[248], // 'Δ°F'=1_Δ°R
    (WORDPTR)&system_unit_names[79],(WORDPTR)&system_unit_defs[255], // 'fath'=6_ftUS
    (WORDPTR)&system_unit_names[81],(WORDPTR)&system_unit_defs[261], // 'fbm'=144_in^3
    (WORDPTR)&system_unit_names[83],(WORDPTR)&system_unit_defs[267], // 'fc'=1_lm/ft^2
    (WORDPTR)&system_unit_names[85],(WORDPTR)&system_unit_defs[277], // 'Fdy'=9648.5332882498760256_C (per CODATA 2014)
    (WORDPTR)&system_unit_names[87],(WORDPTR)&system_unit_defs[303], // 'fermi'=1_fm
    (WORDPTR)&system_unit_names[90],(WORDPTR)&system_unit_defs[309], // 'flam'=1/pi_cd/ft^2
    (WORDPTR)&system_unit_names[92],(WORDPTR)&system_unit_defs[323], // 'ft'=12_in'
    (WORDPTR)&system_unit_names[94],(WORDPTR)&system_unit_defs[329], // 'ftUS'=1200/3937_m
    (WORDPTR)&system_unit_names[96],(WORDPTR)&system_unit_defs[339], // 'g'=0.001_kg
    (WORDPTR)&system_unit_names[98],(WORDPTR)&system_unit_defs[347], // 'ga'=9.80665_m/s^2
    (WORDPTR)&system_unit_names[100],(WORDPTR)&system_unit_defs[359], // 'gal'=231_in^3
    (WORDPTR)&system_unit_names[102],(WORDPTR)&system_unit_defs[365], // 'galC'='galUK'=0.00454609_m^3
    (WORDPTR)&system_unit_names[104],(WORDPTR)&system_unit_defs[365], // 'galC'='galUK'=0.00454609_m^3
    (WORDPTR)&system_unit_names[107],(WORDPTR)&system_unit_defs[373], // 'gf'=1_g*ga
    (WORDPTR)&system_unit_names[109],(WORDPTR)&system_unit_defs[383], // 'grad'=pi/200_r
    (WORDPTR)&system_unit_names[111],(WORDPTR)&system_unit_defs[395], // 'grain'=0.00006479891
    (WORDPTR)&system_unit_names[114],(WORDPTR)&system_unit_defs[403], // 'Gy'=1_J/kg
    (WORDPTR)&system_unit_names[116],(WORDPTR)&system_unit_defs[413], // 'H'=1_Wb/A
    (WORDPTR)&system_unit_names[118],(WORDPTR)&system_unit_defs[423], // 'h'=3600_s
    (WORDPTR)&system_unit_names[120],(WORDPTR)&system_unit_defs[429], // 'hp'=550_ft*lbf/s
    (WORDPTR)&system_unit_names[122],(WORDPTR)&system_unit_defs[443], // 'Hz'=1/s
    (WORDPTR)&system_unit_names[124],(WORDPTR)&system_unit_defs[449], // 'in'=0.0254_m
    (WORDPTR)&system_unit_names[126],(WORDPTR)&system_unit_defs[457], // 'inHg'=13595.1_kg/m^3*in*ga
    (WORDPTR)&system_unit_names[128],(WORDPTR)&system_unit_defs[477], // 'inH2O'=248.84_Pa (at 60°F)
    (WORDPTR)&system_unit_names[131],(WORDPTR)&system_unit_defs[896], // 'J'=1_N*m
    (WORDPTR)&system_unit_names[133],(WORDPTR)&one_bint,              // 'K'=1
    (WORDPTR)&system_unit_names[135],(WORDPTR)&system_unit_defs[1114],// 'ΔK'=1_K
    (WORDPTR)&system_unit_names[137],(WORDPTR)&system_unit_defs[485], // 'kip'=1000_lbf
    (WORDPTR)&system_unit_names[139],(WORDPTR)&system_unit_defs[491], // 'knot'=1_nmi/h
    (WORDPTR)&system_unit_names[141],(WORDPTR)&system_unit_defs[501], // 'kph'=1_km/h
    (WORDPTR)&system_unit_names[143],(WORDPTR)&system_unit_defs[511], // 'l'=1_dm^3
    (WORDPTR)&system_unit_names[145],(WORDPTR)&system_unit_defs[517], // 'lam'=10000/pi_cd/m^2
    (WORDPTR)&system_unit_names[147],(WORDPTR)&system_unit_defs[531], // 'lb'=7000_grain
    (WORDPTR)&system_unit_names[149],(WORDPTR)&system_unit_defs[538], // 'lbf'=1_lb*ga

    (WORDPTR)&system_unit_names[151],(WORDPTR)&system_unit_defs[548], // 'lbt'=5760_grain
    (WORDPTR)&system_unit_names[153],(WORDPTR)&system_unit_defs[555], // 'lm'=1_cd*sr
    (WORDPTR)&system_unit_names[155],(WORDPTR)&system_unit_defs[565], // 'lx'=1_lm*m^2
    (WORDPTR)&system_unit_names[157],(WORDPTR)&system_unit_defs[575], // 'lyr'=365.25_d*c
    (WORDPTR)&system_unit_names[159],(WORDPTR)&system_unit_defs[593], // 'μ'=1_μm
    (WORDPTR)&system_unit_names[161],(WORDPTR)&system_unit_defs[599], // 'mho'=1_A/V
    (WORDPTR)&system_unit_names[163],(WORDPTR)&system_unit_defs[609], // 'mi'=5280_ft
    (WORDPTR)&system_unit_names[165],(WORDPTR)&system_unit_defs[615], // 'mil'=0.001_in
    (WORDPTR)&system_unit_names[167],(WORDPTR)&system_unit_defs[623], // 'min'=60_s
    (WORDPTR)&system_unit_names[169],(WORDPTR)&system_unit_defs[629], // 'miUS'=5280_ftUS
    (WORDPTR)&system_unit_names[171],(WORDPTR)&system_unit_defs[635], // 'mmHg'=1_13595.1_kg/m^3*mm*ga
    (WORDPTR)&system_unit_names[173],(WORDPTR)&one_bint,              // 'mol'=1 (base unit)
    (WORDPTR)&system_unit_names[175],(WORDPTR)&system_unit_defs[655], // 'mph'=1_mi/h
    (WORDPTR)&system_unit_names[177],(WORDPTR)&system_unit_defs[665], // 'N'=1_kg*m/s^2
    (WORDPTR)&system_unit_names[179],(WORDPTR)&system_unit_defs[679], // 'nmi'=1852_m
    (WORDPTR)&system_unit_names[181],(WORDPTR)&system_unit_defs[685], // 'Ω'= 1_V/A
    (WORDPTR)&system_unit_names[183],(WORDPTR)&system_unit_defs[695], // 'oz'= 437.5_grain
    (WORDPTR)&system_unit_names[185],(WORDPTR)&system_unit_defs[704], // 'ozfl'= 1/128_gal
    (WORDPTR)&system_unit_names[187],(WORDPTR)&system_unit_defs[712], // 'ozt'= 480_grain
    (WORDPTR)&system_unit_names[189],(WORDPTR)&system_unit_defs[719], // 'ozUK'= 1/160_galUK = 2.84130625e-5_m^3
    (WORDPTR)&system_unit_names[191],(WORDPTR)&system_unit_defs[728], // 'P'=0.1_Pa*s
    (WORDPTR)&system_unit_names[193],(WORDPTR)&system_unit_defs[740], // 'Pa'=1_N/m^2 = 1_kg/m/s^2
    (WORDPTR)&system_unit_names[195],(WORDPTR)&system_unit_defs[754], // 'pc'=648000/pi_au
    (WORDPTR)&system_unit_names[197],(WORDPTR)&system_unit_defs[766], // 'pdl'= 1_lb*ft/s^2
    (WORDPTR)&system_unit_names[199],(WORDPTR)&system_unit_defs[780], // 'ph'= 1_lm/cm^2
    (WORDPTR)&system_unit_names[201],(WORDPTR)&system_unit_defs[790], // 'pk'=1/4_bu
    (WORDPTR)&system_unit_names[203],(WORDPTR)&system_unit_defs[798], // 'psi'=1_lbf/in^2
    (WORDPTR)&system_unit_names[205],(WORDPTR)&system_unit_defs[808], // 'pt'=1/8_gal
    (WORDPTR)&system_unit_names[207],(WORDPTR)&system_unit_defs[816], // 'qt'=1/4_gal
    (WORDPTR)&system_unit_names[209],(WORDPTR)&one_bint,              // 'r'=1
    (WORDPTR)&system_unit_names[211],(WORDPTR)&system_unit_defs[824], // 'R'=2.58e-4_C/kg
    (WORDPTR)&system_unit_names[213],(WORDPTR)&system_unit_defs[836], // '°R'=5/9_K
    (WORDPTR)&system_unit_names[215],(WORDPTR)&system_unit_defs[846], // 'Δ°R'=5/9_ΔK
    (WORDPTR)&system_unit_names[218],(WORDPTR)&system_unit_defs[856], // 'rad'=0.01_Gy
    (WORDPTR)&system_unit_names[220],(WORDPTR)&system_unit_defs[864], // 'rd'=16.5_ft
    (WORDPTR)&system_unit_names[222],(WORDPTR)&system_unit_defs[872], // 'rdUS'=16.5_ftUS
    (WORDPTR)&system_unit_names[224],(WORDPTR)&system_unit_defs[880], // 'ptUK'=1/8_galUK
    (WORDPTR)&system_unit_names[226],(WORDPTR)&system_unit_defs[888], // 'qtUK'=1/4_galUK

    (WORDPTR)&system_unit_names[228],(WORDPTR)&system_unit_defs[910], // 'rem'=0.01_m^2/s^2
    (WORDPTR)&system_unit_names[230],(WORDPTR)&system_unit_defs[922], // 'rpm'=60_1/s
    (WORDPTR)&system_unit_names[232],(WORDPTR)&system_unit_defs[599], // 'S'=1_A/V
    (WORDPTR)&system_unit_names[234],(WORDPTR)&system_unit_defs[928], // 'sb'=10000_cd/m^2
    (WORDPTR)&system_unit_names[236],(WORDPTR)&system_unit_defs[938], // 'slug'=1_lb*s^2/ft*ga
    (WORDPTR)&system_unit_names[238],(WORDPTR)&one_bint,              // 'sr'=1
    (WORDPTR)&system_unit_names[240],(WORDPTR)&system_unit_defs[956], // 'st'=1_m^3
    (WORDPTR)&system_unit_names[242],(WORDPTR)&system_unit_defs[962], // 'St'=0.0001_m^2/s
    (WORDPTR)&system_unit_names[244],(WORDPTR)&system_unit_defs[974], // 'Sv'=1_m^2/s^2
    (WORDPTR)&system_unit_names[246],(WORDPTR)&system_unit_defs[984], // 't'=1000_kg
    (WORDPTR)&system_unit_names[248],(WORDPTR)&system_unit_defs[990], // 'T'=1_kg/A/s^2
    (WORDPTR)&system_unit_names[250],(WORDPTR)&system_unit_defs[1004], // 'tbsp'=0.5_ozfl
    (WORDPTR)&system_unit_names[252],(WORDPTR)&system_unit_defs[1012], // 'therm'=100000_Btu
    (WORDPTR)&system_unit_names[255],(WORDPTR)&system_unit_defs[1018], // 'ton'=2000_lb
    (WORDPTR)&system_unit_names[257],(WORDPTR)&system_unit_defs[1024], // 'tonUK'=2240_lb
    (WORDPTR)&system_unit_names[260],(WORDPTR)&system_unit_defs[635], // 'torr'=1_mmHg
    (WORDPTR)&system_unit_names[262],(WORDPTR)&system_unit_defs[1030], // 'tsp'=1/6_ozfl
    (WORDPTR)&system_unit_names[264],(WORDPTR)&system_unit_defs[1040], // 'u'= 1.660539040E-27_kg
    (WORDPTR)&system_unit_names[266],(WORDPTR)&system_unit_defs[1049], // 'V'=1_kg*m^2/A/s^3
    (WORDPTR)&system_unit_names[268],(WORDPTR)&system_unit_defs[1067], // 'W'=1_J/s=1_kg*m^2/s^3
    (WORDPTR)&system_unit_names[270],(WORDPTR)&system_unit_defs[1081], // 'Wb'=1_kg*m^2/A/s^2
    (WORDPTR)&system_unit_names[272],(WORDPTR)&system_unit_defs[1099], // 'yd'=3_ft
    (WORDPTR)&system_unit_names[274],(WORDPTR)&system_unit_defs[1105], // 'yr'=31556925.9747_s (tropical year)

    (WORDPTR)&system_unit_names[276],(WORDPTR)&system_unit_defs[587], // 'π'= π0 = 3.141592..... (VARIABLE PRECISION)
    (WORDPTR)&system_unit_names[278],(WORDPTR)&system_unit_defs[588], // '?CDG'=180 (NON-DIMENSIONAL)
    (WORDPTR)&system_unit_names[280],(WORDPTR)&system_unit_defs[589], // '?CFT'=3937 (NON-DIMENSIONAL)


    0,0                                         // NULL TERMINATED LIST
};



// THESE ARE SPECIAL UNITS THAT NEED TO BE REPLACED
// °C AND °F, AND THE REPLACEMENT IS DONE BY ADDING A CONSTANT RATHER THAN MULTIPLYING
// IT EFFECTIVELY CONVERTS °C INTO K, AND °F INTO °R

const WORDPTR const system_unit_special[]={
    // SPECIAL UNIT NAME ,     DELTA UNIT NAME      ,   ABSOLUTE UNIT EQUIVALENT OF THE ZERO IN THE SCALE
    (WORDPTR)&system_unit_names[38],(WORDPTR)&system_unit_names[40],(WORDPTR)&system_unit_defs[287],
    (WORDPTR)&system_unit_names[74],(WORDPTR)&system_unit_names[76],(WORDPTR)&system_unit_defs[295],
    (WORDPTR)&system_unit_names[133],(WORDPTR)&system_unit_names[135],0,
    (WORDPTR)&system_unit_names[213],(WORDPTR)&system_unit_names[215],0,

    // ADD HERE RANKINE AND KELVIN FOR CONVERSION TO THEIR DELTA TYPES
    0,0,0
};








// EXPLODES THE UNIT OBJECT IN THE STACK
// RETURNS THE NUMBER OF LEVELS USED IN THE STACK
// (n): VALUE
// (n-1): IDENTIFIER
// (n-2): NUMERATOR
// (n-3): DENOMINATOR
// (...): [MORE IDENTIFIERS AND THEIR EXPONENTS]
// (3): LAST IDENTIFIER
// (2): NUMERATOR
// (1): DENOMINATOR

BINT rplUnitExplode(WORDPTR unitobj)
{
    WORDPTR *savestk=DSTop;
if(!ISUNIT(*unitobj)) {
    if(!ISBINT(*unitobj) && !ISPROLOG(*unitobj)) {
        // IF IT'S NOT AN OBJECT, THEN IT'S A COMMAND THAT WILL RETURN A CONSTANT OR A UNIT
        rplCallOperator(*unitobj);
        if(DSTop==savestk+1) {
            // IT RETURNED A SINGLE OBJECT, CHECK IF IT'S A UNIT
            if(ISUNIT(*rplPeekData(1))) unitobj=rplPopData();
            else return DSTop-savestk;
        } else return DSTop-savestk;
    }
    else {
    rplPushData(unitobj);
    return 1;
    }
}

ScratchPointer1=unitobj;
ScratchPointer2=rplSkipOb(unitobj);
BINT count=0;
++ScratchPointer1;
while(ScratchPointer1<ScratchPointer2) {
    // PUSH ALL OBJECTS IN THE STACK AS-IS
    rplPushData(ScratchPointer1);
    ++count;
    if(Exceptions) { DSTop=savestk; return 0; }
    ScratchPointer1=rplSkipOb(ScratchPointer1);
}
return count;
}

// COMPOSE A UNIT OBJECT FROM AN EXPLODED UNIT IN THE STACK
// INVERSE OPERATION OR rplUnitExplode()
// EXPECTS AN EXPLODED UNIT IN THE STACK
// DOES NOT CLEAN UP THE STACK
// WARNING: THIS IS LOW-LEVEL, NO VALIDITY CHECKS DONE HERE

WORDPTR rplUnitAssemble(BINT nlevels)
{
    // A SINGLE VALUE OBJECT WHERE ALL UNITS CANCELLED OUT
    // NO NEED TO CREATE A NEW OBJECT
    if(nlevels==1) return rplPeekData(1);
    // COMPUTE THE REQUIRED SIZE
    BINT size=0;
    BINT lvl;

    for(lvl=1;lvl<=nlevels;++lvl) size+=rplObjSize(rplPeekData(lvl));

    // ALLOCATE MEMORY FOR THE NEW OBJECT
    WORDPTR newobj=rplAllocTempOb(size);
    WORDPTR newptr;
    if(!newobj) return 0;

    // AND FILL IT UP
    newobj[0]=MKPROLOG(DOUNIT,size);
    newptr=newobj+1;
    for(lvl=nlevels;lvl>=1;--lvl) {
    rplCopyObject(newptr,rplPeekData(lvl));
    newptr=rplSkipOb(newptr);
    }

    return newobj;

}

// REMOVE AN ITEM AT THE GIVEN LEVEL OF THE STACK, INCLUDING ITS EXPONENT IF IT'S AN IDENT
// RETURNS THE NUMBER OF ELEMENTS REMOVED FROM THE STACK
BINT rplUnitPopItem(BINT level)
{
    BINT nitems;

    if(level>rplDepthData()) return 0;

    if(ISIDENT(*rplPeekData(level))) nitems=3;
    else nitems=1;

    if(level-nitems<=0) {
        rplDropData(level);
        return nitems;
    }

    memmovew(DSTop-level,DSTop-level+nitems,(level-nitems)*sizeof(WORDPTR*)/sizeof(WORD));
    DSTop-=nitems;
    return nitems;
}


// COPY THE ITEM AT THE BOTTOM OF THE STACK
void rplUnitPickItem(BINT level)
{
    BINT nitems;

    if(level>rplDepthData()) return;

    if(ISIDENT(*rplPeekData(level))) nitems=3;
    else nitems=1;

    if(level-nitems<0) return;

    while(nitems--) rplPushData(rplPeekData(level));

}

// TAKES 2 VALUES OR 2 IDENTIFIERS AND MULTIPLIES THEM TOGETHER
// REMOVES THE SECOND IDENTIFIER FROM THE STACK AND OVERWRITES THE FIRST ELEMENT WITH THE RESULT
// LOW LEVEL, NO CHECKS OF ANY KIND DONE HERE
// RETURNS THE NUMBER OF LEVELS CHANGED IN THE STACK (NEGATIVE=REMOVED ELEMENTS)
BINT rplUnitMulItem(BINT level1,BINT level2)
{
    if(ISIDENT(*rplPeekData(level1))) {
        if(!ISIDENT(*rplPeekData(level2))) {
            // ONE IS VALUE AND ONE IS IDENT, NOTHING TO DO
            return 0;
        }
        // MULTIPLY 2 IDENTIFIERS BY ADDING THEIR EXPONENTS
        if(!rplCompareIDENT(rplPeekData(level1),rplPeekData(level2))) return 0;   // NOTHING TO DO IF DIFFERENT IDENTS
        // COPY THE IDENTIFIER TO THE TOP OF STACK
        WORDPTR *stackptr=DSTop;

        rplPushData(rplPeekData(level1));

        // ADD THE EXPONENTS
        rplPushData(rplPeekData(level1));   // FIRST NUMERATOR
        rplPushData(rplPeekData(level1));   // FIRST DENOMINATOR
        rplPushData(rplPeekData(level2+2)); // SECOND NUMERATOR
        rplPushData(rplPeekData(level2+2)); // SECOND DENOMINATOR
        if(Exceptions) { DSTop=stackptr; return 0; }
        BINT sign=rplFractionAdd();
        if(Exceptions) { DSTop=stackptr; return 0; }

        rplFractionSimplify();
        if(Exceptions) { DSTop=stackptr; return 0; }

        if(sign) {
            // ADD THE SIGN TO THE NUMERATOR
            rplPushData(rplPeekData(2));
            rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_NEG));
            if(Exceptions) { DSTop=stackptr; return 0; }

            rplOverwriteData(3,rplPeekData(1));
            rplDropData(1);
        }

        // OVERWRITE level1 WITH THE NEW VALUES
        rplOverwriteData(level1+3,rplPeekData(3));
        rplOverwriteData(level1+2,rplPeekData(2));
        rplOverwriteData(level1+1,rplPeekData(1));

        rplDropData(3);

        // NOW REMOVE THE ORIGINALS FROM THE STACK
         rplUnitPopItem(level2);
        return -3;
    }

    if(ISIDENT(*rplPeekData(level2))) {
        // ONE IS VALUE AND ONE IS IDENT, NOTHING TO DO
        return 0;
    }

    // NOT AN IDENTIFIER, USE THE OVERLOADED OPERATOR TO MULTIPLY

    WORDPTR *savestk=DSTop;
    rplUnitPickItem(level1);
    rplUnitPickItem(level2+1);
    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_MUL));
    if(Exceptions) { DSTop=savestk; return 0; }

    rplOverwriteData(level1+1,rplPeekData(1));
    rplDropData(1);

    // JUST REMOVE THE ORIGINALS
    rplUnitPopItem(level2);

    return -1;
}


// TAKES 2 VALUES OR 2 IDENTIFIERS AND DOES level2=level2**exponent(level1)
// IF level1 IS A VALUE, THEN IT IS USED AS EXPONENT TO ANY VALUES IN level2 AND
// TO MULTIPLY ANY EXPONENTS IN level2 IDENTIFIERS
// IF level1 IS AN IDENTIFIER, ITS EXPONENT IS USED AS EXPONENT TO ANY VALUES IN level2 AND
// TO MULTIPLY ANY EXPONENTS IN level2 IDENTIFIERS
// DOES NOT REMOVE ANYTHING FROM THE STACK, MODIFIES level2 ON THE SPOT
// LOW LEVEL, NO CHECKS OF ANY KIND DONE HERE
void rplUnitPowItem(BINT level1,BINT level2)
{
    if(ISIDENT(*rplPeekData(level2))) {
        // POW 2 IDENTIFIERS BY MULTIPLYING THEIR EXPONENTS
        WORDPTR *stackptr=DSTop;
        BINT isident=ISIDENT(*rplPeekData(level1));
        if(isident) rplPushData(rplPeekData(level1-1));   // FIRST NUMERATOR
        else rplPushData(rplPeekData(level1));
        rplPushData(rplPeekData(level2)); // SECOND NUMERATOR
        rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_MUL));
        if(Exceptions) { DSTop=stackptr; return; }
        if(isident) {
            rplPushData(rplPeekData(level1-1));   // FIRST DENOMINATOR
            rplPushData(rplPeekData(level2)); // SECOND DENOMINATOR
            rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_MUL));
            if(Exceptions) { DSTop=stackptr; return; }
        } else {
            rplPushData(rplPeekData(level2)); // SECOND DENOMINATOR
        }

        rplFractionSimplify();
        if(Exceptions) { DSTop=stackptr; return; }

        // NOW OVERWRITE THE ORIGINALS FROM THE STACK
        rplOverwriteData(level2+1,rplPeekData(2));
        rplOverwriteData(level2,rplPeekData(1));

        rplDropData(2);

       return;
    }

    // NOT AN IDENTIFIER, USE THE OVERLOADED OPERATOR TO DO THE POWER
    WORDPTR *savestk=DSTop;
    rplUnitPickItem(level2);
    if(ISIDENT(*rplPeekData(level1+1))) {
        // DIVIDE THE NUMERATOR AND DENOMINATOR TO GET AN EXPONENT
        rplPushData(rplPeekData(level1));
        if(*rplPeekData(level1)!=MAKESINT(1)) {
            rplPushData(rplPeekData(level1));
            rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_DIV));
            if(Exceptions) { DSTop=savestk; return; }
        }
    } else rplUnitPickItem(level1+1);

    if(*rplPeekData(1)==MAKESINT(-1)) { rplPopData(); rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_INV)); }
    else if(*rplPeekData(1)!=MAKESINT(1)) rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_POW));
    else rplDropData(1);
    if(Exceptions) { DSTop=savestk; return; }

    // HERE WE SHOULD HAVE A SINGLE VALUE
    rplOverwriteData(level2+1,rplPeekData(1));
    rplDropData(1);

}



// SKIPS AN ITEM (VALUE OR IDENT) AND RETURNS THE LEVEL OF THE NEXT ITEM
BINT rplUnitSkipItem(BINT level)
{
    if(ISIDENT(*rplPeekData(level))) return level-3;
    return level-1;
}

// SIMPLIFY A UNIT IN THE STACK BY COLLAPSING REPEATED IDENTS
// AND PERFORMING FRACTION SIMPLIFICATION OF EXPONENTS
// RETURNS THE NUMBER OF LEVELS LEFT IN THE STACK
// AFTER SIMPLIFICATION
BINT rplUnitSimplify(BINT nlevels)
{
    BINT lvl=nlevels,lvl2,reduction;

    while(lvl>0) {
        lvl2=rplUnitSkipItem(lvl);

        while(lvl2>0) {

                    reduction=rplUnitMulItem(lvl,lvl2);
                    if(Exceptions) return nlevels;
                    lvl+=reduction; // POINT TO THE NEXT ITEM, SINCE THIS ONE VANISHED
                    lvl2+=reduction;
                    nlevels+=reduction;
                    if(!reduction) lvl2=rplUnitSkipItem(lvl2);
        }

        if(ISIDENT(*rplPeekData(lvl))) {
        // CHECK IF EXPONENT ENDED UP BEING ZERO FOR THIS UNIT

        if(*rplPeekData(lvl-1)==MAKESINT(0)) {
         // NEED TO REMOVE THIS UNIT FROM THE LIST
            BINT oldlvl=lvl;
            lvl=rplUnitSkipItem(lvl);
            rplUnitPopItem(oldlvl);
            nlevels-=oldlvl-lvl;
        } else  lvl=rplUnitSkipItem(lvl);
        } else lvl=rplUnitSkipItem(lvl);

    }

    // AT THIS POINT THERE SHOULD BE ONLY ONE VALUE AND MANY UNITS
    // NEEDS TO BE SORTED SO THAT THE VALUE IS FIRST AND UNITS ARE
    // AFTERWARDS

    lvl=nlevels;

    while(lvl>0) {
        if(!ISIDENT(*rplPeekData(lvl))) break;
        lvl=rplUnitSkipItem(lvl);
    }

    if(lvl<=0) {
        // ERROR! SOMETHING HAPPENED AND THERE'S NO UNIT VALUE!
        // TRY TO FIX IT BY ADDING A 1
        rplPushData(one_bint);
        ++nlevels;
        lvl=1;
    }

    // UNROLL THIS VALUE TO THE BOTTOM OF THE UNIT
    WORDPTR value=rplPeekData(lvl);
    while(lvl!=nlevels) { rplOverwriteData(lvl,rplPeekData(lvl+1)); ++lvl; }
    rplOverwriteData(nlevels,value);

    return nlevels;

}

// INVERT A SINGLE UNIT IDENTIFIER BY NEGATING ITS EXPONENT
// RECEIVE THE LEVEL OF THE STACK WHERE THE IDENTIFIER IS
void rplUnitInvert(BINT level)
{
if(!ISIDENT(*rplPeekData(level))) return;

WORDPTR savestk=DSTop;

rplPushData(rplPeekData(level-1));
rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_NEG));
if(Exceptions) { DSTop=savestk; return; }
rplOverwriteData(level-1,rplPopData());

}

// DIVIDE TWO UNITS THAT WERE EXPLODED IN THE STACK (1..divlvl) and (divlvl+1 .. numlvl)
// DIVISION IS DONE IN 3 OPERATIONS:
// A) DIVIDE THE VALUES USING OVERLOADED OPERATORS
// B) INVERT THE UNIT PART OF THE DIVISOR
// C) MULTIPLY/SIMPLIFY THE UNIT PORTION
// RETURN THE NUMBER OF ELEMENTS AFTER THE SIMPLIFICATION
BINT rplUnitDivide(BINT numlvl,BINT divlvl)
{
    WORDPTR savestk=DSTop;

    rplPushData(rplPeekData(numlvl));   // GET THE VALUE
    rplPushData(rplPeekData(divlvl+1)); // GET THE VALUE OF THE DIVISOR

    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_DIV));
    if(Exceptions) { DSTop=savestk; return 0; }

    // NOW REPLACE THE NUMERATOR VALUE WITH THE RESULT
    rplOverwriteData(numlvl,rplPopData());
    // AND REMOVE THE VALUE OF THE DIVISOR FROM THE STACK
    rplUnitPopItem(divlvl);

    numlvl--;
    divlvl--;

    // NOW NEGATE ALL UNIT IDENTIFIER'S EXPONENTS
    while(divlvl>0) {
        rplUnitInvert(divlvl);
        divlvl=rplUnitSkipItem(divlvl);
    }

    // UNIT IS READY TO BE SIMPLIFIED

    return rplUnitSimplify(numlvl);


}



// RAISE A UNIT TO A REAL EXPONENT
// RETURN THE NUMBER OF ELEMENTS AFTER THE SIMPLIFICATION
BINT rplUnitPow(BINT lvlexp,BINT nlevels)
{
    BINT lvl=nlevels;

    while(lvl>0) {

        rplUnitPowItem(lvlexp,lvl);

        lvl=rplUnitSkipItem(lvl);
    }

    // UNIT IS READY TO BE SIMPLIFIED

    return rplUnitSimplify(nlevels);


}












#define NUM_SIPREFIXES 21

// THIS IS THE TEXT OF THE 20 SI PREFIXES
// INDEX 0 MEANS NO PREFIX
const char * const siprefix_text[]={
    "",
    "Y",
    "Z",
    "E",
    "P",
    "T",
    "G",
    "M",
    "k",
    "h",
    "da",
    "d",
    "c",
    "m",
    "μ",
    "n",
    "p",
    "f",
    "a",
    "z",
    "y"
};

// EXPONENT MODIFICATION DUE TO SI PREFIX

const BINT const siprefix_exp[]={
        0,
        24,
        21,
        18,
        15,
        12,
        9,
        6,
        3,
        2,
        1,
        -1,
        -2,
        -3,
        -6,
        -9,
        -12,
        -15,
        -18,
        -21,
        -24
        };


// RETURN THE INDEX TO A SI PREFIX



BINT rplUnitGetSIPrefix(WORDPTR ident)
{
BINT k,len,ilen;
BYTEPTR istart,iend;

// FIND START AND END OF THE IDENT TEXT
istart=(BYTEPTR) (ident+1);
iend=rplSkipOb(ident);
while( (iend>istart) && (*(iend-1)==0)) --iend;
ilen=utf8nlen(istart,iend);
if(ilen<2) return 0;

// HERE WE ARE READY FOR STRING COMPARISON

for(k=1;k<NUM_SIPREFIXES;++k)
{
// LEN IN UNICODE CHARATERS OF THE SI PREFIX
if(k==10) len=2; else len=1;
if(!utf8ncmp(istart,siprefix_text[k],len)) {
 // FOUND A VALID PREFIX
 if(ilen>len) return k;
}
}

return 0;
}

// COMPARES AN IDENT TO A BASE IDENT FOR EQUALITY
// THE BASE IDENTIFIER IS NOT SUPPOSED TO HAVE ANY SI PREFIXES
// RETURNS 0 = THEY ARE DIFFERENT
// -1 = THEY ARE IDENTICAL
// n = THEY DIFFER ONLY IN THE SI PREFIX, OTHERWISE IDENTICAL UNIT (n=SI PREFIX INDEX)

BINT rplUnitCompare(WORDPTR ident,WORDPTR baseident)
{
    BINT siidx;

    if(rplCompareIDENT(ident,baseident)) return -1;

    if(LIBNUM(*baseident)!=DOIDENTSIPREFIX) return 0;   // BASE UNIT DOESN'T SUPPORT SI PREFIXES

    siidx=rplUnitGetSIPrefix(ident);

    if(siidx==0) return 0;      // THERE WAS NO PREFIX, SO THEY CANNOT BE THE SAME UNIT

    // THERE'S AN SI PREFIX, NEED TO COMPARE TEXT BY SKIPPING IT
    BYTEPTR st1,end1,st2,end2;
    BINT len1,len2;
    st1=(BYTEPTR) (ident+1);
    st2=(BYTEPTR) (baseident+1);
    end1=rplSkipOb(ident);
    end2=rplSkipOb(baseident);

    // FIND THE END IN BOTH IDENTS
    while( (end1>st1) && (*(end1-1)==0)) --end1;
    while( (end2>st2) && (*(end2-1)==0)) --end2;

    if(siidx==10) st1=utf8nskip(st1,end1,2);
    else st1=utf8nskip(st1,end1,1);

    // NOW DO THE COMPARISON BYTE BY BYTE
    if( (end1-st1)!=(end2-st2)) return 0;

    while(st1!=end1) {
        if(*st1!=*st2) return 0;
        ++st1;
        ++st2;
    }

    return siidx;

}








// GET THE DEFINITION OF A UNIT FROM ITS IDENTIFIER
// FIRST IT STRIPS ANY SI PREFIXES AND SEARCHES FOR
// BOTH THE ORIGINAL UNIT AND THE STRIPPED ONE
// RETURNS A POINTER WITHIN THE DIRECTORY ENTRY
// THE FIRST POINTER POINTS TO THE IDENTIFIER
// THE SECOND TO ITS VALUE
// THE SEARCH IS DONE FIRST IN THE USER'S UNIT DIRECTORY
// THEN IN THE SYSTEM BASE UNITS DEFINED IN ROM
// IF THE siindex POINTER IS NOT NULL, IT STORES THE
// INDEX TO THE SI PREFIX THAT WAS FOUND IN THE GIVEN NAME

WORDPTR *rplUnitFind(WORDPTR ident,BINT *siindex)
{
    const BYTEPTR const unitdir_name[]={(BYTEPTR)"UNITS"};

    WORDPTR unitdir_obj=rplGetSettingsbyName(unitdir_name,unitdir_name+5);
    WORDPTR baseid,baseunit;
    BINT result;
    WORDPTR *entry;

    if(unitdir_obj) {
        // FOUND UNITS DIRECTORY IN SETTINGS, SCAN IT TO FIND OUT IDENT
        entry=rplFindFirstInDir(unitdir_obj);
        while(entry) {
            baseid=entry[0];
            baseunit=entry[1];

            result=rplUnitCompare(ident,baseid);
            if(result) {
                // WE FOUND A MATCH!
                if(result<0) result=0;
                if(siindex) *siindex=result;
                return entry;
            }

            entry=rplFindNext(entry);
        }

        // NOT FOUND IN THE USERS DIR, TRY THE SYSTEM LIST

    }

    // SEARCH THROUGH THE SYSTEM UNITS

    entry=system_unit_dir;
    while(entry[0]) {
        baseid=entry[0];
        baseunit=entry[1];

        result=rplUnitCompare(ident,baseid);
        if(result) {
            // WE FOUND A MATCH!
            if(result<0) result=0;
            if(siindex) *siindex=result;
            return entry;
        }

        entry+=2;
    }

    // UNIT IS NOT DEFINED
    return 0;

}


// GET THE DEFINITION OF A UNIT FROM ITS IDENTIFIER
// FIRST IT STRIPS ANY SI PREFIXES AND SEARCHES FOR
// BOTH THE ORIGINAL UNIT AND THE STRIPPED ONE
// RETURNS A POINTER WITHIN THE DIRECTORY ENTRY
// THE FIRST POINTER POINTS TO THE IDENTIFIER
// THE SECOND TO ITS VALUE
// THE SEARCH IS DONE FIRST IN THE USER'S UNIT DIRECTORY
// THEN IN THE SYSTEM BASE UNITS DEFINED IN ROM
// IF THE siindex POINTER IS NOT NULL, IT STORES THE
// INDEX TO THE SI PREFIX THAT WAS FOUND IN THE GIVEN NAME

WORDPTR *rplUnitFindCustom(WORDPTR ident,BINT *siindex)
{
    const BYTEPTR const unitdir_name[]={(BYTEPTR)"UNITS"};

    WORDPTR unitdir_obj=rplGetSettingsbyName(unitdir_name,unitdir_name+5);
    WORDPTR baseid,baseunit;
    BINT result;
    WORDPTR *entry;

    if(unitdir_obj) {
        // FOUND UNITS DIRECTORY IN SETTINGS, SCAN IT TO FIND OUT IDENT
        entry=rplFindFirstInDir(unitdir_obj);
        while(entry) {
            baseid=entry[0];
            baseunit=entry[1];

            result=rplUnitCompare(ident,baseid);
            if(result) {
                // WE FOUND A MATCH!
                if(result<0) result=0;
                if(siindex) *siindex=result;
                return entry;
            }

            entry=rplFindNext(entry);
        }

        // NOT FOUND IN THE USERS DIR, TRY THE SYSTEM LIST

    }

    // UNIT IS NOT DEFINED
    return 0;

}





// TAKE ONE UNIT IDENTIFIER GIVEN AT LEVEL, REMOVE IT FROM THE STACK WITH ITS EXPONENTS
// AND APPEND THE UNIT DEFINITION TO THE STACK, WITH VALUE AND COEFFICIENTS
// MODIFIED BY THE ORIGINAL EXPONENT
// IF UNIT IS ALREADY A BASE UNIT, LEAVE AS-IS
// RETURNS NUMBER OF LEVELS ADDED TO THE STACK

BINT rplUnitExpand(BINT level)
{
    if(ISIDENT(*rplPeekData(level))) {

            BINT siidx;
            WORDPTR *entry=rplUnitFind(rplPeekData(level),&siidx);

            if(!entry) {
                // NOT FOUND, KEEP AS-IS
                return 0;
            }

            WORDPTR *stktop=DSTop;

            // UNIT WAS FOUND



            BINT nlevels=rplUnitExplode(entry[1]);
            if(Exceptions) { DSTop=stktop; return 0; }

            if(nlevels==1) {
                if(siidx) {
                // UNIT WAS A BASE UNIT WITH AN SI PREFIX
                // ADD THE BASE UNIT WITHOUT THE PREFIX
                rplPushData(entry[0]);
                rplPushData(one_bint);
                rplPushData(one_bint);
                nlevels+=3;
                }
                else {
                    if(*rplPeekData(1)==MAKESINT(1))
                    {
                    // UNIT WAS ALREADY A BASE UNIT, DO NOTHING
                    DSTop=stktop;
                    return 0;
                    }
                }
            }


            if(siidx) {
                // THERE WAS AN SI PREFIX, NEED TO INCLUDE IT IN THE MULTIPLIER
                REAL value;
                rplReadNumberAsReal(rplPeekData(nlevels),&value);
                value.exp+=siprefix_exp[siidx];
                WORDPTR newval=rplNewReal(&value);
                if(!newval) { DSTop=stktop; return 0; }
                rplOverwriteData(nlevels,newval);
            }

             // NOW APPLY THE EXPONENT!

            BINT lvl2=nlevels;

            while(lvl2>0) {
                rplUnitPowItem(level+nlevels,lvl2);
                lvl2=rplUnitSkipItem(lvl2);
            }

            // NOW REMOVE IT FROM THE STACK

            nlevels-=rplUnitPopItem(level+nlevels);

            return nlevels;

            }
    return 0;
}

// RECURSIVELY EXPAND ALL UNITS USING THEIR DEFINITIONS UNTIL A BASE IS REACHED
// RETURN THE NEW TOTAL NUMBER OF ELEMENTS
BINT rplUnitToBase(BINT nlevels)
{
    BINT lvl=nlevels,lvl2,morelevels;

   while(lvl>0) {
        morelevels=rplUnitExpand(lvl);

        if(!morelevels) lvl=rplUnitSkipItem(lvl);
        else {
            lvl+=morelevels;
            nlevels+=morelevels;
        }
    }

    // HERE ALL UNITS WERE EXPANDED TO THEIR BASES
    return nlevels;

}

// RETURN TRUE/FALSE IF THE UNIT IN THE FIRST nlevels ARE CONSISTENT
// WITH THE UNIT IN reflevel TO (nlevels+1)
// BOTH UNITS MUST BE EXPLODED AND REDUCED TO BASE BEFOREHAND
BINT rplUnitIsConsistent(BINT nlevels,BINT reflevel)
{
if(reflevel<nlevels) {
    BINT tmp=reflevel;
    reflevel=nlevels;
    nlevels=tmp;
}

// SPECIAL CASE: THE NUMBER ZERO IS ALWAYS CONSISTENT!
if(nlevels==1) {
    if(*rplPeekData(1)==MAKESINT(0)) return 1;
}
if(reflevel-nlevels==1) {
    if(*rplPeekData(nlevels+1)==MAKESINT(0)) return 1;
}


if(nlevels!=reflevel-nlevels) return 0; // UNITS MUST HAVE THE SAME NUMBER OF IDENTS TO BE CONSISTENT

BINT lvl=nlevels,lvl2=reflevel;

while(lvl>0) {
    if(!ISIDENT(*rplPeekData(lvl))) { lvl=rplUnitSkipItem(lvl); continue; }
    WORDPTR mainident=rplPeekData(lvl);
    lvl2=reflevel;
    while(lvl2>nlevels) {
        if(rplCompareIDENT(mainident,rplPeekData(lvl2))) {
            // FOUND, COMPARE THE EXPONENTS

            if(ISREAL(*rplPeekData(lvl-1))) {
                // DO A REAL COMPARISON
                REAL num1,num2;

                rplReadNumberAsReal(rplPeekData(lvl-1),&num1);
                rplReadNumberAsReal(rplPeekData(lvl2-1),&num2);
                if(!eqReal(&num1,&num2)) return 0;  // INCONSISTENT UNITS
            } else {
                if(!rplCompareObjects(rplPeekData(lvl-1),rplPeekData(lvl2-1))) return 0; // INCONSISTENT UNITS
            }

            if(ISREAL(*rplPeekData(lvl-2))) {
                // DO A REAL COMPARISON
                REAL num1,num2;

                rplReadNumberAsReal(rplPeekData(lvl-2),&num1);
                rplReadNumberAsReal(rplPeekData(lvl2-2),&num2);
                if(!eqReal(&num1,&num2)) return 0;  // INCONSISTENT UNITS
            } else {
                if(!rplCompareObjects(rplPeekData(lvl-2),rplPeekData(lvl2-2))) return 0; // INCONSISTENT UNITS
            }

            // HERE WE HAVE A MATCH

            break;


        }

        lvl2=rplUnitSkipItem(lvl2);


    }

    if(lvl2==nlevels) return 0; // NO MATCH = INCONSISTENT UNITS

    lvl=rplUnitSkipItem(lvl);
}

return 1;
}



// SPECIAL UNITS (TEMPERATURE) WHICH NEED SEPARATE HANDLING DUE TO
// SCALE SHIFTING

BINT rplUnitIsSpecial(WORDPTR unitobj)
{
    if(!ISUNIT(*unitobj)) return   0;

    // THERE HAS TO BE ONE AND ONLY ONE IDENT
    WORDPTR id=unitobj+1,end=rplSkipOb(unitobj);
    BINT count=0;

    while(id!=end) { ++count; id=rplSkipOb(id); }

    if(count!=4) return 0;  // 4 = 1 VALUE + 1 IDENT + 2 EXPONENT NUMBERS

    id=rplSkipOb(unitobj+1);    // POINT TO THE IDENTIFIER

    WORDPTR *ptr=system_unit_special;

    while(*ptr) {
       if(rplCompareIDENT(id,*ptr)) break;
       ptr+=3;
    }

    if(!*ptr) return 0;

    return 1+(ptr-system_unit_special);

}


// REPLACE THE VALUE AND IDENT OF AN EXPLODED SPECIAL UNIT
// WITH THEIR SHIFTED-SCALE ABSOLUTE COUNTERPART
// WARNING: NO CHECKS DONE HERE, MAKE SURE THE UNIT IS SPECIAL BEFORE CALLING THIS!

void rplUnitReplaceSpecial(BINT nlevels)
{
    BINT lvl=nlevels;
    BINT value=0;
    BINT ident=0;

    // FIND THE VALUE AND THE IDENTIFIER IN CASE THEY ARE OUT OF ORDER
    while(lvl>0) {
     if(!ISIDENT(*rplPeekData(lvl))) value=lvl;
     else ident=lvl;

     if(value && ident) break;

     lvl=rplUnitSkipItem(lvl);

    }

     WORDPTR *ptr=system_unit_special;

     while(ptr[2]) {
         if(rplCompareIDENT(rplPeekData(ident),*ptr)) break;
         ptr+=3;
     }

     if(!ptr[2]) return;          // NOTHING SPECIAL IN THIS UNIT

     WORDPTR *savestk=DSTop;
     // DO THE REPLACEMENT
     rplPushData(rplPeekData(value));
     rplPushData(ptr[2]+1);

     rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_ADD));    // DO THE SCALE SHIFT

     if(Exceptions) { DSTop=savestk; return; }

     rplOverwriteData(value,rplPopData());          // REPLACE THE VALUE

     // FINALLY, REPLACE THE IDENT OF THE UNIT (KEEP THE EXPONENTS)

     rplOverwriteData(ident,rplSkipOb(ptr[2]+1));


}

// REPLACE THE VALUE AND IDENT OF AN EXPLODED ABSOLUTE SPECIAL UNIT
// WITH THEIR SHIFTED-SCALE RELATIVE COUNTERPART
// WARNING: NO CHECKS DONE HERE, MAKE SURE THE UNIT IS SPECIAL BEFORE CALLING THIS!

void rplUnitReverseReplaceSpecial(BINT nlevels)
{
    BINT lvl=nlevels;
    BINT value=0;
    BINT ident=0;

    // FIND THE VALUE AND THE IDENTIFIER IN CASE THEY ARE OUT OF ORDER
    while(lvl>0) {
     if(!ISIDENT(*rplPeekData(lvl))) value=lvl;
     else ident=lvl;

     if(value && ident) break;

     lvl=rplUnitSkipItem(lvl);

    }

     WORDPTR *ptr=system_unit_special;

     while(ptr[2]) {
         if(rplCompareIDENT(rplPeekData(ident),rplSkipOb(ptr[2]+1))) break;
         ptr+=3;
     }

     if(!ptr[2]) return;          // NOTHING SPECIAL IN THIS UNIT

     WORDPTR *savestk=DSTop;
     // DO THE REPLACEMENT
     rplPushData(rplPeekData(value));
     rplPushData(ptr[2]+1);

     rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_SUB));    // DO THE SCALE SHIFT

     if(Exceptions) { DSTop=savestk; return; }

     rplOverwriteData(value,rplPopData());          // REPLACE THE VALUE

     // FINALLY, REPLACE THE IDENT OF THE UNIT (KEEP THE EXPONENTS)

     rplOverwriteData(ident,ptr[0]);


}


// REPLACE THE VALUE AND IDENT OF AN EXPLODED SPECIAL UNIT
// WITH THEIR SHIFTED-SCALE ABSOLUTE COUNTERPART
// WARNING: NO CHECKS DONE HERE, MAKE SURE THE UNIT IS SPECIAL BEFORE CALLING THIS!

void rplUnitSpecialToDelta(BINT nlevels)
{
    BINT lvl=nlevels;
    BINT value=0;
    BINT ident=0;

    // FIND THE VALUE AND THE IDENTIFIER IN CASE THEY ARE OUT OF ORDER
    while(lvl>0) {
     if(!ISIDENT(*rplPeekData(lvl))) value=lvl;
     else ident=lvl;

     if(value && ident) break;

     lvl=rplUnitSkipItem(lvl);

    }

     WORDPTR *ptr=system_unit_special;

     while(*ptr) {
         if(rplCompareIDENT(rplPeekData(ident),*ptr)) break;
         ptr+=3;
     }

     if(!*ptr) return;          // NOTHING SPECIAL IN THIS UNIT

     // REPLACE THE IDENT OF THE UNIT (KEEP THE EXPONENTS)

     rplOverwriteData(ident,ptr[1]);


}

