// Created on: 2001-03-29
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



#include <XBOP_Builder.ixx>

#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopExp.hxx>

#include <XBooleanOperations_ShapesDataStructure.hxx>

#include <XBOPTools_SplitShapesPool.hxx>
#include <XBOPTools_CommonBlockPool.hxx>
#include <XBOPTools_ListOfPaveBlock.hxx>
#include <XBOPTools_ListOfCommonBlock.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <XBOPTools_ListIteratorOfListOfCommonBlock.hxx>
#include <XBOPTools_CommonBlock.hxx>
#include <XBOPTools_PaveBlock.hxx>
#include <XBOPTools_ListIteratorOfListOfPaveBlock.hxx>

//=======================================================================
// function: XBOP_Builder::XBOP_Builder
// purpose: 
//=======================================================================
XBOP_Builder::XBOP_Builder() 
: myOperation(XBOP_UNKNOWN),
  myManifoldFlag(Standard_False),
  myIsDone(Standard_False),
  myErrorStatus(1),
  myDraw(0)
{
}
//=======================================================================
// function: Destroy
// purpose: 
//=======================================================================
  void XBOP_Builder::Destroy()
{
}

//=======================================================================
// function: SetShapes
// purpose: 
//=======================================================================
  void XBOP_Builder::SetShapes (const TopoDS_Shape& aS1,
			       const TopoDS_Shape& aS2)
{
  myShape1=aS1;
  myShape2=aS2;
}
//=======================================================================
// function: SetOperation
// purpose: 
//=======================================================================
  void XBOP_Builder::SetOperation (const XBOP_Operation anOp) 
{
  myOperation=anOp;
}
	 

//=======================================================================
// function: SetManifoldFlag
// purpose: 
//=======================================================================
  void XBOP_Builder::SetManifoldFlag (const Standard_Boolean aFlag)
{
  myManifoldFlag=aFlag;
}
//=======================================================================
// function: Shape1
// purpose: 
//=======================================================================
  const TopoDS_Shape& XBOP_Builder::Shape1()const
{
  return myShape1;
}

//=======================================================================
// function: Shape2
// purpose: 
//=======================================================================
  const TopoDS_Shape& XBOP_Builder::Shape2()const
{
  return myShape2;
}
//=======================================================================
// function: Operation
// purpose: 
//=======================================================================
  XBOP_Operation XBOP_Builder::Operation () const
{
  return myOperation;
}
	
//=======================================================================
// function: ManifoldFlag
// purpose: 
//=======================================================================
  Standard_Boolean XBOP_Builder::ManifoldFlag () const 
{
  return myManifoldFlag;
}
//=======================================================================
// function: IsDone
// purpose: 
//=======================================================================
  Standard_Boolean XBOP_Builder::IsDone() const 
{
  return myIsDone;
}
//=======================================================================
// function: ErrorStatus
// purpose: 
//=======================================================================
  Standard_Integer XBOP_Builder::ErrorStatus() const 
{
  return myErrorStatus;
}
//=======================================================================
// function: Result
// purpose: 
//=======================================================================
  const TopoDS_Shape& XBOP_Builder::Result()const
{
  return myResult;
}

//=======================================================================
// function: Do
// purpose: 
//=======================================================================
  void XBOP_Builder::Do()
{
}
//=======================================================================
// function: DoDoWithFiller
// purpose: 
//=======================================================================
  void XBOP_Builder::DoWithFiller(const XBOPTools_DSFiller& )
{
} 

//=======================================================================
// function: BuildResult
// purpose: 
//=======================================================================
  void XBOP_Builder::BuildResult()
{
}
//
//
//=======================================================================
// function: FillModified
// purpose: 
//=======================================================================
  void XBOP_Builder::FillModified()
{
  //
  // Prepare myResultMap
  myResultMap.Clear();
  TopExp::MapShapes(myResult, TopAbs_FACE, myResultMap);
  TopExp::MapShapes(myResult, TopAbs_EDGE, myResultMap);
  //
  // Fill Modified for Edges
  Standard_Integer i, aNbSources, aNbPaveBlocks, nSp, nFace;
  //
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  const XBOPTools_SplitShapesPool& aSplitShapesPool= myDSFiller->SplitShapesPool();
  const XBOPTools_CommonBlockPool& aCommonBlockPool= myDSFiller->CommonBlockPool();
  //
  aNbSources=aDS.NumberOfSourceShapes();
  for (i=1; i<=aNbSources; i++) {
    
    if (aDS.GetShapeType(i) != TopAbs_EDGE){
      continue;
    }
    //
    const TopoDS_Shape& aE=aDS.Shape(i);
    //
    const XBOPTools_ListOfPaveBlock& aSplitEdges=aSplitShapesPool(aDS.RefEdge(i));
    const XBOPTools_ListOfCommonBlock& aCBlocks =aCommonBlockPool(aDS.RefEdge(i));
    //
    aNbPaveBlocks=aSplitEdges.Extent();
    if (!aNbPaveBlocks) {
      continue;
    }
    //
    TopTools_IndexedMapOfShape aM;
    
    XBOPTools_ListIteratorOfListOfCommonBlock aCBIt(aCBlocks);
    for (; aCBIt.More(); aCBIt.Next()) {
      XBOPTools_CommonBlock& aCB=aCBIt.Value();
      //
      nFace=aCB.Face();
      //
      if (nFace) {
	XBOPTools_PaveBlock& aPB=aCB.PaveBlock1(i);
	nSp=aPB.Edge();
	const TopoDS_Shape& aSp=aDS.Shape(nSp);
	//
	if (aM.Contains(aSp)) {
	  continue;
	}
	aM.Add(aSp);
	//
	if (myResultMap.Contains(aSp)) {
	  FillModified(aE, aSp);
	}
      }
      //
      else {
	const XBOPTools_PaveBlock& aPB=aCB.PaveBlock1();
	nSp=aPB.Edge();
	const TopoDS_Shape& aSp=aDS.Shape(nSp);
	//
	if (aM.Contains(aSp)) {
	  continue;
	}
	aM.Add(aSp);
	//
	if (myResultMap.Contains(aSp)) {
	  FillModified(aE, aSp);
	}
      }
    } // for (; aCBIt.More(); aCBIt.Next())
    
    
    XBOPTools_ListIteratorOfListOfPaveBlock aPBIt(aSplitEdges);
    for (; aPBIt.More(); aPBIt.Next()) {
      const XBOPTools_PaveBlock& aPB=aPBIt.Value();
      nSp=aPB.Edge();
      const TopoDS_Shape& aSp=aDS.Shape(nSp);
      //
      if (aM.Contains(aSp)) {
	continue;
      }
      aM.Add(aSp);
      //
      if (myResultMap.Contains(aSp)) {
	FillModified(aE, aSp);
      }
    }// for (; aPBIt.More(); aPBIt.Next())
  }
}
//=======================================================================
// function: FillModified
// purpose: 
//=======================================================================
  void XBOP_Builder::FillModified(const TopoDS_Shape& aE,
				 const TopoDS_Shape& aSp)
{
  if (myModifiedMap.Contains(aE)) {
    TopTools_ListOfShape& aLM=myModifiedMap.ChangeFromKey(aE);
    aLM.Append(aSp);
  }
  else {
    TopTools_ListOfShape aLM;
    aLM.Append(aSp);
    myModifiedMap.Add(aE, aLM);
  }
}
//=======================================================================
// function: FillModified
// purpose: 
//=======================================================================
  void XBOP_Builder::FillModified(const TopoDS_Shape& aS,
				 const TopTools_ListOfShape& aLFx)
{
  TopTools_ListIteratorOfListOfShape anIt(aLFx);
  //
  if (myModifiedMap.Contains(aS)) {
    TopTools_ListOfShape& aLM=myModifiedMap.ChangeFromKey(aS);
    anIt.Initialize(aLFx);
    for (; anIt.More(); anIt.Next()) {
      const TopoDS_Shape& aFx=anIt.Value();
      aLM.Append(aFx);
    }
  }
  
  else {
    TopTools_ListOfShape aLM;
    anIt.Initialize(aLFx);
    for (; anIt.More(); anIt.Next()) {
      const TopoDS_Shape& aFx=anIt.Value();
      aLM.Append(aFx);
    }
    myModifiedMap.Add(aS, aLM);
  }
}
//=======================================================================
// function: Modified
// purpose: 
//=======================================================================
  const TopTools_ListOfShape& XBOP_Builder::Modified(const TopoDS_Shape& aS)const
{
  if (myModifiedMap.Contains(aS)) {
    const TopTools_ListOfShape& aLM=myModifiedMap.FindFromKey(aS);
    return aLM;
  }
  else {
    return myEmptyList;
  }
}
//=======================================================================
// function: IsDeleted
// purpose: 
//=======================================================================
  Standard_Boolean XBOP_Builder::IsDeleted(const TopoDS_Shape& aS)const
{
  Standard_Boolean bFlag=Standard_False;
  //
  if (myResultMap.Contains(aS)) {
    return bFlag;
  }
  //
  const TopTools_ListOfShape& aLM=Modified(aS);
  if (aLM.Extent()) {
    return bFlag;
  }
  //
  return !bFlag;
}


//=======================================================================
//function : SortTypes
//purpose  : 
//=======================================================================
  void XBOP_Builder::SortTypes(TopAbs_ShapeEnum& aType1,
			      TopAbs_ShapeEnum& aType2)
{ 
  Standard_Integer iT1, iT2;

  if (aType1==aType2)
    return;
  
  iT1=(Standard_Integer) aType1;
  iT2=(Standard_Integer) aType2;
  
  if (iT1 < iT2) {
    aType1=(TopAbs_ShapeEnum) iT2;
    aType2=(TopAbs_ShapeEnum) iT1;
  }
}

//=======================================================================
// function: SectionEdges
// purpose: 
//=======================================================================
  const TopTools_ListOfShape& XBOP_Builder::SectionEdges()const
{
  return mySectionEdges;
}


//=======================================================================
// function: SetHistoryCollector
// purpose: 
//=======================================================================
void XBOP_Builder::SetHistoryCollector(const Handle(XBOP_HistoryCollector)& theHistory) 
{
  myHistory = theHistory;
}

//=======================================================================
// function: GetHistoryCollector
// purpose: 
//=======================================================================
Handle(XBOP_HistoryCollector) XBOP_Builder::GetHistoryCollector() const
{
  return myHistory;
}
