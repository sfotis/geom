// Created on: 2001-03-13
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



#include <XBOPTools_PaveFiller.ixx>

#include <Precision.hxx>
#include <SortTools_QuickSortOfReal.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TCollection_CompareOfReal.hxx>
#include <BRep_Tool.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>

#include <XIntTools_Tools.hxx>
#include <XIntTools_EdgeEdge.hxx>
#include <XIntTools_Range.hxx>
#include <XIntTools_ShrunkRange.hxx>
#include <XIntTools_EdgeFace.hxx>
#include <XIntTools_SequenceOfCommonPrts.hxx>
#include <XIntTools_CommonPrt.hxx>
#include <XIntTools_SequenceOfRanges.hxx>

#include <XBooleanOperations_ShapesDataStructure.hxx>
#include <XBooleanOperations_AncestorsSeqAndSuccessorsSeq.hxx>
#include <XBooleanOperations_OnceExplorer.hxx>

#include <XBOPTools_InterferencePool.hxx>
#include <XBOPTools_IteratorOfCoupleOfShape.hxx>
#include <XBOPTools_VVInterference.hxx>
#include <XBOPTools_VEInterference.hxx>
#include <XBOPTools_EEInterference.hxx>
#include <XBOPTools_CArray1OfVEInterference.hxx>
#include <XBOPTools_CArray1OfVVInterference.hxx>
#include <XBOPTools_CArray1OfEEInterference.hxx>
#include <XBOPTools_Tools.hxx>
#include <XBOPTools_Pave.hxx>
#include <XBOPTools_PaveSet.hxx>
#include <XBOPTools_PaveBlockIterator.hxx>
#include <XBOPTools_ListOfPave.hxx>
#include <XBOPTools_ListIteratorOfListOfPave.hxx>
#include <XBOPTools_ListOfPaveBlock.hxx>
#include <XBOPTools_ListIteratorOfListOfPaveBlock.hxx>
#include <XBOPTools_CommonBlock.hxx>
#include <XBOPTools_ListOfCommonBlock.hxx>
#include <XBOPTools_ListIteratorOfListOfCommonBlock.hxx>
#include <XBOPTools_ESInterference.hxx>
#include <XBOPTools_CommonBlockAPI.hxx>
#include <XBOPTools_PaveBlock.hxx>
#include <XBOPTools_ListIteratorOfListOfInterference.hxx>
#include <XBOPTools_Interference.hxx>
#include <XBOPTools_InterferenceLine.hxx>
#include <XBOPTools_CArray1OfInterferenceLine.hxx>

#include <XBOPTColStd_Failure.hxx>

#include <stdio.h>

//=======================================================================
// function: PerformEF
// purpose: 
//=======================================================================
void XBOPTools_PaveFiller::PerformEF() 
{
  myIsDone=Standard_False;
  
  Standard_Integer n1, n2, anIndexIn=0, nE, nF, aNbEFs, aBlockLength;

  XBOPTools_CArray1OfESInterference& aEFs=myIntrPool->ESInterferences();
  //
  // E/F Interferences  [XBooleanOperations_EdgeFace]
  myDSIt.Initialize(TopAbs_EDGE, TopAbs_FACE);
  //
  // BlockLength correction
  aNbEFs=ExpectedPoolLength();
  aBlockLength=aEFs.BlockLength();
  if (aNbEFs > aBlockLength) {
    aEFs.SetBlockLength(aNbEFs);
  }
  //
  for (; myDSIt.More(); myDSIt.Next()) {
    Standard_Boolean justaddinterference = Standard_True;
    myDSIt.Current(n1, n2, justaddinterference);
    
    if(justaddinterference) {
      if (!myIntrPool->IsComputed(n1, n2)) {
	anIndexIn = 0;
	nE=n1; 
	nF=n2; 
	SortTypes(nE, nF);
	myIntrPool->AddInterference (nE, nF, XBooleanOperations_EdgeSurface, anIndexIn);
      }
      continue;
    }
    //
    if (myIntrPool->IsComputed(n1, n2)) {
      continue;
    }
    //
    nE=n1; 
    nF=n2; 
    SortTypes(nE, nF);
    //
    // ListOfCommonBlock for nE and CommonBlockAPI object to avoid
    // CommonBlocks processing
    XBOPTools_ListOfCommonBlock& aLCB=myCommonBlockPool(myDS->RefEdge(nE));
    XBOPTools_CommonBlockAPI aCBAPI(aLCB);
    //
    Standard_Real aTolE, aTolF, aDeflection=0.01;
    Standard_Integer aDiscretize=35;

    // modified by NIZHNY-MKK  Thu Oct 27 12:31:45 2005
//     const TopoDS_Edge& aE=TopoDS::Edge(myDS->GetShape(nE));
    TopoDS_Edge aE=TopoDS::Edge(myDS->GetShape(nE));
    //
    if (BRep_Tool::Degenerated(aE)){
      continue;
    }
    //
    aTolE=BRep_Tool::Tolerance(aE);

    // modified by NIZHNY-MKK  Thu Oct 27 12:31:52 2005
//     const TopoDS_Face& aF=TopoDS::Face(myDS->GetShape(nF));
    TopoDS_Face aF=TopoDS::Face(myDS->GetShape(nF));
    aTolF=BRep_Tool::Tolerance(aF);
// modified by NIZHNY-MKK  Thu Oct 27 12:32:02 2005
//     const Bnd_Box& aBBF=myDS->GetBoundingBox(nF); 
    Bnd_Box aBBF=myDS->GetBoundingBox(nF); 
    //
    XBOPTools_ListOfPaveBlock& aLPB=mySplitShapesPool(myDS->RefEdge(nE));
    XBOPTools_ListIteratorOfListOfPaveBlock anIt(aLPB);

    for (; anIt.More(); anIt.Next()) {

      XBOPTools_PaveBlock& aPB=anIt.Value();

      if (aCBAPI.IsCommonBlock(aPB)) {
	// avoid CommonBlocks processing
	continue;
      }
      
      const XIntTools_ShrunkRange& aShrunkRange=aPB.ShrunkRange();
      const XIntTools_Range& aSR =aShrunkRange.ShrunkRange();
      const Bnd_Box&        aBBE=aShrunkRange.BndBox();

      //////////////////////////////////////////////
      if (aBBF.IsOut (aBBE)) {
	continue;
      }
      // 
      // EF
      XIntTools_EdgeFace aEF;
      aEF.SetEdge (aE);
      aEF.SetFace (aF);
      aEF.SetTolE (aTolE);
      aEF.SetTolF (aTolF);
      aEF.SetDiscretize (aDiscretize);
      aEF.SetDeflection (aDeflection);
      // 
      aEF.SetContext(myContext);
      // 

      BRepAdaptor_Curve aCurve(aE);
      XIntTools_Range anewSR = aSR;
      // 
      // Correction of the Shrunk Range 
      //
      XBOPTools_Tools::CorrectRange(aE, aF, aSR, anewSR);
      //
      aEF.SetRange (anewSR);
      //
      aEF.Perform();
      //
      anIndexIn=0;
      //
      if (aEF.IsDone()) {
	//
	const XIntTools_SequenceOfCommonPrts& aCPrts=aEF.CommonParts();
	Standard_Integer i, aNbCPrts;
	aNbCPrts=aCPrts.Length();

	// modified by NIZHNY-MKK  Thu Aug 26 17:37:07 2004.BEGIN
	if(aNbCPrts != 0) {
	  char buf[512];

	  if(!aShrunkRange.IsDone()) {
	    sprintf (buf, "Can not obtain ShrunkRange for Edge %d", nE);
	    throw XBOPTColStd_Failure(buf) ;
	  }
	}
	// modified by NIZHNY-MKK  Thu Aug 26 17:37:09 2004.END

	for (i=1; i<=aNbCPrts; i++) {
	  const XIntTools_CommonPrt& aCPart=aCPrts.Value(i);
	  //
	  anIndexIn=0;
	  //
	  TopAbs_ShapeEnum aType=aCPart.Type();
	  switch (aType) {
	      
	    case TopAbs_VERTEX:  {
		
	      Standard_Real aT; 
	      Standard_Integer aNewShape;
	      
	      const XIntTools_Range& aR=aCPart.Range1();

	      Standard_Real aRFirst, aRLast;

	      aR.Range(aRFirst, aRLast);
	      aT=0.5*(aRFirst+aRLast);

	      if((aCPart.VertexParameter1() >= aRFirst) &&
		 (aCPart.VertexParameter1() <= aRLast)) {
		aT = aCPart.VertexParameter1();
	      }
	      
	      TopoDS_Vertex aNewVertex;
	      XBOPTools_Tools::MakeNewVertex(aE, aT, aF,aNewVertex);
	      //
	      Standard_Boolean isonpave1, isonpave2;
	      Standard_Real aTolToDecide;
	      aTolToDecide=5.e-8;

	      // modified by NIZHNY-IFV  Feb 15   2007
	      // for occ11565
	      //Check that whole edge belongs face
	      isonpave1 = (Abs(anewSR.First() - aRFirst) < aTolToDecide);
	      isonpave2 = (Abs(anewSR.Last()  - aRLast)  < aTolToDecide);
	      //++
	      if(isonpave1 && isonpave2) {
		//Seems whole edge belongs face
		
		Standard_Boolean aCoinsideFlag = 
		  XBOPTools_Tools::IsBlockInOnFace(aPB, aF, myContext);
		if (aCoinsideFlag) {
		//
		  // Add Interference to the Pool
		  
		  XIntTools_CommonPrt aCP = aCPart;
		  aCP.SetType(TopAbs_EDGE);
		  XBOPTools_ESInterference anInterf (nE, nF, aCP);
		  anIndexIn=aEFs.Append(anInterf);
		  myIntrPool->AddInterference (nE, nF, XBooleanOperations_EdgeSurface, anIndexIn);
		  //
		  XBOPTools_CommonBlock aCB(aPB, nF);
	      
		  aLCB.Append(aCB);
		  break;
		}
	      }
	      // modified by NIZHNY-IFV  Feb 15   2007. end
	      //
	      //decide to add pave or not.begin
	      isonpave1  = (Abs(anewSR.First() - aT) < aTolToDecide);
	      isonpave2  = (Abs(anewSR.Last()  - aT) < aTolToDecide);

	      // modified by NIZHNY-MKK  Wed Nov  6 14:43:07 2002.BEGIN
	      if(!isonpave1 && !isonpave2) {
		isonpave1 = (Abs(anewSR.First() - aR.First()) < aTolToDecide);
		isonpave2 = (Abs(anewSR.Last()  - aR.Last())  < aTolToDecide);
	      }
	      // modified by NIZHNY-MKK  Wed Nov  6 14:43:11 2002.END
	      //++
	      {
		Standard_Integer nVP1, nVP2;
		Standard_Boolean bIsComputed;
		
		// Pave 1
		const XBOPTools_Pave& aPave1=aPB.Pave1();
		nVP1=aPave1.Index();

		bIsComputed=Standard_True;
		if (nVP1 <= myNbSources) {

		  if(((nVP1 <= myDS->NumberOfShapesOfTheObject()) &&
		      (nF <= myDS->NumberOfShapesOfTheObject())) ||
		     ((nVP1 > myDS->NumberOfShapesOfTheObject()) &&
		      (nF > myDS->NumberOfShapesOfTheObject()))) {
		    bIsComputed=myIntrPool->IsComputed(nVP1, nE);

		    if (!bIsComputed) {
		      bIsComputed=IsSuccesstorsComputed(nVP1, nE);
		    }
		  }
		  else {
		    bIsComputed=myIntrPool->IsComputed(nVP1, nF);
		    if (!bIsComputed) {
		      bIsComputed=IsSuccesstorsComputed(nVP1, nF);
		    }
		  }
		}

		if (bIsComputed && isonpave1) {
		  //
		  myIntrPool->AddInterference (nE, nF, XBooleanOperations_EdgeSurface, anIndexIn);
		  const TopoDS_Vertex& aVF = TopoDS::Vertex(myDS->Shape(nVP1));
		  XBOPTools_Tools::UpdateVertex(aE, aT, aVF);
		  continue;
		}
		// Pave 2
		const XBOPTools_Pave& aPave2=aPB.Pave2();
		nVP2=aPave2.Index();

		bIsComputed=Standard_True;
		if (nVP2 <= myNbSources) {

		  if((nVP2 <= myDS->NumberOfShapesOfTheObject() &&
		     nF <= myDS->NumberOfShapesOfTheObject()) ||
		     (nVP2 > myDS->NumberOfShapesOfTheObject() &&
		      nF > myDS->NumberOfShapesOfTheObject())) {
		    bIsComputed=myIntrPool->IsComputed(nVP2, nE);

		    if (!bIsComputed) {
		      bIsComputed=IsSuccesstorsComputed(nVP2, nE);
		    }
		  }
		  else {
		    bIsComputed=myIntrPool->IsComputed(nVP2, nF);
		    if (!bIsComputed) {
		      bIsComputed=IsSuccesstorsComputed(nVP2, nF);
		    }
		  }
		}
		
		if (bIsComputed && isonpave2) {
		  //
		  myIntrPool->AddInterference (nE, nF, XBooleanOperations_EdgeSurface, anIndexIn);
		  const TopoDS_Vertex& aVF = TopoDS::Vertex(myDS->Shape(nVP2));
		  XBOPTools_Tools::UpdateVertex(aE, aT, aVF);
		  continue;
		}
	      }	      
	      //decide to add pave or not.end
	      //
	      // Insert New Vertex in DS;
	      // aNewShape is # of DS-line, where aNewVertex is kept
	      //
	      XBooleanOperations_AncestorsSeqAndSuccessorsSeq anASSeq; 
	      //
	      Standard_Integer nVF, iFlag;
	      nVF=CheckFacePaves(aNewVertex, nF);
	      if (nVF) {
		const TopoDS_Vertex& aVF=TopoDS::Vertex(myDS->Shape(nVF));
		
		Standard_Real aTNew;
		//
		iFlag=myContext->ComputeVE(aVF, aE, aTNew);
		//
		aT=aTNew;
		aNewShape=nVF;
		XBOPTools_Tools::UpdateVertex(aVF, aNewVertex);
	      }
	      //
	      else {
		myDS->InsertShapeAndAncestorsSuccessors(aNewVertex, anASSeq);
		aNewShape=myDS->NumberOfInsertedShapes();
	      }
	      //
	      // Add Interference to the Pool
	      XBOPTools_ESInterference anInterf (nE, nF, aCPart);
	      anIndexIn=aEFs.Append(anInterf);
	      myIntrPool->AddInterference (nE, nF, XBooleanOperations_EdgeSurface, anIndexIn);
	      //
	      // State of the new Vertex is ON
	      myDS->SetState (aNewShape, XBooleanOperations_ON);
	      //
	      // Insert New Vertex in EE Interference
	      XBOPTools_ESInterference& aESInterf= aEFs(anIndexIn);
	      aESInterf.SetNewShape(aNewShape);
	      //
	      // Add Pave to the myPavePoolNew
	      XBOPTools_Pave aPave;
	      aPave.SetInterference(anIndexIn);
	      aPave.SetType (XBooleanOperations_EdgeSurface);
	      aPave.SetIndex(aNewShape);
	      
	      aPave.SetParam(aT);
	      XBOPTools_PaveSet& aPaveSet=myPavePoolNew(myDS->RefEdge(nE));
	      aPaveSet.Append(aPave);
	      
	    }// case TopAbs_VERTEX:
	      break;

	    case TopAbs_EDGE: {
	      
	      Standard_Boolean aCoinsideFlag;
	      aCoinsideFlag=XBOPTools_Tools::IsBlockInOnFace(aPB, aF, myContext);
	      if (!aCoinsideFlag) {
		//
		myIntrPool->AddInterference (nE, nF, XBooleanOperations_EdgeSurface, anIndexIn);
		break;
	      }
	      // Add Interference to the Pool
	      XBOPTools_ESInterference anInterf (nE, nF, aCPart);
	      anIndexIn=aEFs.Append(anInterf);
	      myIntrPool->AddInterference (nE, nF, XBooleanOperations_EdgeSurface, anIndexIn);
	      //
	      XBOPTools_CommonBlock aCB(aPB, nF);
	      
	      aLCB.Append(aCB);
	      
	    }// case TopAbs_EDGE:
	      break;

	    default:
	      break;
	  } // switch (aType) 
	} // for (i=1; i<=aNbCPrts; i++) 
      } //if (aEF.IsDone())
    } // for (; anIt.More(); anIt.Next()) 
  }// for (; myDSIt.More(); myDSIt.Next()) 
  myIsDone=Standard_True;
}


//=======================================================================
// function: DoSDEdges
// purpose: 
//=======================================================================
  void XBOPTools_PaveFiller::DoSDEdges() 
{
  myIsDone=Standard_False;

  Standard_Integer aNbObj, nEObj, nETool, aSplitIndexObj, aSplitIndexTool;
  TopAbs_ShapeEnum aType;

  aNbObj=myDS->NumberOfShapesOfTheObject();
  
  for (nEObj=1; nEObj<=aNbObj; nEObj++) {
    // modified by NIZHNY-MKK  Thu Oct 27 12:32:37 2005
//     const TopoDS_Shape& aS=myDS->GetShape(nEObj);
    TopoDS_Shape aS=myDS->GetShape(nEObj);
    aType=aS.ShapeType();
    if (aType==TopAbs_EDGE) {
      //ZZ const TopoDS_Edge& aE=TopoDS::Edge(myDS->GetShape(nEObj));
      
      XBOPTools_ListOfCommonBlock& aLCBObj=myCommonBlockPool(myDS->RefEdge(nEObj));
      XBOPTools_ListIteratorOfListOfCommonBlock anItCBObj(aLCBObj);
      for (; anItCBObj.More(); anItCBObj.Next()) {
	XBOPTools_CommonBlock& aCBObj=anItCBObj.Value();
	
	if (aCBObj.Face()) {
	  continue;
	}
	
	XBOPTools_PaveBlock& aPBObj1=aCBObj.PaveBlock1(nEObj);
	XBOPTools_PaveBlock& aPBObj2=aCBObj.PaveBlock2(nEObj);
	//
	// Tool
	nETool=aPBObj2.OriginalEdge();

	XBOPTools_ListOfCommonBlock& aLCBTool=myCommonBlockPool(myDS->RefEdge(nETool));
	XBOPTools_ListIteratorOfListOfCommonBlock anItCBTool(aLCBTool);
	for (; anItCBTool.More(); anItCBTool.Next()) {
	  XBOPTools_CommonBlock& aCBTool=anItCBTool.Value();
	  
	  XBOPTools_PaveBlock& aPBTool1=aCBTool.PaveBlock1(nETool);
	  XBOPTools_PaveBlock& aPBTool2=aCBTool.PaveBlock2(nETool);
	  
	  if (aPBTool1.IsEqual(aPBObj2) && aPBTool2.IsEqual(aPBObj1)) {
	    //
	    // That is aCBObj==aCBTool
	    // Find SplitIndices in DS for aPBObj1 and aPBTool1
	    aSplitIndexObj =SplitIndex(aPBObj1);
	    aSplitIndexTool=SplitIndex(aPBTool1);
	    if (aSplitIndexObj && aSplitIndexTool) {
	      
	      aPBObj1. SetEdge(aSplitIndexObj);
	      aPBObj2. SetEdge(aSplitIndexTool);
	      aPBTool1.SetEdge(aSplitIndexTool);
	      aPBTool2.SetEdge(aSplitIndexObj);
	      break;
	    }
	    else {
	      //error: can not find SplitIndex
	      throw 
		XBOPTColStd_Failure(" XBOPTools_PaveFiller::DoSDEdges()=> can not find SplitIndex");
	      
	      myIsDone=Standard_False;
	      return;
	    }
	    
	  } // if (aPBTool1.IsEqual(aPBObj2) && aPBTool2.IsEqual(aPBObj1)) 
	} // for (; anItCBTool.More(); anItCBTool.Next()) 
      } // for (; anItCBObj.More(); anItCBObj.Next())
    } // if (aType==TopAbs_EDGE)
  } // for (nEObj=1; nEObj<=aNbObj; nEObj++)

  //
  // 2. EF CommonBlocks
  //
  Standard_Integer nE, aSplitIndex;
  for (nE=1; nE<=myNbSources; nE++) {
    // modified by NIZHNY-MKK  Thu Oct 27 12:32:57 2005
//     const TopoDS_Shape& aS=myDS->GetShape(nE);
    TopoDS_Shape aS=myDS->GetShape(nE);
    aType=aS.ShapeType();
    if (aType==TopAbs_EDGE) {
      //ZZ const TopoDS_Edge& aE=TopoDS::Edge(myDS->GetShape(nE));

      XBOPTools_ListOfCommonBlock& aLCB=myCommonBlockPool(myDS->RefEdge(nE));
      XBOPTools_ListIteratorOfListOfCommonBlock anItCB(aLCB);
      
      for (; anItCB.More(); anItCB.Next()) {
	XBOPTools_CommonBlock& aCB=anItCB.Value();
	if (aCB.Face()) {
	  XBOPTools_PaveBlock& aPB=aCB.PaveBlock1(nE);
	  aSplitIndex =SplitIndex(aPB);
	  if (aSplitIndex) {
	    aPB.SetEdge(aSplitIndex);
	    //break;
	  }
	  else {
	    //error: can not find SplitIndex
	    throw 
		XBOPTColStd_Failure(" XBOPTools_PaveFiller::DoSDEdges()=> can not find SplitIndex");
	    myIsDone=Standard_False;
	    return;
	  }
	}
      }
    }
  }
  myIsDone=Standard_True;
}

//=======================================================================
// function: SplitIndex
// purpose: 
//=======================================================================
  Standard_Integer XBOPTools_PaveFiller::SplitIndex(const XBOPTools_PaveBlock& aPBx) 
{
  Standard_Integer anOriginalEdge, anEdgeIndex=0;

  anOriginalEdge=aPBx.OriginalEdge();

  XBOPTools_ListOfPaveBlock& aLPB=mySplitShapesPool(myDS->RefEdge(anOriginalEdge));
  XBOPTools_ListIteratorOfListOfPaveBlock anIt(aLPB);

  for (; anIt.More(); anIt.Next()) {
    XBOPTools_PaveBlock& aPB=anIt.Value();
    if (aPB.IsEqual(aPBx)) {
      anEdgeIndex=aPB.Edge();
      return anEdgeIndex;
    }
  }
  return anEdgeIndex;
} 
//=======================================================================
// function: CheckFacePaves
// purpose: 
//=======================================================================
  Standard_Integer XBOPTools_PaveFiller::CheckFacePaves (const TopoDS_Vertex& aNewVertex,
							const Standard_Integer nF)
{
  Standard_Integer nEF, nVF, iFlag;
  XBOPTools_ListIteratorOfListOfPave anIt;
  // 1.
  XBooleanOperations_OnceExplorer aExp(*myDS);
  aExp.Init(nF, TopAbs_EDGE);
  for (; aExp.More(); aExp.Next()) {
    nEF=aExp.Current();
    XBOPTools_PaveSet& aPaveSet=myPavePoolNew(myDS->RefEdge(nEF));
    const XBOPTools_ListOfPave& aLP=aPaveSet.Set();
    anIt.Initialize(aLP);
    for (; anIt.More(); anIt.Next()) {
      const XBOPTools_Pave& aPave=anIt.Value();
      nVF=aPave.Index();
      const TopoDS_Vertex& aVF=TopoDS::Vertex(myDS->Shape(nVF));
      iFlag=XIntTools_Tools::ComputeVV(aNewVertex, aVF);
      if (!iFlag) {
	return nVF;
      }
    }
  }
  //
  // 2.
  aExp.Init(nF, TopAbs_EDGE);
  for (; aExp.More(); aExp.Next()) {
    nEF=aExp.Current();
    XBOPTools_PaveSet& aPaveSet=myPavePool(myDS->RefEdge(nEF));
    const XBOPTools_ListOfPave& aLP=aPaveSet.Set();
    anIt.Initialize(aLP);
    for (; anIt.More(); anIt.Next()) {
      const XBOPTools_Pave& aPave=anIt.Value();
      nVF=aPave.Index();
      const TopoDS_Vertex& aVF=TopoDS::Vertex(myDS->Shape(nVF));
      iFlag=XIntTools_Tools::ComputeVV(aNewVertex, aVF);
      if (!iFlag) {
	return nVF;
      }
    }
  }
  
  return 0;
} 
//


