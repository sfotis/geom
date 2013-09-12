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



#include <XBOPTools_InterferenceLine.ixx>
#include <XBOPTools_ListIteratorOfListOfInterference.hxx>

//=======================================================================
//function : XBOPTools_InterferenceLine::XBOPTools_InterferenceLine
//purpose  : 
//=======================================================================
XBOPTools_InterferenceLine::XBOPTools_InterferenceLine() {}

//=======================================================================
//function : RealList
//purpose  : 
//=======================================================================
  const XBOPTools_ListOfInterference& XBOPTools_InterferenceLine::RealList() const
{
  Standard_Integer anInd;
  List();
  XBOPTools_ListOfInterference aTmpList;
  XBOPTools_ListIteratorOfListOfInterference anIt;

  anIt.Initialize(myList);
  for (; anIt.More(); anIt.Next()) {
    const XBOPTools_Interference& anInterference=anIt.Value();
    aTmpList.Append(anInterference);
  }
  //
  XBOPTools_ListOfInterference* pList=(XBOPTools_ListOfInterference*)&myList;
  pList->Clear();
  //
  anIt.Initialize(aTmpList);
  for (; anIt.More(); anIt.Next()) {
    const XBOPTools_Interference& anInterference=anIt.Value();
    anInd=anInterference.Index();
    if (anInd) {
      pList->Append(anInterference);
    }
  }
  return myList;
}

//=======================================================================
//function : List
//purpose  : 
//=======================================================================
  const XBOPTools_ListOfInterference& XBOPTools_InterferenceLine::List () const
{
  XBOPTools_ListOfInterference* pList=(XBOPTools_ListOfInterference*)&myList;
  
  pList->Clear();

  XBOPTools_ListIteratorOfListOfInterference anIt;

  anIt.Initialize(mySSList);
  for (; anIt.More(); anIt.Next()) {
    const XBOPTools_Interference& anInterference=anIt.Value();
    pList->Append(anInterference);
  }

  anIt.Initialize(myESList);
  for (; anIt.More(); anIt.Next()) {
    const XBOPTools_Interference& anInterference=anIt.Value();
    pList->Append(anInterference);
  }

  anIt.Initialize(myVSList);
  for (; anIt.More(); anIt.Next()) {
    const XBOPTools_Interference& anInterference=anIt.Value();
    pList->Append(anInterference);
  }

  anIt.Initialize(myEEList);
  for (; anIt.More(); anIt.Next()) {
    const XBOPTools_Interference& anInterference=anIt.Value();
    pList->Append(anInterference);
  }

  anIt.Initialize(myVEList);
  for (; anIt.More(); anIt.Next()) {
    const XBOPTools_Interference& anInterference=anIt.Value();
    pList->Append(anInterference);
  }

  anIt.Initialize(myVVList);
  for (; anIt.More(); anIt.Next()) {
    const XBOPTools_Interference& anInterference=anIt.Value();
    pList->Append(anInterference);
  }

  return myList;
}

//=======================================================================
//function : GetOnType
//purpose  : 
//=======================================================================
  const XBOPTools_ListOfInterference& XBOPTools_InterferenceLine::GetOnType
                       (const XBooleanOperations_KindOfInterference theType) const
{
  switch (theType) {
  case XBooleanOperations_SurfaceSurface:
    return mySSList;

  case XBooleanOperations_EdgeSurface:
    return myESList;
    
  case XBooleanOperations_VertexSurface:
    return myVSList;
    
  case XBooleanOperations_EdgeEdge:
    return myEEList;

  case XBooleanOperations_VertexEdge:
    return myVEList;
    
  case XBooleanOperations_VertexVertex:
    return  myVVList; 
  default:
    return  myEmptyList;
  }
  
}

//=======================================================================
//function : HasInterference
//purpose  : 
//=======================================================================
  Standard_Boolean XBOPTools_InterferenceLine::HasInterference () const
{
  Standard_Integer anInd;
  Standard_Boolean bFlag=Standard_False;
  XBOPTools_ListIteratorOfListOfInterference anIt;
  //
  List();
  //
  anIt.Initialize(myList);
  for (; anIt.More(); anIt.Next()) {
    const XBOPTools_Interference& anInterference=anIt.Value();
    anInd=anInterference.Index();
    if (anInd) {
      return !bFlag;
    }
  }
  return bFlag;
}

//=======================================================================
//function : IsComputed
//purpose  : 
//=======================================================================
  Standard_Boolean XBOPTools_InterferenceLine::IsComputed 
           (const Standard_Integer theWith,
	    const XBooleanOperations_KindOfInterference theType)const
{
  XBooleanOperations_KindOfInterference aType;
  Standard_Integer aWith, anInd;

  const XBOPTools_ListOfInterference& aList=GetOnType(theType);
  XBOPTools_ListIteratorOfListOfInterference anIt(aList);
  for (; anIt.More(); anIt.Next()) {
    const XBOPTools_Interference& anInterference=anIt.Value();
    aType=anInterference.Type();
    aWith=anInterference.With();
    //
    anInd=anInterference.Index();
    //
    if ((aType==theType && aWith==theWith) && anInd) {
      return Standard_True;
    }
  }
  return Standard_False;
}
//=======================================================================
//function : AddInterference
//purpose  : 
//=======================================================================
  void XBOPTools_InterferenceLine::AddInterference(const XBOPTools_Interference& anInterference) 
{
  Standard_Integer aWith, anInd;
  XBooleanOperations_KindOfInterference aType;

  aWith=anInterference.With();
  aType=anInterference.Type();
  anInd=anInterference.Index();
  AddInterference(aWith, aType, anInd);
}


//=======================================================================
//function : AddInterference
//purpose  : 
//=======================================================================
  void XBOPTools_InterferenceLine::AddInterference(const Standard_Integer aWith,
						  const XBooleanOperations_KindOfInterference aType,
						  const Standard_Integer anIndex) 
{
  XBOPTools_Interference anInterference(aWith, aType, anIndex);
  const XBOPTools_ListOfInterference& aList=GetOnType(aType);
  XBOPTools_ListOfInterference* pList=(XBOPTools_ListOfInterference*) &aList;
  pList->Append(anInterference);
}

