/***************************************************************************
     engine/astrovars.cpp - functions for use in various calculations
                             -------------------
    begin                : 2013-06-28
    copyright            : (C) 2013 by Henry de Valence
    email                : hdevalence@hdevalence.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "astrovars.h"

using namespace Eigen;
using KSEngine::Radian;
using KSEngine::JulianDate;

/*
 * These variables are taken from a table in chapter 20 of Meeus.
 */
static const int NUTTERMS = 63;
EIGEN_ALIGN16
static const double _arguments_data[NUTTERMS][5] = {
    { 0, 0, 0, 0, 1},
    {-2, 0, 0, 2, 2},
    { 0, 0, 0, 2, 2},
    { 0, 0, 0, 0, 2},
    { 0, 1, 0, 0, 0},
    { 0, 0, 1, 0, 0},
    {-2, 1, 0, 2, 2},
    { 0, 0, 0, 2, 1},
    { 0, 0, 1, 2, 2},
    {-2,-1, 0, 2, 2},
    {-2, 0, 1, 0, 0},
    {-2, 0, 0, 2, 1},
    { 0, 0,-1, 2, 2},
    { 2, 0, 0, 0, 0},
    { 0, 0, 1, 0, 1},
    { 2, 0,-1, 2, 2},
    { 0, 0,-1, 0, 1},
    { 0, 0, 1, 2, 1},
    {-2, 0, 2, 0, 0},
    { 0, 0,-2, 2, 1},
    { 2, 0, 0, 2, 2},
    { 0, 0, 2, 2, 2},
    { 0, 0, 2, 0, 0},
    {-2, 0, 1, 2, 2},
    { 0, 0, 0, 2, 0},
    {-2, 0, 0, 2, 0},
    { 0, 0,-1, 2, 1},
    { 0, 2, 0, 0, 0},
    { 2, 0,-1, 0, 1},
    {-2, 2, 0, 2, 2},
    { 0, 1, 0, 0, 1},
    {-2, 0, 1, 0, 1},
    { 0,-1, 0, 0, 1},
    { 0, 0, 2,-2, 0},
    { 2, 0,-1, 2, 1},
    { 2, 0, 1, 2, 2},
    { 0, 1, 0, 2, 2},
    {-2, 1, 1, 0, 0},
    { 0,-1, 0, 2, 2},
    { 2, 0, 0, 2, 1},
    { 2, 0, 1, 0, 0},
    {-2, 0, 2, 2, 2},
    {-2, 0, 1, 2, 1},
    { 2, 0,-2, 0, 1},
    { 2, 0, 0, 0, 1},
    { 0,-1, 1, 0, 0},
    {-2,-1, 0, 2, 1},
    {-2, 0, 0, 0, 1},
    { 0, 0, 2, 2, 1},
    {-2, 0, 2, 0, 1},
    {-2, 1, 0, 2, 1},
    { 0, 0, 1,-2, 0},
    {-1, 0, 1, 0, 0},
    {-2, 1, 0, 0, 0},
    { 1, 0, 0, 0, 0},
    { 0, 0, 1, 2, 0},
    { 0, 0,-2, 2, 2},
    {-1,-1, 1, 0, 0},
    { 0, 1, 1, 0, 0},
    { 0,-1, 1, 2, 2},
    { 2,-1,-1, 2, 2},
    { 0, 0, 3, 2, 2},
    { 2,-1, 0, 2, 2}
};

EIGEN_ALIGN16
static const double _amp_data[NUTTERMS][4] = {
    {-171996,-1742, 92025, 89},
    { -13187,  -16,  5736,-31},
    {  -2274,   -2,   977, -5},
    {   2062,    2,  -895,  5},
    {   1426,  -34,    54, -1},
    {    712,    1,    -7,  0},
    {   -517,   12,   224, -6},
    {   -386,   -4,   200,  0},
    {   -301,    0,   129, -1},
    {    217,   -5,   -95,  3},
    {   -158,    0,     0,  0},
    {    129,    1,   -70,  0},
    {    123,    0,   -53,  0},
    {     63,    0,     0,  0},
    {     63,    1,   -33,  0},
    {    -59,    0,    26,  0},
    {    -58,   -1,    32,  0},
    {    -51,    0,    27,  0},
    {     48,    0,     0,  0},
    {     46,    0,   -24,  0},
    {    -38,    0,    16,  0},
    {    -31,    0,    13,  0},
    {     29,    0,     0,  0},
    {     29,    0,   -12,  0},
    {     26,    0,     0,  0},
    {    -22,    0,     0,  0},
    {     21,    0,   -10,  0},
    {     17,   -1,     0,  0},
    {     16,    0,    -8,  0},
    {    -16,    1,     7,  0},
    {    -15,    0,     9,  0},
    {    -13,    0,     7,  0},
    {    -12,    0,     6,  0},
    {     11,    0,     0,  0},
    {    -10,    0,     5,  0},
    {     -8,    0,     3,  0},
    {      7,    0,    -3,  0},
    {     -7,    0,     0,  0},
    {     -7,    0,     3,  0},
    {     -7,    0,     3,  0},
    {      6,    0,     0,  0},
    {      6,    0,    -3,  0},
    {      6,    0,    -3,  0},
    {     -6,    0,     3,  0},
    {     -6,    0,     3,  0},
    {      5,    0,     0,  0},
    {     -5,    0,     3,  0},
    {     -5,    0,     3,  0},
    {     -5,    0,     3,  0},
    {      4,    0,     0,  0},
    {      4,    0,     0,  0},
    {      4,    0,     0,  0},
    {     -4,    0,     0,  0},
    {     -4,    0,     0,  0},
    {     -4,    0,     0,  0},
    {      3,    0,     0,  0},
    {     -3,    0,     0,  0},
    {     -3,    0,     0,  0},
    {     -3,    0,     0,  0},
    {     -3,    0,     0,  0},
    {     -3,    0,     0,  0},
    {     -3,    0,     0,  0},
    {     -3,    0,     0,  0}
};

static const Eigen::Map<const Eigen::Matrix<double,NUTTERMS,5,Eigen::RowMajor>,
                        Eigen::Aligned>
                arguments((double*)_arguments_data);
static const Eigen::Map<const Eigen::Matrix<double,NUTTERMS,4,Eigen::RowMajor>,
                        Eigen::Aligned>
                amp((double*)_amp_data);

static Array<double, 36, 1> ronVondrakAngles(const JulianDate jd)
{
    double T = KSEngine::AstroVars::centuriesSinceJ2000(jd);
    // Mean longitudes for the planets, in radians
    Radian LVenus   = 3.1761467+1021.3285546*T; // Venus
    Radian LMars    = 1.7534703+ 628.3075849*T; // Mars
    Radian LEarth   = 6.2034809+ 334.0612431*T; // Earth
    Radian LJupiter = 0.5995465+  52.9690965*T; // Jupiter
    Radian LSaturn  = 0.8740168+  21.3299095*T; // Saturn
    Radian LNeptune = 5.3118863+   3.8133036*T; // Neptune
    Radian LUranus  = 5.4812939+   7.4781599*T; // Uranus

    Radian LMRad = 3.8103444+8399.6847337*T; // Moon
    Radian DRad  = 5.1984667+7771.3771486*T;
    Radian MMRad = 2.3555559+8328.6914289*T; // Moon
    Radian FRad  = 1.6279052+8433.4661601*T;

    Array<double, 36, 1> angles;
    angles <<   LMars,
                2*LMars,
                LJupiter,
                LMRad,
                3*LMars,
                LSaturn,
                FRad,
                LMRad+MMRad,
                2*LJupiter,
                2*LMars-LJupiter,
                3*LMars-8*LEarth+3*LJupiter,
                5*LMars-8*LEarth+3*LJupiter,
                2*LVenus-LMars,
                LVenus,
                LNeptune,
                LMars-2*LJupiter,
                LUranus,
                LMars+LJupiter,
                2*LVenus-2*LMars,
                LMars-LJupiter,
                4*LMars,
                3*LMars-2*LJupiter,
                LVenus-2*LMars,
                2*LVenus-3*LMars,
                2*LSaturn,
                2*LVenus-4*LMars,
                3*LMars-2*LEarth,
                LMRad+2*DRad-MMRad,
                8*LVenus-12*LMars,
                8*LVenus-14*LMars,
                2*LEarth,
                3*LVenus-4*LMars,
                2*LMars-2*LJupiter,
                3*LVenus-3*LMars,
                2*LMars-2*LEarth,
                LMRad-2*DRad;
    return angles;
}

static Array<double, 36, 6> ronVondrakMatrix(const JulianDate jd)
{
    double T = KSEngine::AstroVars::centuriesSinceJ2000(jd);
    Array<double, 36, 6> arr;
    arr <<  -1719914-2*T,        -25,   25-13*T,1578089+156*T,   10+32*T,684185-358*T,
              6434+141*T,28007-107*T,25697-95*T,  -5904-130*T,11141-48*T,  -2559-55*T,
                     715,           0,        6,         -657,       -15,        -282,
                     715,           0,        0,         -656,         0,        -285,
                 486-5*T,    -236-4*T, -216-4*T,     -446+5*T,       -94,        -193,
                     159,           0,        2,         -147,        -6,         -61,
                       0,           0,        0,           26,         0,         -59,
                      39,           0,        0,          -36,         0,         -16,
                      33,         -10,       -9,          -30,        -5,         -13,
                      31,           1,        1,          -28,         0,         -12,
                       8,         -28,       25,            8,        11,           3,
                       8,         -28,      -25,           -8,       -11,          -3,
                      21,           0,        0,          -19,         0,          -8,
                     -19,           0,        0,           17,         0,           8,
                      17,           0,        0,          -16,         0,          -7,
                      16,           0,        0,           15,         1,           7,
                      16,           0,        1,          -15,        -3,          -6,
                      11,          -1,       -1,          -10,        -1,          -5,
                       0,         -11,      -10,            0,        -4,           0,
                     -11,          -2,       -2,            9,        -1,           4,
                      -7,          -8,       -8,            6,        -3,           3,
                     -10,           0,        0,            9,         0,           4,
                      -9,           0,        0,           -9,         0,          -4,
                      -9,           0,        0,           -8,         0,          -4,
                       0,          -9,       -8,            0,        -3,           0,
                       0,          -9,        8,            0,         3,           0,
                       8,           0,        0,           -8,         0,          -3,
                       8,           0,        0,           -7,         0,          -3,
                      -4,          -7,       -6,            4,        -3,           2,
                      -4,          -7,        6,           -4,         3,          -2,
                      -6,          -5,       -4,            5,        -2,           2,
                      -1,          -1,       -2,           -7,         1,          -4,
                       4,          -6,       -5,           -4,        -2,          -2,
                       0,          -7,       -6,            0,        -3,           0,
                       5,          -5,       -4,           -5,        -2,          -2,
                       5,           0,        0,           -5,         0,          -2;
    return arr;
}

namespace KSEngine {
namespace AstroVars {

double sunEquationOfCenter(const JulianDate jd) {
    double T = centuriesSinceJ2000(jd);
    Radian M = sunMeanAnomaly(jd);
    double C = ( 1.914600 - 0.004817*T - 0.000014*T*T ) * sin( M )
                            + ( 0.019993 - 0.000101*T ) * sin( 2.0*M )
                                             + 0.000290 * sin( 3.0*M );
    return C*DEG2RAD;
}

double earthEccentricity(const JulianDate jd)
{
    double T = centuriesSinceJ2000(jd);
    return 0.016708617 - 0.000042037*T - 0.0000001236*T*T;
}

Radian sunMeanLongitude(const JulianDate jd)
{
    double T = centuriesSinceJ2000(jd);
    return DEG2RAD*(280.46645 + 36000.76983*T + 0.0003032*T*T);
}

Radian sunTrueLongitude(const JulianDate jd)
{
    return sunMeanLongitude(jd) + sunEquationOfCenter(jd);
}

double centuriesSinceJ2000(const JulianDate jd)
{
    return (jd - EpochJ2000) / 36525.;
}

Radian moonArgumentOfLatitude(const JulianDate jd)
{
    double T = centuriesSinceJ2000(jd);
    return DEG2RAD*(93.27191 + 483202.017538*T - 0.0036825*T*T + T*T*T/327270.);
}

Radian moonMeanAnomaly(const JulianDate jd)
{
    double T = centuriesSinceJ2000(jd);
    return DEG2RAD*(134.96298 + 477198.867398*T + 0.0086972*T*T + T*T*T/56250.0);
}

Radian sunMeanAnomaly(const JulianDate jd)
{
    double T = centuriesSinceJ2000(jd);
    return DEG2RAD*(357.52910 + 35999.05030*T - 0.0001559*T*T - 0.00000048*T*T*T);
}

Radian meanElongationOfMoon(const JulianDate jd)
{
    double T = centuriesSinceJ2000(jd);
    return DEG2RAD*(297.85036 + 445267.111480*T - 0.0019142*T*T + T*T*T/189474.);
}

Radian lonMoonAscendingNode(const JulianDate jd)
{
    double T = centuriesSinceJ2000(jd);
    return DEG2RAD*(125.04452 - 1934.136261*T + 0.0020708*T*T + T*T*T/450000.0);
}

Vector3d earthVelocity(const JulianDate jd)
{
    typedef Array<double, 36, 1> rvCol;
    rvCol angles = ronVondrakAngles(jd);
    Array<double, 36, 6> coeffs = ronVondrakMatrix(jd);
    rvCol angleSin = angles.sin();
    rvCol angleCos = angles.cos();
    double x = (coeffs.col(0)*angleSin + coeffs.col(1)*angleCos).sum();
    double y = (coeffs.col(2)*angleSin + coeffs.col(3)*angleCos).sum();
    double z = (coeffs.col(4)*angleSin + coeffs.col(5)*angleCos).sum();
    // The above are in 10e-8 AU/day, so we need to convert to km/s
    const double UA2km  =  1.49597870/86400.;
    return UA2km*Vector3d(x,y,z);
}

void nutationVars(const JulianDate jd, double *deltaEcLong, double *deltaObliquity)
{
    typedef Array<double, NUTTERMS, 1> nutArr;
    Matrix<double, 5, 1> params;
    params << meanElongationOfMoon(jd),
              sunMeanAnomaly(jd),
              moonMeanAnomaly(jd),
              moonArgumentOfLatitude(jd),
              lonMoonAscendingNode(jd);
    // This vector has all of the dot products of the
    // params vector with the coefficients in the argument matrix.
    nutArr sums = (arguments*params).array();
    double T = centuriesSinceJ2000(jd);
    nutArr ecLongColumn = 1e-4*(amp.col(0) + (T/10.)*amp.col(1)).array();
    nutArr obliqColumn  = 1e-4*(amp.col(2) + (T/10.)*amp.col(3)).array();
    // Take the dot product, and convert from arcsec to degrees.
    *deltaEcLong = (ecLongColumn * sums.sin()).sum()/3600.;
    *deltaObliquity = (obliqColumn * sums.cos()).sum()/3600.;
}


}
}
