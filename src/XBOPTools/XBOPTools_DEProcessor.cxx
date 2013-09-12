// Created on: 2001-09-12
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


#include <XBOPTools_DEProcessor.ixx>


#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Solid.hxx>

#include <TopExp.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>

#include <gp_Pnt2d.hxx>
#include <gp_Pnt.hxx>
#include <gp_Sphere.hxx>

#include <IntRes2d_IntersectionPoint.hxx>

#include <Precision.hxx>

#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <Geom2dInt_GInter.hxx>

#include <BRep_Tool.hxx>
#include <BRep_Builder.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepAdaptor_Surface.hxx>

#include <BRepExtrema_DistShapeShape.hxx>

#include <TColStd_ListIteratorOfListOfInteger.hxx>
#include <TColStd_ListOfInteger.hxx>

#include <XBooleanOperations_ShapesDataStructure.hxx>
#include <XBooleanOperations_AncestorsSeqAndSuccessorsSeq.hxx>

#include <XIntTools_Range.hxx>
#include <XIntTools_Tools.hxx>
#include <XIntTools_Context.hxx>

#include <XBOPTools_DEInfo.hxx>
#include <XBOPTools_ListOfPaveBlock.hxx>
#include <XBOPTools_SSInterference.hxx>
#include <XBOPTools_PaveBlock.hxx>
#include <XBOPTools_ListIteratorOfListOfPaveBlock.hxx>
#include <XBOPTools_SequenceOfCurves.hxx>
#include <XBOPTools_Curve.hxx>
#include <XBOPTools_PavePool.hxx>
#include <XBOPTools_Pave.hxx>
#include <XBOPTools_PaveSet.hxx>
#include <XBOPTools_Tools3D.hxx>
#include <XBOPTools_PaveBlockIterator.hxx>
#include <XBOPTools_ListOfPave.hxx>
#include <XBOPTools_ListIteratorOfListOfPave.hxx>
#include <XBOPTools_InterferencePool.hxx>
#include <XBOPTools_CArray1OfSSInterference.hxx>
#include <XBOPTools_ListIteratorOfListOfPaveBlock.hxx>
#include <XBOPTools_PaveFiller.hxx>
#include <XBOPTools_ListOfPaveBlock.hxx>
#include <XBOPTools_SplitShapesPool.hxx>
#include <XBOPTools_StateFiller.hxx>

//=======================================================================
// function: XBOPTools_DEProcessor::XBOPTools_DEProcessor
// purpose: 
//=======================================================================
XBOPTools_DEProcessor::XBOPTools_DEProcessor(const XBOPTools_PaveFiller& aFiller,
					     const Standard_Integer aDim)
:
   myIsDone(Standard_False)
{
  myFiller=(XBOPTools_PaveFiller*) &aFiller;
  myDS=myFiller->DS();
  
  myDim=aDim;
  if (aDim<2 || aDim>3) {
    myDim=3;
  }
  
}

//=======================================================================
// function: IsDone
// purpose: 
//=======================================================================
  Standard_Boolean XBOPTools_DEProcessor::IsDone() const
{
  return myIsDone;
}
//=======================================================================
// function:  Do
// purpose: 
//=======================================================================
  void XBOPTools_DEProcessor::Do()
{
  Standard_Integer aNbE;
  myIsDone=Standard_False;

  FindDegeneratedEdges();
  aNbE=myDEMap.Extent();
  
  if (!aNbE) {
    myIsDone=Standard_True;
    return;
  }
  
  DoPaves();
}

//=======================================================================
// function:  FindDegeneratedEdges
// purpose: 
//=======================================================================
  void XBOPTools_DEProcessor::FindDegeneratedEdges()
{
  const XBooleanOperations_ShapesDataStructure& aDS=*myDS;
  const XBOPTools_PaveFiller& aPaveFiller=*myFiller;

  Standard_Integer i, aNbSourceShapes, nV, nF, nVx, ip, iRankE;
  TopAbs_ShapeEnum aType;

  const TopoDS_Shape& anObj=aDS.Object();
  const TopoDS_Shape& aTool=aDS.Tool();
  
  TopTools_IndexedDataMapOfShapeListOfShape aMEF;
  TopExp::MapShapesAndAncestors (anObj, TopAbs_EDGE, TopAbs_FACE, aMEF);
  TopExp::MapShapesAndAncestors (aTool, TopAbs_EDGE, TopAbs_FACE, aMEF);

  aNbSourceShapes=aDS.NumberOfSourceShapes();
  for (i=1; i<=aNbSourceShapes; i++) {
    const TopoDS_Shape& aS=aDS.Shape(i);
    aType=aS.ShapeType();
    if (aType==TopAbs_EDGE) {
      const TopoDS_Edge& aE=TopoDS::Edge(aS);
      if (BRep_Tool::Degenerated(aE)) {

	iRankE=aDS.Rank(i);

	TopoDS_Vertex aV=TopExp::FirstVertex(aE);

	nVx=aDS.ShapeIndex(aV, iRankE);
	//
	nV=nVx;
	ip=aPaveFiller.FindSDVertex(nV);
	if (ip) {
	  nV=ip;
	}
	//
	TColStd_ListOfInteger aLFn;
	const TopTools_ListOfShape& aLF=aMEF.FindFromKey(aE);
	TopTools_ListIteratorOfListOfShape anIt(aLF);
	for (; anIt.More(); anIt.Next()) {
	  const TopoDS_Shape& aF=anIt.Value();

	  nF=aDS.ShapeIndex(aF, iRankE);

	  aLFn.Append(nF);
	}
	XBOPTools_DEInfo aDEInfo;
	aDEInfo.SetVertex(nV);
	aDEInfo.SetFaces(aLFn);

	myDEMap.Add (i, aDEInfo);

      }
    }
  }
  
}
//=======================================================================
// function:  DoPaves
// purpose: 
//=======================================================================
  void XBOPTools_DEProcessor::DoPaves()
{

  Standard_Integer i, aNbE, nED, nVD, nFD=0;
  
  aNbE=myDEMap.Extent();
  for (i=1; i<=aNbE; i++) {
    nED=myDEMap.FindKey(i);
    
    const XBOPTools_DEInfo& aDEInfo=myDEMap(i);
    nVD=aDEInfo.Vertex();
    // Fill PaveSet for the edge nED
    const TColStd_ListOfInteger& nLF=aDEInfo.Faces();
    TColStd_ListIteratorOfListOfInteger anIt(nLF);
    for (; anIt.More(); anIt.Next()) {
      nFD=anIt.Value();
      
      XBOPTools_ListOfPaveBlock aLPB;
      FindPaveBlocks(nED, nVD, nFD, aLPB);
      FillPaveSet (nED, nVD, nFD, aLPB);
    }
    // 
    // Fill aSplitEdges for the edge nED
    FillSplitEdgesPool(nED);
    //
    // MakeSplitEdges
    MakeSplitEdges(nED, nFD);
    //
    // Compute States for Split parts
    if (myDim==3) {
      DoStates(nED, nFD);
    }
    if (myDim==2) {
      DoStates2D(nED, nFD);
    }
  }// next nED
}

//=======================================================================
// function:  DoStates
// purpose: 
//=======================================================================
  void XBOPTools_DEProcessor::DoStates (const Standard_Integer nED, 
				       const Standard_Integer nFD)
{
  
  
  const XBOPTools_SplitShapesPool& aSplitShapesPool=myFiller->SplitShapesPool();
  const XBOPTools_ListOfPaveBlock& aSplitEdges=aSplitShapesPool(myDS->RefEdge(nED));

  const TopoDS_Edge& aDE=TopoDS::Edge(myDS->Shape(nED));
  const TopoDS_Face& aDF=TopoDS::Face(myDS->Shape(nFD));
  
  Standard_Integer nSp, iRank;
  Standard_Real aT, aT1, aT2, aTol=1e-7;
  TopoDS_Face aF;
  gp_Pnt2d aPx2DNear;
  gp_Pnt aPxNear;

  iRank=myDS->Rank(nED);
  const TopoDS_Shape& aReference=(iRank==1) ? myDS->Tool() : myDS->Object();

  BRepExtrema_DistShapeShape aDSS;
  aDSS.LoadS1(aReference);

  aF=aDF;
  aF.Orientation(TopAbs_FORWARD);

  XBOPTools_ListIteratorOfListOfPaveBlock aPBIt(aSplitEdges);

  for (; aPBIt.More(); aPBIt.Next()) {
    XBOPTools_PaveBlock& aPB=aPBIt.Value();
    
    nSp=aPB.Edge();
    const TopoDS_Edge& aSp=TopoDS::Edge(myDS->Shape(nSp));
    
    aPB.Parameters(aT1, aT2);
    aT=XIntTools_Tools::IntermediatePoint(aT1, aT2);

    TopoDS_Edge aDERight, aSpRight;
    aDERight=aDE;
    aSpRight=aSp;

    XBOPTools_Tools3D::OrientEdgeOnFace (aDE, aF, aDERight);
    aSpRight.Orientation(aDERight.Orientation());
    //
    {
      BRepAdaptor_Surface aBAS;
      aBAS.Initialize (aDF, Standard_False);

      if (aBAS.GetType()==GeomAbs_Sphere) {
	Standard_Real aDt2D, aR, aDelta=1.e-14;
	
	gp_Sphere aSphere=aBAS.Sphere();
	aR=aSphere.Radius();
	//
	aDt2D=acos (1.-4.*aTol/aR)+aDelta ;
	//
	XBOPTools_Tools3D::PointNearEdge(aSpRight, aF, aT, aDt2D, aPx2DNear, aPxNear);
      }
      else {
	XBOPTools_Tools3D::PointNearEdge(aSpRight, aF, aT, aPx2DNear, aPxNear);
      }
    }
    // 
    TopAbs_State aState;
    //
    TopAbs_ShapeEnum aTypeReference;
    aTypeReference=aReference.ShapeType();

    if (aTypeReference==TopAbs_SOLID) {
      // ... \ Solid processing 
      const Handle(XIntTools_Context)& aContext=myFiller->Context();
      const TopoDS_Solid& aReferenceSolid=TopoDS::Solid(aReference);
      BRepClass3d_SolidClassifier& SC=aContext->SolidClassifier(aReferenceSolid);
      //
      SC.Perform(aPxNear, aTol);
      //
      aState=SC.State();
    }
    //
    
    else if (aTypeReference==TopAbs_SHELL || 
	     aTypeReference==TopAbs_FACE) {
      // ... \ Shell processing 
      TopoDS_Vertex aVxNear;
      BRep_Builder BB;
      
      BB.MakeVertex(aVxNear, aPxNear, aTol);

      aDSS.LoadS2(aVxNear);
      aDSS.Perform();
      
      aState=TopAbs_OUT;
      if (aDSS.IsDone()) {
	Standard_Real aDist=aDSS.Value();
	if (aDist < aTol) {
	  aState=TopAbs_ON;
	}
      }
    }
    else {
      // unknown aTypeReference
      aState=TopAbs_OUT;
    }
    //
    XBooleanOperations_StateOfShape aSt;

    aSt=XBOPTools_StateFiller::ConvertState(aState);

    myDS->SetState(nSp, aSt);
  }
}
//=======================================================================
// function:  DoStates2D
// purpose: 
//=======================================================================
  void XBOPTools_DEProcessor::DoStates2D (const Standard_Integer nED, 
					 const Standard_Integer nFD)
{
  
  
  const XBOPTools_SplitShapesPool& aSplitShapesPool=myFiller->SplitShapesPool();
  const XBOPTools_ListOfPaveBlock& aSplitEdges=aSplitShapesPool(myDS->RefEdge(nED));

  const TopoDS_Edge& aDE=TopoDS::Edge(myDS->Shape(nED));
  const TopoDS_Face& aDF=TopoDS::Face(myDS->Shape(nFD));
  
  Standard_Integer nSp, iRank;
  Standard_Real aT, aT1, aT2;
  TopoDS_Face aF;
  gp_Pnt2d aPx2DNear;
  gp_Pnt aPxNear;

  iRank=myDS->Rank(nED);
  const TopoDS_Shape& aReference=(iRank==1) ? myDS->Tool() : myDS->Object();
  const TopoDS_Face& aFaceReference=TopoDS::Face(aReference);

  aF=aDF;
  aF.Orientation(TopAbs_FORWARD);

  XBOPTools_ListIteratorOfListOfPaveBlock aPBIt(aSplitEdges);

  for (; aPBIt.More(); aPBIt.Next()) {
    XBOPTools_PaveBlock& aPB=aPBIt.Value();
    
    nSp=aPB.Edge();
    const TopoDS_Edge& aSp=TopoDS::Edge(myDS->Shape(nSp));
    
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
    Standard_Boolean bIsValidPoint;
    TopAbs_State aState=TopAbs_OUT;
    //
    const Handle(XIntTools_Context)& aContext=myFiller->Context();
    bIsValidPoint=aContext->IsValidPointForFace(aPxNear, aFaceReference, 1.e-3);
    //
    if (bIsValidPoint) {
      aState=TopAbs_IN;
    }
    //
    XBooleanOperations_StateOfShape aSt;
    
    aSt=XBOPTools_StateFiller::ConvertState(aState);

    myDS->SetState(nSp, aSt);
  }
}

//=======================================================================
// function:  FillSplitEdgesPool
// purpose: 
//=======================================================================
  void XBOPTools_DEProcessor::FillSplitEdgesPool (const Standard_Integer nED)
{
  XBOPTools_SplitShapesPool& aSplitShapesPool=myFiller->ChangeSplitShapesPool();
  //
  XBOPTools_ListOfPaveBlock& aSplitEdges=aSplitShapesPool.ChangeValue(myDS->RefEdge(nED));
  //
  aSplitEdges.Clear();
  //
  const XBOPTools_PavePool& aPavePool=myFiller->PavePool();
  XBOPTools_PavePool* pPavePool=(XBOPTools_PavePool*) &aPavePool;
  XBOPTools_PaveSet& aPaveSet= pPavePool->ChangeValue(myDS->RefEdge(nED));
  
  XBOPTools_PaveBlockIterator aPBIt(nED, aPaveSet);
  for (; aPBIt.More(); aPBIt.Next()) {
    XBOPTools_PaveBlock& aPB=aPBIt.Value();
    aSplitEdges.Append(aPB);
  }
}

//=======================================================================
// function:  MakeSplitEdges
// purpose: 
//=======================================================================
  void XBOPTools_DEProcessor::MakeSplitEdges (const Standard_Integer nED,
					     const Standard_Integer nFD)
{
  const XBOPTools_SplitShapesPool& aSplitShapesPool=myFiller->SplitShapesPool();
  const XBOPTools_ListOfPaveBlock& aSplitEdges=aSplitShapesPool(myDS->RefEdge(nED));

  Standard_Integer nV1, nV2, aNewShapeIndex;
  Standard_Real    t1, t2;
  TopoDS_Edge aE, aESplit;
  TopoDS_Vertex aV1, aV2;

  const TopoDS_Edge aDE=TopoDS::Edge(myDS->Shape(nED));
  const TopoDS_Face aDF=TopoDS::Face(myDS->Shape(nFD));

  XBOPTools_ListIteratorOfListOfPaveBlock aPBIt(aSplitEdges);

  for (; aPBIt.More(); aPBIt.Next()) {
    XBOPTools_PaveBlock& aPB=aPBIt.Value();
    
    const XBOPTools_Pave& aPave1=aPB.Pave1();
    nV1=aPave1.Index();
    t1=aPave1.Param();
    aV1=TopoDS::Vertex(myDS->GetShape(nV1));
    aV1.Orientation(TopAbs_FORWARD);
    
    const XBOPTools_Pave& aPave2=aPB.Pave2();
    nV2=aPave2.Index();
    t2=aPave2.Param();
    aV2=TopoDS::Vertex(myDS->GetShape(nV2));
    aV2.Orientation(TopAbs_REVERSED);
    
    MakeSplitEdge(aDE, aDF, aV1, t1, aV2, t2, aESplit); 
    //
    // Add Split Part of the Original Edge to the DS
    XBooleanOperations_AncestorsSeqAndSuccessorsSeq anASSeq;
    
    anASSeq.SetNewSuccessor(nV1);
    anASSeq.SetNewOrientation(aV1.Orientation());
    
    anASSeq.SetNewSuccessor(nV2);
    anASSeq.SetNewOrientation(aV2.Orientation());
    
    myDS->InsertShapeAndAncestorsSuccessors(aESplit, anASSeq);
    aNewShapeIndex=myDS->NumberOfInsertedShapes();
    myDS->SetState(aNewShapeIndex, XBooleanOperations_UNKNOWN);
    //
    // Fill Split Set for the Original Edge
    aPB.SetEdge(aNewShapeIndex);
    //
  }
}
//=======================================================================
// function:  MakeSplitEdge
// purpose: 
//=======================================================================
  void XBOPTools_DEProcessor::MakeSplitEdge (const TopoDS_Edge&   aE,
					    const TopoDS_Face&   aF,
					    const TopoDS_Vertex& aV1,
					    const Standard_Real  aP1,
					    const TopoDS_Vertex& aV2,
					    const Standard_Real  aP2,
					    TopoDS_Edge& aNewEdge)
{
  Standard_Real aTol=1.e-7;

  TopoDS_Edge E=aE;

  E.EmptyCopy();
  BRep_Builder BB;
  BB.Add  (E, aV1);
  BB.Add  (E, aV2);

  BB.Range(E, aF, aP1, aP2);

  BB.Degenerated(E, Standard_True);

  BB.UpdateEdge(E, aTol);
  aNewEdge=E;
}


			
//=======================================================================
// function:  FillPaveSet
// purpose: 
//=======================================================================
  void XBOPTools_DEProcessor::FillPaveSet (const Standard_Integer nED,
					  const Standard_Integer nVD,
					  const Standard_Integer nFD,
					  XBOPTools_ListOfPaveBlock& aLPB)
{
  Standard_Boolean bIsDone, bXDir;
  Standard_Integer nE, aNbPoints, j;
  Standard_Real aTD1, aTD2, aT1, aT2, aTolInter, aX, aDT;
  //
  aDT=Precision::PConfusion();
  //
  XBOPTools_PaveSet& aPaveSet= (myFiller->ChangePavePool()).ChangeValue(myDS->RefEdge(nED));
  //
  // Clear aPaveSet, aSplitEdges
  aPaveSet.ChangeSet().Clear();
  //
  const TopoDS_Edge& aDE=TopoDS::Edge(myDS->Shape(nED));
  const TopoDS_Face& aDF=TopoDS::Face(myDS->Shape(nFD));
  //
  // 2D Curve of degenerated edge on the face aDF
  Handle(Geom2d_Curve) aC2DDE=BRep_Tool::CurveOnSurface(aDE, aDF, aTD1, aTD2);
  //
  // Choose direction for Degenerated Edge
  gp_Pnt2d aP2d1, aP2d2;
  aC2DDE->D0(aTD1, aP2d1);
  aC2DDE->D0(aTD2, aP2d2);

  bXDir=Standard_False;
  if (fabs(aP2d1.Y()-aP2d2.Y()) < aDT){
    bXDir=!bXDir;
  }
  //
  // Prepare bounding Paves
  XBOPTools_Pave aPave1 (nVD, aTD1, XBooleanOperations_UnknownInterference);
  aPaveSet.Append(aPave1);
  XBOPTools_Pave aPave2 (nVD, aTD2, XBooleanOperations_UnknownInterference);
  aPaveSet.Append(aPave2);
  //
  // Fill other paves 
  XBOPTools_ListIteratorOfListOfPaveBlock anIt(aLPB);
  for (; anIt.More(); anIt.Next()) {
    const XBOPTools_PaveBlock& aPB=anIt.Value();
    nE=aPB.Edge();
    const TopoDS_Edge& aE=TopoDS::Edge(myDS->Shape(nE));
    
    Handle(Geom2d_Curve) aC2D=BRep_Tool::CurveOnSurface(aE, aDF, aT1, aT2);
    //
    // Intersection
    aTolInter=0.001;
    
    
    Geom2dAdaptor_Curve aGAC1, aGAC2;
    
    aGAC1.Load(aC2DDE, aTD1, aTD2);
    Handle(Geom2d_Line) aL2D= Handle(Geom2d_Line)::DownCast(aC2D);
    if (!aL2D.IsNull()) {
      aGAC2.Load(aC2D);
    }
    else {
      aGAC2.Load(aC2D, aT1, aT2);
    }
    
    Geom2dInt_GInter aGInter(aGAC1, aGAC2, aTolInter, aTolInter);
    
    bIsDone=aGInter.IsDone();
    if(bIsDone) {
      aNbPoints=aGInter.NbPoints();
      if (aNbPoints) { 
	for (j=1; j<=aNbPoints; ++j) {
	  gp_Pnt2d aP2D=aGInter.Point(j).Value();
	  //
	  aX=(bXDir) ? aP2D.X(): aP2D.Y();
	  //
	  if (fabs (aX-aTD1) < aDT || fabs (aX-aTD2) < aDT) {
	    continue; 
	  }
	  if (aX < aTD1 || aX > aTD2) {
	    continue; 
	  }
	  //
	  Standard_Boolean bRejectFlag=Standard_False;
	  const XBOPTools_ListOfPave& aListOfPave=aPaveSet.Set();
	  XBOPTools_ListIteratorOfListOfPave aPaveIt(aListOfPave);
	  for (; aPaveIt.More(); aPaveIt.Next()) {
	    const XBOPTools_Pave& aPavex=aPaveIt.Value();
	    Standard_Real aXx=aPavex.Param();
	    if (fabs (aX-aXx) < aDT) {
	      bRejectFlag=Standard_True;
	      break;
	    }
	  }
	  if (bRejectFlag) {
	    continue; 
	  }
	  //
	  XBOPTools_Pave aPave(nVD, aX, XBooleanOperations_UnknownInterference);
	  aPaveSet.Append(aPave);
	}
      }
    }
  }
}

//=======================================================================
// function:  FindPaveBlocks
// purpose: 
//=======================================================================
  void XBOPTools_DEProcessor::FindPaveBlocks(const Standard_Integer ,
					    const Standard_Integer nVD,
					    const Standard_Integer nFD,
					    XBOPTools_ListOfPaveBlock& aLPBOut)
{

  XBOPTools_CArray1OfSSInterference& aFFs=(myFiller->InterfPool())->SSInterferences();
  
  XBOPTools_ListIteratorOfListOfPaveBlock anIt;
  Standard_Integer i, aNb, nF2, nSp, nV;

  //ZZ const TopoDS_Edge& aDE=TopoDS::Edge(myDS->Shape(nED));
  
  aNb=aFFs.Extent();
  for (i=1; i<=aNb; i++) {
    XBOPTools_SSInterference& aFF=aFFs(i);
    //
    nF2=aFF.OppositeIndex(nFD);
    if (!nF2) {
      continue;
    }
    //
    // Split Parts 
    const XBOPTools_ListOfPaveBlock& aLPBSplits=aFF.PaveBlocks();
    anIt.Initialize(aLPBSplits);
    for (; anIt.More(); anIt.Next()) {
      const XBOPTools_PaveBlock& aPBSp=anIt.Value();
      nSp=aPBSp.Edge();
      
      const XBOPTools_Pave& aPave1=aPBSp.Pave1();
      nV=aPave1.Index();
      if (nV==nVD) {
	aLPBOut.Append(aPBSp);
	continue;
      }
      
      const XBOPTools_Pave& aPave2=aPBSp.Pave2();
      nV=aPave2.Index();
      if (nV==nVD) {
	aLPBOut.Append(aPBSp);
	continue;
      }
    }
    //
    // Section Parts
    Standard_Integer j, aNbCurves;   
    XBOPTools_SequenceOfCurves& aSC=aFF.Curves();
    aNbCurves=aSC.Length();
    
    for (j=1; j<=aNbCurves; j++) {
      const XBOPTools_Curve& aBC=aSC(j);
      const XBOPTools_ListOfPaveBlock& aLPBSe=aBC.NewPaveBlocks();

      anIt.Initialize(aLPBSe);
      for (; anIt.More(); anIt.Next()) {
	const XBOPTools_PaveBlock& aPBSe=anIt.Value();
	
	const XBOPTools_Pave& aPv1=aPBSe.Pave1();
	nV=aPv1.Index();
	if (nV==nVD) {
	  aLPBOut.Append(aPBSe);
	  continue;
	}
	
	const XBOPTools_Pave& aPv2=aPBSe.Pave2();
	nV=aPv2.Index();
	if (nV==nVD) {
	  aLPBOut.Append(aPBSe);
	  continue;
	}
      }
    }

  } // for (i=1; i<=aNb; i++) Next FF interference
  
}
