// Created on: 2002-02-04
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



#include <XBOP_WireShell.ixx>

#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>

#include <TopAbs_ShapeEnum.hxx>

#include <XBooleanOperations_ShapesDataStructure.hxx>

#include <XBOPTColStd_Dump.hxx>

#include <XBOPTools_DSFiller.hxx>
#include <XBOPTools_PaveFiller.hxx>
#include <XBOPTools_WireStateFiller.hxx>

#include <XBOP_CorrectTolerances.hxx>

//=======================================================================
// function: XBOP_WireShell::XBOP_WireShell
// purpose: 
//=======================================================================
XBOP_WireShell::XBOP_WireShell()
{
}
//=======================================================================
// function: Destroy
// purpose: 
//=======================================================================
  void XBOP_WireShell::Destroy() 
{
}
//=======================================================================
// function: Do
// purpose: 
//=======================================================================
  void XBOP_WireShell::Do() 
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
  void XBOP_WireShell::DoWithFiller(const XBOPTools_DSFiller& aDSFiller) 
{
  myErrorStatus=0;
  myIsDone=Standard_False;
  //
  myResultMap.Clear();
  myModifiedMap.Clear();
  //
  myDSFiller=(XBOPTools_DSFiller*) &aDSFiller;
  //
  try {
    OCC_CATCH_SIGNALS

    if(!myDSFiller->IsDone()) {
      myErrorStatus = 1;
      XBOPTColStd_Dump::PrintMessage("DSFiller is invalid: Can not build result\n");
      return;
    }

    Standard_Boolean bCheckTypes;
    //
    bCheckTypes=CheckArgTypes();
    if (!bCheckTypes) {
      myErrorStatus=10;
      return;
    }
    //
    Standard_Boolean bIsNewFiller;
    bIsNewFiller=aDSFiller.IsNewFiller();
    
    if (bIsNewFiller) {
      //
      // Preparing the States
      const XBOPTools_PaveFiller& aPaveFiller=myDSFiller->PaveFiller();
      XBOPTools_WireStateFiller aStateFiller(aPaveFiller);
      aStateFiller.Do();
      
      aDSFiller.SetNewFiller(!bIsNewFiller);
    }
    //
    BuildResult();
    //
    XBOP_CorrectTolerances::CorrectTolerances(myResult, 0.01);
    //
    FillModified();
    myIsDone=Standard_True;
  }
  catch ( Standard_Failure ) {
    myErrorStatus = 1;
    XBOPTColStd_Dump::PrintMessage("Can not build result\n");
  }
}

//=======================================================================
// function: BuildResult
// purpose: 
//=======================================================================
  void XBOP_WireShell::BuildResult()
{
  AddSplitPartsINOUT();
  AddSplitPartsON();
  //
  MakeResult();
}

//=======================================================================
// function: CheckArgTypes
// purpose: 
//=======================================================================
Standard_Boolean XBOP_WireShell::CheckArgTypes(const TopAbs_ShapeEnum theType1,
					      const TopAbs_ShapeEnum theType2,
					      const XBOP_Operation theOperation) 
{
  Standard_Boolean bFlag=Standard_False;
  
  //
  if (theType1==TopAbs_WIRE && theType2==TopAbs_SHELL) {
    if (theOperation==XBOP_FUSE || theOperation==XBOP_CUT21) {
      return bFlag;
    }
  }
  //
  if (theType1==TopAbs_SHELL && theType2==TopAbs_WIRE) {
    if (theOperation==XBOP_FUSE || theOperation==XBOP_CUT) {
      return bFlag;
    }
  }
  //
  return !bFlag;
}


//=======================================================================
// function: CheckArgTypes
// purpose: 
//=======================================================================
  Standard_Boolean XBOP_WireShell::CheckArgTypes() const
{
//   Standard_Boolean bFlag=Standard_False;
  
  TopAbs_ShapeEnum aT1, aT2;
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();

  aT1=aDS.Object().ShapeType();
  aT2=aDS.Tool().ShapeType();
  //
//   if (aT1==TopAbs_WIRE && aT2==TopAbs_SHELL) {
//     if (myOperation==XBOP_FUSE || myOperation==XBOP_CUT21) {
//       return bFlag;
//     }
//   }
//   //
//   if (aT1==TopAbs_SHELL && aT2==TopAbs_WIRE) {
//     if (myOperation==XBOP_FUSE || myOperation==XBOP_CUT) {
//       return bFlag;
//     }
//   }
//   //
//   return !bFlag;
  return CheckArgTypes(aT1, aT2, myOperation);
}
