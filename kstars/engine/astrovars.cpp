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
#include <iostream>

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

namespace KSEngine {
namespace AstroVars {

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