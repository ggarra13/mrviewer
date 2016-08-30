/*
  File:       Stubs.cpp
 
  Contains:   Useful classes and functions for profile creation utilities.
 
  Version:    V1
 
  Copyright:  © see below
*/

/*
 * The ICC Software License, Version 0.2
 *
 *
 * Copyright (c) 2003-2010 The International Color Consortium. All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. In the absence of prior written permission, the names "ICC" and "The
 *    International Color Consortium" must not be used to imply that the
 *    ICC organization endorses or promotes products derived from this
 *    software.
 *
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNATIONAL COLOR CONSORTIUM OR
 * ITS CONTRIBUTING MEMBERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the The International Color Consortium. 
 *
 *
 * Membership in the ICC is encouraged when this software is used for
 * commercial purposes. 
 *
 *  
 * For more information on The International Color Consortium, please
 * see <http://www.color.org/>.
 *  
 * 
 */

#include "Stubs.h"

#include <iostream>
using namespace std;

ostream&
operator<<(ostream& stream, const CIEXYZ& val)
{
  return stream << val.X() << " " << val.Y() << " " << val.Z();
}

ostream&
operator<<(ostream& stream, const DPX& val)
{
  return stream << val.R() << " " << val.G() << " " << val.B();
}

Matrix3d::Matrix3d()
  : m00_(1), m01_(0), m02_(0),
    m10_(0), m11_(1), m12_(0),
    m20_(0), m21_(0), m22_(1)
{}

Matrix3d::Matrix3d
( double m00, double m01, double m02,
  double m10, double m11, double m12,
  double m20, double m21, double m22)
  : m00_(m00), m01_(m01), m02_(m02),
    m10_(m10), m11_(m11), m12_(m12),
    m20_(m20), m21_(m21), m22_(m22)
{}

//Matrix3d::Matrix3d(const Matrix3d& m)
//  : m00_(m.m00_), m01_(m.m01_), m02_(m.m02_),
//    m10_(m.m10_), m11_(m.m11_), m12_(m.m12_),
//    m20_(m.m20_), m21_(m.m21_), m22_(m.m22_)
//{}
//
//Matrix3d&
//Matrix3d::operator=(const Matrix3d& rhs)
//{
//  if (this != &rhs)
//  {
//    m00_ = rhs.m00_;
//    m01_ = rhs.m01_;
//    m02_ = rhs.m02_;
//    m10_ = rhs.m10_;
//    m11_ = rhs.m11_;
//    m12_ = rhs.m12_;
//    m20_ = rhs.m20_;
//    m21_ = rhs.m21_;
//    m22_ = rhs.m22_;
//  }
//  return *this;
//}

double
Matrix3d::Determinant() const
{
  // http://en.wikipedia.org/wiki/Determinant
  return m00_ * (m11_ * m22_ - m12_ * m21_)
    - m01_ * (m10_ * m22_ - m12_ * m20_)
    + m02_ * (m10_ * m21_ - m11_ * m20_);
}

Matrix3d
Matrix3d::operator*(double s) const
{
  return Matrix3d(m00_ * s, m01_ * s, m02_ * s,
                  m10_ * s, m11_ * s, m12_ * s,
                  m20_ * s, m21_ * s, m22_ * s);
}

Matrix3d
Matrix3d::Inverse() const
{
  // http://en.wikipedia.org/wiki/Invertible_matrix#Inversion_of_3_x_3_matrices
  Matrix3d tmp( m11_ * m22_ - m12_ * m21_,
                m02_ * m21_ - m01_ * m22_,
                m01_ * m12_ - m02_ * m11_,
                m12_ * m20_ - m10_ * m22_,
                m00_ * m22_ - m02_ * m20_,
                m02_ * m10_ - m00_ * m12_,
                m10_ * m21_ - m11_ * m20_,
                m01_ * m20_ - m00_ * m21_,
                m00_ * m11_ - m01_ * m10_);
  double d = Determinant();
  Matrix3d result(tmp * (1 / d));
  return result;
}

Vector3d
Matrix3d::operator*(const Vector3d& v) const
{
  return Vector3d
    (m00_ * v.X() + m01_ * v.Y() + m02_ * v.Z(),
     m10_ * v.X() + m11_ * v.Y() + m12_ * v.Z(),
     m20_ * v.X() + m21_ * v.Y() + m22_ * v.Z());
}
