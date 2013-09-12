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



#include <XBOP_WireSolid.ixx>

#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>

#include <XBooleanOperations_ShapesDataStructure.hxx>
#include <XBooleanOperations_StateOfShape.hxx>

#include <XBOPTColStd_Dump.hxx>

#include <XBOPTools_SplitShapesPool.hxx>
#include <XBOPTools_ListOfPaveBlock.hxx>
#include <XBOPTools_ListIteratorOfListOfPaveBlock.hxx>
#include <XBOPTools_PaveBlock.hxx>
#include <XBOPTools_DSFiller.hxx>
#include <XBOPTools_PaveFiller.hxx>
#include <XBOPTools_WireStateFiller.hxx>


#include <XBOP_CorrectTolerances.hxx>
#include <XBOP_BuilderTools.hxx>
#include <XBOP_WireSolidHistoryCollector.hxx>

//=======================================================================
// function: XBOP_WireSolid::XBOP_WireSolid
// purpose: 
//=======================================================================
XBOP_WireSolid::XBOP_WireSolid()
{
}
//=======================================================================
// function: Destroy
// purpose: 
//=======================================================================
  void XBOP_WireSolid::Destroy() 
{
}
//=======================================================================
// function: Do
// purpose: 
//=======================================================================
  void XBOP_WireSolid::Do() 
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
  void XBOP_WireSolid::DoWithFiller(const XBOPTools_DSFiller& aDSFiller) 
{
  myErrorStatus=0;
  myIsDone=Standard_False;
  myResultMap.Clear();
  myModifiedMap.Clear();
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

    if(!myHistory.IsNull()) {
      Handle(XBOP_WireSolidHistoryCollector) aHistory = 
	Handle(XBOP_WireSolidHistoryCollector)::DownCast(myHistory);
      aHistory->SetResult(myResult, myDSFiller);
    }
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
  void XBOP_WireSolid::BuildResult()
{
  
  AddSplitParts();
  //
  MakeResult();
}

//=======================================================================
// function: CheckArgTypes
// purpose: 
//=======================================================================
Standard_Boolean XBOP_WireSolid::CheckArgTypes(const TopAbs_ShapeEnum theType1,
					      const TopAbs_ShapeEnum theType2,
					      const XBOP_Operation    theOperation) 
{
  Standard_Boolean bFlag=Standard_False;

  if (theType1==TopAbs_WIRE && theType2==TopAbs_SOLID) {
    if (theOperation==XBOP_FUSE || theOperation==XBOP_CUT21) {
      return bFlag;
    }
  }
  //
  if (theType1==TopAbs_SOLID && theType2==TopAbs_WIRE) {
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
  Standard_Boolean XBOP_WireSolid::CheckArgTypes() const
{
//   Standard_Boolean bFlag=Standard_False;
  
  TopAbs_ShapeEnum aT1, aT2;
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();

  aT1=aDS.Object().ShapeType();
  aT2=aDS.Tool().ShapeType();
  //
//   if (aT1==TopAbs_WIRE && aT2==TopAbs_SOLID) {
//     if (myOperation==XBOP_FUSE || myOperation==XBOP_CUT21) {
//       return bFlag;
//     }
//   }
//   //
//   if (aT1==TopAbs_SOLID && aT2==TopAbs_WIRE) {
//     if (myOperation==XBOP_FUSE || myOperation==XBOP_CUT) {
//       return bFlag;
//     }
//   }
//   //
//   return !bFlag;
  return CheckArgTypes(aT1, aT2, myOperation);
}

//=======================================================================
// function: AddSplitParts
// purpose: 
//=======================================================================
  void XBOP_WireSolid::AddSplitParts() 
{
  
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  const XBOPTools_PaveFiller& aPaveFiller=myDSFiller->PaveFiller();
  const XBOPTools_SplitShapesPool& aSplitShapesPool=aPaveFiller.SplitShapesPool();
  //
  Standard_Integer i, aNbPB, iRank, nSp, iBeg, iEnd;
  TopAbs_ShapeEnum aType;
  XBooleanOperations_StateOfShape aState, aStateCmp;

  iBeg=1;
  iEnd=aDS.NumberOfShapesOfTheObject();
  if (aDS.Tool().ShapeType()==TopAbs_WIRE) {
    iBeg=iEnd+1;
    iEnd=aDS.NumberOfSourceShapes();
  }

  for (i=iBeg; i<=iEnd; ++i) {
    aType=aDS.GetShapeType(i);
    if (aType!=TopAbs_EDGE) {
      continue;
    }
    const TopoDS_Edge& aE=TopoDS::Edge(aDS.Shape(i));
    iRank=aDS.Rank(i);
  
    aStateCmp=XBOP_BuilderTools::StateToCompare(iRank, myOperation);
  
    const XBOPTools_ListOfPaveBlock& aLPB=aSplitShapesPool(aDS.RefEdge(i));
    aNbPB=aLPB.Extent();
    //
    if (!aNbPB) {
      aState=aDS.GetState(i);
      if (aState==aStateCmp) {
	myLS.Append(aE);
      }
      if (myOperation==XBOP_COMMON && aState==XBooleanOperations_ON) {
	myLS.Append(aE);
      }
    }
    //
    else {
      XBOPTools_ListIteratorOfListOfPaveBlock aPBIt(aLPB);
      for (; aPBIt.More(); aPBIt.Next()) {
	const XBOPTools_PaveBlock& aPB=aPBIt.Value();
	nSp=aPB.Edge();
	const TopoDS_Edge& aSS=TopoDS::Edge(aDS.Shape(nSp));
	aState=aDS.GetState(nSp);
	if (aState==aStateCmp) {
	  myLS.Append(aSS);
	}
	if (myOperation==XBOP_COMMON && aState==XBooleanOperations_ON) {
	  myLS.Append(aSS);
	}
      }
    }
  }
}

//=======================================================================
// function: SetHistoryCollector
// purpose: 
//=======================================================================
void XBOP_WireSolid::SetHistoryCollector(const Handle(XBOP_HistoryCollector)& theHistory) 
{
  if(theHistory.IsNull() ||
     !theHistory->IsKind(STANDARD_TYPE(XBOP_WireSolidHistoryCollector)))
    myHistory.Nullify();
  else
    myHistory = theHistory;
}
