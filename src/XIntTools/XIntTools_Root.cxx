// Created on: 2000-05-22
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



#include <XIntTools_Root.ixx>

//=======================================================================
//function :XIntTools_Root::XIntTools_Root
//purpose  : 
//=======================================================================
XIntTools_Root::XIntTools_Root():
  myRoot(0.),myType(-1),myLayerHeight(0.),
  myStateBefore(TopAbs_UNKNOWN), 
  myStateAfter(TopAbs_UNKNOWN),
  myt1(0.), 
  myt2(0.), 
  myf1(0.), 
  myf2(0.) 
{}
 
//=======================================================================
//function :XIntTools_Root::XIntTools_Root
//purpose  : 
//=======================================================================
  XIntTools_Root::XIntTools_Root(const Standard_Real aRoot,
			       const Standard_Integer aType):
  myLayerHeight(0.),
  myStateBefore(TopAbs_UNKNOWN), 
  myStateAfter(TopAbs_UNKNOWN),
  myt1(0.), 
  myt2(0.), 
  myf1(0.), 
  myf2(0.)  
{
  myRoot=aRoot;
  myType=aType;
} 

//=======================================================================
//function :SetRoot
//purpose  : 
//=======================================================================
  void XIntTools_Root::SetRoot(const Standard_Real aRoot)
{
  myRoot=aRoot;
} 
//=======================================================================
//function :SetType
//purpose  : 
//=======================================================================
  void XIntTools_Root::SetType(const Standard_Integer aType)
{
  myType=aType;
} 
//=======================================================================
//function :SetStateBefore
//purpose  : 
//=======================================================================
  void XIntTools_Root::SetStateBefore(const TopAbs_State aState)
{
  myStateBefore=aState;
} 
//=======================================================================
//function :SetStateAfter
//purpose  : 
//=======================================================================
  void XIntTools_Root::SetStateAfter(const TopAbs_State aState)
{
  myStateAfter=aState;
}
//=======================================================================
//function :SetLayerHeight
//purpose  : 
//=======================================================================
  void XIntTools_Root::SetLayerHeight(const Standard_Real aHeight)
{
  myLayerHeight=aHeight;
} 
//=======================================================================
//function :SetInterval
//purpose  : 
//=======================================================================
  void XIntTools_Root::SetInterval(const Standard_Real t1,
				  const Standard_Real t2,
				  const Standard_Real f1,
				  const Standard_Real f2)
{
  myt1=t1;
  myt2=t2;
  myf1=f1;
  myf2=f2;
} 
//=======================================================================
//function :Interval
//purpose  : 
//=======================================================================
  void XIntTools_Root::Interval(Standard_Real& t1,
			       Standard_Real& t2,
			       Standard_Real& f1,
			       Standard_Real& f2) const
{
  t1=myt1;
  t2=myt2;
  f1=myf1;
  f2=myf2;
}
 
//=======================================================================
//function :Root
//purpose  : 
//=======================================================================
  Standard_Real XIntTools_Root::Root() const
{
  return myRoot;
}

//=======================================================================
//function :Type
//purpose  : 
//=======================================================================
  Standard_Integer XIntTools_Root::Type() const
{
  return myType;
}
//=======================================================================
//function :StateBefore
//purpose  : 
//=======================================================================
  TopAbs_State XIntTools_Root::StateBefore() const
{
  return myStateBefore;
} 
//=======================================================================
//function :StateAfter
//purpose  : 
//=======================================================================
  TopAbs_State XIntTools_Root::StateAfter() const
{
  return myStateAfter;
} 
//=======================================================================
//function :LayerHeight
//purpose  : 
//=======================================================================
  Standard_Real XIntTools_Root::LayerHeight() const
{
  return myLayerHeight;
}

//=======================================================================
//function :IsValid
//purpose  : 
//=======================================================================
  Standard_Boolean XIntTools_Root::IsValid() const
{
  if (myStateBefore==TopAbs_OUT && myStateAfter==TopAbs_IN )  
    return Standard_True;
  if (myStateBefore==TopAbs_OUT && myStateAfter==TopAbs_ON )  
    return Standard_True;
  if (myStateBefore==TopAbs_ON  && myStateAfter==TopAbs_OUT)  
    return Standard_True;
  if (myStateBefore==TopAbs_IN  && myStateAfter==TopAbs_OUT)  
    return Standard_True;

  return Standard_False;
} 
