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


#include <XBOPTools_Interference.ixx>

//=======================================================================
//function : XBOPTools_Interference::XBOPTools_Interference
//purpose  : 
//=======================================================================
XBOPTools_Interference::XBOPTools_Interference()
:
  myWith(0),
  myType(XBooleanOperations_UnknownInterference),
  myIndex(0)
{
}

//=======================================================================
//function : XBOPTools_Interference::XBOPTools_Interference
//purpose  : 
//=======================================================================
  XBOPTools_Interference::XBOPTools_Interference(const Standard_Integer aWith,
					       const XBooleanOperations_KindOfInterference aType,
					       const Standard_Integer anIndex)
:
  myWith(aWith),
  myType(aType),
  myIndex(anIndex)
{
}
//=======================================================================
//function : SetWith
//purpose  : 
//=======================================================================
  void XBOPTools_Interference::SetWith(const Standard_Integer aWith) 
{
  myWith=aWith;
}

//=======================================================================
//function : SetType
//purpose  : 
//=======================================================================
  void XBOPTools_Interference::SetType(const XBooleanOperations_KindOfInterference aType) 
{
  myType=aType;
}

//=======================================================================
//function : SetIndex
//purpose  : 
//=======================================================================
  void XBOPTools_Interference::SetIndex(const Standard_Integer anIndex) 
{
  myIndex=anIndex;
}

//=======================================================================
//function : With
//purpose  : 
//=======================================================================
  Standard_Integer XBOPTools_Interference::With() const
{
  return myWith;
}

//=======================================================================
//function : Type
//purpose  : 
//=======================================================================
  XBooleanOperations_KindOfInterference XBOPTools_Interference::Type() const
{
  return myType;
}

//=======================================================================
//function : Index
//purpose  : 
//=======================================================================
  Standard_Integer XBOPTools_Interference::Index() const
{
  return myIndex;
}
