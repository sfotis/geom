// Created on: 2001-10-29
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


#include <XBOP_ShellShell.ixx>


#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Compound.hxx>

#include <BRep_Tool.hxx>
#include <BRep_Builder.hxx>

#include <TopAbs_Orientation.hxx>
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>

#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>

#include <TopTools_IndexedMapOfShape.hxx>

#include <TColStd_IndexedMapOfInteger.hxx>

#include <XBOPTColStd_Dump.hxx>

#include <XBooleanOperations_ShapesDataStructure.hxx>
#include <XBooleanOperations_StateOfShape.hxx>

#include <XBOPTools_PaveFiller.hxx>
#include <XBOPTools_InterferencePool.hxx>
#include <XBOPTools_CArray1OfSSInterference.hxx>
#include <XBOPTools_CArray1OfInterferenceLine.hxx>
#include <XBOPTools_InterferenceLine.hxx>

#include <XBOPTColStd_IndexedDataMapOfIntegerIndexedMapOfInteger.hxx>
#include <XBOPTools_SSInterference.hxx>

#include <XBOP_WireEdgeSet.hxx>
#include <XBOP_SDFWESFiller.hxx>
#include <XBOP_FaceBuilder.hxx>
#include <XBOP_Draw.hxx>
#include <XBOP_CorrectTolerances.hxx>
#include <XBOP_BuilderTools.hxx>
#include <XBOP_Refiner.hxx>
#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>
#include <TopTools_DataMapOfShapeInteger.hxx>



//=======================================================================
// function: XBOP_ShellShell::XBOP_ShellShell
// purpose: 
//=======================================================================
XBOP_ShellShell::XBOP_ShellShell()
{
}

//=======================================================================
// function: Destroy
// purpose: 
//=======================================================================
void XBOP_ShellShell::Destroy() {
}

//=======================================================================
// function: DoWithFiller
// purpose: 
//=======================================================================
void XBOP_ShellShell::DoWithFiller(const XBOPTools_DSFiller& aDSFiller) 
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
    //
    Standard_Boolean bIsNewFiller;
    bIsNewFiller=aDSFiller.IsNewFiller();
    
    if (bIsNewFiller) {
      Prepare();
      aDSFiller.SetNewFiller(!bIsNewFiller);
    }
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

//=================================================================================
// function: BuildResult
// purpose: 
//=================================================================================
void XBOP_ShellShell::BuildResult()
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  
  Standard_Integer i, j, aNb, iRank, aNbFaces;
  XBooleanOperations_StateOfShape aState, aStateToCompare;
  BRep_Builder aBB;
  TopoDS_Compound aFCompound, aRes;
  //
  Standard_Boolean bHasInterference;
  XBOPTools_InterferencePool* pInterfPool=
    (XBOPTools_InterferencePool*) &myDSFiller->InterfPool();
  XBOPTools_CArray1OfInterferenceLine* pTable=
    (XBOPTools_CArray1OfInterferenceLine*) &pInterfPool->InterferenceTable();
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
      //
      XBOPTools_InterferenceLine& anInterfLine=pTable->ChangeValue(i);
      bHasInterference=anInterfLine.HasInterference();
      if (bHasInterference) {
	continue;
      }
      //
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
  void XBOP_ShellShell::DoNewFaces() 
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  XBOPTools_InterferencePool* pIntrPool=(XBOPTools_InterferencePool*)&myDSFiller->InterfPool();
  XBOPTools_CArray1OfSSInterference& aFFs=pIntrPool->SSInterferences();
  //
  // vars
  Standard_Boolean bIsTouchCase, bIsTouch;
  Standard_Integer i, aNb, j, aNbj, iFF, nF1, iRank, nF2;
  TopTools_ListOfShape aListOfNewFaces;
  TopTools_IndexedMapOfShape anEMap;
  TopAbs_Orientation anOriF1;
  //
  // DoMap
  XBOPTColStd_IndexedDataMapOfIntegerIndexedMapOfInteger aFFMap;
  XBOP_BuilderTools::DoMap(aFFs, aFFMap);
  //
  aNb=aFFMap.Extent();
  //
  for (i=1; i<=aNb; i++) {
    // 
    // a. Prepare info about the Face nF1 and create WES for nF1
    nF1=aFFMap.FindKey(i);
    const TopoDS_Face& aF1=TopoDS::Face(aDS.Shape(nF1));
    
    anOriF1=aF1.Orientation();
    iRank=aDS.Rank(nF1);
    
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
	  AddSectionPartsSh(nF1, iFF, aWES);
	}
      }
      // 3. Add IN2D, ON2D Parts to the WES 
      //
      //modified by NIZNHY-PKV Fri Sep 14 10:00:44 2012f
      XBOP_WireEdgeSet aWES1 (myFace);
      //
      for (j=1; j<=aNbj; j++) {
	iFF=aFFIndicesMap(j);
	XBOPTools_SSInterference& aFF=aFFs(iFF);
	bIsTouch=aFF.IsTangentFaces();
	if (bIsTouch) {
	  nF2=aFF.OppositeIndex(nF1);
	  AddINON2DPartsSh(nF1, iFF, aWES1);
	}
      }
      //
      if (iRank==2 || (iRank==1 && myOperation==XBOP_CUT)) {
	// #0023431
	// Refine WES to remove duplicated edges: 
	// - for the faces of the Object: Cut operation
	// - for the faces of the Tool: all operations
	//
	// The duplications caused by the separated treatment
	// the faces of an argument for the cases when: 
	// -these faces contain shared edges and 
	// -they are same domain faces with the faces of the other argument.
	TopTools_DataMapOfShapeInteger aDMSI;

	//--
	aWES1.InitStartElements();
	for (; aWES1.MoreStartElements(); aWES1.NextStartElement()) {
	  const TopoDS_Edge& aE=*((TopoDS_Edge*)&aWES1.StartElement()); 
	  if (!aDMSI.IsBound(aE)) {
	    Standard_Integer iCnt=1;
	    //
	    aDMSI.Bind(aE, iCnt);
	  }
	  else {
	    Standard_Integer& iCnt=aDMSI.ChangeFind(aE);
	    ++iCnt;
	  }
	}
	//
	aWES1.InitStartElements();
	for (; aWES1.MoreStartElements(); aWES1.NextStartElement()) {
	  const TopoDS_Shape& aE=aWES1.StartElement(); 
	  const Standard_Integer& iCnt=aDMSI.Find(aE);
	  if (iCnt==1) {
	    aWES.AddStartElement(aE);
	  }
	}
      }
      else {
	aWES1.InitStartElements();
	for (; aWES1.MoreStartElements(); aWES1.NextStartElement()) {
	  const TopoDS_Shape& aE=aWES1.StartElement(); 
	  aWES.AddStartElement(aE);
	}
      }
      //--
      /*
      for (j=1; j<=aNbj; j++) {
	iFF=aFFIndicesMap(j);
	XBOPTools_SSInterference& aFF=aFFs(iFF);
	bIsTouch=aFF.IsTangentFaces();
	if (bIsTouch) {
	  Standard_Integer nF2;
	  nF2=aFF.OppositeIndex(nF1);
	  AddINON2DPartsSh(nF1, iFF, aWES);
	}
      }
      */
      //modified by NIZNHY-PKV Fri Sep 14 10:00:48 2012t
      // 4. Add EF parts (E (from F2) on F1 ),
      // where F2 is non-same-domain face to F1
      anEMap.Clear();
      //
      // anEMap will contain all Split parts that has already in aWES
      const TopTools_ListOfShape& aLE=aWES.StartElements();

      Standard_Integer aNbEdges1 = aLE.Extent();

      TopTools_ListIteratorOfListOfShape anIt;
      anIt.Initialize (aLE);
      for (; anIt.More(); anIt.Next()) {
	TopoDS_Shape& anE=anIt.Value();
	anEMap.Add(anE);
      }
      //
      // IFV's workaround for occ13538:
      // It is necessary to avoid building SD faces twice in case if SD faces of object and tool
      // fully coincide and face of object has adjacent faces along all boundaries.
      // For such cases WES for second SD faces are built from EE edges.
      // The sence of workarond is to find such situation by checking of number of EF edges.
      // If number of EF edges == 0, it means that SD faces fully coincide.
      Standard_Integer aNbEF;
      for (j=1; j<=aNbj; j++) {
	iFF=aFFIndicesMap(j);
	XBOPTools_SSInterference& aFF=aFFs(iFF);
	bIsTouch=aFF.IsTangentFaces();
	if (!bIsTouch) {
	  AddPartsEFNonSDSh (nF1, iFF, anEMap, aWES);
	}
      }
      //
      aNbEF = aWES.StartElements().Extent() - aNbEdges1;
      //
      if((aNbEdges1 > 0) && (aNbEF > 0)) {
	for (j=1; j<=aNbj; j++) {
	  iFF=aFFIndicesMap(j);
	  XBOPTools_SSInterference& aFF=aFFs(iFF);
	  bIsTouch=aFF.IsTangentFaces();
	  if (!bIsTouch) {
	    AddPartsEENonSDSh (nF1, iFF, anEMap, aWES);
	  }
	}
      }
      // IFV's workaround for occ13538 - end
      //
    }// end of  if (bIsTouchCase)
    else {
      // 1. Add Split Parts having states in accordance with operation
      AddSplitPartsINOUT (nF1, aWES);
      // 2. Add Split Parts with state ON
      AddSplitPartsONSh (nF1, aWES);
      // 3. Add Section Edges to the WES 
      for (j=1; j<=aNbj; j++) {
	iFF=aFFIndicesMap(j);
	AddSectionPartsSh(nF1, iFF, aWES);
      }
      // 4. Add EF parts (E (from F2) on F1 )
      anEMap.Clear();
      for (j=1; j<=aNbj; j++) {
	iFF=aFFIndicesMap(j);
	AddPartsEFSh(nF1, iFF, anEMap, aWES);
      }
    }// end of (bIsTouchCase)'s else
    //
    //
    // d. Build new Faces from myFace
    XBOP_FaceBuilder aFB;
    aFB.SetTreatment(0); // 0-Do Internal Edges
    aFB.SetTreatSDScales(1);
    aFB.Do(aWES);

    const TopTools_ListOfShape& aLF=aFB.NewFaces();
    // 
    // e. Do Internal Vertices
    // 
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
    }
    //
    // Fill "Modified"
    FillModified(aF1, aLFx); 
    //
  }//  for (i=1; i<=aNb; i++)
  //
 
  myNewFaces.Clear();
  myNewFaces.Append(aListOfNewFaces);
}
/* DEB
    {
      TopoDS_Compound aCx;
      BRep_Builder aBB;
      //
      aBB.MakeCompound(aCx);
      aBB.Add(aCx, myFace);
      //
      aWES.InitStartElements();
      for (; aWES.MoreStartElements(); aWES.NextStartElement()) {
	const TopoDS_Shape& aE = aWES.StartElement(); 
	aBB.Add(aCx, aE);
      }
      int a=0;
    }
 
*/
