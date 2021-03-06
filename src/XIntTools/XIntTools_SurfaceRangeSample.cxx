// Created on: 2005-10-05
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



#include <XIntTools_SurfaceRangeSample.ixx>

XIntTools_SurfaceRangeSample::XIntTools_SurfaceRangeSample()
{
}

XIntTools_SurfaceRangeSample::XIntTools_SurfaceRangeSample(const Standard_Integer theIndexU,const Standard_Integer theDepthU,
							 const Standard_Integer theIndexV,const Standard_Integer theDepthV)
{
  myRangeU.SetRangeIndex(theIndexU);
  myRangeU.SetDepth(theDepthU);
  myRangeV.SetRangeIndex(theIndexV);
  myRangeV.SetDepth(theDepthV);
}

XIntTools_SurfaceRangeSample::XIntTools_SurfaceRangeSample(const XIntTools_CurveRangeSample& theRangeU,
							 const XIntTools_CurveRangeSample& theRangeV)
{
  myRangeU = theRangeU;
  myRangeV = theRangeV;
}

XIntTools_SurfaceRangeSample::XIntTools_SurfaceRangeSample(const XIntTools_SurfaceRangeSample& Other)
{
  Assign(Other);
}

XIntTools_SurfaceRangeSample& XIntTools_SurfaceRangeSample::Assign(const XIntTools_SurfaceRangeSample& Other) 
{
  myRangeU = Other.myRangeU;
  myRangeV = Other.myRangeV;
  return (*this);
}


XIntTools_Range XIntTools_SurfaceRangeSample::GetRangeU(const Standard_Real theFirstU,const Standard_Real theLastU,
						      const Standard_Integer theNbSampleU) const
{
  return myRangeU.GetRange(theFirstU, theLastU, theNbSampleU);
}

XIntTools_Range XIntTools_SurfaceRangeSample::GetRangeV(const Standard_Real theFirstV,const Standard_Real theLastV,
						      const Standard_Integer theNbSampleV) const
{
  return myRangeV.GetRange(theFirstV, theLastV, theNbSampleV);
}
