// Created on: 2005-10-14
// Created by: Mikhail KLOKOV
// Copyright (c) 2005-2012 OPEN CASCADE SAS
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



#include <XIntTools_CurveRangeLocalizeData.ixx>
#include <XIntTools_ListIteratorOfListOfCurveRangeSample.hxx>
#include <XIntTools_ListIteratorOfListOfBox.hxx>
#include <XIntTools_MapIteratorOfMapOfCurveSample.hxx>

XIntTools_CurveRangeLocalizeData::XIntTools_CurveRangeLocalizeData(const Standard_Integer theNbSample,
								 const Standard_Real theMinRange)
{
  myNbSampleC = theNbSample;
  myMinRangeC = theMinRange;
}

void XIntTools_CurveRangeLocalizeData::AddOutRange(const XIntTools_CurveRangeSample& theRange) 
{
  myMapRangeOut.Add(theRange);
  myMapBox.UnBind(theRange);
}

void XIntTools_CurveRangeLocalizeData::AddBox(const XIntTools_CurveRangeSample& theRange,
					     const Bnd_Box& theBox) 
{
  myMapBox.Bind(theRange, theBox);
}

Standard_Boolean XIntTools_CurveRangeLocalizeData::FindBox(const XIntTools_CurveRangeSample& theRange,Bnd_Box& theBox) const
{
  if(myMapBox.IsBound(theRange)) {
    theBox = myMapBox(theRange);
    return Standard_True;
  }
  return Standard_False;
}

Standard_Boolean XIntTools_CurveRangeLocalizeData::IsRangeOut(const XIntTools_CurveRangeSample& theRange) const
{
  return myMapRangeOut.Contains(theRange);
}

void XIntTools_CurveRangeLocalizeData::ListRangeOut(XIntTools_ListOfCurveRangeSample& theList) const
{
  XIntTools_MapIteratorOfMapOfCurveSample anIt(myMapRangeOut);

  for(; anIt.More(); anIt.Next())
    theList.Append(anIt.Key());
}

