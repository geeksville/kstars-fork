/*  Ekos guide tool
    SPDX-FileCopyrightText: 2012 Andrew Stepanenko

    Modified by Jasem Mutlaq <mutlaqja@ikarustech.com> for KStars.

    SPDX-License-Identifier: GPL-2.0-or-later
*/

//---------------------------------------------------------------------------
#ifndef matrH
#define matrH

#include "vect.h"

namespace Ekos
{
class Matrix
{
  public:
    double x[4][4];
    Matrix();
    explicit Matrix(double);
    Matrix &operator+=(const Matrix &);
    Matrix &operator-=(const Matrix &);
    Matrix &operator*=(const Matrix &);
    Matrix &operator*=(double);
    Matrix &operator/=(double);
    void Invert();
    void Transpose();
    friend Matrix operator+(const Matrix &, const Matrix &);
    friend Matrix operator-(const Matrix &, const Matrix &);
    friend Matrix operator*(const Matrix &, double);
    friend Matrix operator*(const Matrix &, const Matrix &);
    friend Vector operator*(const Vector &, const Matrix &);
};

Matrix Translate(const Vector &);
Matrix Scale(const Vector &);
Matrix RotateX(double);
Matrix RotateY(double);
Matrix RotateZ(double);
Matrix Rotate(const Vector &v, double angle);
Matrix Transform(const Vector &v1, const Vector &v2, const Vector &v3);
Matrix MirrorX();
Matrix MirrorY();
Matrix MirrorZ();
}
//---------------------------------------------------------------------------
#endif
