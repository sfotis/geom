// Created on: 2001-05-18
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



#include <XBOP_Section.ixx>

#include <XBooleanOperations_ShapesDataStructure.hxx>

#include <TopTools_IndexedMapOfShape.hxx>

#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>

#include <BRep_Builder.hxx>

#include <XBOPTools_SSInterference.hxx>
#include <XBOPTools_ListOfPaveBlock.hxx>
#include <XBOPTools_ListIteratorOfListOfPaveBlock.hxx>
#include <XBOPTools_PaveBlock.hxx>
#include <XBOPTools_SequenceOfCurves.hxx>
#include <XBOPTools_Curve.hxx>
#include <XBOPTools_InterferencePool.hxx>
#include <XBOPTools_CArray1OfSSInterference.hxx>
#include <XBOPTools_PaveFiller.hxx>
#include <XBOPTools_SSIntersectionAttribute.hxx>
#include <XBOPTools_Tools2D.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Curve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS.hxx>

#include <XBOP_CorrectTolerances.hxx>

#include <XBOPTColStd_Dump.hxx>

#include <XBOP_SectionHistoryCollector.hxx>

#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>

//=======================================================================
// function: XBOP_Section::XBOP_Section
// purpose: 
//=======================================================================
XBOP_Section::XBOP_Section() 
{
  SetOperation (XBOP_SECTION);
}
//=======================================================================
// function: Destroy
// purpose: 
//=======================================================================
  void XBOP_Section::Destroy() {}

//=======================================================================
// function: Do
// purpose: 
//=======================================================================
  void XBOP_Section::Do()
{
  myErrorStatus=0;
  myIsDone=Standard_False;

  TopAbs_ShapeEnum aT1, aT2;

  aT1=myShape1.ShapeType();
  aT2=myShape2.ShapeType();

  XBOP_Builder::SortTypes (aT1, aT2);
  //
  // Filling the DS
  XBOPTools_DSFiller aDSFiller;
  aDSFiller.SetShapes (myShape1, myShape2);
  aDSFiller.Perform (); 
  DoWithFiller(aDSFiller);
}

//
//=======================================================================
// function: Do
// purpose: 
//=======================================================================
  void XBOP_Section::Do(const Standard_Boolean toApprox,
		       const Standard_Boolean toComputePCurve1,
		       const Standard_Boolean toComputePCurve2) 
{
  myErrorStatus=0;
  myIsDone=Standard_False;

  TopAbs_ShapeEnum aT1, aT2;

  aT1=myShape1.ShapeType();
  aT2=myShape2.ShapeType();

  XBOP_Builder::SortTypes (aT1, aT2);
  //
  // Filling the DS
  XBOPTools_DSFiller aDSFiller;
  aDSFiller.SetShapes (myShape1, myShape2);

  XBOPTools_SSIntersectionAttribute aSectionAttribute(toApprox, 
						     toComputePCurve1, 
						     toComputePCurve2);
  aDSFiller.Perform (aSectionAttribute); 
  DoWithFiller(aDSFiller);
}
//

//=======================================================================
// function: DoDoWithFiller
// purpose: 
//=======================================================================
  void XBOP_Section::DoWithFiller(const XBOPTools_DSFiller& aDSFiller) 
{
  myErrorStatus=0;
  myIsDone=Standard_False;

  //
  myResultMap.Clear();
  myModifiedMap.Clear();
  myDSFiller=(XBOPTools_DSFiller*) &aDSFiller;
  //

  try {
    OCC_CATCH_SIGNALS
    Standard_Boolean addPCurve1 = aDSFiller.PaveFiller().SectionAttribute().PCurveOnS1();
    Standard_Boolean addPCurve2 = aDSFiller.PaveFiller().SectionAttribute().PCurveOnS2();
  
    Standard_Integer i, j, nF1, nF2,  aNbFFs, aNbS, aNbCurves, nSect;
    
    const XBooleanOperations_ShapesDataStructure& aDS=aDSFiller.DS();
    const XBOPTools_InterferencePool& anInterfPool=aDSFiller.InterfPool();
    XBOPTools_InterferencePool* pInterfPool= 
      (XBOPTools_InterferencePool*) &anInterfPool;
    XBOPTools_CArray1OfSSInterference& aFFs=
      pInterfPool->SSInterferences();
    //
    TopTools_IndexedMapOfShape aMap;
    //
    aNbFFs=aFFs.Extent();
    for (i=1; i<=aNbFFs; ++i) {
      XBOPTools_SSInterference& aFFi=aFFs(i);
      //
      nF1=aFFi.Index1();
      nF2=aFFi.Index2();
      
      TopoDS_Face aF1FWD = TopoDS::Face(aDSFiller.DS().Shape(nF1));
      aF1FWD.Orientation(TopAbs_FORWARD);
      TopoDS_Face aF2FWD = TopoDS::Face(aDSFiller.DS().Shape(nF2));
      aF2FWD.Orientation(TopAbs_FORWARD);
      //
      // Old Section Edges
      const XBOPTools_ListOfPaveBlock& aSectList=aFFi.PaveBlocks();
      aNbS=aSectList.Extent();
      XBOPTools_ListIteratorOfListOfPaveBlock anIt(aSectList);
      for (; anIt.More();anIt.Next()) {
	const XBOPTools_PaveBlock& aPB=anIt.Value();
	nSect=aPB.Edge();
	const TopoDS_Shape& aS=aDS.GetShape(nSect);
	
	const TopoDS_Edge& aE = TopoDS::Edge(aS);
	
	if(addPCurve1) {
	  XBOPTools_Tools2D::BuildPCurveForEdgeOnFace(aE, aF1FWD);
	  }
	
	if(addPCurve2) {
	  XBOPTools_Tools2D::BuildPCurveForEdgeOnFace(aE, aF2FWD);
	  }
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
	  
	  if(addPCurve1 || addPCurve2) {
	    const XIntTools_Curve& aIC = aBC.Curve();
	    const TopoDS_Edge& aE = TopoDS::Edge(aS);
	    Standard_Real f, l;
	    const Handle(Geom_Curve)& aC3DE = BRep_Tool::Curve(aE, f, l);
	    Handle(Geom_TrimmedCurve) aC3DETrim;
	    
	    if(!aC3DE.IsNull()) {
	      aC3DETrim = new Geom_TrimmedCurve(aC3DE, f, l);
	    }
	    BRep_Builder aBB;
	    Standard_Real aTolEdge = BRep_Tool::Tolerance(aE);
	    Standard_Real aTolR2D  = aFFi.TolR2D();
	    Standard_Real aTolFact = Max(aTolEdge, aTolR2D);
	    
	    if(addPCurve1 && !XBOPTools_Tools2D::HasCurveOnSurface(aE, aF1FWD)) {
	      Handle(Geom2d_Curve) aC2d = aIC.FirstCurve2d();
	      
	      if(!aC3DETrim.IsNull()) {
		Handle(Geom2d_Curve) aC2dNew;
		
		if(aC3DE->IsPeriodic()) {
		  XBOPTools_Tools2D::AdjustPCurveOnFace(aF1FWD, f, l,  aC2d, aC2dNew);
		  }
		else {
		  XBOPTools_Tools2D::AdjustPCurveOnFace(aF1FWD, aC3DETrim, aC2d, aC2dNew); 
		  }
		aC2d = aC2dNew;
	      }
	      aBB.UpdateEdge(aE, aC2d, aF1FWD, aTolFact);
	    }
	    
	    if(addPCurve2 && !XBOPTools_Tools2D::HasCurveOnSurface(aE, aF2FWD)) {
	      Handle(Geom2d_Curve) aC2d = aIC.SecondCurve2d();
	      
	      if(!aC3DETrim.IsNull()) {
		Handle(Geom2d_Curve) aC2dNew;
		
		if(aC3DE->IsPeriodic()) {
		  XBOPTools_Tools2D::AdjustPCurveOnFace(aF2FWD, f, l,  aC2d, aC2dNew);
		  }
		else {
		  XBOPTools_Tools2D::AdjustPCurveOnFace(aF2FWD, aC3DETrim, aC2d, aC2dNew); 
		  }
		aC2d = aC2dNew;
	      }
	      aBB.UpdateEdge(aE, aC2d, aF2FWD, aTolFact);
	    }
	  }
	  aMap.Add(aS);
	}
      }
    }
    //
    BRep_Builder BB;
    TopoDS_Compound aCompound;
    BB.MakeCompound(aCompound);
    
    aNbS=aMap.Extent();
    
    for (i=1; i<=aNbS; i++) {
      const TopoDS_Shape& aS=aMap(i);
      BB.Add(aCompound, aS);
      mySectionEdges.Append(aS);
    }
    myResult=aCompound;
    XBOP_CorrectTolerances::CorrectTolerances(myResult, 0.01);
    //
    if (!myErrorStatus) {
      FillModified();

      if(!myHistory.IsNull()) {
	Handle(XBOP_SectionHistoryCollector) aHistory = 
	  Handle(XBOP_SectionHistoryCollector)::DownCast(myHistory);
	aHistory->SetResult(myResult, myDSFiller);
      }
      myIsDone=Standard_True;
    }
  }
  catch ( Standard_Failure ) {
    myErrorStatus = 1;
    XBOPTColStd_Dump::PrintMessage("Can not build result\n");
  }
}

//=======================================================================
// function: SetHistoryCollector
// purpose: 
//=======================================================================
void XBOP_Section::SetHistoryCollector(const Handle(XBOP_HistoryCollector)& theHistory) 
{
  if(theHistory.IsNull() ||
     !theHistory->IsKind(STANDARD_TYPE(XBOP_SectionHistoryCollector)))
    myHistory.Nullify();
  else
    myHistory = theHistory;
}
