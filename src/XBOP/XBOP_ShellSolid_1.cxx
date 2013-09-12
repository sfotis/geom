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

#include <TColStd_ListOfInteger.hxx>
#include <TColStd_ListIteratorOfListOfInteger.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>

#include <TopoDS_Face.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>

#include <TopAbs_Orientation.hxx>

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_IndexedMapOfOrientedShape.hxx>

#include <TopExp_Explorer.hxx>

#include <BRep_Tool.hxx>

#include <XBooleanOperations_ShapesDataStructure.hxx>
#include <XBooleanOperations_IndexedDataMapOfShapeInteger.hxx>

#include <XIntTools_Context.hxx>

#include <XBOPTools_InterferencePool.hxx>
#include <XBOPTools_CArray1OfSSInterference.hxx>
#include <XBOPTools_SSInterference.hxx>

#include <XBOPTools_SequenceOfCurves.hxx>
#include <XBOPTools_Curve.hxx>
#include <XBOPTools_ListOfPaveBlock.hxx>
#include <XBOPTools_ListIteratorOfListOfPaveBlock.hxx>
#include <XBOPTools_PaveBlock.hxx>
#include <XBOPTools_PaveFiller.hxx>
#include <XBOPTools_CommonBlockPool.hxx>

#include <XBOPTools_ListOfCommonBlock.hxx>
#include <XBOPTools_ListIteratorOfListOfCommonBlock.hxx>
#include <XBOPTools_CommonBlock.hxx>
#include <XBOPTools_Tools3D.hxx>
#include <XBOPTools_InterferencePool.hxx>
#include <XBOPTools_CArray1OfSSInterference.hxx>

#include <XBOP_SDFWESFiller.hxx>

#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <TopExp.hxx>

static 
  Standard_Boolean CheckSplitToAvoid(const TopoDS_Edge&          theSplit,
				     const XBOPTools_CommonBlock& theCB, 
				     const Standard_Integer      theEdgeIndex,
				     const Standard_Integer      theFaceIndex,
				     const XBOPTools_PDSFiller&   theDSFiller, 
				     const XBOP_Operation&        theOperation,
				     const Handle(XIntTools_Context)& theContext);

//=======================================================================
// 
// the WES components for a shell
//
//=======================================================================
// function: AddSectionPartsSh
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::AddSectionPartsSh (const Standard_Integer nF1, 
					  const Standard_Integer iFF,
					  XBOP_WireEdgeSet& aWES)
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  XBOPTools_InterferencePool* pIntrPool=(XBOPTools_InterferencePool*)&myDSFiller->InterfPool();
  XBOPTools_CArray1OfSSInterference& aFFs=pIntrPool->SSInterferences();
  //
  Standard_Integer i, aNbCurves, nF2,  nE, iRankF1;
  //
  iRankF1=aDS.Rank(nF1);
  //
  XBOPTools_SSInterference& aFF=aFFs(iFF);
  nF2=aFF.OppositeIndex(nF1);
  //
  XBOPTools_SequenceOfCurves& aSC=aFF.Curves();
  aNbCurves=aSC.Length();
  for (i=1; i<=aNbCurves; i++) {
    const XBOPTools_Curve& aBC=aSC(i);
    const XBOPTools_ListOfPaveBlock& aLPB=aBC.NewPaveBlocks();
    XBOPTools_ListIteratorOfListOfPaveBlock anIt(aLPB);
    for (; anIt.More(); anIt.Next()) {
      const XBOPTools_PaveBlock& aPB=anIt.Value();
      nE=aPB.Edge();
      const TopoDS_Edge& aE=TopoDS::Edge(aDS.Shape(nE));
      
      TopoDS_Edge aES=aE;
      
      if (myOperation==XBOP_FUSE) {
	aWES.AddStartElement (aES);
	aES.Reverse();
	aWES.AddStartElement (aES);
      }
	
    }
  }
}
//=======================================================================
// function: AddSplitPartsONSh
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::AddSplitPartsONSh(const Standard_Integer nF1,
					 XBOP_WireEdgeSet& aWES)
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  const XBOPTools_PaveFiller& aPaveFiller=myDSFiller->PaveFiller();
  
  XBOPTools_PaveFiller* pPaveFiller=(XBOPTools_PaveFiller*)&aPaveFiller;
  XBOPTools_CommonBlockPool& aCBPool=pPaveFiller->ChangeCommonBlockPool();
  //
  const Handle(XIntTools_Context)& aContext=pPaveFiller->Context();
  //
  Standard_Integer nEF1, nF2, nSpF1, nSpF2, nEF2, nSpTaken, iRankF1;
  Standard_Boolean bToReverse;
  TopAbs_Orientation anOrEF1, anOrEF2;
  TopExp_Explorer anExp;
  TopTools_IndexedMapOfShape aM;
  TopoDS_Edge aSSF1, aSSF2;
  //
  iRankF1=aDS.Rank(nF1);
  //
  anExp.Init(myFace, TopAbs_EDGE);
  for (; anExp.More(); anExp.Next()) {
    const TopoDS_Edge& anEF1=TopoDS::Edge(anExp.Current());
    anOrEF1=anEF1.Orientation();
    nEF1=aDS.ShapeIndex(anEF1, iRankF1);
    
    XBOPTools_ListOfCommonBlock& aLCB=aCBPool(aDS.RefEdge(nEF1));
    
    XBOPTools_ListIteratorOfListOfCommonBlock anItCB(aLCB);
    for (; anItCB.More(); anItCB.Next()) {
      XBOPTools_CommonBlock& aCB=anItCB.Value();

      XBOPTools_PaveBlock& aPBEF1=aCB.PaveBlock1(nEF1);
      XBOPTools_PaveBlock& aPBEF2=aCB.PaveBlock2(nEF1);
      nF2=aCB.Face();
      if (nF2) { 
	// Splits that are ON (IN 2D) for other Face (aF2)
	nSpF1=aPBEF1.Edge();
	const TopoDS_Shape& aSplit=aDS.Shape(nSpF1);
	aSSF1=TopoDS::Edge(aSplit);
	//
	//iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
	// Internal edges treatment
	{
	  if (anOrEF1==TopAbs_INTERNAL) {
	    if (myOperation==XBOP_FUSE) {
	      aSSF1.Orientation(TopAbs_FORWARD);
	      aWES.AddStartElement (aSSF1);
	      aSSF1.Reverse();
	      aWES.AddStartElement (aSSF1);
	    }
	    else if (myOperation==XBOP_CUT) {
	      if (iRankF1==1) {
		aWES.AddStartElement (aSSF1);
	      }
	    }
	    else if (myOperation==XBOP_CUT21) {
	      if (iRankF1==2) {
		aWES.AddStartElement (aSSF1);
	      }
	    }
	    continue;
	  }
	}
	//iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
	//
	aSSF1.Orientation(anOrEF1);
	aWES.AddStartElement (aSSF1);
      }

      else {
	// Splits that are ON other Edge from other Face
	nSpF1=aPBEF1.Edge();
	nSpF2=aPBEF2.Edge();
	nEF2=aPBEF2.OriginalEdge();
	
	const TopoDS_Edge& anEF2=TopoDS::Edge(aDS.Shape(nEF2));
	anOrEF2=anEF2.Orientation();

	const TopoDS_Shape& aSpF1=aDS.Shape(nSpF1);
	const TopoDS_Shape& aSpF2=aDS.Shape(nSpF2);
	//
	// Pave Block from which new edge will be taken
	const XBOPTools_PaveBlock& aPB=aCB.PaveBlock1();
	nSpTaken=aPB.Edge();
	//
	//iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
	// Internal edges treatment
	{
	  Standard_Boolean bInternal1, bInternal2;
	  bInternal1=(anOrEF1==TopAbs_INTERNAL);
	  bInternal2=(anOrEF2==TopAbs_INTERNAL);
	  
	  if (bInternal1) {
	    aSSF1=TopoDS::Edge(aDS.Shape(nSpTaken));

	    if (myOperation==XBOP_FUSE) {
	      aSSF1.Orientation(TopAbs_FORWARD);
	      aWES.AddStartElement (aSSF1);
	      aSSF1.Reverse();
	      aWES.AddStartElement (aSSF1);
	      continue;
	    }

	    if (myOperation==XBOP_CUT && iRankF1==1) {
	      aSSF1.Orientation(TopAbs_INTERNAL);
	      aWES.AddStartElement (aSSF1);
	      continue;
	    }

	    if (myOperation==XBOP_CUT21 && iRankF1==2) {
	      aSSF1.Orientation(TopAbs_INTERNAL);
	      aWES.AddStartElement (aSSF1);
	      continue;
	    }
	  }
	  
	  else if (!bInternal1 && bInternal2) {
	    if (nSpTaken!=nSpF1) {
	      
	      if ((myOperation==XBOP_FUSE)||
		  (myOperation==XBOP_CUT && iRankF1==1) ||
		  (myOperation==XBOP_CUT21 && iRankF1==2)) { 
	      
		aSSF1=TopoDS::Edge(aSpF1);
		aSSF1.Orientation(anOrEF1);
		
		aSSF2=TopoDS::Edge(aSpF2);
	      
		aSSF2.Orientation(TopAbs_FORWARD);
		bToReverse=XBOPTools_Tools3D::IsSplitToReverse1 (aSSF1, aSSF2, aContext);
		if (bToReverse) {
		  aSSF2.Reverse();
		}
	      
		aWES.AddStartElement (aSSF2);
		continue;
	      }
	    }
	  }
	}
	//
	aSSF1=TopoDS::Edge(aSpF1);
	aSSF1.Orientation(anOrEF1);
	
	if (nSpTaken==nSpF1) {
	  // Common Edge is from nEF1
	  if(CheckSplitToAvoid(aSSF1, aCB, nEF1, nF1, myDSFiller, myOperation, aContext)){
	    continue;
	  }
	  aWES.AddStartElement (aSSF1);
	}
	
	else {
	  // Common Edge is from nEF2 nSpTaken!=nSpF2
	  aSSF2=TopoDS::Edge(aSpF2);
	  
	  bToReverse=XBOPTools_Tools3D::IsSplitToReverse1 (aSSF1, aSSF2, aContext);
	  if (bToReverse) {
	    aSSF2.Reverse();
	  }
	  //
	  if (BRep_Tool::IsClosed(aSSF1, myFace)) {
	    if (aM.Contains(aSSF2)){
	      continue;
	    }
	    aM.Add(aSSF2);
	    //
	    if (!BRep_Tool::IsClosed(aSSF2, myFace)) {
	      XBOPTools_Tools3D::DoSplitSEAMOnFace (aSSF2, myFace);
	    }

	    aWES.AddStartElement (aSSF2);
	    aSSF2.Reverse();
	    aWES.AddStartElement (aSSF2);
	    continue;  
	  }
	  //
	  if(CheckSplitToAvoid(aSSF2, aCB, nEF1, nF1, myDSFiller, myOperation, aContext)) {
	    continue;
	  }
	  aWES.AddStartElement (aSSF2);
	}
      }
    }
  }
}
//=======================================================================
// function: AddPartsEFSh
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::AddPartsEFSh (const Standard_Integer nF1, 
				     const Standard_Integer iFF,
				     TopTools_IndexedMapOfShape& anEMap,
				     XBOP_WireEdgeSet& aWES)
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  XBOPTools_InterferencePool* pIntrPool=(XBOPTools_InterferencePool*)&myDSFiller->InterfPool();
  XBOPTools_CArray1OfSSInterference& aFFs=pIntrPool->SSInterferences();
  
  const XBOPTools_PaveFiller& aPF=myDSFiller->PaveFiller();
  XBOPTools_PaveFiller* pPaveFiller=(XBOPTools_PaveFiller*)&aPF;
  XBOPTools_CommonBlockPool& aCBPool=pPaveFiller->ChangeCommonBlockPool();
  //
  Standard_Integer iRankF1, iRankF2, nF2, nSpEF2, nEF2,  nFace;
  TopExp_Explorer anExp2;
  TopAbs_Orientation anOrEF2;
  //
  XBOPTools_SSInterference& aFF=aFFs(iFF);
  nF2=aFF.OppositeIndex(nF1);
  //
  const TopoDS_Face& aF2=TopoDS::Face(aDS.Shape(nF2));
  //
  iRankF1=aDS.Rank(nF1);
  iRankF2=aDS.Rank(nF2);
  //
  // EF2\F1 Processing
  anExp2.Init (aF2, TopAbs_EDGE);
  for (; anExp2.More(); anExp2.Next()) {
    const TopoDS_Edge& aEF2= TopoDS::Edge(anExp2.Current());
    anOrEF2=aEF2.Orientation();

    nEF2=aDS.ShapeIndex (aEF2, iRankF2);

    XBOPTools_ListOfCommonBlock& aLCB=aCBPool(aDS.RefEdge(nEF2));
    
    XBOPTools_ListIteratorOfListOfCommonBlock anItCB(aLCB);
    for (; anItCB.More(); anItCB.Next()) {
      XBOPTools_CommonBlock& aCB=anItCB.Value();
      nFace=aCB.Face();
      if (nFace==nF1) {
	XBOPTools_PaveBlock& aPB=aCB.PaveBlock1(nEF2);

	nSpEF2=aPB.Edge();
	const TopoDS_Shape& aSpEF2=aDS.Shape(nSpEF2);
	
	if (anEMap.Contains(aSpEF2)) {
	  continue;// next CB
	}
	anEMap.Add(aSpEF2);
	
	TopoDS_Edge aSS=TopoDS::Edge(aSpEF2);
	//
	//
	//iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
	// Internal edges treatment
	{
	  if (anOrEF2==TopAbs_INTERNAL) {
	    aSS.Orientation(TopAbs_FORWARD);
	  }
	}
	//iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
	//
	if (myOperation== XBOP_FUSE) {
	  aWES.AddStartElement (aSS);
	  aSS.Reverse();
	  aWES.AddStartElement (aSS);
	}
      }
    } // next CB on nEF2
  }
}
//xf
//=======================================================================
// function: AddINON2DPartsSh
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::AddINON2DPartsSh(const Standard_Integer nF1,
					const Standard_Integer iFF,
					XBOP_WireEdgeSet& aWES)
{
  TopTools_IndexedMapOfShape anEMap;
  AddINON2DPartsSh(nF1, iFF, aWES, anEMap);
}
//xt
//=======================================================================
// function: AddINON2DPartsSh
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::AddINON2DPartsSh(const Standard_Integer nF1,
					const Standard_Integer iFF,
					XBOP_WireEdgeSet& aWES,
					TopTools_IndexedMapOfShape& anEMap) //xft
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  XBOPTools_InterferencePool* pIntrPool=(XBOPTools_InterferencePool*)&myDSFiller->InterfPool();
  XBOPTools_CArray1OfSSInterference& aFFs=pIntrPool->SSInterferences();
  //
  Standard_Integer iRankF1, nF2, iSenseFlag;

  iRankF1=aDS.Rank(nF1);
  
  XBOPTools_SSInterference& aFF=aFFs(iFF);
  nF2=aFF.OppositeIndex(nF1);
  //
  iSenseFlag=aFF.SenseFlag();
  //
  XBOP_SDFWESFiller aWESFiller;
  
  aWESFiller.SetDSFiller(*myDSFiller);
  aWESFiller.SetFaces(nF1, nF2);
  aWESFiller.SetStatesMap(aFF.StatesMap());
  //
  aWESFiller.SetSenseFlag(iSenseFlag);
 
  switch (myOperation) {
  
  case XBOP_FUSE: 
    if (myRank==2) {
      //shell\solid case when the solid is the first arg.
      iRankF1=1;
    }
    if (iRankF1==1) {
      aWESFiller.SetOperation(XBOP_CUT);	
      aWESFiller.Do(aWES);
      aWESFiller.SetOperation(XBOP_COMMON);	
      aWESFiller.Do(aWES);
    }
    else {
      aWESFiller.SetOperation(XBOP_CUT);	
      aWESFiller.Do(aWES);
    }
    break;
    
  case XBOP_COMMON:
    if (myRank==2) {
      //shell\solid case when the solid is the first arg.
      iRankF1=1;
    }
    if (iRankF1==1) {
      aWESFiller.SetOperation(XBOP_COMMON);	
      aWESFiller.Do(aWES);
    }
    break;
    
  case XBOP_CUT: 
    if (iRankF1==1) {
      aWESFiller.SetOperation(XBOP_CUT);	
      aWESFiller.Do(aWES);
    }
    break;
    
  case XBOP_CUT21: 
    if (iRankF1==2) {
      aWESFiller.SetOperation(XBOP_CUT);	
      aWESFiller.Do(aWES);
    }
    break;
    
  default:
    break;
  }
  //
  //xf
  // Collect all split edges of nF1 that are CB with 
  // splis of all SD faces to nFx,
  // but not included in aWES (RejectedOnParts).
  // This is necessary to prevent inclusion these splits in 
  // AddPartsEENonSDSh(...) 
  // see XBOP_SDFWESFiller,  XBOP_ShellSolid::DoNewFaces()
  //  for more details;
  TopTools_ListIteratorOfListOfShape aIt;
  //
  const TopTools_ListOfShape& aLRE=aWESFiller.RejectedOnParts();
  aIt.Initialize(aLRE);
  for(; aIt.More(); aIt.Next()) {
    const TopoDS_Shape& aE=aIt.Value();
    anEMap.Add(aE);
  }
  //xt
}
//=======================================================================
// function: AddPartsEFNonSDSh
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::AddPartsEFNonSDSh (const Standard_Integer nF1, 
					  const Standard_Integer iFF,
					  TopTools_IndexedMapOfShape& anEMap,
					  XBOP_WireEdgeSet& aWES)
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  XBOPTools_InterferencePool* pIntrPool=(XBOPTools_InterferencePool*)&myDSFiller->InterfPool();
  XBOPTools_CArray1OfSSInterference& aFFs=pIntrPool->SSInterferences();
  
  const XBOPTools_PaveFiller& aPF=myDSFiller->PaveFiller();
  XBOPTools_PaveFiller* pPaveFiller=(XBOPTools_PaveFiller*)&aPF;
  XBOPTools_CommonBlockPool& aCBPool=pPaveFiller->ChangeCommonBlockPool();
  //
  Standard_Integer nF2, nSpEF2, nEF2,  nFace, iRankF2;
  TopExp_Explorer anExp2;
  TopAbs_Orientation anOrEF2 = TopAbs_FORWARD;
  //
  XBOPTools_SSInterference& aFF=aFFs(iFF);
  nF2=aFF.OppositeIndex(nF1);
  //
  const TopoDS_Face& aF2=TopoDS::Face(aDS.Shape(nF2));

  iRankF2=aDS.Rank(nF2);
  //
  TopTools_IndexedMapOfOrientedShape aWESMap;
  {
    const TopTools_ListOfShape& aWESList=aWES.StartElements();
    TopTools_ListIteratorOfListOfShape anIt(aWESList);
    for (; anIt.More(); anIt.Next()) {
      const TopoDS_Shape& aS=anIt.Value();
      aWESMap.Add(aS);
    }
  }
  //
  // EF2\F1 Processing
  anExp2.Init (aF2, TopAbs_EDGE);
  for (; anExp2.More(); anExp2.Next()) {
    const TopoDS_Edge& aEF2= TopoDS::Edge(anExp2.Current());

    nEF2=aDS.ShapeIndex(aEF2, iRankF2);

    XBOPTools_ListOfCommonBlock& aLCB=aCBPool(aDS.RefEdge(nEF2));
    
    XBOPTools_ListIteratorOfListOfCommonBlock anItCB(aLCB);
    for (; anItCB.More(); anItCB.Next()) {
      XBOPTools_CommonBlock& aCB=anItCB.Value();
      nFace=aCB.Face();
      
      if (nFace==nF1) {
	XBOPTools_PaveBlock& aPB=aCB.PaveBlock1(nEF2);

	nSpEF2=aPB.Edge();
	const TopoDS_Shape& aSpEF2=aDS.Shape(nSpEF2);
	//
	if (anEMap.Contains(aSpEF2)) {
	  continue;// next CB
	}
	anEMap.Add(aSpEF2);
	//
	if (aWESMap.Contains(aSpEF2)) {
	  continue;// next CB
	}
	aWESMap.Add(aSpEF2);
	//
	TopoDS_Edge aSS=TopoDS::Edge(aSpEF2);
	//
	//
	//iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
	// Internal edges treatment
	{
	  if (anOrEF2==TopAbs_INTERNAL) {
	    aSS.Orientation(TopAbs_FORWARD);
	  }
	}
	//iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
	//
	if (myOperation==XBOP_FUSE) {
	  aWES.AddStartElement(aSS);
	  aSS.Reverse();
	  aWES.AddStartElement(aSS);
	}
	//
      } //if (nFace==nF1) {
    } // next CB on nEF2
  }
}
//=======================================================================
// function: AddPartsEENonSDSh
// purpose: 
//=======================================================================
  void XBOP_ShellSolid::AddPartsEENonSDSh (const Standard_Integer nF1, 
					  const Standard_Integer iFF,
					  TopTools_IndexedMapOfShape& anEMap,
					  XBOP_WireEdgeSet& aWES)
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  XBOPTools_InterferencePool* pIntrPool=(XBOPTools_InterferencePool*)&myDSFiller->InterfPool();
  XBOPTools_CArray1OfSSInterference& aFFs=pIntrPool->SSInterferences();
  
  const XBOPTools_PaveFiller& aPF=myDSFiller->PaveFiller();
  XBOPTools_PaveFiller* pPaveFiller=(XBOPTools_PaveFiller*)&aPF;
  XBOPTools_CommonBlockPool& aCBPool=pPaveFiller->ChangeCommonBlockPool();
  //
  const Handle(XIntTools_Context)& aContext=pPaveFiller->Context();
  //
  Standard_Integer nEF1, nF2, nSpF1, nSpF2, nEF2, nSpTaken, nF2x, iRankF1;
  Standard_Boolean bToReverse;
  TopAbs_Orientation anOrEF1, anOrEF2;
  TopExp_Explorer anExp;
  TopTools_IndexedMapOfShape aM;
  TColStd_ListOfInteger aSplitsOnF1;
  TColStd_ListIteratorOfListOfInteger anItSp;
  TColStd_IndexedMapOfInteger aMSplitsOnF1;
  TopoDS_Edge aSSF1, aSSF2;
  //
  // nF1
  iRankF1=aDS.Rank(nF1);
  //
  // nF2
  XBOPTools_SSInterference& aFF=aFFs(iFF);
  nF2=aFF.OppositeIndex(nF1);
  //
  pPaveFiller->SplitsOnFace(0, nF1, nF2, aSplitsOnF1);
  anItSp.Initialize(aSplitsOnF1);
  for (; anItSp.More(); anItSp.Next()) {
    nSpF1=anItSp.Value();
    aMSplitsOnF1.Add(nSpF1);
  }
  //
  TopTools_IndexedMapOfOrientedShape aWESMap;
  {
    const TopTools_ListOfShape& aWESList=aWES.StartElements();
    TopTools_ListIteratorOfListOfShape anIt(aWESList);
    for (; anIt.More(); anIt.Next()) {
      const TopoDS_Shape& aS=anIt.Value();
      aWESMap.Add(aS);
    }
  }
  //
  anExp.Init(myFace, TopAbs_EDGE);
  for (; anExp.More(); anExp.Next()) {
    const TopoDS_Edge& anEF1=TopoDS::Edge(anExp.Current());
    anOrEF1=anEF1.Orientation();
    nEF1=aDS.ShapeIndex(anEF1, iRankF1);
    
    XBOPTools_ListOfCommonBlock& aLCB=aCBPool(aDS.RefEdge(nEF1));
    
    XBOPTools_ListIteratorOfListOfCommonBlock anItCB(aLCB);
    for (; anItCB.More(); anItCB.Next()) {
      XBOPTools_CommonBlock& aCB=anItCB.Value();
    
      XBOPTools_PaveBlock& aPBEF1=aCB.PaveBlock1(nEF1);
      XBOPTools_PaveBlock& aPBEF2=aCB.PaveBlock2(nEF1);
      
      nF2x=aCB.Face();
      if (nF2x) {
	continue;
      }
      // Splits that are ON other Edge from other Face
      nSpF1=aPBEF1.Edge();
      //
      if (!aMSplitsOnF1.Contains(nSpF1)) {
	continue;// next CB
      }
      //
      nSpF2=aPBEF2.Edge();
      nEF2=aPBEF2.OriginalEdge();

      const TopoDS_Edge& anEF2=TopoDS::Edge(aDS.Shape(nEF2));
      anOrEF2=anEF2.Orientation();
      
      const TopoDS_Shape& aSpF1=aDS.Shape(nSpF1);
      const TopoDS_Shape& aSpF2=aDS.Shape(nSpF2);
      
      //
      if (anEMap.Contains(aSpF1)) {
	continue;// next CB
      }
      anEMap.Add(aSpF1);
      //
      if (anEMap.Contains(aSpF2)) {
	continue;// next CB
      }
      anEMap.Add(aSpF2);
      //
      
      // Pave Block from which new edge will be taken
      const XBOPTools_PaveBlock& aPB=aCB.PaveBlock1();
      nSpTaken=aPB.Edge();
      //
      //iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
      // Internal edges treatment
      {
	Standard_Boolean bInternal1, bInternal2;
	bInternal1=(anOrEF1==TopAbs_INTERNAL);
	bInternal2=(anOrEF2==TopAbs_INTERNAL);
	
	if (bInternal1) {
	  aSSF1=TopoDS::Edge(aDS.Shape(nSpTaken));
	  
	  if (myOperation==XBOP_FUSE) {
	    aSSF1.Orientation(TopAbs_FORWARD);
	    aWES.AddStartElement (aSSF1);
	    aSSF1.Reverse();
	    aWES.AddStartElement (aSSF1);
	    continue;
	  }
	  
	  if (myOperation==XBOP_CUT && iRankF1==1) {
	    aSSF1.Orientation(TopAbs_INTERNAL);
	    aWES.AddStartElement (aSSF1);
	    continue;
	  }
	  
	  if (myOperation==XBOP_CUT21 && iRankF1==2) {
	    aSSF1.Orientation(TopAbs_INTERNAL);
	    aWES.AddStartElement (aSSF1);
	    continue;
	  }
	}
	  
	else if (!bInternal1 && bInternal2) {
	  if (nSpTaken!=nSpF1) {
	    
	    if ((myOperation==XBOP_FUSE)||
		(myOperation==XBOP_CUT && iRankF1==1) ||
		(myOperation==XBOP_CUT21 && iRankF1==2)) { 
	      
	      aSSF1=TopoDS::Edge(aSpF1);
	      aSSF1.Orientation(anOrEF1);
	      
	      aSSF2=TopoDS::Edge(aSpF2);
	      
	      aSSF2.Orientation(TopAbs_FORWARD);
	      bToReverse=XBOPTools_Tools3D::IsSplitToReverse1 (aSSF1, aSSF2, aContext);
	      if (bToReverse) {
		aSSF2.Reverse();
	      }
	      
	      aWES.AddStartElement (aSSF2);
	      continue;
	    }
	  }
	}
      }
      //iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
      //
      aSSF1=TopoDS::Edge(aSpF1);
      aSSF1.Orientation(anOrEF1);
      
      if (nSpTaken==nSpF1) {
	// Common Edge is from nEF1
	aWES.AddStartElement (aSSF1);
      }
      else  {
	// Common Edge is from nEF2 nSpTaken!=nSpF2
	aSSF2=TopoDS::Edge(aSpF2);
	
	bToReverse=XBOPTools_Tools3D::IsSplitToReverse1 (aSSF1, aSSF2, aContext);
	if (bToReverse) {
	  aSSF2.Reverse();
	}
	//
	if (BRep_Tool::IsClosed(aSSF1, myFace)) {
	  if (aM.Contains(aSSF2)){
	    continue;
	  }
	  aM.Add(aSSF2);
	  //
	  if (!BRep_Tool::IsClosed(aSSF2, myFace)) {
	    XBOPTools_Tools3D::DoSplitSEAMOnFace (aSSF2, myFace);
	  }
	  aWES.AddStartElement (aSSF2);
	  aSSF2.Reverse();
	  aWES.AddStartElement (aSSF2);
	  continue;  
	}
	//
	aWES.AddStartElement (aSSF2);
      }// else /*if (nSpTaken==nSpF2)*/ {
    }// for (; anItCB.More(); anItCB.Next())
  }// for (; anExp.More(); anExp.Next())
}
//=======================================================================
//function : CheckSplitToAvoid
//purpose  : 
//=======================================================================
Standard_Boolean CheckSplitToAvoid(const TopoDS_Edge&          theSplit,
				   const XBOPTools_CommonBlock& theCB, 
				   const Standard_Integer      theEdgeIndex,
				   const Standard_Integer      theFaceIndex,
				   const XBOPTools_PDSFiller&   theDSFiller, 
				   const XBOP_Operation&        theOperation,
				   const Handle(XIntTools_Context)&  theContext) {

  Standard_Integer anE = -1;

  if(theCB.PaveBlock1().OriginalEdge() == theEdgeIndex) {
    anE = theCB.PaveBlock2().OriginalEdge();
  }
  else if(theCB.PaveBlock2().OriginalEdge() == theEdgeIndex) {
    anE = theCB.PaveBlock1().OriginalEdge();
  }

  if(anE >= 0) {
    const TopoDS_Shape& anEdge = theDSFiller->DS().Shape(anE);
    TopoDS_Face aFaceCur = TopoDS::Face(theDSFiller->DS().Shape(theFaceIndex));
    aFaceCur.Orientation(TopAbs_FORWARD);

    TopTools_IndexedDataMapOfShapeListOfShape aMapEF;
    Standard_Integer aRank = theDSFiller->DS().Rank(anE);
    TopoDS_Shape aSource = (aRank == 1) ? theDSFiller->Shape1() : theDSFiller->Shape2();
    TopExp::MapShapesAndAncestors(aSource, TopAbs_EDGE, TopAbs_FACE, aMapEF);

    if(aMapEF.Contains(anEdge)) {
      const TopTools_ListOfShape& aLF = aMapEF.FindFromKey(anEdge);

      if(!aLF.IsEmpty()) {
	TopTools_ListIteratorOfListOfShape anIt(aLF);
	Standard_Boolean avoid = Standard_True;

	for(; anIt.More(); anIt.Next()) {
	  const TopoDS_Face& aFace = TopoDS::Face(anIt.Value());
	  Standard_Real f = 0., l = 0.;
	  Handle(Geom2d_Curve) aCurve = BRep_Tool::CurveOnSurface(theSplit, aFaceCur, f, l);

	  if(!aCurve.IsNull()) {
	    Standard_Real amidpar = (f + l) * 0.5;

	    if(theOperation == XBOP_COMMON) {
	      gp_Pnt2d aPoint2d;
	      gp_Pnt aPoint3d;
	      Standard_Real aTolerance = BRep_Tool::Tolerance(theSplit); //???
	      XBOPTools_Tools3D::PointNearEdge(theSplit, aFaceCur, amidpar, aTolerance, aPoint2d, aPoint3d);
	      GeomAPI_ProjectPointOnSurf& aProjector =  theContext->ProjPS(aFace);
	      aProjector.Perform(aPoint3d);

	      if(aProjector.IsDone()) {
		Standard_Real U = 0., V = 0.;
		Standard_Real adist = aProjector.LowerDistance();

		if(adist < BRep_Tool::Tolerance(aFace)) {
		  aProjector.LowerDistanceParameters(U, V);

		  if(theContext->IsPointInFace(aFace, gp_Pnt2d(U, V))) {
		    avoid = Standard_False;
		    break;
		  }
		  else {
		  }
		}
	      }
	    }
	    else if(theOperation == XBOP_CUT) {
	      if(theDSFiller->DS().Rank(theFaceIndex) != 2) {
		avoid = Standard_False;
		continue;
	      }
	      gp_Pnt2d aPoint2d;
	      gp_Pnt aPoint3d;
	      Standard_Real aTolerance = BRep_Tool::Tolerance(theSplit); //???
	      XBOPTools_Tools3D::PointNearEdge(theSplit, aFaceCur, amidpar, aTolerance, aPoint2d, aPoint3d);
	      GeomAPI_ProjectPointOnSurf& aProjector =  theContext->ProjPS(aFace);
	      aProjector.Perform(aPoint3d);

	      if(aProjector.IsDone()) {
		Standard_Real U = 0., V = 0.;
		Standard_Real adist = aProjector.LowerDistance();

		if(adist < BRep_Tool::Tolerance(aFace)) {
		  aProjector.LowerDistanceParameters(U, V);

		  if(theContext->IsPointInFace(aFace, gp_Pnt2d(U, V))) {
		    avoid = Standard_False;
		    break;
		  }
		  else {
		  }
		}
	      }
	    }
	    else if(theOperation == XBOP_CUT21) {
	      if(theDSFiller->DS().Rank(theFaceIndex) != 1) {
		avoid = Standard_False;
		continue;
	      }
	      gp_Pnt2d aPoint2d;
	      gp_Pnt aPoint3d;
	      Standard_Real aTolerance = BRep_Tool::Tolerance(theSplit); //???
	      XBOPTools_Tools3D::PointNearEdge(theSplit, aFaceCur, amidpar, aTolerance, aPoint2d, aPoint3d);
	      GeomAPI_ProjectPointOnSurf& aProjector =  theContext->ProjPS(aFace);
	      aProjector.Perform(aPoint3d);

	      if(aProjector.IsDone()) {
		Standard_Real U = 0., V = 0.;
		Standard_Real adist = aProjector.LowerDistance();

		if(adist < BRep_Tool::Tolerance(aFace)) {
		  aProjector.LowerDistanceParameters(U, V);

		  if(theContext->IsPointInFace(aFace, gp_Pnt2d(U, V))) {
		    avoid = Standard_False;
		    break;
		  }
		  else {
		  }
		}
	      }
	    }
	    // end if(theOperation == XBOP_CUT21...
	    else {
	      avoid = Standard_False;
	      break;
	    }
	  }
	}

	if(avoid) {
	  return Standard_True;
	}
      }
      // end if(!aLF.IsEmpty...
    }
  }

  return Standard_False;
}
