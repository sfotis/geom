// Created on: 2001-02-16
// Created by: Peter KURNEV
// Copyright (c) 2001-2012 OPEN CASCADE SAS
//
// The content of this file is subject to the Open CASCADE Technology Public
// License Version 6.5 (the "License"). You may not use the content of this file
// except in compliance with the License. Please obtain a copy of the License
// at http://www.opencascade.org and read it completely before using this file.
//
// The Initial Developer of the Original Code is Open CASCADE S.A.S., having its
// main offices at: 1, place des Freres Montgolfier, 78280 Guyancourt, France.
//
// The Original Code and all software distributed under the License is
// distributed on an "AS IS" basis, without warranty of any kind, and the
// Initial Developer hereby disclaims all such warranties, including without
// limitation, any warranties of merchantability, fitness for a particular
// purpose or non-infringement. Please see the License for the specific terms
// and conditions governing the rights and limitations under the License.



#include <XBOPTools_ComparePave.ixx>

//=======================================================================
// function: XBOPTools_ComparePave::XBOPTools_ComparePave
// purpose: 
//=======================================================================
XBOPTools_ComparePave::XBOPTools_ComparePave()
  :myTol(1.e-12) 
  {}

//=======================================================================
// function: XBOPTools_ComparePave::XBOPTools_ComparePave
// purpose: 
//=======================================================================
  XBOPTools_ComparePave::XBOPTools_ComparePave(const Standard_Real aTol)
{
  myTol=aTol;
}
//=======================================================================
// function: IsLower
// purpose: 
//=======================================================================
  Standard_Boolean XBOPTools_ComparePave::IsLower(const XBOPTools_Pave& aLeft,
						 const XBOPTools_Pave& aRight)const
{
  return aLeft.Param()<aRight.Param();
}
//=======================================================================
// function: IsGreater
// purpose: 
//=======================================================================
  Standard_Boolean XBOPTools_ComparePave::IsGreater(const XBOPTools_Pave& aLeft,
						   const XBOPTools_Pave& aRight)const
{
  return aLeft.Param()>aRight.Param();
}
//=======================================================================
// function: IsEqual
// purpose: 
//=======================================================================
  Standard_Boolean XBOPTools_ComparePave::IsEqual(const XBOPTools_Pave& aLeft,
						 const XBOPTools_Pave& aRight)const
{
  Standard_Real a, b;
  a=aLeft.Param();
  b=aRight.Param();
  return fabs(a-b) < myTol;
}
