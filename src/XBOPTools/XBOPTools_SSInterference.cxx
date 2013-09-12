// Created on: 2000-11-21
// Created by: Peter KURNEV
// Copyright (c) 2000-2012 OPEN CASCADE SAS
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



#include <XBOPTools_SSInterference.ixx>

#include <XIntTools_Curve.hxx>
#include <XIntTools_SequenceOfCurves.hxx>
#include <XIntTools_Tools.hxx>
#include <XIntTools_PntOn2Faces.hxx>

#include <XBOPTools_Curve.hxx>
//modified by NIZNHY-PKV Fri Jun 30 10:08:51 2006
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopoDS_Shape.hxx>
#include <TColStd_ListOfInteger.hxx>
#include <TColStd_ListIteratorOfListOfInteger.hxx>


//=======================================================================
//function :  XBOPTools_SSInterference
//purpose  : 
//=======================================================================
XBOPTools_SSInterference::XBOPTools_SSInterference()
:
  XBOPTools_ShapeShapeInterference(0, 0),
  myTangentFacesFlag(Standard_False),
  mySenseFlag(0)
{}

//=======================================================================
//function :  XBOPTools_SSInterference::XBOPTools_SSInterference
//purpose  : 
//=======================================================================
  XBOPTools_SSInterference::XBOPTools_SSInterference
  (const Standard_Integer anIndex1,
   const Standard_Integer anIndex2,
   const Standard_Real aTolR3D,
   const Standard_Real aTolR2D,
   const XIntTools_SequenceOfCurves& aCvs,
   const XIntTools_SequenceOfPntOn2Faces& aPnts)
:  
  XBOPTools_ShapeShapeInterference(anIndex1, anIndex2),
  myTangentFacesFlag(Standard_False),
  mySenseFlag(0)
{
  myTolR3D=aTolR3D;
  myTolR2D=aTolR2D;
  
  myCurves.Clear();
  Standard_Integer i, aNbCurves;
  aNbCurves=aCvs.Length();
  for (i=1; i<=aNbCurves; i++) {
    const XIntTools_Curve& aIC=aCvs(i);
    XBOPTools_Curve aBC(aIC);
    myCurves.Append(aBC);
  }
  //
  SetAlonePnts(aPnts);
}
//=======================================================================
//function :  AppendBlock
//purpose  : 
//=======================================================================
  void XBOPTools_SSInterference::AppendBlock(const XBOPTools_PaveBlock& aPB) 
{
  myPBs.Append(aPB);
}

//=======================================================================
//function :  PaveBlocks
//purpose  : 
//=======================================================================
  const XBOPTools_ListOfPaveBlock& XBOPTools_SSInterference::PaveBlocks() const
{
  return myPBs;
}

//=======================================================================
//function :  TolR3D
//purpose  : 
//=======================================================================
  Standard_Real XBOPTools_SSInterference::TolR3D() const
{
  return myTolR3D;
}

//=======================================================================
//function :  TolR2D
//purpose  : 
//=======================================================================
  Standard_Real XBOPTools_SSInterference::TolR2D() const
{
  return myTolR2D;
}

//=======================================================================
//function :  Curves
//purpose  : 
//=======================================================================
  XBOPTools_SequenceOfCurves& XBOPTools_SSInterference::Curves() 
{
  return myCurves;
}

//=======================================================================
//function :  NewPaveSet
//purpose  : 
//=======================================================================
  XBOPTools_PaveSet& XBOPTools_SSInterference::NewPaveSet() 
{
  return myNewPaveSet;
}

//=======================================================================
//function :  SetTangentFacesFlag
//purpose  : 
//=======================================================================
  void XBOPTools_SSInterference::SetTangentFacesFlag(const Standard_Boolean aFlag) 
{
  myTangentFacesFlag=aFlag;
}
//=======================================================================
//function :  IsTangentFaces
//purpose  : 
//=======================================================================
  Standard_Boolean XBOPTools_SSInterference::IsTangentFaces()const 
{
  return myTangentFacesFlag;
}

//=======================================================================
// function: SetSenseFlag
// purpose: 
//=======================================================================
  void XBOPTools_SSInterference::SetSenseFlag (const Standard_Integer iFlag)
				 
{
   mySenseFlag=iFlag;
}
//=======================================================================
// function: SenseFlag
// purpose: 
//=======================================================================
  Standard_Integer XBOPTools_SSInterference::SenseFlag () const
				 
{
   return mySenseFlag;
}
//=======================================================================
// function: SetStatesMap
// purpose: 
//=======================================================================
  void XBOPTools_SSInterference::SetStatesMap (const XBOPTools_IndexedDataMapOfIntegerState& aMap)
{
  myStatesMap=aMap;
}
//=======================================================================
// function: StatesMap
// purpose: 
//=======================================================================
  const XBOPTools_IndexedDataMapOfIntegerState& XBOPTools_SSInterference::StatesMap () const
{
  return myStatesMap;
}

//=======================================================================
// function: SetAlonePnts
// purpose: 
//=======================================================================
  void XBOPTools_SSInterference::SetAlonePnts (const XIntTools_SequenceOfPntOn2Faces& aPnts)
{
  Standard_Integer i, aNb;
  myAlonePnts.Clear();

  aNb=aPnts.Length();
  for (i=1; i<=aNb; ++i) {
    const XIntTools_PntOn2Faces& aPntOn2Faces=aPnts(i);
    myAlonePnts.Append(aPntOn2Faces);
  }
}

//=======================================================================
// function: AlonePnts
// purpose: 
//=======================================================================
  const XIntTools_SequenceOfPntOn2Faces& XBOPTools_SSInterference::AlonePnts ()const 
{
  return myAlonePnts;
}

//=======================================================================
// function: AloneVertices
// purpose: 
//=======================================================================
  TColStd_ListOfInteger& XBOPTools_SSInterference::AloneVertices () 
{
  return myAloneVertices;
}
//modified by NIZNHY-PKV Fri Jun 30 10:06:12 2006f
//=======================================================================
// function: SetSharedEdges
// purpose: 
//=======================================================================
  void XBOPTools_SSInterference::SetSharedEdges (const TColStd_ListOfInteger& aLS) 
{
  Standard_Integer nE;
  TColStd_ListIteratorOfListOfInteger aIt;
  //
  aIt.Initialize(aLS);
  for(; aIt.More(); aIt.Next()) {
    nE=aIt.Value();
    mySharedEdges.Append(nE);
  }
}
//=======================================================================
// function: SharedEdges
// purpose: 
//=======================================================================
  const TColStd_ListOfInteger& XBOPTools_SSInterference::SharedEdges()const 
{
  return mySharedEdges;
}
/*
//=======================================================================
// function: SetSharedEdges
// purpose: 
//=======================================================================
  void XBOPTools_SSInterference::SetSharedEdges (const TopTools_ListOfShape& aLS) 
{
  TopTools_ListIteratorOfListOfShape aIt;
  //
  aIt.Initialize(aLS);
  for(; aIt.More(); aIt.Next()) {
    const TopoDS_Shape& aE=aIt.Value();
    mySharedEdges.Append(aE);
  }
}
//=======================================================================
// function: SharedEdges
// purpose: 
//=======================================================================
  const TopTools_ListOfShape& XBOPTools_SSInterference::SharedEdges()const 
{
  return mySharedEdges;
}
*/
//modified by NIZNHY-PKV Fri Jun 30 10:06:14 2006t
