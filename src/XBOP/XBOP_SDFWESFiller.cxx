// Created on: 2001-06-06
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


#include <XBOP_SDFWESFiller.ixx>

#include <gp_Pnt.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>

#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfOrientedShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopAbs_Orientation.hxx>

#include <BRep_Tool.hxx>

#include <TColStd_ListOfInteger.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>
#include <TColStd_ListIteratorOfListOfInteger.hxx>

#include <XBooleanOperations_ShapesDataStructure.hxx>
#include <XBooleanOperations_StateOfShape.hxx>

#include <XIntTools_Tools.hxx>
#include <XIntTools_Context.hxx>

#include <XBOPTools_Tools2D.hxx>
#include <XBOPTools_Tools3D.hxx>
#include <XBOPTools_ListOfPaveBlock.hxx>
#include <XBOPTools_ListIteratorOfListOfPaveBlock.hxx>
#include <XBOPTools_PaveBlock.hxx>
#include <XBOPTools_PaveFiller.hxx>
#include <XBOPTools_SplitShapesPool.hxx>
#include <XBOPTools_CommonBlockPool.hxx>
#include <XBOPTools_ListOfCommonBlock.hxx>
#include <XBOPTools_ListIteratorOfListOfCommonBlock.hxx>
#include <XBOPTools_CommonBlock.hxx>
#include <XBOPTools_Tools.hxx>
#include <XBOPTools_PointBetween.hxx>
#include <XBOPTools_SolidStateFiller.hxx>

#include <XBOPTColStd_Dump.hxx>
#include <TopTools_MapOfShape.hxx>
static Standard_Boolean IsValidSeam(const TopoDS_Edge& aE,
				    const TopoDS_Face& aF,
				    const Standard_Real aT,
				    const Handle(XIntTools_Context)& aContext);

static void CorrespondantSeam(const TopoDS_Edge& aSpE1Seam11,
			      const Standard_Real aT1,
			      const TopoDS_Edge& aSpE1Seam21,
			      const TopoDS_Edge& aSpE1Seam22,
			      const Standard_Real aT2,
			      const TopoDS_Face& aF1FWD,
			      TopoDS_Edge& aSS);

static void TreatSDSeams (const TopoDS_Edge& aSpE1Seam11,
			  const Standard_Real aT1,
			  const TopoDS_Face& aF1FWD,
			  const TopoDS_Edge& aSpE1Seam21,
			  const Standard_Real aT2,
			  const TopoDS_Face& aF2FWD,
			  const Standard_Boolean bIsTakenSp1,
			  XBOP_WireEdgeSet& aWES,
			  const Handle(XIntTools_Context)& aContext);

//modified by NIZNHY-PKV Tue Sep 25 14:26:14 2012f
static 
  Standard_Boolean IsClosed(const TopoDS_Edge& aE,
			    const TopoDS_Face& aF);
//modified by NIZNHY-PKV Tue Sep 25 14:26:17 2012t

//=======================================================================
// function: XBOP_SDFWESFiller::XBOP_SDFWESFiller
// purpose: 
//=======================================================================
  XBOP_SDFWESFiller::XBOP_SDFWESFiller()
:
  myDSFiller(NULL),
  myOperation(XBOP_UNKNOWN),
  myNF1(0),
  myNF2(0),
  mySenseFlag(0)
{}

//=======================================================================
// function: XBOP_SDFWESFiller::XBOP_SDFWESFiller
// purpose: 
//=======================================================================
  XBOP_SDFWESFiller::XBOP_SDFWESFiller(const Standard_Integer nF1,
				     const Standard_Integer nF2,
				     const XBOPTools_DSFiller& aDSFiller)
:
  myNF1(nF1),
  myNF2(nF2),
  mySenseFlag(0)
{
  myDSFiller=(XBOPTools_DSFiller*) &aDSFiller;
  Prepare();
}
//=======================================================================
// function: SetStatesMap
// purpose: 
//=======================================================================
  void XBOP_SDFWESFiller::SetStatesMap (const XBOPTools_IndexedDataMapOfIntegerState& aMap)
{
  myStatesMap.Clear();
  myStatesMap=aMap;
}
//=======================================================================
// function: StatesMap
// purpose: 
//=======================================================================
  const XBOPTools_IndexedDataMapOfIntegerState& XBOP_SDFWESFiller::StatesMap ()const
{
  return myStatesMap;
}

//=======================================================================
// function: SetFaces
// purpose: 
//=======================================================================
  void XBOP_SDFWESFiller::SetFaces (const Standard_Integer nF1,
				   const Standard_Integer nF2)
{
   myNF1=nF1;
   myNF2=nF2;
}
//=======================================================================
// function: SetSenseFlag
// purpose: 
//=======================================================================
  void XBOP_SDFWESFiller::SetSenseFlag (const Standard_Integer iFlag)
				 
{
   mySenseFlag=iFlag;
}

//=======================================================================
// function: SenseFlag
// purpose: 
//=======================================================================
  Standard_Integer XBOP_SDFWESFiller::SenseFlag () const
				 
{
   return mySenseFlag;
}

//=======================================================================
// function: Faces
// purpose: 
//=======================================================================
  void XBOP_SDFWESFiller::Faces (Standard_Integer& nF1,
				Standard_Integer& nF2) const
{
   nF1=myNF1;
   nF2=myNF2;
}
//=======================================================================
// function: SetDSFiller
// purpose: 
//=======================================================================
  void XBOP_SDFWESFiller::SetDSFiller(const XBOPTools_DSFiller& aDSFiller)
{
   myDSFiller=(XBOPTools_DSFiller*) &aDSFiller;
}

//=======================================================================
// function: DSFiller
// purpose: 
//=======================================================================
  const XBOPTools_DSFiller& XBOP_SDFWESFiller::DSFiller()const
{
  return *myDSFiller;
}

//=======================================================================
// function: SetOperation
// purpose: 
//=======================================================================
  void XBOP_SDFWESFiller::SetOperation(const XBOP_Operation anOp)
{
  myOperation=anOp;
}
//=======================================================================
// function: Operation
// purpose: 
//=======================================================================
  XBOP_Operation XBOP_SDFWESFiller::Operation()const
{
  return myOperation;
}
//xf
//=======================================================================
//function : RejectedOnParts
//purpose  : 
//=======================================================================
  const TopTools_ListOfShape& XBOP_SDFWESFiller::RejectedOnParts()const
{
  return myRejectedOnParts;
}
//xt
//=======================================================================
// function: Prepare
// purpose: 
//======================================================================= 
  void XBOP_SDFWESFiller::Prepare()
{
  if (!myNF1 || !myNF2) {
    return;
  }
  //
  // 1. Prepare States 2D for the Faces' entities  (myStatesMap)
  AssignStates(myNF1, myNF2);
  AssignStates(myNF2, myNF1);
  //
  AssignDEStates(myNF1, myNF2);
  AssignDEStates(myNF2, myNF1);
  //
  //  
  // 2.
  PrepareOnParts();
  //
}

//=======================================================================
// function: Do
// purpose: 
//======================================================================= 
  void XBOP_SDFWESFiller::Do(const XBOP_WireEdgeSet& pWES)
{
  
  myWES=(XBOP_WireEdgeSet*) &pWES;

  if (!myNF1 || !myNF2) {
    return;
  }
  
  //
  // WES
  switch (myOperation) {
    case XBOP_COMMON: {
      PrepareWESForZone (myNF1, myNF2);
      break;
    }
    case XBOP_CUT: {
      PrepareWESForCut (myNF1, myNF2);
      break;
    }
    case XBOP_CUT21: {
      PrepareWESForCut (myNF2, myNF1);
      break;
    }
    default: {
      return;
    }
  }
}

//=======================================================================
// function: AssignStates
// purpose: 
//======================================================================= 
  void XBOP_SDFWESFiller::AssignStates(const Standard_Integer nF1,
				      const Standard_Integer nF2) 
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  const XBOPTools_PaveFiller& aPaveFiller=myDSFiller->PaveFiller();
  XBOPTools_PaveFiller* pPaveFiller=(XBOPTools_PaveFiller*) &aPaveFiller;
  const XBOPTools_SplitShapesPool& aSplitShapesPool=aPaveFiller.SplitShapesPool();
  //
  Standard_Integer aBid=0, nSplit, nE, nW, nSp, anEdgeFlag, aWireFlag, aNbPB, iRankF1;
  TColStd_ListOfInteger aList1IN2, aList1ON2;
  TColStd_IndexedMapOfInteger aSpMapIN, aSpMapON;
  TColStd_ListIteratorOfListOfInteger anIt;
  //
  iRankF1=aDS.Rank(nF1);
  //
  const TopoDS_Face& aF1=TopoDS::Face(aDS.Shape(nF1));
  // 
  //  Splits that are from nF2 and are IN2D for nF2
  pPaveFiller->SplitsInFace (aBid, nF1, nF2, aList1IN2);
  
  anIt.Initialize(aList1IN2);
  for (; anIt.More();  anIt.Next()) {
    nSplit=anIt.Value();
    aSpMapIN.Add(nSplit);
  }
  //
  // that are from nF2 and are ON2D for nF2
  pPaveFiller->SplitsOnFace (aBid, nF1, nF2, aList1ON2);
  
  anIt.Initialize(aList1ON2);
  for (; anIt.More();  anIt.Next()) {
    nSplit=anIt.Value();
    aSpMapON.Add(nSplit);
  }
  //
  // Treatment of the Face's entities
  aWireFlag=1;
  TopExp_Explorer anExpWire(aF1, TopAbs_WIRE);
  for (; anExpWire.More(); anExpWire.Next()) {
    const TopoDS_Shape& aWire=anExpWire.Current();
    nW=aDS.ShapeIndex(aWire, iRankF1);
    anEdgeFlag=1;

    TopExp_Explorer anExpEdge (aWire, TopAbs_EDGE);
    for (; anExpEdge.More(); anExpEdge.Next()) {
      const TopoDS_Shape& anEdge=anExpEdge.Current();
      nE=aDS.ShapeIndex(anEdge, iRankF1);
      
      const XBOPTools_ListOfPaveBlock& aLPB=aSplitShapesPool(aDS.RefEdge(nE));

      aNbPB=aLPB.Extent();
      if (!aNbPB) {
	// the whole edge is OUT
	myStatesMap.Add(nE, XBooleanOperations_OUT);
	continue;
      }

      XBOPTools_ListIteratorOfListOfPaveBlock aPBIt(aLPB);

      for (;  aPBIt.More();  aPBIt.Next()) {
	const XBOPTools_PaveBlock& aPB=aPBIt.Value();
	nSp=aPB.Edge();
	
	if (aSpMapIN.Contains(nSp)) {// IN
	  myStatesMap.Add(nSp, XBooleanOperations_IN);
	  anEdgeFlag=0;
	}
	else if (aSpMapON.Contains(nSp)) {// ON
	  myStatesMap.Add(nSp, XBooleanOperations_ON);
	  anEdgeFlag=0;
	}
	else {// if (nSp!=nE) {// OUT
	  myStatesMap.Add(nSp, XBooleanOperations_OUT);
	}
      } 
    } //  enf of for (; anExpEdge.More(); anExpEdge.Next()) 
     
    if (anEdgeFlag) {// all Wire is out
      myStatesMap.Add(nW, XBooleanOperations_OUT);
    }
    else {
      aWireFlag=0;
    }
  } // enf of for (; anExpEdge.More(); anExpEdge.Next())
  
  if (aWireFlag) { // all Face is out of nF2
    myStatesMap.Add(nF1, XBooleanOperations_OUT);
  }
}

//=======================================================================
// function: PrepareOnParts
// purpose: 
//=======================================================================
  void XBOP_SDFWESFiller::PrepareOnParts ()
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  const XBOPTools_PaveFiller& aPaveFiller=myDSFiller->PaveFiller();
  XBOPTools_PaveFiller* pPaveFiller=(XBOPTools_PaveFiller*) &aPaveFiller;
  XBOPTools_CommonBlockPool& aCBPool=pPaveFiller->ChangeCommonBlockPool();
  //
  Standard_Integer aBid=0, nE1, nE2, aNbSpON, nSp1, nSp2, iRankF1;
  Standard_Real aT1, aT2, aT, aTs; /*, U, V;*/ 
  Standard_Boolean aFlag;
  TColStd_ListOfInteger aLs;
  TColStd_IndexedMapOfInteger aMap;
  TopExp_Explorer anExpF1, anExpF2;
  //
  iRankF1=aDS.Rank(myNF1);

  gp_Pnt aPx1;
  //
  TopoDS_Face aF1FWD, aF2FWD;
  PrepareFaces(myNF1, myNF2, aF1FWD, aF2FWD);
  //
  // Process 
  anExpF1.Init(aF1FWD, TopAbs_EDGE);
  for (; anExpF1.More(); anExpF1.Next()) {
    const TopoDS_Edge& anE1=TopoDS::Edge(anExpF1.Current());
    //
    if (BRep_Tool::Degenerated(anE1)){
      continue;
    }
    //
    nE1=aDS.ShapeIndex(anE1, iRankF1);

    aLs.Clear();
    pPaveFiller->SplitsOnFace(nE1, myNF2, aLs);
    
    aNbSpON=aLs.Extent();
    if (!aNbSpON) {
      continue;
    }

    aMap.Clear();
    TColStd_ListIteratorOfListOfInteger anItLs(aLs);
    for (; anItLs.More(); anItLs.Next()) {
      aBid=anItLs.Value();
      aMap.Add(aBid);
    }

    XBOPTools_ListOfCommonBlock& aLCB=aCBPool(aDS.RefEdge(nE1));
    XBOPTools_ListIteratorOfListOfCommonBlock anItCB(aLCB);
    for (; anItCB.More(); anItCB.Next()) {
      XBOPTools_CommonBlock& aCB=anItCB.Value();
      XBOPTools_PaveBlock& aPB1=aCB.PaveBlock1(nE1);
      nSp1=aPB1.Edge();
      if (aMap.Contains(nSp1)) {
	//
	// aPB1
	aPB1.Parameters(aT1, aT2);
	aT=XBOPTools_Tools2D::IntermediatePoint(aT1, aT2);
	XBOPTools_Tools::PointOnEdge(anE1, aT, aPx1);

	XBOPTools_PointBetween aPointBetween;
	aPointBetween.SetParameter(aT);
	aPointBetween.SetPnt(aPx1);
	
	aPB1.SetPointBetween(aPointBetween);
	//
	// aPB2
	XBOPTools_PaveBlock& aPB2=aCB.PaveBlock2(nE1);
	nE2=aPB2.OriginalEdge();
	nSp2=aPB2.Edge();
	const TopoDS_Edge& anE2=TopoDS::Edge(aDS.GetShape(nE2));
	//
	const Handle(XIntTools_Context)& aContext=pPaveFiller->Context();
	aFlag=aContext->ProjectPointOnEdge(aPx1, anE2, aTs);
	//
	if (!aFlag) {
	  XBOPTColStd_Dump::PrintMessage(" XBOP_SDFWESFiller::PrepareOnParts() failed\n");
	  return;
	}

	aPointBetween.SetParameter(aTs);
	aPointBetween.SetPnt(aPx1);
	
	aPB2.SetPointBetween(aPointBetween);
	//
	XBOPTools_ListOfCommonBlock& aLCB2=aCBPool(aDS.RefEdge(nE2));
	XBOPTools_ListIteratorOfListOfCommonBlock anItCB2(aLCB2);
	for (; anItCB2.More(); anItCB2.Next()){
	  XBOPTools_CommonBlock& aCB2=anItCB2.Value();
	  XBOPTools_PaveBlock& aPB21=aCB2.PaveBlock1(nE2);
	  XBOPTools_PaveBlock& aPB22=aCB2.PaveBlock2(nE2);
	  
	  if ((aPB21.IsEqual(aPB1) && aPB22.IsEqual(aPB2)) 
	      ||
	      (aPB21.IsEqual(aPB2) && aPB22.IsEqual(aPB1))) {
	    
	    aPointBetween.SetPnt(aPx1);
	    
	    aPointBetween.SetParameter(aTs);
	    aPB21.SetPointBetween(aPointBetween);
	    
	    aPointBetween.SetParameter(aT);
	    aPB22.SetPointBetween(aPointBetween);
	    
	    break;
	  }
	}
	//
      }
    }
  }
}

//=======================================================================
// function: PrepareWESForZone
// purpose: 
//=======================================================================
  void XBOP_SDFWESFiller::PrepareWESForZone (const Standard_Integer nF1, 
					    const Standard_Integer nF2)
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  const XBOPTools_PaveFiller& aPaveFiller=myDSFiller->PaveFiller();
  const XBOPTools_SplitShapesPool& aSplitShapesPool=aPaveFiller.SplitShapesPool();
  XBOP_WireEdgeSet& aWES=*myWES;
  //
  Standard_Integer nE, nSp, iRankF1, iRankF2; 
  
  TopAbs_Orientation anOr;
  XBooleanOperations_StateOfShape aState;
  TopTools_IndexedMapOfOrientedShape aMap;
  //
  iRankF1=aDS.Rank(nF1);
  iRankF2=aDS.Rank(nF2);
  // W E S 
  TopoDS_Face aF1FWD, aF2FWD; 
  PrepareFaces(nF1, nF2, aF1FWD, aF2FWD);

  TopExp_Explorer anExp;
  //
  // aF1FWD
  anExp.Init(aF1FWD, TopAbs_EDGE);
  for (; anExp.More(); anExp.Next()) {
    const TopoDS_Shape& anE=anExp.Current();
    anOr=anE.Orientation();

    nE=aDS.ShapeIndex(anE, iRankF1);

    const XBOPTools_ListOfPaveBlock& aLPB=aSplitShapesPool(aDS.RefEdge(nE));
    XBOPTools_ListIteratorOfListOfPaveBlock aPBIt(aLPB);
    for (; aPBIt.More(); aPBIt.Next()) {
      const XBOPTools_PaveBlock& aPB=aPBIt.Value();
      nSp=aPB.Edge();
      //>>>
      if (!myStatesMap.Contains(nSp)) {
	continue;
      }
      //>>>
      aState=myStatesMap.FindFromKey(nSp);
      
      if (aState==XBooleanOperations_IN) {
	
	const TopoDS_Shape& aSplit=aDS.Shape(nSp);
	TopoDS_Edge aSS=TopoDS::Edge(aSplit);
	aSS.Orientation(anOr);
	//
	if (aMap.Contains(aSS)) {
	  continue;
	}
	//
	aWES.AddStartElement (aSS);
	aMap.Add(aSS);  

	if (BRep_Tool::IsClosed(aSS, aF1FWD)){
	  TopoDS_Shape EE=aSS.Reversed();
	  aWES.AddStartElement (EE);
	  aMap.Add(EE);
	}
      }
    }
  }
  //
  // aF2FWD
  aMap.Clear();
  anExp.Init(aF2FWD, TopAbs_EDGE);
  for (; anExp.More(); anExp.Next()) {
    const TopoDS_Shape& anE=anExp.Current();

    anOr=anE.Orientation();

    nE=aDS.ShapeIndex(anE, iRankF2);

    const XBOPTools_ListOfPaveBlock& aLPB=aSplitShapesPool(aDS.RefEdge(nE));
    
    XBOPTools_ListIteratorOfListOfPaveBlock aPBIt(aLPB);
    for (; aPBIt.More(); aPBIt.Next()) {
      const XBOPTools_PaveBlock& aPB=aPBIt.Value();
      nSp=aPB.Edge();
      //>>>
      if (!myStatesMap.Contains(nSp)) {
	continue;
      }
      //>>>
      aState=myStatesMap.FindFromKey(nSp);
      
      if (aState==XBooleanOperations_IN) {
	const TopoDS_Shape& aSplit=aDS.Shape(nSp);
	TopoDS_Edge aSS=TopoDS::Edge(aSplit);
	//
	if (!XBOPTools_Tools2D::HasCurveOnSurface(aSS, aF1FWD)) {
	  continue;
	}
	//
	aSS.Orientation(anOr);
	//
	if (aMap.Contains(aSS)) {
	  continue;
	}
	//	
	aWES.AddStartElement (aSS);
	aMap.Add(aSS);

	if (BRep_Tool::IsClosed(aSS, aF2FWD)){
	  TopoDS_Shape EE=aSS.Reversed();
	  aWES.AddStartElement (EE);
	  aMap.Add(EE);
	}
      }
    }
  }

  PrepareOnParts(nF1, nF2, XBOP_COMMON);
}

//=======================================================================
// function: PrepareWESForCut
// purpose: 
//=======================================================================
  void XBOP_SDFWESFiller::PrepareWESForCut (const Standard_Integer nF1, 
					   const Standard_Integer nF2)
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  const XBOPTools_PaveFiller& aPaveFiller=myDSFiller->PaveFiller();
  const XBOPTools_SplitShapesPool& aSplitShapesPool=aPaveFiller.SplitShapesPool();
  XBOP_WireEdgeSet& aWES=*myWES;
  //
  Standard_Integer  nE, nSp, nPB, iRankF2;
  TopAbs_Orientation anOr;
  XBooleanOperations_StateOfShape aState;

  iRankF2=aDS.Rank(nF2);
  //
  // W E S 
  TopoDS_Face aF1FWD, aF2FWD; 
  PrepareFaces(nF1, nF2, aF1FWD, aF2FWD);

  aF2FWD.Reverse();
  
  TopExp_Explorer anExp;
  //
  //  aF2FWD
  anExp.Init(aF2FWD, TopAbs_EDGE);
  for (; anExp.More(); anExp.Next()) {
    const TopoDS_Shape& anE=anExp.Current();
    anOr=anE.Orientation();

    nE=aDS.ShapeIndex(anE, iRankF2);

    const XBOPTools_ListOfPaveBlock& aLPB=aSplitShapesPool(aDS.RefEdge(nE));

    nPB=aLPB.Extent();
    if (nPB) {
      XBOPTools_ListIteratorOfListOfPaveBlock aPBIt(aLPB);
      for (; aPBIt.More(); aPBIt.Next()) {
	const XBOPTools_PaveBlock& aPB=aPBIt.Value();
	nSp=aPB.Edge();
	//>>>
	if (!myStatesMap.Contains(nSp)) {
	  continue;
	}
	//>>>
	aState=myStatesMap.FindFromKey(nSp);
	
	if (aState==XBooleanOperations_IN) {
	  const TopoDS_Shape& aSplit=aDS.Shape(nSp);
	  TopoDS_Edge aSS=TopoDS::Edge(aSplit);
	  //
	  if (!XBOPTools_Tools2D::HasCurveOnSurface(aSS, aF1FWD)) {
	    continue;
	  }
	  //
	  aSS.Orientation(anOr);
	  //
	  aWES.AddStartElement (aSS);
	  //
	  //modified by NIZNHY-PKV Tue Sep 25 14:25:13 2012f
	  if (IsClosed(aSS, aF2FWD)){
	  //if (BRep_Tool::IsClosed(aSS, aF2FWD)){
	    //modified by NIZNHY-PKV Tue Sep 25 14:25:35 2012t
	    TopoDS_Shape EE=aSS.Reversed();
	    aWES.AddStartElement (EE);
	  }
	}
      }
    }
    else {
      //>>>
      if (!myStatesMap.Contains(nE)) {
	continue;
      }
      //>>>
      aState=myStatesMap.FindFromKey(nE);
      if (aState==XBooleanOperations_IN) {
	TopoDS_Edge aSS=TopoDS::Edge(anE);
	//
	aWES.AddStartElement (aSS);
      }
    }
  } // end of for (; anExp.More(); anExp.Next()) {
  
  PrepareOnParts(nF1, nF2, XBOP_CUT);
}
//=======================================================================
// function: PrepareOnParts
// purpose: 
//=======================================================================
  void XBOP_SDFWESFiller::PrepareOnParts (const Standard_Integer nF1, 
					 const Standard_Integer nF2,
					 const XBOP_Operation anOperation)
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  const XBOPTools_PaveFiller& aPaveFiller=myDSFiller->PaveFiller();
  XBOPTools_PaveFiller* pPaveFiller=(XBOPTools_PaveFiller*) &aPaveFiller;
  XBOPTools_CommonBlockPool& aCBPool=pPaveFiller->ChangeCommonBlockPool();
  XBOP_WireEdgeSet& aWES=*myWES;
  //
  const Handle(XIntTools_Context)& aContext=pPaveFiller->Context();
  //
  Standard_Integer nE1, nE2, aNbSpON, nSp1, aBid, nSpTaken, nSp2, iRankF1, iRankF2;
  Standard_Real aT1, aT2, aU, aV, aScPr;
  Standard_Boolean bToReverse, bInternal1, bInternal2, bAdded;
  
  TopoDS_Edge aSS, aSSx;
  TopoDS_Face aF1FWD, aF2FWD;
  TopAbs_Orientation anOr, anOr2;

  TColStd_ListOfInteger aLs;
  TColStd_IndexedMapOfInteger aMap;
  TopTools_IndexedMapOfShape aM;
  TopTools_MapOfShape aMFence;//xft
  //
  gp_Vec aN1, aN2, aTau1, aTau2, aBiN1, aBiN2;
  TopExp_Explorer anExpF1, anExpF2;
  //
  // Source faces
  iRankF1=aDS.Rank(nF1);
  iRankF2=aDS.Rank(nF2);
  //
  PrepareFaces(nF1, nF2, aF1FWD, aF2FWD);
  //
  myRejectedOnParts.Clear();//xft
  //
  anExpF1.Init(aF1FWD, TopAbs_EDGE);
  for (; anExpF1.More(); anExpF1.Next()) {
    const TopoDS_Edge& anE1=TopoDS::Edge(anExpF1.Current());
    anOr=anE1.Orientation();
    //
    if (BRep_Tool::Degenerated(anE1)){
      continue;
    }
    //
    nE1=aDS.ShapeIndex(anE1, iRankF1); 

    anExpF2.Init(aF2FWD, TopAbs_EDGE);
    for (; anExpF2.More(); anExpF2.Next()) {
      const TopoDS_Edge& anE2=TopoDS::Edge(anExpF2.Current());
      anOr2=anE2.Orientation();
      //
      if (BRep_Tool::Degenerated(anE2)){
	continue;
      }
      //
      nE2=aDS.ShapeIndex(anE2, iRankF2);
      aLs.Clear();
      pPaveFiller->SplitsOnEdge(nE1, nE2, aLs);
      
      aNbSpON=aLs.Extent();
      if (!aNbSpON) {
	continue;
      }

      aMap.Clear();
      TColStd_ListIteratorOfListOfInteger anItLs(aLs);
      for (; anItLs.More(); anItLs.Next()) {
	aBid=anItLs.Value();
	aMap.Add(aBid);
      }

      XBOPTools_ListOfCommonBlock& aLCB=aCBPool(aDS.RefEdge(nE1));
      XBOPTools_ListIteratorOfListOfCommonBlock anItCB(aLCB);
      for (; anItCB.More(); anItCB.Next()) {
	bAdded=Standard_False;//xft
	XBOPTools_CommonBlock& aCB=anItCB.Value();
	// Pave Block from which new edge will be taken
	const XBOPTools_PaveBlock& aPB=aCB.PaveBlock1();
	nSpTaken=aPB.Edge();
	const TopoDS_Shape& aSpTaken=aDS.Shape(nSpTaken);
	//
	XBOPTools_PaveBlock& aPB1=aCB.PaveBlock1(nE1);
	nSp1=aPB1.Edge();
	
	if (aMap.Contains(nSp1)) {
	  XBOPTools_PaveBlock& aPB2=aCB.PaveBlock2(nE1);
	  nSp2=aPB2.Edge();
	  //
	  //
	  //iiiiiiiiiiiiiiiii Tue Dec 25 15:10:09 2001 iiiiii
	  //
	  // Internal eges' processing
	  bInternal1=(anOr ==TopAbs_INTERNAL);
	  bInternal2=(anOr2==TopAbs_INTERNAL);
	  //
	  if (bInternal1 || bInternal2) {
	    aSS=TopoDS::Edge(aDS.Shape(nSpTaken));
	    // a.
	    if (bInternal1 && bInternal2) {
	      if (anOperation==XBOP_COMMON) {
		aWES.AddStartElement (aSS);
		bAdded=Standard_True;//xft
	      }
	    }
	    // b.
	    else { // else x 
	      if (bInternal1 && !bInternal2) {
		if (nSpTaken==nSp1) {
		  aSS.Orientation(TopAbs_FORWARD);
		  aSSx=TopoDS::Edge(aDS.Shape(nSp2));
		  aSSx.Orientation(anOr2);
		  //
		  bToReverse=XBOPTools_Tools3D::IsSplitToReverse1 (aSSx, aSS, aContext);
		  //
		  if (bToReverse) {
		    aSS.Reverse();
		  }
		}
		else {//nSpTaken!=nSp1
		  aSS.Orientation(anOr2);
		}
	      }
	      // c.
	      else if (!bInternal1 && bInternal2) {
		if (nSpTaken==nSp2) {
		  aSS.Orientation(TopAbs_FORWARD);
		  aSSx=TopoDS::Edge(aDS.Shape(nSp1));
		  aSSx.Orientation(anOr);
		  //
		  bToReverse=XBOPTools_Tools3D::IsSplitToReverse1 (aSSx, aSS, aContext);
		  //
		  if (bToReverse) {
		    aSS.Reverse();
		  }
		}
		else {//nSpTaken!=nSp2
		  aSS.Orientation(anOr);
		}
	      }
	      // writting
	      if (anOperation==XBOP_COMMON) {
		aWES.AddStartElement (aSS);
		bAdded=Standard_True;//xft
	      }
	      if (anOperation==XBOP_CUT) { 
		aSS.Reverse();
		aWES.AddStartElement (aSS);
		bAdded=Standard_True;//xft
	      }
	    } // else x 
	    continue;
	  }  
	  //
	  //iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
	  //
	  //
	  const XBOPTools_PointBetween &aPbn1=aPB1.PointBetween();
	  aT1=aPbn1.Parameter();
	  XBOPTools_Tools2D::EdgeTangent(anE1, aT1, aTau1);
	  
	  XBOPTools_Tools2D::PointOnSurface(anE1, aF1FWD, aT1, aU, aV);
	  XBOPTools_Tools2D::FaceNormal(aF1FWD, aU, aV, aN1);
	  aBiN1=aN1^aTau1;
	  
	  const XBOPTools_PointBetween &aPbn2=aPB2.PointBetween();
	  aT2=aPbn2.Parameter();
	  XBOPTools_Tools2D::EdgeTangent(anE2, aT2, aTau2);
	  
	  XBOPTools_Tools2D::PointOnSurface(anE2, aF2FWD, aT2, aU, aV);
	  XBOPTools_Tools2D::FaceNormal(aF2FWD, aU, aV, aN2);
	  aBiN2=aN2^aTau2;

	  aScPr=aBiN1*aBiN2;
	  //
	  if (aScPr < 0.) {
	    if (anOperation==XBOP_CUT) { 
	      
	      if (nSpTaken==nSp1) {
		aSS=TopoDS::Edge(aDS.GetShape(nSp1));
		aSS.Orientation(anOr);
	      }
	      
	      else {
		const TopoDS_Shape& aSp1=aDS.Shape(nSp1);
		TopoDS_Edge aSpE1=TopoDS::Edge(aSp1);
		aSpE1.Orientation(anOr);

		const TopoDS_Shape& aSp2=aDS.Shape(nSp2);
		TopoDS_Edge aSpE2=TopoDS::Edge(aSp2);
		//
		bToReverse=XBOPTools_Tools3D::IsSplitToReverse1 (aSpE1, aSpE2, aContext);
		//
		if (bToReverse) {
		  aSpE2.Reverse();
		}
		aSS=aSpE2;
	      }
	      // modified by NIZHNY-MKK  Thu May 29 12:39:32 2003.BEGIN
	      if (BRep_Tool::IsClosed(anE1, aF1FWD) && (!aSS.IsSame(anE1))) {
		Standard_Boolean areverse = Standard_False;
		XBOPTools_Tools3D::DoSplitSEAMOnFace(aSS, anE1, aF1FWD, areverse);
	      }

	      if(BRep_Tool::IsClosed(anE2, aF2FWD) && (!aSS.IsSame(anE2))) {
		Standard_Boolean areverse = Standard_False;
		XBOPTools_Tools3D::DoSplitSEAMOnFace(aSS, anE2, aF2FWD, areverse);
	      }
	      // modified by NIZHNY-MKK  Thu May 29 12:39:35 2003.END
	      //
	      if (BRep_Tool::IsClosed(anE1, aF1FWD) &&
		  BRep_Tool::IsClosed(anE2, aF2FWD)){
		//
		Standard_Boolean bIsTakenSp1;
		TopoDS_Edge aSpE1Seam11, aSpE1Seam21;

		bIsTakenSp1=(nSpTaken==nSp1);
		aSpE1Seam11=TopoDS::Edge(aDS.Shape(nSp1));
		aSpE1Seam21=TopoDS::Edge(aDS.Shape(nSp2));
		//
		if (aM.Contains(aSpE1Seam11)){
		  continue;
		}
		aM.Add(aSpE1Seam11);
		//
		if (aM.Contains(aSpE1Seam21)){
		  continue;
		}
		aM.Add(aSpE1Seam21);
		//
		TreatSDSeams (aSpE1Seam11, aT1, aF1FWD, 
			      aSpE1Seam21, aT2, aF2FWD,
			      bIsTakenSp1, aWES,
			      aContext);
		//
		continue;
	      }
	      //
	      aWES.AddStartElement (aSS);
	      bAdded=Standard_True;//xft
	    } 
	    
	  }
	  
	  else {
	    if (anOperation==XBOP_COMMON) {
	      
	      if (nSpTaken==nSp1) {
		aSS=TopoDS::Edge(aDS.GetShape(nSp1));
		aSS.Orientation(anOr);
	      }
	      
	      else {
		const TopoDS_Shape& aSp1=aDS.Shape(nSp1);
		TopoDS_Edge aSpE1=TopoDS::Edge(aSp1);
		aSpE1.Orientation(anOr);

		const TopoDS_Shape& aSp2=aDS.Shape(nSp2);
		TopoDS_Edge aSpE2=TopoDS::Edge(aSp2);
		//
		bToReverse=XBOPTools_Tools3D::IsSplitToReverse1 (aSpE1, aSpE2, aContext);
		//
		if (bToReverse) {
		  aSpE2.Reverse();
		}
		aSS=aSpE2;
		//
		if (BRep_Tool::IsClosed(aSpE1, aF1FWD)) {
		  //
		  if (aM.Contains(aSpE2)){
		    continue;
		  }
		  aM.Add(aSpE2);
		  //
		  if (!BRep_Tool::IsClosed(aSpE2, aF1FWD)) {
		    XBOPTools_Tools3D::DoSplitSEAMOnFace (aSpE2, aF1FWD);
		  }
		  aWES.AddStartElement (aSpE2);
		  aSpE2.Reverse();
		  aWES.AddStartElement (aSpE2);
		  bAdded=Standard_True;	//xft
		  continue;  
		}
		//
	      }
	      //
	      aWES.AddStartElement (aSS);
	      bAdded=Standard_True; //xft
	    }// if (anOperation==XBOP_COMMON) {
	  }// else {
	  if (!bAdded) {
	    if(aMFence.Add(aSpTaken)) { 
	      myRejectedOnParts.Append(aSpTaken);
	    }
	  }
	}// if (aMap.Contains(nSp1)) { 
      }// for (; anItCB.More(); anItCB.Next()) {
    }// for (; anExpF2.More(); anExpF2.Next()) {
  }//for (; anExpF1.More(); anExpF1.Next()) {
}

//=======================================================================
// function: PrepareFaces
// purpose: 
//=======================================================================
  void XBOP_SDFWESFiller::PrepareFaces   (const Standard_Integer nF1, 
					 const Standard_Integer nF2, 
					 TopoDS_Face& aF1FWD,
					 TopoDS_Face& aF2FWD) const
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  TopAbs_Orientation anOr1, anOr2;

  const TopoDS_Shape& aF1=aDS.GetShape(nF1); 
  aF1FWD=TopoDS::Face(aF1);
  anOr1=aF1.Orientation();

  const TopoDS_Shape& aF2=aDS.GetShape(nF2);
  aF2FWD=TopoDS::Face(aF2);
  anOr2=aF2.Orientation();

  aF1FWD.Orientation(TopAbs_FORWARD);

  if (mySenseFlag==1) {
    if      (anOr1==TopAbs_FORWARD   && anOr2==TopAbs_FORWARD) {
      aF2FWD.Orientation(TopAbs_FORWARD);
    }
    else if (anOr1==TopAbs_REVERSED  && anOr2==TopAbs_REVERSED) {
      aF2FWD.Orientation(TopAbs_FORWARD);
    }
    else if (anOr1==TopAbs_FORWARD  && anOr2==TopAbs_REVERSED) {
      aF2FWD.Orientation(TopAbs_REVERSED);
    }
    else if (anOr1==TopAbs_REVERSED  && anOr2==TopAbs_FORWARD) {
      aF2FWD.Orientation(TopAbs_REVERSED);
    }
  }

  else{
    if      (anOr1==TopAbs_FORWARD   && anOr2==TopAbs_FORWARD) {
      aF2FWD.Orientation(TopAbs_REVERSED);
    }
    else if (anOr1==TopAbs_REVERSED  && anOr2==TopAbs_REVERSED) {
      aF2FWD.Orientation(TopAbs_REVERSED);
    }
    else if (anOr1==TopAbs_FORWARD  && anOr2==TopAbs_REVERSED) {
      aF2FWD.Orientation(TopAbs_FORWARD);
    }
    else if (anOr1==TopAbs_REVERSED  && anOr2==TopAbs_FORWARD) {
      aF2FWD.Orientation(TopAbs_FORWARD);
    }
  }
}
//
//=======================================================================
// function: AssignDEStates
// purpose: 
//======================================================================= 
  void XBOP_SDFWESFiller::AssignDEStates(const Standard_Integer nF1,
					const Standard_Integer nF2) 
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  
  Standard_Integer nE1, iRankF1;
  TopExp_Explorer anExpF1;

  iRankF1=aDS.Rank(nF1);

  const TopoDS_Face& aF1=TopoDS::Face(aDS.Shape(nF1));

  anExpF1.Init(aF1, TopAbs_EDGE);
  for (; anExpF1.More(); anExpF1.Next()) {
    const TopoDS_Edge& anE1=TopoDS::Edge(anExpF1.Current());
    //
    if (!BRep_Tool::Degenerated(anE1)){
      continue;
    }
    //
    nE1=aDS.ShapeIndex(anE1, iRankF1);
    AssignDEStates (nF1, nE1, nF2); 
  }
}

//=======================================================================
// function: AssignDEStates
// purpose: 
//======================================================================= 
  void XBOP_SDFWESFiller::AssignDEStates(const Standard_Integer nFD,
					const Standard_Integer nED, 
					const Standard_Integer nF2) 
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  const XBOPTools_PaveFiller& aPaveFiller=myDSFiller->PaveFiller();
  XBOPTools_PaveFiller* pPaveFiller=(XBOPTools_PaveFiller*) &aPaveFiller;
  const XBOPTools_SplitShapesPool& aSplitShapesPool=aPaveFiller.SplitShapesPool();
  //
  const XBOPTools_ListOfPaveBlock& aSplitEdges=aSplitShapesPool(aDS.RefEdge(nED));
  //
  const Handle(XIntTools_Context)& aContext=pPaveFiller->Context();
  const TopoDS_Edge& aDE=TopoDS::Edge(aDS.Shape(nED));
  const TopoDS_Face& aDF=TopoDS::Face(aDS.Shape(nFD));
  const TopoDS_Face& aFaceReference=TopoDS::Face(aDS.Shape(nF2));
  //
  Standard_Boolean bIsValidPoint;
  Standard_Integer nSp;
  Standard_Real aT, aT1, aT2;
  TopAbs_State aState;
  TopoDS_Face aF;
  gp_Pnt2d aPx2DNear;
  gp_Pnt aPxNear;
  //
  aF=aDF;
  aF.Orientation(TopAbs_FORWARD);
  
  XBOPTools_ListIteratorOfListOfPaveBlock aPBIt(aSplitEdges);
  for (; aPBIt.More(); aPBIt.Next()) {
    XBOPTools_PaveBlock& aPB=aPBIt.Value();
    
    nSp=aPB.Edge();
    const TopoDS_Edge& aSp=TopoDS::Edge(aDS.Shape(nSp));
    
    aPB.Parameters(aT1, aT2);
    aT=XIntTools_Tools::IntermediatePoint(aT1, aT2);

    TopoDS_Edge aDERight, aSpRight;
    aDERight=aDE;
    aSpRight=aSp;

    XBOPTools_Tools3D::OrientEdgeOnFace (aDE, aF, aDERight);
    aSpRight.Orientation(aDERight.Orientation());
    //
    XBOPTools_Tools3D::PointNearEdge(aSpRight, aDF, aT, aPx2DNear, aPxNear);
    //
    aState=TopAbs_OUT;
    //
    bIsValidPoint=aContext->IsValidPointForFace(aPxNear, aFaceReference, 1.e-3);
    //
    if (bIsValidPoint) {
      aState=TopAbs_IN;
    }
    //
    XBooleanOperations_StateOfShape aSt;
    
    aSt=XBOPTools_SolidStateFiller::ConvertState(aState);
    if (myStatesMap.Contains(nSp)) {
      XBooleanOperations_StateOfShape& aBooState=myStatesMap.ChangeFromKey(nSp);
      aBooState=aSt;
    }
    else {
      myStatesMap.Add(nSp, aSt);
    }
  }
}

//=======================================================================
// function: UpdateDEStates3D
// purpose: 
//======================================================================= 
  void XBOP_SDFWESFiller::UpdateDEStates3D()
{
  XBooleanOperations_ShapesDataStructure* pDS=
    (XBooleanOperations_ShapesDataStructure*)&myDSFiller->DS();
  
  Standard_Integer i, aNb, nSp;
  XBooleanOperations_StateOfShape aSt;

  aNb=myStatesMap.Extent();
  for (i=1; i<=aNb; i++) {
    nSp=myStatesMap.FindKey(i);
    aSt=pDS->GetState(nSp);
    aSt=XBooleanOperations_UNKNOWN;
    pDS->SetState(nSp, aSt);
  }
}
//
#include <Geom2d_Curve.hxx>
#include <gp_Pnt2d.hxx>

//=======================================================================
// function: TreatSDSeams
// purpose: 
//=======================================================================
void TreatSDSeams (const TopoDS_Edge& aSpE1Seam11,
		   const Standard_Real aT1,
		   const TopoDS_Face& aF1FWD,
	
		   const TopoDS_Edge& aSpE1Seam21,
		   const Standard_Real aT2,
		   const TopoDS_Face& aF2FWD,
		   
		   const Standard_Boolean bIsTakenSp1,
		   XBOP_WireEdgeSet& aWES,
		   const Handle(XIntTools_Context)& aContext)
{
  Standard_Boolean bIsValidSeam11, bIsValidSeam12, 
                   bIsValidSeam21, bIsValidSeam22;
  Standard_Real aScPr;
  TopoDS_Edge aSS, aSpE1Seam12,aSpE1Seam22;
  gp_Dir aDB11, aDB12, aDB21, aDB22;
	    
  aSpE1Seam12=TopoDS::Edge(aSpE1Seam11.Reversed());
  aSpE1Seam22=TopoDS::Edge(aSpE1Seam21.Reversed());
	    //
  if (!bIsTakenSp1) {
    XBOPTools_Tools3D::DoSplitSEAMOnFace (aSpE1Seam21, aF1FWD);
    aSpE1Seam22=TopoDS::Edge(aSpE1Seam21.Reversed());
  }
  //
  bIsValidSeam11=IsValidSeam(aSpE1Seam11, aF1FWD, aT1, aContext);
  bIsValidSeam12=IsValidSeam(aSpE1Seam12, aF1FWD, aT1, aContext);
  bIsValidSeam21=IsValidSeam(aSpE1Seam21, aF2FWD, aT2, aContext);
  bIsValidSeam22=IsValidSeam(aSpE1Seam22, aF2FWD, aT2, aContext);
  // 1
  if (bIsValidSeam11 && bIsValidSeam12) {
    XBOPTools_Tools3D::GetBiNormal(aSpE1Seam11, aF1FWD, aT1, aDB11);
    XBOPTools_Tools3D::GetBiNormal(aSpE1Seam12, aF1FWD, aT1, aDB12);
    // 1.1
    if (bIsValidSeam21 && !bIsValidSeam22) {
      XBOPTools_Tools3D::GetBiNormal(aSpE1Seam21, aF2FWD, aT2, aDB21);
      aScPr=aDB11*aDB21;
      if (aScPr<0.) {
	if (bIsTakenSp1) {
	  aSS=aSpE1Seam11;
	}
	else {
	  //aSS=aSpE1Seam21;
	  CorrespondantSeam(aSpE1Seam11, aT1, 
			    aSpE1Seam21, aSpE1Seam22, aT2, 
			    aF1FWD, aSS);
	}
      }
      else { //if (aScPr>0.)
	if (bIsTakenSp1) {
	  aSS=aSpE1Seam12;
	}
	else {
	  //aSS=aSpE1Seam22;
	  CorrespondantSeam(aSpE1Seam12, aT1, 
			    aSpE1Seam21, aSpE1Seam22, aT2, 
			    aF1FWD, aSS);
	}
      }
      aWES.AddStartElement (aSS);
    } //if (bIsValidSeam21 && !bIsValidSeam22)
    //  
    // 1.2	
    if (!bIsValidSeam21 && bIsValidSeam22) {
      XBOPTools_Tools3D::GetBiNormal(aSpE1Seam22, aF2FWD, aT2, aDB22);
      aScPr=aDB11*aDB22;
      if (aScPr<0.) {
	if (bIsTakenSp1) {
	  aSS=aSpE1Seam11;
	}
	else {
	  //aSS=aSpE1Seam22;
	  CorrespondantSeam(aSpE1Seam11, aT1, 
			    aSpE1Seam21, aSpE1Seam22, aT2, 
			    aF1FWD, aSS);
	}
      }
      else {//if (aScPr>0.)
	if (bIsTakenSp1) {
	  aSS=aSpE1Seam12;
	}
	else {
	  //aSS=aSpE1Seam21;
	  CorrespondantSeam(aSpE1Seam12, aT1, 
			    aSpE1Seam21, aSpE1Seam22, aT2, 
			    aF1FWD, aSS);
	}
      }
      aWES.AddStartElement (aSS);
    }// if (!bIsValidSeam21 && bIsValidSeam22)
  } //if (bIsValidSeam11 && bIsValidSeam12)
}

//=======================================================================
// function: IsValidSeam
// purpose: 
//======================================================================= 
  Standard_Boolean IsValidSeam(const TopoDS_Edge& aE,
			       const TopoDS_Face& aF,
			       const Standard_Real aT,
			       const Handle(XIntTools_Context)& aContext)
{
  Standard_Boolean bIsPointInOnFace;
  gp_Pnt2d aPx2DNear;
  gp_Pnt aPxNear;

  XBOPTools_Tools3D::PointNearEdge(aE, aF, aT, aPx2DNear, aPxNear);
  //
  bIsPointInOnFace=aContext->IsPointInOnFace(aF, aPx2DNear);
  return bIsPointInOnFace;
}
//=======================================================================
// function: CorrespondantSeam
// purpose: 
//======================================================================= 
  void CorrespondantSeam(const TopoDS_Edge& aSpE1Seam11,
			 const Standard_Real aT1,
			 const TopoDS_Edge& aSpE1Seam21,
			 const TopoDS_Edge& aSpE1Seam22,
			 const Standard_Real aT2,
			 const TopoDS_Face& aF1FWD,
			 TopoDS_Edge& aSS)
				      
{
  Standard_Real a, b, aD1121, aD1122, aTol=1.e-7;
  Handle(Geom2d_Curve) aC2DSeam11, aC2DSeam21, aC2DSeam22;
  gp_Pnt2d aP2D11, aP2D21, aP2D22;

  aC2DSeam11=BRep_Tool::CurveOnSurface(aSpE1Seam11, aF1FWD, a, b);
  aC2DSeam11->D0(aT1, aP2D11);

  aC2DSeam21=BRep_Tool::CurveOnSurface(aSpE1Seam21, aF1FWD, a, b);
  aC2DSeam21->D0(aT2, aP2D21);

  aC2DSeam22=BRep_Tool::CurveOnSurface(aSpE1Seam22, aF1FWD, a, b);
  aC2DSeam22->D0(aT2, aP2D22);

  aD1121=aP2D11.Distance(aP2D21);
  aD1122=aP2D11.Distance(aP2D22);
  
  aSS=aSpE1Seam22;
  if (aD1121<aTol) {
    aSS=aSpE1Seam21;
  }
}
//modified by NIZNHY-PKV Tue Sep 25 14:25:53 2012f
//=======================================================================
//function : IsClosed
//purpose  :
//=======================================================================
Standard_Boolean IsClosed(const TopoDS_Edge& aE,
			  const TopoDS_Face& aF)
{
  Standard_Boolean bRet;
  //
  bRet=BRep_Tool::IsClosed(aE, aF);
  if (bRet) {
    Standard_Integer iCnt;
    TopoDS_Shape aE1;
    //
    bRet=!bRet;
    iCnt=0;
    TopExp_Explorer aExp(aF, TopAbs_EDGE);
    for (; aExp.More(); aExp.Next()) {
      const TopoDS_Shape& aEx=aExp.Current();
      //
      if (aEx.IsSame(aE)) {
	++iCnt;
	if (iCnt==1) {
	  aE1=aEx;
	}
	else if (iCnt==2){
	  aE1.Reverse();
	  bRet=(aE1==aEx);
	  break;
	}
      }
    }
  }
  return bRet;
}
//modified by NIZNHY-PKV Tue Sep 25 14:25:56 2012t
