// Created on: 2004-09-02
// Created by: Oleg FEDYAEV
// Copyright (c) 2004-2012 OPEN CASCADE SAS
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


#include <XBOP_CheckResult.ixx>

//=======================================================================
// function:  XBOP_CheckResult()
// purpose: 
//=======================================================================
XBOP_CheckResult::XBOP_CheckResult() : myStatus(XBOP_CheckUnknown)
{
}

void XBOP_CheckResult::SetShape1(const TopoDS_Shape& TheShape)
{
  myShape1 = TheShape;
}

void XBOP_CheckResult::AddFaultyShape1(const TopoDS_Shape& TheShape)
{
  myFaulty1.Append(TheShape);
}

void XBOP_CheckResult::SetShape2(const TopoDS_Shape& TheShape)
{
  myShape2 = TheShape;
}

void XBOP_CheckResult::AddFaultyShape2(const TopoDS_Shape& TheShape)
{
  myFaulty2.Append(TheShape);
}

const TopoDS_Shape& XBOP_CheckResult::GetShape1() const
{
  return myShape1;
}

const TopoDS_Shape & XBOP_CheckResult::GetShape2() const
{
  return myShape2;
}

const TopTools_ListOfShape& XBOP_CheckResult::GetFaultyShapes1() const
{
  return myFaulty1;
}

const TopTools_ListOfShape& XBOP_CheckResult::GetFaultyShapes2() const
{
  return myFaulty2;
}

void XBOP_CheckResult::SetCheckStatus(const XBOP_CheckStatus TheStatus)
{
  myStatus = TheStatus;
}

XBOP_CheckStatus XBOP_CheckResult::GetCheckStatus() const
{
  return myStatus;
}
