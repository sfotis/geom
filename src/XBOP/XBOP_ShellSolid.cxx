// Created on: 2001-11-02
// Created by: Peter KURNEV
// Copyright (c) 2001-2012 OPEN CASCADE SAS
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


#include <XBOP_ShellSolid.ixx>

#include <gp_Dir.hxx>
#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>

#include <TColStd_ListIteratorOfListOfInteger.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Compound.hxx>

#include <TopExp.hxx>

#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>

#include <XBooleanOperations_ShapesDataStructure.hxx>

#include <XIntTools_Tools.hxx>
#include <XIntTools_Context.hxx>

#include <XBOP_WireEdgeSet.hxx>
#include <XBOP_SDFWESFiller.hxx>
#include <XBOP_FaceBuilder.hxx>
#include <XBOP_BuilderTools.hxx>
#include <XBOP_CorrectTolerances.hxx>
#include <XBOP_Draw.hxx>
#include <XBOP_Refiner.hxx>

#include <XBOPTools_DSFiller.hxx>
#include <XBOPTools_InterferencePool.hxx>
#include <XBOPTools_CArray1OfSSInterference.hxx>
#include <XBOPTools_SSInterference.hxx>
#include <XBOPTools_ESInterference.hxx>
#include <XBOPTools_VSInterference.hxx>

#include <XBOPTools_ListOfPaveBlock.hxx>
#include <XBOPTools_PaveBlock.hxx>
#include <XBOPTools_Tools3D.hxx>
#include <XBOPTools_Curve.hxx>

#include <XBOPTools_PaveFiller.hxx>
#include <XBOPTools_SplitShapesPool.hxx>
#include <XBOPTools_ListIteratorOfListOfPaveBlock.hxx>

#include <XBOPTools_SolidStateFiller.hxx>
#include <XBOPTools_PCurveMaker.hxx>
#include <XBOPTools_DEProcessor.hxx>

#include <XBOPTColStd_IndexedDataMapOfIntegerIndexedMapOfInteger.hxx>
#include <XBOPTColStd_Dump.hxx>

#include <BRepTools.hxx>
#include <Geom_Surface.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>

#include <XBOP_ShellSolidHistoryCollector.hxx>

static Standard_Boolean CheckSameDomainFaceInside(const TopoDS_Face& theFace1,
						  const TopoDS_Face& theFace2);

//////

//=======================================================================
// function: XBOP_ShellSolid::XBOP_ShellSolid
// purpose: 
//=======================================================================
  XBOP_ShellSolid::XBOP_ShellSolid()
{
  char* xr=getenv("MDISP");
  if (xr!=NULL) {
    myDraw=0;
    if (!strcmp (xr, "yes")) {
      myDraw=1;
    }
  }
  else {
    myDraw=0;
  }
  myRank=0;
}
//=======================================================================
// function: Destroy
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::Destroy() {}

//=======================================================================
// function: Do
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::Do() 
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
  void XBOP_ShellSolid::DoWithFiller(const XBOPTools_DSFiller& aDSFiller) 
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
    
    bCheckTypes=CheckArgTypes();
    if (!bCheckTypes) {
      myErrorStatus=10;
      return;
    }
    //
    Standard_Boolean bIsNewFiller;
    bIsNewFiller=aDSFiller.IsNewFiller();
    
    if (bIsNewFiller) {
      Prepare();
      aDSFiller.SetNewFiller(!bIsNewFiller);
    }
    //
    myRank=(myDSFiller->DS().Object().ShapeType()==TopAbs_SHELL) ? 1 : 2;
    //
    DoNewFaces();
    //
    BuildResult();
    //
    // Treat of internals
    CollectInternals();
    XBOP_Refiner aRefiner;
    aRefiner.SetShape(myResult);
    aRefiner.SetInternals(myInternals);
    aRefiner.Do();
    //
    //
    XBOP_CorrectTolerances::CorrectTolerances(myResult, 0.01);
    //
    FillModified();

    if(!myHistory.IsNull()) {
      Handle(XBOP_ShellSolidHistoryCollector) aHistory = 
	Handle(XBOP_ShellSolidHistoryCollector)::DownCast(myHistory);
      aHistory->SetResult(myResult, myDSFiller);
    }
    myIsDone=Standard_True;
  }
  catch ( Standard_Failure ) {
    myErrorStatus = 1;
    XBOPTColStd_Dump::PrintMessage("Can not build result\n");
  }
}

//=================================================================================
// function: BuildResult
// purpose: 
//=================================================================================
  void XBOP_ShellSolid::BuildResult() 
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();

  Standard_Integer i, j, aNb, iRank, aNbFaces;
  XBooleanOperations_StateOfShape aState, aStateToCompare;
  BRep_Builder aBB;
  TopoDS_Compound aFCompound, aRes;
  //
  aBB.MakeCompound(aRes);
  //
  // 1. Make aCompound containing all faces for thr Result
  aBB.MakeCompound(aFCompound);
  //
  // 1.1. Old Faces with right 3D-state
  aNb=aDS.NumberOfSourceShapes();
  for (i=1; i<=aNb; i++) {
    const TopoDS_Shape& aS=aDS.Shape(i);
    
    if (aS.ShapeType()==TopAbs_FACE){
      if (aDS.Rank(i)==myRank){ 
	aState=aDS.GetState(i);
	if (aState==XBooleanOperations_IN ||
	    aState==XBooleanOperations_OUT) {
	  iRank=aDS.Rank(i);
	  aStateToCompare=XBOP_BuilderTools::StateToCompare(iRank, myOperation);
	  if (aState==aStateToCompare) {
	    aBB.Add(aFCompound, aS);
	  }
	}
      }
    }
  }
  //
  // 1.2. aListOfNewFaces
  TopTools_ListIteratorOfListOfShape anIt(myNewFaces);
  for(; anIt.More(); anIt.Next()) {
    aBB.Add(aFCompound, anIt.Value());
  }
  //
  // 2.
  TopTools_IndexedDataMapOfShapeListOfShape aEFMap;
  TopTools_IndexedMapOfShape aProcessedEdges;
  
  TopExp::MapShapesAndAncestors(aFCompound, TopAbs_EDGE, TopAbs_FACE, aEFMap);
  aNb=aEFMap.Extent();
  for (i=1; i<=aNb; i++) {
    const TopoDS_Shape& aE=aEFMap.FindKey(i);
    TopTools_IndexedMapOfShape aFaces;
    Path (aE, aEFMap, aFaces, aProcessedEdges);
    
    TopoDS_Shell aShell, aShellNew;
    aBB.MakeShell(aShell);
    
    aNbFaces=aFaces.Extent();
    if (aNbFaces) {
      for (j=1; j<=aNbFaces; j++) {
	const TopoDS_Shape& aF=aFaces(j);
	aBB.Add(aShell, aF);
      }

      OrientFacesOnShell(aShell, aShellNew);

      aBB.Add(aRes, aShellNew);
    }
  }
  myResult=aRes;
}

//=======================================================================
// function: DoNewFaces
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::DoNewFaces()
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  XBOPTools_InterferencePool* pIntrPool=(XBOPTools_InterferencePool*)&myDSFiller->InterfPool();
  XBOPTools_CArray1OfSSInterference& aFFs=pIntrPool->SSInterferences();
  //
  // EF Maps
  const TopoDS_Shape& anObj=aDS.Object();
  const TopoDS_Shape& aTool=aDS.Tool();
  
  TopTools_IndexedMapOfShape anEMap;
  TopTools_IndexedDataMapOfShapeListOfShape aMEFObj, aMEFTool;
  TopExp::MapShapesAndAncestors (anObj, TopAbs_EDGE , TopAbs_FACE , aMEFObj);
  TopExp::MapShapesAndAncestors (aTool, TopAbs_EDGE , TopAbs_FACE , aMEFTool);
  //
  // vars
  Standard_Boolean bIsTouchCase, bIsTouch;
  Standard_Integer i, aNb, j, aNbj, iFF, nF1, iRank;
  TopTools_ListOfShape aListOfNewFaces;
  TopAbs_Orientation anOriF1;
  //
  // DoMap
  XBOPTColStd_IndexedDataMapOfIntegerIndexedMapOfInteger aFFMap;
  XBOP_BuilderTools::DoMap(aFFs, aFFMap);
  
  //
  aNb=aFFMap.Extent();
  for (i=1; i<=aNb; i++) {
    // 
    // a. Prepare info about the Face nF1 and create WES for nF1
    nF1=aFFMap.FindKey(i);
    const TopoDS_Face& aF1=TopoDS::Face(aDS.Shape(nF1));
    
    anOriF1=aF1.Orientation();
    iRank=aDS.Rank(nF1);
    
    if (iRank!=myRank) {
      continue;
    }
    
    myFace=aF1;
    myFace.Orientation(TopAbs_FORWARD);
    XBOP_WireEdgeSet aWES (myFace);
    
    const TColStd_IndexedMapOfInteger& aFFIndicesMap=aFFMap.FindFromIndex(i);
    aNbj=aFFIndicesMap.Extent();
    // 
    // b. The Switch: Same Domain Faces or Non-Same Domain Faces 
    bIsTouchCase=Standard_False;
    for (j=1; j<=aNbj; j++) {
      iFF=aFFIndicesMap(j);
      XBOPTools_SSInterference& aFF=aFFs(iFF);
      bIsTouchCase=aFF.IsTangentFaces();
      if (bIsTouchCase) {
	break;
      }
    }
    //
    // c. Filling the WES for nF1
    if (bIsTouchCase) { 
      // 1. Add Split Parts having states in accordance with operation
      AddSplitPartsINOUT (nF1, aWES);
      // 2. Add Section Edges to the WES 
      for (j=1; j<=aNbj; j++) {
	iFF=aFFIndicesMap(j);
	XBOPTools_SSInterference& aFF=aFFs(iFF);
	bIsTouch=aFF.IsTangentFaces();
	if (!bIsTouch) {
	  AddSectionPartsSo(nF1, iFF, aWES);
	}
      }
      // 3. Add IN2D, ON2D Parts to the WES 
      anEMap.Clear();// xft
      for (j=1; j<=aNbj; j++) {
	iFF=aFFIndicesMap(j);
	XBOPTools_SSInterference& aFF=aFFs(iFF);
	bIsTouch=aFF.IsTangentFaces();
	if (bIsTouch) {
	  //xf
	  //AddINON2DPartsSh(nF1, iFF, aWES);
	  AddINON2DPartsSh(nF1, iFF, aWES, anEMap);
	  //xt
	}
      }
      // 4. Add EF parts (E (from F2) on F1 ),
      // where F2 is non-same-domain face to F1
      //anEMap.Clear();//xft
      //
      // anEMap will contain all Split parts that has already in aWES
      const TopTools_ListOfShape& aLE=aWES.StartElements();
      TopTools_ListIteratorOfListOfShape anIt;
      anIt.Initialize (aLE);
      for (; anIt.More(); anIt.Next()) {
	TopoDS_Shape& anE=anIt.Value();
	anEMap.Add(anE);
      }
      //
      for (j=1; j<=aNbj; j++) {
	iFF=aFFIndicesMap(j);
	XBOPTools_SSInterference& aFF=aFFs(iFF);
	bIsTouch=aFF.IsTangentFaces();
	if (!bIsTouch) {
	  AddPartsEFNonSDSh (nF1, iFF, anEMap, aWES);
	  AddPartsEENonSDSh (nF1, iFF, anEMap, aWES);
	}
      }
      //
    }// end of  if (bIsTouchCase)
    else {
      // 1. Add Split Parts having states in accordance with operation
      AddSplitPartsINOUT (nF1, aWES);
      // 2. Add Split Parts with state ON
      AddSplitPartsONSo (nF1, aMEFObj, aMEFTool, aWES);
      // 3. Add Section Edges to the WES 
      for (j=1; j<=aNbj; j++) {
	iFF=aFFIndicesMap(j);
	AddSectionPartsSo(nF1, iFF, aWES);
      }
      // 4. Add EF parts (E (from F2) on F1 )
      anEMap.Clear();
      for (j=1; j<=aNbj; j++) {
	iFF=aFFIndicesMap(j);
	AddPartsEFSo(nF1, iFF, aMEFObj, aMEFTool, anEMap, aWES);
      }
      
    }// end of (bIsTouchCase)'s else
    //
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Display the WES
    if (myDraw)  {
      const TopTools_ListOfShape& aWESL=aWES.StartElements();
      XBOP_Draw::DrawListOfEdgesWithPC (myFace, aWESL, i, "ew_"); 
      XBOP_Draw::Wait();
    }
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //
    // d. Build new Faces from myFace
    XBOP_FaceBuilder aFB;
    aFB.SetTreatSDScales(1);
    aFB.SetTreatment(0); // 0-Do Internal Edges
    aFB.Do(aWES);

    const TopTools_ListOfShape& aLF=aFB.NewFaces();
    //
    // e. Do Internal Vertices
    DoInternalVertices(nF1, aLF);
    // 
    // f. Orient new faces
    TopTools_ListOfShape aLFx;
    TopTools_ListIteratorOfListOfShape anIt;
    anIt.Initialize(aLF);
    for (; anIt.More(); anIt.Next()) {
      TopoDS_Shape& aFx=anIt.Value();
      aFx.Orientation(anOriF1);
      aListOfNewFaces.Append(aFx);
      aLFx.Append(aFx);

      if(!myHistory.IsNull()) {
	Handle(XBOP_ShellSolidHistoryCollector) aHistory = 
	  Handle(XBOP_ShellSolidHistoryCollector)::DownCast(myHistory);

	if(!aHistory.IsNull()) {
	  aHistory->AddNewFace(aF1, aFx, myDSFiller);
	}
      }
    }
    //
    // Fill "Modified"
    FillModified(aF1, aLFx); 
    //
  }//  for (i=1; i<=aNb; i++)
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Display the new Faces
  if (myDraw) { 
    XBOP_Draw::DrawListOfShape(aListOfNewFaces, "fn_");
  }
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  myNewFaces.Clear();
  myNewFaces.Append(aListOfNewFaces);
}

//=======================================================================
// function: DetectSDFaces
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::DetectSDFaces()
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  XBOPTools_InterferencePool* pIntrPool=(XBOPTools_InterferencePool*)&myDSFiller->InterfPool();
  XBOPTools_CArray1OfSSInterference& aFFs=pIntrPool->SSInterferences();
  //
  Standard_Boolean bFlag;
  Standard_Integer i, aNb, nF1, nF2,  iZone, aNbSps, iSenseFlag;
  gp_Dir aDNF1, aDNF2;

  aNb=aFFs.Extent();
  for (i=1; i<=aNb; i++) {
    bFlag=Standard_False;
    
    XBOPTools_SSInterference& aFF=aFFs(i);
    
    nF1=aFF.Index1();
    nF2=aFF.Index2();
    const TopoDS_Face& aF1=TopoDS::Face(aDS.Shape(nF1));
    const TopoDS_Face& aF2=TopoDS::Face(aDS.Shape(nF2));
    //
    // iSenseFlag;
    const XBOPTools_ListOfPaveBlock& aLPB=aFF.PaveBlocks();
    aNbSps=aLPB.Extent();

    if (!aNbSps) {
      continue;
    }
    
    const XBOPTools_PaveBlock& aPB=aLPB.First();
    const TopoDS_Edge& aSpE=TopoDS::Edge(aDS.Shape(aPB.Edge()));
    
    XBOPTools_Tools3D::GetNormalToFaceOnEdge (aSpE, aF1, aDNF1); 
    XBOPTools_Tools3D::GetNormalToFaceOnEdge (aSpE, aF2, aDNF2);
    iSenseFlag=XBOPTools_Tools3D::SenseFlag (aDNF1, aDNF2);
    //
    if (iSenseFlag==1 || iSenseFlag==-1) {
    //
    //
      TopoDS_Face aF1FWD=aF1;
      aF1FWD.Orientation (TopAbs_FORWARD);
      
      XBOP_WireEdgeSet aWES (aF1FWD);
      XBOP_SDFWESFiller aWESFiller(nF1, nF2, *myDSFiller);
      aWESFiller.SetSenseFlag(iSenseFlag);
      aWESFiller.SetOperation(XBOP_COMMON);
      aWESFiller.Do(aWES);
      
      XBOP_FaceBuilder aFB;
      aFB.Do(aWES);
      const TopTools_ListOfShape& aLF=aFB.NewFaces();

      iZone=0;
      TopTools_ListIteratorOfListOfShape anIt(aLF);
      for (; anIt.More(); anIt.Next()) {
	const TopoDS_Shape& aFR=anIt.Value();

	if (aFR.ShapeType()==TopAbs_FACE) {
	  const TopoDS_Face& aFaceResult=TopoDS::Face(aFR);
	  //
	  Standard_Boolean bIsValidIn2D, bNegativeFlag;
	  bIsValidIn2D=XBOPTools_Tools3D::IsValidArea (aFaceResult, bNegativeFlag);
	  if (bIsValidIn2D) { 

	    if(CheckSameDomainFaceInside(aFaceResult, aF2)) {
	      iZone=1;
	      break;
	    }
	  }
	  //
	}
      }
      
      if (iZone) { 
	bFlag=Standard_True;
	aFF.SetStatesMap(aWESFiller.StatesMap());
      }
      
    }// if (iSenseFlag)
  
  aFF.SetTangentFacesFlag(bFlag);
  aFF.SetSenseFlag (iSenseFlag);
  }// end of for (i=1; i<=aNb; i++) 
} 

//=======================================================================
// function: AddSplitPartsINOUT
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::AddSplitPartsINOUT(const Standard_Integer nF1, 
					  XBOP_WireEdgeSet& aWES)
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  const XBOPTools_PaveFiller& aPaveFiller=myDSFiller->PaveFiller();
  const XBOPTools_SplitShapesPool& aSplitShapesPool=aPaveFiller.SplitShapesPool();

  Standard_Integer nE, nSp, iRankF1, aNbPB;
  XBooleanOperations_StateOfShape aState, aStateCmp;
  TopAbs_Orientation anOr;
  TopExp_Explorer anExp;
  TopoDS_Edge aSS;
  //
  iRankF1=aDS.Rank(nF1);
  aStateCmp=XBOP_BuilderTools::StateToCompare(iRankF1, myOperation);
  
  anExp.Init(myFace, TopAbs_EDGE);
  for (; anExp.More(); anExp.Next()) {
    const TopoDS_Shape& anE=anExp.Current();
    anOr=anE.Orientation();

    nE=aDS.ShapeIndex(anE, iRankF1);

    const XBOPTools_ListOfPaveBlock& aLPB=aSplitShapesPool(aDS.RefEdge(nE));
    aNbPB=aLPB.Extent();
    // case: No splits on Egde
    if (!aNbPB) {
      aState=aDS.GetState(nE);
      if (aState==aStateCmp) {
	aSS=TopoDS::Edge(anE);
	//modified by NIZNHY-PKV Mon Sep 19 09:13:59 2011f
	if (anOr==TopAbs_INTERNAL) {
	  aSS.Orientation(TopAbs_FORWARD);
	  aWES.AddStartElement (aSS);
	  aSS.Orientation(TopAbs_REVERSED);
	  aWES.AddStartElement (aSS);
	}
	else{
	  aSS.Orientation(anOr);
	  aWES.AddStartElement (aSS);
	}
	//aSS.Orientation(anOr);
	//aWES.AddStartElement (aSS);
	//modified by NIZNHY-PKV Mon Sep 19 09:14:02 2011t
      }
      continue;
    }
    // case: There are splits on Egde
    XBOPTools_ListIteratorOfListOfPaveBlock aPBIt(aLPB);
    for (; aPBIt.More(); aPBIt.Next()) {
      const XBOPTools_PaveBlock& aPB=aPBIt.Value();
      nSp=aPB.Edge();
      
      aState=aDS.GetState(nSp);
      if (aState==aStateCmp) {
	const TopoDS_Shape& aSplit=aDS.Shape(nSp);
	aSS=TopoDS::Edge(aSplit);
	//modified by NIZNHY-PKV Mon Sep 19 08:58:23 2011f
	if (anOr==TopAbs_INTERNAL) {
	  aSS.Orientation(TopAbs_FORWARD);
	  aWES.AddStartElement (aSS);
	  aSS.Orientation(TopAbs_REVERSED);
	  aWES.AddStartElement (aSS);
	}
	else{
	  aSS.Orientation(anOr);
	  aWES.AddStartElement (aSS);
	}
	//aSS.Orientation(anOr);
	//aWES.AddStartElement (aSS);
	//modified by NIZNHY-PKV Mon Sep 19 08:58:33 2011t
      }
    }
  }
}
//=======================================================================
// function: Prepare
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::Prepare() 
{
  //...
  const XBOPTools_PaveFiller& aPaveFiller=myDSFiller->PaveFiller();
  // 
  // 1 States
  XBOPTools_SolidStateFiller aStateFiller(aPaveFiller);
  aStateFiller.Do();
  //
  // 2 Project section edges on corresp. faces -> P-Curves on edges.
  XBOPTools_PCurveMaker aPCurveMaker(aPaveFiller);
  aPCurveMaker.Do();
  //
  // 3. Degenerated Edges Processing
  XBOPTools_DEProcessor aDEProcessor(aPaveFiller);
  aDEProcessor.Do();
  //
  // 4. DetectSame Domain Faces
  DetectSDFaces();
  //
  // 5. FillSectionEdges
  FillSectionEdges();
}

///////////////////
//=======================================================================
// function: OrientFacesOnShell
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::OrientFacesOnShell (const TopoDS_Shell& aShell, 
					   TopoDS_Shell& aShellNew)
{
  Standard_Boolean bIsProcessed1, bIsProcessed2;
  Standard_Integer i, aNbE, aNbF, j;
  TopAbs_Orientation anOrE1, anOrE2;

  TopTools_IndexedDataMapOfShapeListOfShape aEFMap;
  TopTools_IndexedMapOfShape aProcessedFaces;
  BRep_Builder aBB;

  aBB.MakeShell(aShellNew);
  
  TopExp::MapShapesAndAncestors(aShell, TopAbs_EDGE, TopAbs_FACE, aEFMap);
  
  aNbE=aEFMap.Extent();
  // 
  // One seam edge  in aEFMap contains  2 equivalent faces.
  for (i=1; i<=aNbE; i++) {
    TopTools_ListOfShape& aLF=aEFMap.ChangeFromIndex(i);

    if (aLF.Extent()>1) {
      TopTools_ListOfShape aLFTmp;
      TopTools_IndexedMapOfShape aFM;

      TopTools_ListIteratorOfListOfShape anIt(aLF);
      for (; anIt.More(); anIt.Next()) {
	const TopoDS_Shape& aF=anIt.Value();
	if (!aFM.Contains(aF)) {
	  aFM.Add(aF);
	  aLFTmp.Append(aF);
	}
      }
      aLF.Clear();
      aLF=aLFTmp;
    }
  }
  //
  // Do
  for (i=1; i<=aNbE; i++) {
    const TopoDS_Edge& aE=TopoDS::Edge(aEFMap.FindKey(i));

    if (BRep_Tool::Degenerated(aE)) {
      continue;
    }

    const TopTools_ListOfShape& aLF=aEFMap.FindFromIndex(i);
    aNbF=aLF.Extent();
    
    if (aNbF==2) {
      TopoDS_Face& aF1=TopoDS::Face(aLF.First());
      TopoDS_Face& aF2=TopoDS::Face(aLF.Last() );
      
      
      bIsProcessed1=aProcessedFaces.Contains(aF1);
      bIsProcessed2=aProcessedFaces.Contains(aF2);
     
      if (bIsProcessed1 && bIsProcessed2) {
	continue;
      }

      if (!bIsProcessed1 && !bIsProcessed2) {
	aProcessedFaces.Add(aF1);
	aBB.Add(aShellNew, aF1);

	bIsProcessed1=!bIsProcessed1;
      }

      //
      TopoDS_Face aF1x, aF2x;
      
      aF1x=aF1;
      if (bIsProcessed1) {
	j=aProcessedFaces.FindIndex(aF1);
	aF1x=TopoDS::Face(aProcessedFaces.FindKey(j));
      }
      
      aF2x=aF2;
      if (bIsProcessed2) {
	j=aProcessedFaces.FindIndex(aF2);
	aF2x=TopoDS::Face(aProcessedFaces.FindKey(j));
      }
      //

      anOrE1=Orientation(aE, aF1x); 
      anOrE2=Orientation(aE, aF2x);

      if (bIsProcessed1 && !bIsProcessed2) {
	
	if (anOrE1==anOrE2) {
	  if (!BRep_Tool::IsClosed(aE, aF1) &&
	      !BRep_Tool::IsClosed(aE, aF2)) {
	    aF2.Reverse();
	  }
	}
	aProcessedFaces.Add(aF2);
	aBB.Add(aShellNew, aF2);
      }
      
      else if (!bIsProcessed1 && bIsProcessed2) {
	if (anOrE1==anOrE2) {
	  if (!BRep_Tool::IsClosed(aE, aF1) &&
	      !BRep_Tool::IsClosed(aE, aF2)) {
	    aF1.Reverse();
	  }
	}
	aProcessedFaces.Add(aF1);
	aBB.Add(aShellNew, aF1);
      }
    }
  }
  //
  //
  for (i=1; i<=aNbE; i++) {
    const TopoDS_Edge& aE=TopoDS::Edge(aEFMap.FindKey(i));

    if (BRep_Tool::Degenerated(aE)) {
      continue;
    }

    const TopTools_ListOfShape& aLF=aEFMap.FindFromIndex(i);
    aNbF=aLF.Extent();
    if (aNbF!=2) {
      TopTools_ListIteratorOfListOfShape anIt(aLF);
      for(; anIt.More(); anIt.Next()) {
	const TopoDS_Face& aF=TopoDS::Face(anIt.Value());
	if (!aProcessedFaces.Contains(aF)) {
	  aProcessedFaces.Add(aF);
	  aBB.Add(aShellNew, aF);
	}
      }
    }
  }
}

//=======================================================================
//function : Orientation
//purpose  :
//=======================================================================
  TopAbs_Orientation XBOP_ShellSolid::Orientation(const TopoDS_Edge& anE,
						 const TopoDS_Face& aF)
{
  TopAbs_Orientation anOr=TopAbs_INTERNAL;

  TopExp_Explorer anExp;
  anExp.Init(aF, TopAbs_EDGE);
  for (; anExp.More(); anExp.Next()) {
    const TopoDS_Edge& anEF1=TopoDS::Edge(anExp.Current());
    if (anEF1.IsSame(anE)) {
      anOr=anEF1.Orientation();
      break;
    }
  }
  return anOr;
}

//=======================================================================
// function: Path
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::Path (const TopoDS_Shape& aE,
			     const TopTools_IndexedDataMapOfShapeListOfShape& aEFMap,
			     TopTools_IndexedMapOfShape& aFaces,
			     TopTools_IndexedMapOfShape& aProcessedEdges)
{
  if (!aProcessedEdges.Contains(aE)) {
    aProcessedEdges.Add(aE);

    Standard_Integer i, aNbE;
    const TopTools_ListOfShape& aFList=aEFMap.FindFromKey(aE);
    
    TopTools_ListIteratorOfListOfShape anIt(aFList);
    for(; anIt.More(); anIt.Next()) {
      const TopoDS_Shape& aF=anIt.Value();
      aFaces.Add(aF);
      
      TopTools_IndexedMapOfShape aME;
      TopExp::MapShapes(aF, TopAbs_EDGE, aME);
      aNbE=aME.Extent();
      for (i=1; i<=aNbE; i++) {
	const TopoDS_Shape& aENext=aME(i);
	Path(aENext, aEFMap, aFaces, aProcessedEdges);
      }
    }
  }
}

//=======================================================================
// function: CheckArgTypes
// purpose: 
//=======================================================================
Standard_Boolean XBOP_ShellSolid::CheckArgTypes(const TopAbs_ShapeEnum theType1,
					       const TopAbs_ShapeEnum theType2,
					       const XBOP_Operation theOperation) 
{
  Standard_Boolean bFlag=Standard_False;

  if (theType1==TopAbs_SHELL && theType2==TopAbs_SOLID) {
    if (theOperation==XBOP_FUSE || theOperation==XBOP_CUT21) {
      return bFlag;
    }
  }
  //
  if (theType1==TopAbs_SOLID && theType2==TopAbs_SHELL) {
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
Standard_Boolean XBOP_ShellSolid::CheckArgTypes() const
{
//   Standard_Boolean bFlag=Standard_False;
  
  TopAbs_ShapeEnum aT1, aT2;
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();

  aT1=aDS.Object().ShapeType();
  aT2=aDS.Tool().ShapeType();
  //
//   if (aT1==TopAbs_SHELL && aT2==TopAbs_SOLID) {
//     if (myOperation==XBOP_FUSE || myOperation==XBOP_CUT21) {
//       return bFlag;
//     }
//   }
//   //
//   if (aT1==TopAbs_SOLID && aT2==TopAbs_SHELL) {
//     if (myOperation==XBOP_FUSE || myOperation==XBOP_CUT) {
//       return bFlag;
//     }
//   }
  //
//   return !bFlag;
  return CheckArgTypes(aT1, aT2, myOperation);
}



//=======================================================================
// function: DoInternalVertices
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::DoInternalVertices(const Standard_Integer nF1,
					  const TopTools_ListOfShape& aListOfFaces)
{
  
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  XBOPTools_InterferencePool* pIntrPool=(XBOPTools_InterferencePool*)&myDSFiller->InterfPool();
  XBOPTools_CArray1OfVSInterference& aVFs=pIntrPool->VSInterferences();
  XBOPTools_CArray1OfESInterference& aEFs=pIntrPool->ESInterferences();
  XBOPTools_CArray1OfSSInterference& aFFs=pIntrPool->SSInterferences();
  
  XBOPTools_PaveFiller* pPaveFiller=(XBOPTools_PaveFiller*) &(myDSFiller->PaveFiller());
  //
  Standard_Integer i, j, nF, aNbFF, aNbVF, aWhat, aWith; 
  Standard_Integer nV, iRankF, nE, aNbE, nFx, iOppositeRank, iFlag, iBreakFlag ;
  Standard_Integer aNbEF, nVx1, nVx2, aWh, aWt, aNbC, k;
  Standard_Real U, V;
  //
  
  nF=nF1;
  iRankF=aDS.Rank(nF);
  iOppositeRank=(iRankF==1) ?  2 : 1;
  TopTools_IndexedDataMapOfShapeListOfShape aVEMap, aVFMap, aEFMap;
  const TopoDS_Shape& aS=(iRankF==1) ? myShape2 : myShape1;
  TopExp::MapShapesAndAncestors (aS, TopAbs_VERTEX, TopAbs_EDGE, aVEMap);
  TopExp::MapShapesAndAncestors (aS, TopAbs_VERTEX, TopAbs_FACE, aVFMap);
  TopExp::MapShapesAndAncestors (aS, TopAbs_EDGE  , TopAbs_FACE, aEFMap);
  
  TopTools_ListIteratorOfListOfShape anIt;
  ///////////////////////////////////////////////////////////////
  //
  // VF interferences processing
  aNbVF=aVFs.Extent();
  for (i=1; i<=aNbVF; i++) {
    iBreakFlag=0;
 
    XBOPTools_VSInterference& aVF=aVFs(i);
    
    aWhat=aVF.Index1();
    aWith=aVF.Index2();

    if (aWhat==nF || aWith==nF) {
      TColStd_ListOfInteger aLs;

      nV=aVF.OppositeIndex(nF);

      Standard_Integer iRankV;
      iRankV=aDS.Rank(nV);

      const TopoDS_Vertex& aV=TopoDS::Vertex(aDS.Shape(nV));

      if (aVEMap.Contains(aV)) {
	const TopTools_ListOfShape& aLE=aVEMap.FindFromKey(aV);
	
	anIt.Initialize (aLE);
	for (; anIt.More() && !iBreakFlag; anIt.Next()) {
	  const TopoDS_Shape& aE=anIt.Value();

	  nE=aDS.ShapeIndex(aE, iRankV);
	  //
	  aLs.Clear();
	  //
	  pPaveFiller->SplitsInFace(nE, nF, aLs);
	  aNbE=aLs.Extent();
	  if (aNbE) {
	    iBreakFlag=1; // next i
	    //
	    break;
	    //
	  }
	}
      }
      if (iBreakFlag) {
	continue;
      }
      // 
      if (aVFMap.Contains(aV)) {
	const TopTools_ListOfShape& aLF=aVFMap.FindFromKey(aV);
	
	anIt.Initialize (aLF);
	for (; anIt.More()&& !iBreakFlag; anIt.Next()) {
	  const TopoDS_Shape& aFx=anIt.Value();

	  nFx=aDS.ShapeIndex(aFx, iRankV);

	  aNbFF=aFFs.Extent();
	  for (j=1; j<=aNbFF && !iBreakFlag ; j++) {
	    XBOPTools_SSInterference& aFF=aFFs(j);
	    
	    aWh=aFF.Index1();
	    aWt=aFF.Index2();
	    if ((aWh==nF && aWt==nFx) || (aWh==nFx && aWt==nF)) {
	      XBOPTools_SequenceOfCurves& aSCurves=aFF.Curves();
	      aNbC=aSCurves.Length();
	      for (k=1; k<=aNbC && !iBreakFlag; k++) {
		const XBOPTools_Curve& aC=aSCurves(k);
		const XBOPTools_ListOfPaveBlock& aLPB=aC.NewPaveBlocks();
		XBOPTools_ListIteratorOfListOfPaveBlock anItLPB;
		anItLPB.Initialize (aLPB);
		for (; anItLPB.More() && !iBreakFlag; anItLPB.Next()) {
		  const XBOPTools_PaveBlock& aPB=anItLPB.Value();
		  //
		  nVx1=aPB.Pave1().Index();
		  nVx2=aPB.Pave1().Index();
		  if ((nVx1==nV) || (nVx2==nV)) {
		    iBreakFlag=1;
		    //
		    break;
		    //
		  }
		}
	      }
	    }
	  }
	}
      }
      //
      if (iBreakFlag) {
	continue;
      }
      //
      // Put Vertex nV  on Face j from aListOfFaces
      Standard_Boolean bIsPointInFace;
      
      aVF.UV (U, V);
      gp_Pnt2d aP2D (U, V);
      
      TopoDS_Vertex aVInternal=aV;
      aVInternal.Orientation(TopAbs_INTERNAL);

      anIt.Initialize(aListOfFaces);
      for (; anIt.More(); anIt.Next()) {
	TopoDS_Face& aF=TopoDS::Face(anIt.Value());
	//
	XIntTools_Context aCtx;
	bIsPointInFace=aCtx.IsPointInFace(aF, aP2D);
	//
	if (bIsPointInFace) {
	  BRep_Builder aBB;
	  aBB.Add(aF, aVInternal);
	  break;
	}
      }
    }// if (aWhat==nF || aWith==nF)
  }// for (i=1; i<=aNbVF; i++)
  ///////////////////////////////////////////////////////////////
  //
  // EF interferences processing
  aNbEF=aEFs.Extent();
  
  for (i=1; i<=aNbEF; i++) {
    iBreakFlag=0;
    XBOPTools_ESInterference& aEF=aEFs(i);
    aWhat=aEF.Index1();
    aWith=aEF.Index2();
    //
    if (aWhat!=nF) {
      if (aWith!=nF) {
	continue;
      }
    }
    //
    nE=aEF.OppositeIndex(nF);
    
    const TopoDS_Edge& aE=TopoDS::Edge(aDS.Shape(nE));
    
    nV=aEF.NewShape();
    //
    if(!nV){
      continue;
    }
    //
    const TopoDS_Shape& aV=aDS.Shape(nV);
    //
    if (aV.ShapeType()!=TopAbs_VERTEX) {
      continue;
    }
    //
    XBOPTools_ListOfPaveBlock aLPBSplits;
    //
    // check split parts in2D for F1
    pPaveFiller->SplitsInFace(nE, nF, aLPBSplits);
    
    XBOPTools_ListIteratorOfListOfPaveBlock anItx(aLPBSplits);
    for (; anItx.More() && !iBreakFlag; anItx.Next()) {
      const XBOPTools_PaveBlock& aPB=anItx.Value();
      nVx1=aPB.Pave1().Index();
      nVx2=aPB.Pave2().Index();
      if (nV==nVx1 || nV==nVx2) {
	iBreakFlag=1;
	break;
      }
    }
    
    if (iBreakFlag){
      continue;
    }
    //
    // 
    if (aEFMap.Contains(aE)) {
      const TopTools_ListOfShape& aLF=aEFMap.FindFromKey(aE);
      
      anIt.Initialize (aLF);
      for (; anIt.More() && !iBreakFlag; anIt.Next()) {
	const TopoDS_Shape& aFx=anIt.Value();
	nFx=aDS.ShapeIndex(aFx, iOppositeRank);
	
	aNbFF=aFFs.Extent();
	for (j=1; j<=aNbFF&& !iBreakFlag; j++) {
	  XBOPTools_SSInterference& aFF=aFFs(j);
	  
	  aWh=aFF.Index1();
	  aWt=aFF.Index2();
	  if ((aWh==nF && aWt==nFx) || (aWt==nF && aWh==nFx)) {
	    XBOPTools_SequenceOfCurves& aSCurves=aFF.Curves();
	    aNbC=aSCurves.Length();
	    for (k=1; k<=aNbC && !iBreakFlag; k++) {
	      const XBOPTools_Curve& aC=aSCurves(k);
	      const XBOPTools_ListOfPaveBlock& aLPB=aC.NewPaveBlocks();
	      XBOPTools_ListIteratorOfListOfPaveBlock anItLPB;
	      anItLPB.Initialize (aLPB);
	      for (; anItLPB.More()&& !iBreakFlag; anItLPB.Next()) {
		const XBOPTools_PaveBlock& aPB=anItLPB.Value();
		//
		nVx1=aPB.Pave1().Index();
		nVx2=aPB.Pave2().Index();
		if (nV==nVx1 || nV==nVx2) {
		  iBreakFlag=1;
		}
	      }
	    }
	  }
	}
      }
      if (iBreakFlag){
	continue;
      }
    }
    //
    // Put Vertex nV  on Face j from aListOfFaces
    TopoDS_Vertex aVInternal=TopoDS::Vertex(aV);
    aVInternal.Orientation(TopAbs_INTERNAL);
    TopTools_ListIteratorOfListOfShape anItF;
    anItF.Initialize(aListOfFaces);
    for (; anItF.More(); anItF.Next()) {
      TopoDS_Face& aFx=TopoDS::Face(anItF.Value());
      //
      XIntTools_Context aCtx;
      iFlag=aCtx.ComputeVS(aVInternal, aFx, U, V);
      //
      if (!iFlag) {
	BRep_Builder aBB;
	aBB.Add(aFx, aVInternal);
	break;
      }
    }
  }
  //
  // Alone Vertices' processing 
  //
  Standard_Integer  aNbVx,  nVx;
  aNbFF=aFFs.Extent();
  for (i=1; i<=aNbFF; i++) {
    XBOPTools_SSInterference& aFF=aFFs(i);
    aWh=aFF.Index1();
    aWt=aFF.Index2();
    if (aWh==nF || aWt==nF) {
      TColStd_ListOfInteger& anAloneVertices=aFF.AloneVertices();
      aNbVx=anAloneVertices.Extent();
      if (aNbVx) {
	TColStd_ListIteratorOfListOfInteger anItVx(anAloneVertices);
	for (; anItVx.More(); anItVx.Next()) {
	  nVx=anItVx.Value();
	  const TopoDS_Vertex& aVx=TopoDS::Vertex(aDS.Shape(nVx));
	  //
	  TopoDS_Vertex aVInternal=aVx;
	  aVInternal.Orientation(TopAbs_INTERNAL);
	  //
	  anIt.Initialize(aListOfFaces);
	  for (; anIt.More(); anIt.Next()) {
	    TopoDS_Face& aF=TopoDS::Face(anIt.Value());
	    //
	    XIntTools_Context aCtx;
	    iFlag=aCtx.ComputeVS (aVInternal, aF, U, V);
	    //
	    if (!iFlag) {
	      BRep_Builder aBB;
	      aBB.Add(aF, aVInternal);
	    }
	  }
	  
	}
      }
    }
  }
} 

//=======================================================================
// function: CollectInternals
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::CollectInternals()
{
  Standard_Integer i, aNb, nSp;
  TopAbs_ShapeEnum aType;
  TopAbs_Orientation anOr;
  TopTools_IndexedMapOfShape aMInternals;

  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  const XBOPTools_SplitShapesPool& aSplitShapesPool=myDSFiller->SplitShapesPool();

  aNb=aDS.NumberOfShapesOfTheObject()+aDS.NumberOfShapesOfTheTool();
  for (i=1; i<= aNb; ++i) {
    const TopoDS_Shape& aS=aDS.Shape(i);
    aType=aS.ShapeType();
    if (aType==TopAbs_EDGE || aType==TopAbs_VERTEX) {
      anOr=aS.Orientation();
      if (anOr==TopAbs_INTERNAL) {
	//
	aMInternals.Add(aS);
	//
	if (aType==TopAbs_EDGE) {
	  
	  const XBOPTools_ListOfPaveBlock& aSplits=aSplitShapesPool(aDS.RefEdge(i));
	  XBOPTools_ListIteratorOfListOfPaveBlock aPBIt(aSplits);
	  for (; aPBIt.More(); aPBIt.Next()) {
	    XBOPTools_PaveBlock& aPB=aPBIt.Value();
	    nSp=aPB.Edge();
	    const TopoDS_Shape& aSp=aDS.Shape(nSp);
	    //
	    aMInternals.Add(aSp);
	  }
	}
      }
    }
  }

  aNb=aMInternals.Extent();
  for (i=1; i<= aNb; ++i) {
    const TopoDS_Shape& aS=aMInternals(i);
    myInternals.Append(aS);
  }
}
//=======================================================================
// function: Internals
// purpose: 
//=======================================================================
  const TopTools_ListOfShape& XBOP_ShellSolid::Internals()const
{
  return myInternals;
}
//=======================================================================
// function: FillSectionEdges
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::FillSectionEdges()
{
  Standard_Integer i, j, nF1, nF2,  aNbFFs, aNbS, aNbCurves, nSect;
  //
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  const XBOPTools_InterferencePool& anInterfPool=myDSFiller->InterfPool();
  XBOPTools_InterferencePool* pInterfPool=(XBOPTools_InterferencePool*) &anInterfPool;
  XBOPTools_CArray1OfSSInterference& aFFs=pInterfPool->SSInterferences();
  //
  TopTools_IndexedMapOfShape aMap;
  //
  aNbFFs=aFFs.Extent();
  for (i=1; i<=aNbFFs; ++i) {
    XBOPTools_SSInterference& aFFi=aFFs(i);
    //
    nF1=aFFi.Index1();
    nF2=aFFi.Index2();
    //
    // Old Section Edges
    const XBOPTools_ListOfPaveBlock& aSectList=aFFi.PaveBlocks();
    aNbS=aSectList.Extent();
    XBOPTools_ListIteratorOfListOfPaveBlock anIt(aSectList);
    for (; anIt.More();anIt.Next()) {
      const XBOPTools_PaveBlock& aPB=anIt.Value();
      nSect=aPB.Edge();
      const TopoDS_Shape& aS=aDS.GetShape(nSect);
      aMap.Add(aS);
    }
    //
    // New Section Edges
    XBOPTools_SequenceOfCurves& aBCurves=aFFi.Curves();
    aNbCurves=aBCurves.Length();
    for (j=1; j<=aNbCurves; j++) {
      XBOPTools_Curve& aBC=aBCurves(j);
      const XBOPTools_ListOfPaveBlock& aSectEdges=aBC.NewPaveBlocks();
      aNbS=aSectEdges.Extent();
      
      XBOPTools_ListIteratorOfListOfPaveBlock aPBIt(aSectEdges);
      for (; aPBIt.More(); aPBIt.Next()) {
	XBOPTools_PaveBlock& aPB=aPBIt.Value();
	nSect=aPB.Edge();
	const TopoDS_Shape& aS=aDS.GetShape(nSect);
	aMap.Add(aS);
      }
    }
  }
  // 
  mySectionEdges.Clear();

  aNbS=aMap.Extent();
  
  for (i=1; i<=aNbS; i++) {
    const TopoDS_Shape& aS=aMap(i);
    mySectionEdges.Append(aS);
  }
}

void XBOP_ShellSolid::SetHistoryCollector(const Handle(XBOP_HistoryCollector)& theHistory) 
{
  if(theHistory.IsNull() ||
     !theHistory->IsKind(STANDARD_TYPE(XBOP_ShellSolidHistoryCollector)))
    myHistory.Nullify();
  else 
    myHistory = theHistory;
}

// -----------------------------------------------------------------
// static function: CheckSameDomainFaceInside
// purpose: Check if distance between several points of theFace1 and
//          theFace2 is not more than sum of maximum of tolerances of
//          theFace1's edges and tolerance of theFace2
// -----------------------------------------------------------------
Standard_Boolean CheckSameDomainFaceInside(const TopoDS_Face& theFace1,
					   const TopoDS_Face& theFace2) {

  Standard_Real umin = 0., umax = 0., vmin = 0., vmax = 0.;
  BRepTools::UVBounds(theFace1, umin, umax, vmin, vmax);
  XIntTools_Context aContext;
  Handle(Geom_Surface) aSurface = BRep_Tool::Surface(theFace1);
  Standard_Real aTolerance = BRep_Tool::Tolerance(theFace1);

  TopExp_Explorer anExpE(theFace1, TopAbs_EDGE);

  for(; anExpE.More(); anExpE.Next()) {
    const TopoDS_Edge& anEdge = TopoDS::Edge(anExpE.Current());
    Standard_Real anEdgeTol = BRep_Tool::Tolerance(anEdge);
    aTolerance = (aTolerance < anEdgeTol) ? anEdgeTol : aTolerance;
  }
  aTolerance += BRep_Tool::Tolerance(theFace2);

  Standard_Integer nbpoints = 5;
  Standard_Real adeltau = (umax - umin) / (nbpoints + 1);
  Standard_Real adeltav = (vmax - vmin) / (nbpoints + 1);
  Standard_Real U = umin + adeltau;
  GeomAPI_ProjectPointOnSurf& aProjector = aContext.ProjPS(theFace2);

  for(Standard_Integer i = 1; i <= nbpoints; i++, U+=adeltau) {
    Standard_Real V = vmin + adeltav;

    for(Standard_Integer j = 1; j <= nbpoints; j++, V+=adeltav) {
      gp_Pnt2d aPoint(U,V);

      if(aContext.IsPointInFace(theFace1, aPoint)) {
	gp_Pnt aP3d = aSurface->Value(U, V);
	aProjector.Perform(aP3d);

	if(aProjector.IsDone()) {

	  if(aProjector.LowerDistance() > aTolerance)
	    return Standard_False;
	}
      }
    }
  }

  return Standard_True;
}

