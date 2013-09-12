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


#include <XBOP_WireShape.ixx>

#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>

#include <TopAbs_ShapeEnum.hxx>
#include <TopAbs_Orientation.hxx>

#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>

#include <BRep_Tool.hxx>
#include <BRep_Builder.hxx>

#include <XBOPTools_DSFiller.hxx>
#include <XBOPTools_PaveFiller.hxx>
#include <XBOPTools_SplitShapesPool.hxx>
#include <XBOPTools_PaveBlock.hxx>
#include <XBOPTools_ListOfPaveBlock.hxx>
#include <XBOPTools_ListIteratorOfListOfPaveBlock.hxx>
#include <XBOPTools_CommonBlockPool.hxx>
#include <XBOPTools_CommonBlock.hxx>
#include <XBOPTools_ListOfCommonBlock.hxx>
#include <XBOPTools_ListIteratorOfListOfCommonBlock.hxx>

#include <XBOP_CorrectTolerances.hxx>
#include <XBOP_BuilderTools.hxx>

#include <XBooleanOperations_ShapesDataStructure.hxx>
#include <XBooleanOperations_StateOfShape.hxx>

#include <XBOP_BuilderTools.hxx>
#include <XBOP_ConnexityBlock.hxx>
#include <XBOP_ListOfConnexityBlock.hxx>
#include <XBOP_ListIteratorOfListOfConnexityBlock.hxx>


static
  Standard_Integer InOrOut(const TopoDS_Vertex& ,
			   const TopoDS_Edge& );
static
  TopAbs_Orientation Orientation(const TopoDS_Vertex& ,
				 const TopoDS_Edge& );
static
  void OrientEdgesOnWire(const TopoDS_Wire& , 
			 TopoDS_Wire& );

//=======================================================================
// function: XBOP_WireShape::XBOP_WireShape
// purpose: 
//=======================================================================
  XBOP_WireShape::XBOP_WireShape()
{
}
//=======================================================================
// function: MakeResult
// purpose: 
//=======================================================================
  void XBOP_WireShape::MakeResult()
{
  BRep_Builder aBB;
  TopoDS_Compound aCompound;

  aBB.MakeCompound(aCompound);
  
  TopoDS_Wire aWNew;
  XBOP_ListOfConnexityBlock aLCB;
  XBOP_BuilderTools::MakeConnexityBlocks(myLS, TopAbs_EDGE, aLCB);
  XBOP_ListIteratorOfListOfConnexityBlock aLCBIt(aLCB);
  for (; aLCBIt.More(); aLCBIt.Next()) {
    const XBOP_ConnexityBlock& aCB=aLCBIt.Value();
    const TopTools_ListOfShape& aLE=aCB.Shapes();
    TopoDS_Wire aW;
    aBB.MakeWire(aW);
    TopTools_ListIteratorOfListOfShape anIt(aLE);
    for (; anIt.More(); anIt.Next()) {
      const TopoDS_Edge& aE=TopoDS::Edge(anIt.Value());
      aBB.Add(aW, aE);
    }
    OrientEdgesOnWire(aW, aWNew);
    aBB.Add(aCompound, aWNew);
  }
  myResult=aCompound;
}

//=======================================================================
// function: AddSplitPartsINOUT
// purpose: 
//=======================================================================
  void XBOP_WireShape::AddSplitPartsINOUT()
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  const XBOPTools_PaveFiller& aPaveFiller=myDSFiller->PaveFiller();
  const XBOPTools_SplitShapesPool& aSplitShapesPool=aPaveFiller.SplitShapesPool();
  //
  Standard_Integer i, aNbPB, iRank, nSp, iBeg, iEnd;
  TopAbs_ShapeEnum aType, aTypeArg1, aTypeArg2;
  XBooleanOperations_StateOfShape aState, aStateCmp;
  //
  aTypeArg1=aDS.Object().ShapeType();
  aTypeArg2=aDS.Tool().ShapeType();
  
  iBeg=1;
  iEnd=aDS.NumberOfShapesOfTheObject();

  if (aTypeArg1!=TopAbs_WIRE && aTypeArg2==TopAbs_WIRE) {
    iBeg=iEnd+1;
    iEnd=aDS.NumberOfSourceShapes();
  }
  else if (aTypeArg1==TopAbs_WIRE && aTypeArg2==TopAbs_WIRE){
    iBeg=1;
    iEnd=aDS.NumberOfSourceShapes();
  }
  //

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
      }
    }
  }
}

//=======================================================================
// function: AddSplitPartsON
// purpose: 
//=======================================================================
  void XBOP_WireShape::AddSplitPartsON()
{
  if (myOperation==XBOP_CUT || myOperation==XBOP_CUT21) {
    return;
  }

  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  const XBOPTools_PaveFiller& aPaveFiller=myDSFiller->PaveFiller();
  const XBOPTools_CommonBlockPool& aCommonBlockPool=aPaveFiller.CommonBlockPool();

  Standard_Integer i, aNbCB, nSpTaken, iBeg, iEnd;
  TopAbs_ShapeEnum aType, aTypeArg1, aTypeArg2;
  XBOPTools_ListIteratorOfListOfCommonBlock anItCB;
  //
  aTypeArg1=aDS.Object().ShapeType();
  aTypeArg2=aDS.Tool().ShapeType();
  
  iBeg=1;
  iEnd=aDS.NumberOfShapesOfTheObject();
  if (aTypeArg1!=TopAbs_WIRE && aTypeArg2==TopAbs_WIRE) {
    iBeg=iEnd+1;
    iEnd=aDS.NumberOfSourceShapes();
  }
  else if (aTypeArg1==TopAbs_WIRE && aTypeArg2==TopAbs_WIRE){
    iEnd=aDS.NumberOfSourceShapes();
  }
  //
  for (i=iBeg; i<=iEnd; ++i) {
    aType=aDS.GetShapeType(i);
    if (aType!=TopAbs_EDGE) {
      continue;
    }
    //
    const XBOPTools_ListOfCommonBlock& aLCB=aCommonBlockPool(aDS.RefEdge(i));
    aNbCB=aLCB.Extent();

    anItCB.Initialize(aLCB);
    for (; anItCB.More(); anItCB.Next()) {
      XBOPTools_CommonBlock& aCB=anItCB.Value();
      const XBOPTools_PaveBlock& aPB=aCB.PaveBlock1();
      nSpTaken=aPB.Edge();
      const TopoDS_Edge& aSS=TopoDS::Edge(aDS.Shape(nSpTaken));
      myLS.Append(aSS);
    }
  }
}

//=======================================================================
// function: OrientEdgesOnWire
// purpose: 
//=======================================================================
void OrientEdgesOnWire(const TopoDS_Wire& aWire, 
		       TopoDS_Wire& aWireNew)
{
  Standard_Integer i, aNbV, aNbE, j, iCnt, iInOrOut, aNbRest;

  TopTools_IndexedDataMapOfShapeListOfShape aVEMap;
  TopTools_IndexedMapOfShape aProcessedEdges, aRestEdges, aEMap;
  TopTools_ListIteratorOfListOfShape anIt;
  BRep_Builder aBB;

  aBB.MakeWire(aWireNew);
  
  TopExp::MapShapesAndAncestors(aWire, TopAbs_VERTEX, TopAbs_EDGE, aVEMap);
  
  aNbV=aVEMap.Extent();
  //
  // Do
  for (i=1; i<=aNbV; i++) {
    const TopoDS_Vertex& aV=TopoDS::Vertex(aVEMap.FindKey(i));
    
    const TopTools_ListOfShape& aLE=aVEMap.FindFromIndex(i);
    aNbE=aLE.Extent();
    
    if (aNbE>=2) {
      iCnt=0;
      anIt.Initialize(aLE);
      for(; anIt.More(); anIt.Next()) {
	const TopoDS_Edge& aE=TopoDS::Edge(anIt.Value());
	if (aProcessedEdges.Contains(aE)) {
	  iInOrOut=InOrOut(aV, aE);
	  iCnt+=iInOrOut;
	}
	else {
	  aRestEdges.Add(aE);
	}
      }

      TopoDS_Edge* pE;
      aNbRest=aRestEdges.Extent();
      for (j=1; j<=aNbRest; j++) {
	const TopoDS_Edge& aE=TopoDS::Edge(aRestEdges(j));
	pE=(TopoDS_Edge*)&aE;
	
	iInOrOut=InOrOut(aV, aE);
	if (iCnt>0) {
	  if (iInOrOut>0) {
	    pE->Reverse();
	  }
	  --iCnt;
	}
	else if (iCnt<=0){
	  if (iInOrOut<0) {
	    pE->Reverse();
	  }
	  ++iCnt;
	}
	aProcessedEdges.Add(*pE);
      }
    }//if (aNbE>=2) 
  }
  //
  //
  aNbE=aProcessedEdges.Extent();
  for (i=1; i<=aNbE; i++) {
    const TopoDS_Edge& aE=TopoDS::Edge(aProcessedEdges(i));
    aBB.Add(aWireNew, aE);
  }

  TopExp::MapShapes(aWire, TopAbs_EDGE, aEMap);
  
  aNbE=aEMap.Extent();
  for (i=1; i<=aNbE; i++) {
    const TopoDS_Edge& aE=TopoDS::Edge(aEMap(i));
    if (!aProcessedEdges.Contains(aE)) {
      aProcessedEdges.Add(aE);
      aBB.Add(aWireNew, aE);
    }
  }
}

//=======================================================================
//function : Orientation
//purpose  :
//=======================================================================
  TopAbs_Orientation Orientation(const TopoDS_Vertex& aV,
				 const TopoDS_Edge& aE)
{
  TopAbs_Orientation anOr=TopAbs_INTERNAL;

  TopExp_Explorer anExp;
  anExp.Init(aE, TopAbs_VERTEX);
  for (; anExp.More(); anExp.Next()) {
    const TopoDS_Vertex& aVE1=TopoDS::Vertex(anExp.Current());
    if (aVE1.IsSame(aV)) {
      anOr=aVE1.Orientation();
      break;
    }
  }
  return anOr;
}

///=======================================================================
//function : InOrOut
//purpose  :
//=======================================================================
  Standard_Integer InOrOut(const TopoDS_Vertex& aV,
			   const TopoDS_Edge& aE)
{
  TopAbs_Orientation anOrV, anOrE;
  anOrV=aV.Orientation();
  anOrE=aE.Orientation();
  if (anOrV==TopAbs_INTERNAL){
    return 0;
  }
  
  anOrV=Orientation(aV, aE);
  
  if (anOrV==anOrE) {
    return -1; // escape
  }
  else {
    return 1; // entry
  }
}
