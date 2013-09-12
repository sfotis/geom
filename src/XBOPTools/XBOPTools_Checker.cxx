// Created on: 2002-08-05
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



#include <XBOPTools_Checker.ixx>

#include <stdio.h>  
#include <stdlib.h> 

#include <Precision.hxx>

#include <gp_Pnt.hxx>

#include <Geom_CartesianPoint.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_Curve.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>

#include <TopTools_IndexedMapOfShape.hxx>

#include <TopExp.hxx>

#include <Bnd_Box.hxx>

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>

#include <XBOPTools_Pave.hxx>
#include <XBOPTools_PaveSet.hxx>
#include <XBOPTools_ListOfPaveBlock.hxx>
#include <XBOPTools_ListIteratorOfListOfPaveBlock.hxx>
#include <XBOPTools_PaveBlock.hxx>
#include <XBOPTools_Tools.hxx>
#include <XBOPTools_PaveBlockIterator.hxx>
// modified by NIZHNY-MKK  Fri Sep  3 16:00:15 2004.BEGIN
#include <XBOPTools_CheckResult.hxx>
// modified by NIZHNY-MKK  Fri Sep  3 16:00:18 2004.END

#include <XIntTools_ShrunkRange.hxx>
#include <XIntTools_Range.hxx>
#include <XIntTools_EdgeEdge.hxx>
#include <XIntTools_SequenceOfCommonPrts.hxx>
#include <XIntTools_CommonPrt.hxx>
#include <XIntTools_SequenceOfRanges.hxx>
#include <XIntTools_EdgeFace.hxx>
#include <XIntTools_FaceFace.hxx>
#include <XIntTools_Curve.hxx>
#include <XIntTools_PntOn2Faces.hxx>
#include <XIntTools_PntOnFace.hxx>
#include <XIntTools_Tools.hxx>

#include <XBooleanOperations_ShapesDataStructure.hxx>
#include <XBooleanOperations_AncestorsSeqAndSuccessorsSeq.hxx>

#include <XBOPTColStd_Failure.hxx>
#include <XIntTools_Context.hxx>

//=======================================================================
// function:  XBOPTools_Checker::XBOPTools_Checker
// purpose: 
//=======================================================================
XBOPTools_Checker::XBOPTools_Checker() : XBOPTools_PaveFiller()
{
  myEntryType=1;
  myStopOnFirst = Standard_False;
}
//=======================================================================
// function:  XBOPTools_Checker::XBOPTools_Checker
// purpose: 
//=======================================================================
XBOPTools_Checker::XBOPTools_Checker(const TopoDS_Shape& aS) : XBOPTools_PaveFiller()
{
  myEntryType=1;
  myStopOnFirst = Standard_False;
  SetShape(aS);
} 
//=======================================================================
// function:  XBOPTools_Checker::XBOPTools_Checker
// purpose: 
//=======================================================================
XBOPTools_Checker::XBOPTools_Checker(const XBOPTools_InterferencePool& aPool) : XBOPTools_PaveFiller(aPool)
{
  myStopOnFirst = Standard_False;
  myEntryType=0;
  myIsDone=Standard_False;
  void* p=(void*) &aPool;
  myIntrPool=(XBOPTools_InterferencePool*) p;
  myDS=myIntrPool->DS();
  myNbSources=myDS->NumberOfShapesOfTheObject()+myDS->NumberOfShapesOfTheTool();
  myNbEdges=myDS->NbEdges();
}

//=======================================================================
// function: SetShape
// purpose: 
//=======================================================================
void  XBOPTools_Checker::SetShape(const TopoDS_Shape& aS)
{
  myShape=aS;

  Destroy();
  myDS = new XBooleanOperations_ShapesDataStructure (aS, aS);
  
  myIntrPool = new XBOPTools_InterferencePool (*myDS);

  myNbSources=myDS->NumberOfShapesOfTheObject()+myDS->NumberOfShapesOfTheTool();
  myNbEdges=myDS->NbEdges();
}

//=======================================================================
// function: Destroy
// purpose: 
//=======================================================================
void XBOPTools_Checker::Destroy()
{
  if (myEntryType) {
    //
    if (myIntrPool!=NULL) {
      delete myIntrPool; myIntrPool = NULL;
    }
    if (myDS!=NULL) {
      delete myDS; myDS = NULL;
    }
  }
  myCheckResults.Clear();
}

//=======================================================================
// function: SetPerformType
// purpose: 
//=======================================================================

void XBOPTools_Checker::SetPerformType(const Standard_Boolean StopOnFirstFaulty)
{
  myStopOnFirst = StopOnFirstFaulty;
}

//=======================================================================
// function: Perform
// purpose: 
//=======================================================================
void XBOPTools_Checker::Perform()
{
  myCheckResults.Clear();
  try {
    //
    if (myContext.IsNull()) {
      myContext=new XIntTools_Context;
    }
    //
    // 0. Prepare the IteratorOfCoupleOfShape
    myDSIt.SetDataStructure(myDS);
    //
    // 1.VV
    PerformVV();
    //
    // 2.VE
    myPavePool.Resize (myNbEdges);
    PrepareEdges();
    PerformVE();
    //
    // 3.VF
    PerformVF();
    //
    // 4.EE
    myCommonBlockPool.Resize (myNbEdges);
    mySplitShapesPool.Resize (myNbEdges);
    myPavePoolNew    .Resize (myNbEdges);
    
    PreparePaveBlocks(TopAbs_VERTEX, TopAbs_EDGE);
    PreparePaveBlocks(TopAbs_EDGE, TopAbs_EDGE);
    
    PerformEE();
    //
    // 5.EF
    PreparePaveBlocks(TopAbs_EDGE, TopAbs_FACE);
    
    PerformEF();
    //
    // 6. FF
    PerformFF ();
  }// end of try block
  //
  catch (XBOPTColStd_Failure& x) {
    cout << x.Message() << endl << flush;
  }
}
//=======================================================================
// function: PerformVV
// purpose: 
//=======================================================================
void XBOPTools_Checker::PerformVV()
{
  myIsDone=Standard_False;
  Standard_Boolean bJustAddInterference;
  Standard_Integer n1, n2, aFlag;
  //
  // V/V  XBooleanOperations_VertexVertex
  myDSIt.Initialize(TopAbs_VERTEX, TopAbs_VERTEX);
  //
  for (; myDSIt.More(); myDSIt.Next()) {
    bJustAddInterference = Standard_False;
    myDSIt.Current(n1, n2, bJustAddInterference);
    //
    const TopoDS_Shape& aS1=myDS->Shape(n1);
    const TopoDS_Shape& aS2=myDS->Shape(n2);
    //
    if (aS1.IsSame(aS2)){
      continue;
    }
    //
    if(bJustAddInterference) {
      continue;
    }
    //
    const TopoDS_Vertex& aV1=TopoDS::Vertex(aS1);
    const TopoDS_Vertex& aV2=TopoDS::Vertex(aS2);
    
    aFlag=XIntTools_Tools::ComputeVV (aV1, aV2);
    
    if (!aFlag) {
      char buf[512];
      sprintf (buf, "VV: (%d, %d)", n1, n2);

      XBOPTools_CheckResult aChRes;
      aChRes.AddShape(aV1);
      aChRes.AddShape(aV2);
      aChRes.SetCheckStatus(XBOPTools_VERTEXVERTEX);
      myCheckResults.Append(aChRes);

      if(myStopOnFirst)
        throw XBOPTColStd_Failure(buf) ;
    }
  }
  myIsDone=Standard_True;
}

//=======================================================================
// function: PerformVE
// purpose: 
//=======================================================================
void XBOPTools_Checker::PerformVE()
{
  myIsDone=Standard_False;
  Standard_Boolean bSameFlag, bJustAddInterference;
  Standard_Integer n1, n2, aFlag, aWhat, aWith;
  Standard_Real aT;
  //
  // V/E Interferences  [XBooleanOperations_VertexEdge]
  myDSIt.Initialize (TopAbs_VERTEX, TopAbs_EDGE);
  //
  for (; myDSIt.More(); myDSIt.Next()) {
    bJustAddInterference = Standard_False;
    myDSIt.Current(n1, n2, bJustAddInterference);
    //
    aWhat=n1; // Vertex
    aWith=n2; // Edge

    SortTypes(aWhat, aWith);
    
    const TopoDS_Shape& aS1=myDS->Shape(aWhat);
    const TopoDS_Shape& aS2=myDS->Shape(aWith);
    
    const TopoDS_Vertex& aV1=TopoDS::Vertex(aS1);
    const TopoDS_Edge&   aE2=TopoDS::Edge  (aS2);
    
    if (BRep_Tool::Degenerated(aE2)){
      continue;
    }
    //
    TopTools_IndexedMapOfShape aM2;
    //
    bSameFlag=Standard_False;
    //
    XBOPTools_Tools::MapShapes(aE2, aM2);
    //
    if (aM2.Contains(aV1)) {
      bSameFlag=Standard_True;
    }
    //
    if (bSameFlag){
      continue;
    }
    //
    aFlag=myContext->ComputeVE (aV1, aE2, aT);
    //
    if (!aFlag) {
      char buf[512];
      sprintf (buf, "VE: (%d, %d)", aWhat, aWith);

      XBOPTools_CheckResult aChRes;
      aChRes.AddShape(aV1);
      aChRes.AddShape(aE2);
      aChRes.SetCheckStatus(XBOPTools_VERTEXEDGE);
      myCheckResults.Append(aChRes);
      //
      if(myStopOnFirst)
        throw XBOPTColStd_Failure(buf) ;
    }
  }
  myIsDone=Standard_True;
}

//=======================================================================
// function: PerformVF
// purpose: 
//=======================================================================
void XBOPTools_Checker::PerformVF()
{
  myIsDone=Standard_False;
  Standard_Boolean justaddinterference, bSameFlag;
  Standard_Integer n1, n2, aFlag, aWhat, aWith;
  Standard_Real aU, aV;
  //
  // V/V  XBooleanOperations_VertexFace
  myDSIt.Initialize(TopAbs_VERTEX, TopAbs_FACE);
  //
  for (; myDSIt.More(); myDSIt.Next()) {
    justaddinterference = Standard_False;
    myDSIt.Current(n1, n2, justaddinterference);
    //
    aWhat=n1; // Vertex
    aWith=n2; // Face
    SortTypes(aWhat, aWith);
    
    const TopoDS_Shape& aS1=myDS->Shape(aWhat);
    const TopoDS_Shape& aS2=myDS->Shape(aWith);
   
    const TopoDS_Vertex& aV1=TopoDS::Vertex(aS1);
    const TopoDS_Face&   aF2=TopoDS::Face  (aS2);
    //
    TopTools_IndexedMapOfShape aM2;
    //
    bSameFlag=Standard_False;
    //
    XBOPTools_Tools::MapShapes(aF2, aM2);
    //
    if (aM2.Contains(aV1)) {
      bSameFlag=Standard_True;
    }
    //
    if (bSameFlag){
      continue;
    }
    //
    aFlag=myContext->ComputeVS (aV1, aF2, aU, aV);
    //
    if (!aFlag) {
      char buf[512];
      sprintf (buf, "VF: (%d, %d)", aWhat, aWith);

      XBOPTools_CheckResult aChRes;
      aChRes.AddShape(aV1);
      aChRes.AddShape(aF2);
      aChRes.SetCheckStatus(XBOPTools_VERTEXFACE);
      myCheckResults.Append(aChRes);

      if(myStopOnFirst)
        throw XBOPTColStd_Failure(buf) ;
    }
  }
  myIsDone=Standard_True;
}

//=======================================================================
// function: PerformEE
// purpose: 
//=======================================================================
void XBOPTools_Checker::PerformEE()
{
  myIsDone=Standard_False;

  Standard_Boolean justaddinterference;
  Standard_Integer n1, n2, anIndexIn=0, nE1, nE2;
  Standard_Integer aTmp, aWhat, aWith;
  Standard_Integer i, aNbCPrts;
  //
  // E/E Interferences  [XBooleanOperations_EdgeEdge]
  myDSIt.Initialize(TopAbs_EDGE, TopAbs_EDGE);
  //
  for (; myDSIt.More(); myDSIt.Next()) {
    justaddinterference = Standard_False;
    myDSIt.Current(n1, n2, justaddinterference);
    //
    nE1=n1; 
    nE2=n2; 
    SortTypes(nE1, nE2);
    //
    Standard_Real aTolE1, aTolE2, aDeflection=0.01;
    Standard_Integer aDiscretize=30;

    const TopoDS_Edge& aE1=TopoDS::Edge(myDS->GetShape(nE1));
    const TopoDS_Edge& aE2=TopoDS::Edge(myDS->GetShape(nE2));
    //
    if (BRep_Tool::Degenerated(aE1)){
      continue;
    }
    if (BRep_Tool::Degenerated(aE2)){
      continue;
    }
    //
    // 
    Standard_Boolean bSameFlag;
    TopTools_IndexedMapOfShape aM1, aM2;
    //
    bSameFlag=aE1.IsSame(aE2);
    //
    if (bSameFlag){
      continue;
    }
    //
    aTolE1=BRep_Tool::Tolerance(aE1);
    aTolE2=BRep_Tool::Tolerance(aE2);
    //
    XBOPTools_ListOfPaveBlock& aLPB1=mySplitShapesPool(myDS->RefEdge(nE1));
    XBOPTools_ListIteratorOfListOfPaveBlock anIt1(aLPB1);

    for (; anIt1.More(); anIt1.Next()) {
      XBOPTools_PaveBlock& aPB1=anIt1.Value();
      const XIntTools_ShrunkRange& aShrunkRange1=aPB1.ShrunkRange();
    
      const XIntTools_Range& aSR1=aShrunkRange1.ShrunkRange();
      const Bnd_Box&        aBB1=aShrunkRange1.BndBox();

      XBOPTools_ListOfPaveBlock& aLPB2=mySplitShapesPool(myDS->RefEdge(nE2));
      XBOPTools_ListIteratorOfListOfPaveBlock anIt2(aLPB2);
      
      for (; anIt2.More(); anIt2.Next()) {
	XBOPTools_PaveBlock& aPB2=anIt2.Value();
	const XIntTools_ShrunkRange& aShrunkRange2=aPB2.ShrunkRange();
      
	const XIntTools_Range& aSR2=aShrunkRange2.ShrunkRange();
	const Bnd_Box&        aBB2=aShrunkRange2.BndBox();
	
	//////////////////////////////////////////////
	if (aBB1.IsOut (aBB2)) {
	  continue;
	}
	// 
	// EE
	XIntTools_EdgeEdge aEE;
	aEE.SetEdge1 (aE1);
	aEE.SetEdge2 (aE2);
	aEE.SetTolerance1 (aTolE1);
	aEE.SetTolerance2 (aTolE2);
	aEE.SetDiscretize (aDiscretize);
	aEE.SetDeflection (aDeflection);
	//
	XIntTools_Range anewSR1 = aSR1;
	XIntTools_Range anewSR2 = aSR2;
	//
	XBOPTools_Tools::CorrectRange (aE1, aE2, aSR1, anewSR1);
	XBOPTools_Tools::CorrectRange (aE2, aE1, aSR2, anewSR2);
	//
	aEE.SetRange1(anewSR1);
	aEE.SetRange2(anewSR2);
	  
	aEE.Perform();
	//
	anIndexIn=0;
	//
	if (aEE.IsDone()) {
	  //
	  // reverse order if it is necessary
	  TopoDS_Edge aEWhat, aEWith;
	  aEWhat=aE1;
	  aEWith=aE2;
	  aWhat=nE1;
	  aWith=nE2;
	  if (aEE.Order()) {
	    aTmp=aWhat;
	    aWhat=aWith;
	    aWith=aTmp;
	    aEWhat=aE2;
	    aEWith=aE1;
	  }
	  //
	  const XIntTools_SequenceOfCommonPrts& aCPrts=aEE.CommonParts();
	  
	  aNbCPrts=aCPrts.Length();
	  for (i=1; i<=aNbCPrts; i++) {
	    const XIntTools_CommonPrt& aCPart=aCPrts(i);
	    //
	    anIndexIn=0;
	    //
	    TopAbs_ShapeEnum aType=aCPart.Type();
	    switch (aType) {
	      
  	      case TopAbs_VERTEX:  {
		
		Standard_Real aT1, aT2; 
		
		const XIntTools_Range& aR1=aCPart.Range1();
		aT1=0.5*(aR1.First()+aR1.Last());

		if((aCPart.VertexParameter1() >= aR1.First()) &&
		   (aCPart.VertexParameter1() <= aR1.Last())) {
		  aT1 = aCPart.VertexParameter1();
		}

		const XIntTools_SequenceOfRanges& aRanges2=aCPart.Ranges2();
		const XIntTools_Range& aR2=aRanges2(1);
		aT2=0.5*(aR2.First()+aR2.Last());

		if((aCPart.VertexParameter2() >= aR2.First()) &&
		   (aCPart.VertexParameter2() <= aR2.Last())) {
		  aT2 = aCPart.VertexParameter2();
		}
		//
		char buf[512];
		sprintf (buf, "EE: (%d, %d), vertex at t1=%f, t2=%f", aWhat, aWith, aT1, aT2);
		//
		gp_Pnt aPnt;
		XBOPTools_Tools::PointOnEdge(aEWhat, aT1, aPnt);
		Handle (Geom_CartesianPoint) aCPnt= new Geom_CartesianPoint(aPnt);
// 		myInerference=aCPnt;

                XBOPTools_CheckResult aChRes;
                aChRes.AddShape(aE1);
                aChRes.AddShape(aE2);
                aChRes.SetCheckStatus(XBOPTools_EDGEEDGE);
// modified by NIZHNY-MKK  Fri Sep  3 16:01:52 2004
//                 aChRes.SetInterferenceGeometry(myInerference);
		aChRes.SetInterferenceGeometry(aCPnt);
                myCheckResults.Append(aChRes);

                if(myStopOnFirst)
                  throw XBOPTColStd_Failure(buf) ;
		//
	      }
	      break;

	      case TopAbs_EDGE: {
	      
		const XIntTools_SequenceOfRanges& aRanges2=aCPart.Ranges2();
		Standard_Integer aNbComPrt2=aRanges2.Length();
		
		if (aNbComPrt2>1) {
		  break;
		}

		Standard_Boolean aCoinsideFlag;
		
		aCoinsideFlag=IsBlocksCoinside(aPB1, aPB2);
		//
		if (!aCoinsideFlag) {
		  break;
		}
		//
		char buf[512];
		sprintf (buf, "EE: (%d, %d), common block ", aWhat, aWith);
                
                XBOPTools_CheckResult aChRes;
                aChRes.AddShape(aE1);
                aChRes.AddShape(aE2);
                aChRes.SetCheckStatus(XBOPTools_EDGEEDGECOMBLK);
                myCheckResults.Append(aChRes);

                if(myStopOnFirst)
                  throw XBOPTColStd_Failure(buf) ;
		//
	      }
	      break;

	    default:
	      break;
	    } // switch (aType) 
	  } // for (i=1; i<=aNbCPrts; i++) 
	}// if (aEE.IsDone())
	
	//////////////////////////////////////////////
      } // for (; anIt2.More(); anIt2.Next()) 
    } // for (; anIt1.More(); anIt1.Next()) 
  }// for (; myDSIt.More(); myDSIt.Next()) 
  myIsDone=Standard_True;
}

//=======================================================================
// function: PerformEF
// purpose: 
//=======================================================================
void XBOPTools_Checker::PerformEF()
{
  myIsDone=Standard_False;
  //
  Standard_Boolean justaddinterference, bSameFlag;
  Standard_Integer n1, n2,  nE, nF, i, aNbCPrts;
  //
  // E/F Interferences  [XBooleanOperations_EdgeFace]
  myDSIt.Initialize(TopAbs_EDGE, TopAbs_FACE);
  //
  for (; myDSIt.More(); myDSIt.Next()) {
    justaddinterference = Standard_True;
    myDSIt.Current(n1, n2, justaddinterference);
    //
    nE=n1; 
    nF=n2; 
    SortTypes(nE, nF);
    //
    Standard_Real aTolE, aTolF, aDeflection=0.01;
    Standard_Integer aDiscretize=35;

    const TopoDS_Edge& aE=TopoDS::Edge(myDS->GetShape(nE));
    const TopoDS_Face& aF=TopoDS::Face(myDS->GetShape(nF));
    //
    if (BRep_Tool::Degenerated(aE)){
      continue;
    }
    // 
    TopTools_IndexedMapOfShape aMF;
    //
    bSameFlag=Standard_False;
    //
    TopExp::MapShapes(aF, TopAbs_EDGE, aMF);
    if (aMF.Contains(aE)) { 
      bSameFlag=Standard_True;
    }
    //
    if (bSameFlag){
      continue;
    }
    //
    aTolE=BRep_Tool::Tolerance(aE);
    aTolF=BRep_Tool::Tolerance(aF);
    //
    const Bnd_Box& aBBF=myDS->GetBoundingBox(nF); 
    //
    XBOPTools_ListOfPaveBlock& aLPB=mySplitShapesPool(myDS->RefEdge(nE));
    XBOPTools_ListIteratorOfListOfPaveBlock anIt(aLPB);

    for (; anIt.More(); anIt.Next()) {
      XBOPTools_PaveBlock& aPB=anIt.Value();
      const XIntTools_ShrunkRange& aShrunkRange=aPB.ShrunkRange();
      const XIntTools_Range& aSR =aShrunkRange.ShrunkRange();
      const Bnd_Box&        aBBE=aShrunkRange.BndBox();
      //
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

      XIntTools_Range anewSR = aSR;
      //
      XBOPTools_Tools::CorrectRange(aE, aF, aSR, anewSR);
      //
      aEF.SetRange (anewSR);
      //
      aEF.Perform();
      //
      if (aEF.IsDone()) {
	//
	const XIntTools_SequenceOfCommonPrts& aCPrts=aEF.CommonParts();
	aNbCPrts=aCPrts.Length();
	for (i=1; i<=aNbCPrts; i++) {
	  const XIntTools_CommonPrt& aCPart=aCPrts(i);
	  //
	  TopAbs_ShapeEnum aType=aCPart.Type();
	  switch (aType) {
	      
	    case TopAbs_VERTEX:  {
		
	      Standard_Real aT; 
	      
	      const XIntTools_Range& aR=aCPart.Range1();

	      Standard_Real aRFirst, aRLast;

	      aR.Range(aRFirst, aRLast);
	      aT=0.5*(aRFirst+aRLast);

	      if((aCPart.VertexParameter1() >= aRFirst) &&
		 (aCPart.VertexParameter1() <= aRLast)) {
		aT = aCPart.VertexParameter1();
	      }
	      //
	      char buf[512];
	      sprintf (buf, "EF: (%d, %d), vertex at t=%f", nE, nF, aT);
	      //
	      gp_Pnt aPnt;
	      XBOPTools_Tools::PointOnEdge(aE, aT, aPnt);
	      Handle (Geom_CartesianPoint) aCPnt= new Geom_CartesianPoint(aPnt);
// 	      myInerference=aCPnt;

              XBOPTools_CheckResult aChRes;
              aChRes.AddShape(aE);
              aChRes.AddShape(aF);
              aChRes.SetCheckStatus(XBOPTools_EDGEFACE);
// modified by NIZHNY-MKK  Fri Sep  3 16:02:10 2004
//               aChRes.SetInterferenceGeometry(myInerference);
              aChRes.SetInterferenceGeometry(aCPnt);
              myCheckResults.Append(aChRes);

              if(myStopOnFirst)
                throw XBOPTColStd_Failure(buf) ;
	    }// case TopAbs_VERTEX:
	      break;

	    case TopAbs_EDGE: {
	      
	      Standard_Boolean aCoinsideFlag;
	      aCoinsideFlag=XBOPTools_Tools::IsBlockInOnFace(aPB, aF, myContext);
	      if (!aCoinsideFlag) {
		break;
	      }
	      //
	      char buf[512];
	      sprintf (buf, "EF: (%d, %d), common block ", nE, nF);

              XBOPTools_CheckResult aChRes;
              aChRes.AddShape(aE);
              aChRes.AddShape(aF);
              aChRes.SetCheckStatus(XBOPTools_EDGEFACECOMBLK);
              myCheckResults.Append(aChRes);

              if(myStopOnFirst)
                throw XBOPTColStd_Failure(buf) ;
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
// function: PerformFF
// purpose: 
//=======================================================================
  void XBOPTools_Checker::PerformFF()
{
  myIsDone=Standard_False;
  //
  Standard_Boolean justaddinterference, bSameFlag;
  Standard_Integer n1, n2, nF1, nF2, i, aNbS1;
  //
  //  F/F Interferences  [XBooleanOperations_SurfaceSurface]
  myDSIt.Initialize(TopAbs_FACE, TopAbs_FACE);
  //
  for (; myDSIt.More(); myDSIt.Next()) {
    justaddinterference = Standard_True;
    myDSIt.Current(n1, n2, justaddinterference);
    //
    nF1=n1; 
    nF2=n2; 
    if (nF1 > nF2) {
      Standard_Integer iTmp;
      iTmp=nF1;
      nF1=nF2;
      nF2=iTmp;
    }
    //
    const TopoDS_Face& aF1=TopoDS::Face(myDS->Shape(nF1));
    const TopoDS_Face& aF2=TopoDS::Face(myDS->Shape(nF2));
    //
    TopTools_IndexedMapOfShape aM1, aM2;
    //
    bSameFlag=Standard_False;
    //
    TopExp::MapShapes(aF1, TopAbs_EDGE, aM1);
    TopExp::MapShapes(aF2, TopAbs_EDGE, aM2);
    //
    aNbS1=aM1.Extent();

    for (i=1; i<=aNbS1; ++i) {
      const TopoDS_Shape& aS1=aM1(i);
      if (aM2.Contains(aS1)) {
	bSameFlag=Standard_True;
	break;
      }
    }
    //
    if (bSameFlag){
      continue;
    }
    //
    // FF
    Standard_Boolean bToApproxC3d, bToApproxC2dOnS1, bToApproxC2dOnS2;
    Standard_Real anApproxTol, aTolR3D, aTolR2D;
    //
    bToApproxC3d     = mySectionAttribute.Approximation();
    bToApproxC2dOnS1 = mySectionAttribute.PCurveOnS1();
    bToApproxC2dOnS2 = mySectionAttribute.PCurveOnS2();
    //
    anApproxTol=1.e-7;

    XIntTools_FaceFace aFF;
    aFF.SetParameters (bToApproxC3d, 
		       bToApproxC2dOnS1, 
		       bToApproxC2dOnS2,
		       anApproxTol);
	  
    aFF.Perform(aF1, aF2);

    if (aFF.IsDone()) {
      // Add Interference to the Pool
      aTolR3D=aFF.TolReached3d();
      aTolR2D=aFF.TolReached2d();
      if (aTolR3D < 1.e-7){
	aTolR3D=1.e-7;
      } 
      aFF.PrepareLines3D();
      //
      //
      Standard_Integer j, aNbCurves, aNbPoints;
      //
      const XIntTools_SequenceOfCurves& aCvs=aFF.Lines();
      aNbCurves=aCvs.Length();
      //
      const XIntTools_SequenceOfPntOn2Faces& aPnts=aFF.Points();
      aNbPoints=aPnts.Length();
      
      if (aNbPoints) {
	char buf[512];
	sprintf (buf, "FF: (%d, %d) ", nF1, nF2);
	//
	const XIntTools_PntOn2Faces& aPntOn2Faces=aPnts(1);
	const XIntTools_PntOnFace& aPntOnFace=aPntOn2Faces.P1();
	const gp_Pnt& aPnt=aPntOnFace.Pnt();
	Handle (Geom_CartesianPoint) aCPnt= new Geom_CartesianPoint(aPnt);
// 	myInerference=aCPnt;

        XBOPTools_CheckResult aChRes;
        aChRes.AddShape(aF1);
        aChRes.AddShape(aF2);
        aChRes.SetCheckStatus(XBOPTools_FACEFACE);
// modified by NIZHNY-MKK  Fri Sep  3 16:02:25 2004
//         aChRes.SetInterferenceGeometry(myInerference);
        aChRes.SetInterferenceGeometry(aCPnt);
        myCheckResults.Append(aChRes);

        if(myStopOnFirst)
          throw XBOPTColStd_Failure(buf) ;
      }
      
      if (aNbCurves) {
	for (j=1; j<=aNbCurves; j++) {
	  const XIntTools_Curve& aC=aCvs(j);
	  if (aC.HasBounds()) {
	    Standard_Real aT1, aT2;
	    Standard_Boolean bValid;
	    gp_Pnt aP1, aP2;
	    
	    aC.Bounds(aT1, aT2, aP1, aP2);
	    //
	    bValid=myContext->IsValidBlockForFaces(aT1, aT2, aC, aF1, aF2, 1.e-3);
	    //
	    if (bValid) {
	      char buf[512];
	      sprintf (buf, "FF: (%d, %d) ", nF1, nF2);
	      //
	      Handle (Geom_Curve) aC3D=aC.Curve();
	      Handle (Geom_TrimmedCurve) aTC3D=Handle (Geom_TrimmedCurve)::DownCast(aC3D);
// 	      myInerference=aTC3D;

              XBOPTools_CheckResult aChRes;
              aChRes.AddShape(aF1);
              aChRes.AddShape(aF2);
              aChRes.SetCheckStatus(XBOPTools_FACEFACE);
// modified by NIZHNY-MKK  Fri Sep  3 16:02:40 2004
//               aChRes.SetInterferenceGeometry(myInerference);
              aChRes.SetInterferenceGeometry(aTC3D);
              myCheckResults.Append(aChRes);

              if(myStopOnFirst)
                throw XBOPTColStd_Failure(buf) ;
	    }
	  }
	}
      }// if (aNbCurves)
      
    }// if (aFF.IsDone())
  }// for (; myDSIt.More(); myDSIt.Next()) 
  myIsDone=Standard_True;
}
//=======================================================================
// function: PrepareEdges
// purpose: 
//=======================================================================
  void XBOPTools_Checker::PrepareEdges()
{
  Standard_Integer  i, nV, ii, aNBSuc;
  Standard_Real aT;
  TopAbs_Orientation anOr;
  TopoDS_Edge   aE;
  TopoDS_Vertex aV;

  for (i=1; i<=myNbSources; i++) {
    if (myDS->GetShapeType(i)==TopAbs_EDGE) {
      aE=TopoDS::Edge(myDS->GetShape(i));
      //
      if (BRep_Tool::Degenerated(aE)){
	continue;
      }
      //
      XBOPTools_PaveSet& aPaveSet= myPavePool(myDS->RefEdge(i));
      //
      //                                                   cto900/M2
      // Some of Edges can be [Semi] Infinite.  Such  Edges have no 
      // vertices on correspondant INF ends.   So we  must  provide 
      // these vertices formally (to obtain  Shrunk  Ranges for e.g). 
      // In reality this vertex(-es) does not belong to the INF Edge.
      // It just has reference in the DS.
      //                            PKV Tue Apr 23 10:21:45 2002                 
      {
	Standard_Real aT1, aT2, aTolE;
	Standard_Boolean bInf1, bInf2;
	gp_Pnt aPx;
	TopoDS_Vertex aVx; 
	BRep_Builder aBB;
	XBooleanOperations_AncestorsSeqAndSuccessorsSeq anASSeq; 
	//
	aTolE=BRep_Tool::Tolerance(aE);
	Handle(Geom_Curve) aC3D=BRep_Tool::Curve (aE, aT1, aT2);
	bInf1=Precision::IsNegativeInfinite(aT1);
	bInf2=Precision::IsPositiveInfinite(aT2);

	if (bInf1) {
	  aC3D->D0(aT1, aPx);
	  aBB.MakeVertex(aVx, aPx, aTolE);
	  myDS->InsertShapeAndAncestorsSuccessors(aVx, anASSeq);
	  nV=myDS->NumberOfInsertedShapes();
	  XBOPTools_Pave aPave(nV, aT1); 
	  aPaveSet.Append (aPave);
	}

	if (bInf2) {
	  aC3D->D0(aT2, aPx);
	  aBB.MakeVertex(aVx, aPx, aTolE);
	  myDS->InsertShapeAndAncestorsSuccessors(aVx, anASSeq);
	  nV=myDS->NumberOfInsertedShapes();
	  XBOPTools_Pave aPave(nV, aT2);
	  aPaveSet.Append (aPave); 
	}
      }
      //
      aNBSuc=myDS->NumberOfSuccessors(i);
      for (ii=1; ii <= aNBSuc; ii++) {
	nV=myDS->GetSuccessor(i, ii);
	anOr=myDS->GetOrientation(i, ii);

	aV=TopoDS::Vertex(myDS->GetShape(nV));
	aV.Orientation(anOr);
	aT=BRep_Tool::Parameter(aV, aE);
	//
	XBOPTools_Pave aPave(nV, aT); 
	aPaveSet.Append (aPave);
      }
    }
  }
}
//=======================================================================
// function: PreparePaveBlocks
// purpose: 
//=======================================================================
  void XBOPTools_Checker::PreparePaveBlocks(const TopAbs_ShapeEnum aType1, 
					   const TopAbs_ShapeEnum aType2)
{
  XBOPTools_PaveFiller::PreparePaveBlocks(aType1, aType2);
}
//=======================================================================
// function: PreparePaveBlocks
// purpose: 
//=======================================================================
  void XBOPTools_Checker::PreparePaveBlocks(const Standard_Integer nE)
{
  myIsDone=Standard_False;
  
  Standard_Integer nV1, nV2;

  TopoDS_Edge aE;
  TopoDS_Vertex aV1, aV2;
    
  // SplitShapesPool
  XBOPTools_ListOfPaveBlock& aLPB=mySplitShapesPool(myDS->RefEdge(nE));
  // Edge 
  aE=TopoDS::Edge(myDS->GetShape(nE));
  //
  if (!BRep_Tool::Degenerated(aE)){
    //
    XBOPTools_PaveSet& aPS=myPavePool(myDS->RefEdge(nE));
    
    XBOPTools_PaveBlockIterator aPBIt(nE, aPS);
    for (; aPBIt.More(); aPBIt.Next()) {
      XBOPTools_PaveBlock& aPB=aPBIt.Value();
      
      const XIntTools_Range& aRange=aPB.Range();
      
      const XBOPTools_Pave& aPave1=aPB.Pave1();
      nV1=aPave1.Index();
      aV1=TopoDS::Vertex(myDS->GetShape(nV1));
      
      const XBOPTools_Pave& aPave2=aPB.Pave2();
      nV2=aPave2.Index();
      aV2=TopoDS::Vertex(myDS->GetShape(nV2));
      //
      // ShrunkRange
      XIntTools_ShrunkRange aSR (aE, aV1, aV2, aRange, myContext);
      //
      Standard_Integer anErrorStatus;
      anErrorStatus=aSR.ErrorStatus();

      char buf[512];
      if (!aSR.IsDone()) {
	sprintf (buf, "Can not obtain ShrunkRange for Edge %d", nE);
        
        XBOPTools_CheckResult aChRes;
        aChRes.AddShape(aE);
        aChRes.SetCheckStatus(XBOPTools_BADSHRANKRANGE);
        myCheckResults.Append(aChRes);

        if(myStopOnFirst)
          throw XBOPTColStd_Failure(buf) ;
      }
      //
      if (anErrorStatus==6) {
	sprintf(buf,
		"Warning: [PreparePaveBlocks()] Max.Dummy Shrunk Range for Edge %d\n", nE);

        XBOPTools_CheckResult aChRes;
        aChRes.AddShape(aE);
        aChRes.SetCheckStatus(XBOPTools_NULLSRANKRANGE);
        myCheckResults.Append(aChRes);

        if(myStopOnFirst)
          throw XBOPTColStd_Failure(buf);
      }
      else {
	// Check left paves and correct ShrunkRange if it is necessary
	CorrectShrunkRanges (0, aPave1, aSR);
	CorrectShrunkRanges (1, aPave2, aSR);
      }
      //
      aPB.SetShrunkRange(aSR);
      aLPB.Append(aPB);
    } //for (; aPBIt1.More(); aPBIt1.Next()) 
  }
  myIsDone=Standard_True;
}

//=======================================================================
// function: GetCheckResult
// purpose: 
//=======================================================================
const XBOPTools_ListOfCheckResults& XBOPTools_Checker::GetCheckResult() const
{
  return myCheckResults;
}

//=======================================================================
// function: HasFaulty
// purpose: 
//=======================================================================
  Standard_Boolean XBOPTools_Checker::HasFaulty()const 
{
  return (!myIsDone || !myCheckResults.IsEmpty());
}

//=======================================================================
// function: Shape
// purpose: 
//=======================================================================
  const TopoDS_Shape& XBOPTools_Checker::Shape()const 
{
  return myShape;
}



