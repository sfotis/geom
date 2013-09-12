// Created on: 2002-02-01
// Created by: Peter KURNEV
// Copyright (c) 2002-2012 OPEN CASCADE SAS
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


#include <XBOP_EmptyBuilder.ixx>

#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>

#include <TopoDS_Compound.hxx>

#include <BRep_Builder.hxx>

#include <XBOPTColStd_Dump.hxx>
#include <XBOPTools_DSFiller.hxx>
#include <XBOPTools_Tools3D.hxx>




//=======================================================================
// function: XBOP_EmptyBuilder::XBOP_EmptyBuilder
// purpose: 
//=======================================================================
XBOP_EmptyBuilder::XBOP_EmptyBuilder()
{
}
//=======================================================================
// function: Destroy
// purpose: 
//=======================================================================
  void XBOP_EmptyBuilder::Destroy() 
{
}
//=======================================================================
// function: Do
// purpose: 
//=======================================================================
  void XBOP_EmptyBuilder::Do() 
{
  myErrorStatus=0;
  myIsDone=Standard_False;
  //
  // Filling the DS
  XBOPTools_DSFiller aDSFiller;
  aDSFiller.SetShapes (myShape1, myShape2);
  //
  aDSFiller.Perform ();
  //
  DoWithFiller(aDSFiller);
}

//=======================================================================
// function: DoWithFiller
// purpose: 
//=======================================================================
  void XBOP_EmptyBuilder::DoWithFiller(const XBOPTools_DSFiller& aDSFiller) 
{
  myErrorStatus=0;
  myIsDone=Standard_False;
  //
  myResultMap.Clear();
  myModifiedMap.Clear();
  //
  myDSFiller=(XBOPTools_DSFiller*) &aDSFiller;
  //
  // 
  try {
    OCC_CATCH_SIGNALS
    Standard_Boolean bIsNewFiller;
    bIsNewFiller=aDSFiller.IsNewFiller();
    
    if (bIsNewFiller) {
      aDSFiller.SetNewFiller(!bIsNewFiller);
    }
    //
    BuildResult();
    //
    //
    //FillModified();
    myIsDone=Standard_True;
  }

  catch ( Standard_Failure ) {
    myErrorStatus = 1;
    XBOPTColStd_Dump::PrintMessage("Can not build result\n");
  }
  //
}

//=======================================================================
// function: BuildResult
// purpose: 
//=======================================================================
  void XBOP_EmptyBuilder::BuildResult()
{
  Standard_Boolean bIsEmptyShape1, bIsEmptyShape2;
  BRep_Builder aBB;
  TopoDS_Compound aCompound;

  aBB.MakeCompound(aCompound);
  //
  bIsEmptyShape1=XBOPTools_Tools3D::IsEmptyShape(myShape1);
  bIsEmptyShape2=XBOPTools_Tools3D::IsEmptyShape(myShape2);
  //
  if (!bIsEmptyShape1 && bIsEmptyShape2) {
    if (myOperation==XBOP_FUSE || myOperation==XBOP_CUT) {
      aBB.Add(aCompound, myShape1);
    }
  }
  //
  else if (bIsEmptyShape1 && !bIsEmptyShape2) {
    if (myOperation==XBOP_FUSE || myOperation==XBOP_CUT21) {
      aBB.Add(aCompound, myShape2);
    }
  }
  //
  myResult=aCompound;
}

