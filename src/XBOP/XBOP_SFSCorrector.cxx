// Created on: 2001-08-02
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


#include <XBOP_SFSCorrector.ixx>

#include <TopTools_IndexedMapOfOrientedShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>

#include <BRep_Builder.hxx>

#include <XBOP_ConnexityBlock.hxx>
#include <XBOP_ListIteratorOfListOfConnexityBlock.hxx>
#include <XBOP_ShellSplitter.hxx>
#include <XBOPTColStd_ListOfListOfShape.hxx>
#include <XBOPTColStd_ListIteratorOfListOfListOfShape.hxx>


static
  void MakeShell (const TopTools_ListOfShape& aLE, 
		  TopoDS_Shell& newShell);

//=======================================================================
// function: XBOP_SFSCorrector::XBOP_SFSCorrector
// purpose: 
//=======================================================================
  XBOP_SFSCorrector::XBOP_SFSCorrector()
:
  myIsDone(Standard_False),
  myErrorStatus(1)
{}
//=======================================================================
// function: SetSFS
// purpose: 
//=======================================================================
  void XBOP_SFSCorrector::SetSFS (const XBOP_ShellFaceSet& aSFS)
{
  XBOP_ShellFaceSet* pSFS=(XBOP_ShellFaceSet*) &aSFS;
  mySFS=pSFS;
}
//=======================================================================
// function: SFS
// purpose: 
//=======================================================================
  XBOP_ShellFaceSet& XBOP_SFSCorrector::SFS () 
{
  return *mySFS;
}
//=======================================================================
// function: NewSFS
// purpose: 
//=======================================================================
  XBOP_ShellFaceSet& XBOP_SFSCorrector::NewSFS () 
{
  return myNewSFS;
}
//=======================================================================
// function: IsDone
// purpose: 
//=======================================================================
  Standard_Boolean XBOP_SFSCorrector::IsDone () const 
{
  return myIsDone;
}
//=======================================================================
// function: ErrorStatus
// purpose: 
//=======================================================================
  Standard_Integer XBOP_SFSCorrector::ErrorStatus () const 
{
  return myErrorStatus;
}
//=======================================================================
// function: Do
// purpose: 
//=======================================================================
  void XBOP_SFSCorrector::Do()
{
  DoConnexityBlocks();
  DoCorrections();
  myIsDone=Standard_True;
}
//=======================================================================
// function: DoConnexityBlocks
// purpose: 
//=======================================================================
  void XBOP_SFSCorrector::DoConnexityBlocks()
{
  Standard_Boolean EnewinM, aGoOn;
  Standard_Integer Mextent, IsRegular, aNbNeighbours, Eindex;
  TopTools_IndexedMapOfOrientedShape myOrientedShapeMap, aMap;
  //
  mySFS->InitStartElements();
  for (; mySFS->MoreStartElements(); mySFS->NextStartElement()) {
    const TopoDS_Shape& anE = mySFS->StartElement(); 
    Mextent = myOrientedShapeMap.Extent();

    Eindex  = myOrientedShapeMap.Add(anE); 

    EnewinM = (Eindex > Mextent);
    if (EnewinM) {
      //
      // make a new block starting at element Eindex
      IsRegular=Standard_True; 
      aNbNeighbours=0;
      Mextent = myOrientedShapeMap.Extent();
      //
      aMap.Clear();
      aMap.Add(anE);
      //
      aGoOn = (Eindex <= Mextent);
      while (aGoOn) {
	const TopoDS_Shape& anEE = myOrientedShapeMap(Eindex);
	aNbNeighbours = mySFS->MaxNumberSubShape(anEE);
	
	IsRegular = IsRegular && (aNbNeighbours == 2);
	//
	mySFS->InitNeighbours(anEE);
	for (; mySFS->MoreNeighbours(); mySFS->NextNeighbour()) {
	  const TopoDS_Shape& aNeignbE = mySFS->Neighbour();
	  myOrientedShapeMap.Add(aNeignbE);
	  //
	  aMap.Add(aNeignbE);
	}
	
	Eindex++;
	Mextent = myOrientedShapeMap.Extent();
	aGoOn = (Eindex <= Mextent);
      } // end of while aGoOn
      
      XBOP_ConnexityBlock aConnexityBlock;
      aConnexityBlock.SetShapes(aMap);
      aConnexityBlock.SetRegularity(IsRegular);
      myConnexityBlocks.Append(aConnexityBlock);
    } // end of if (EnewinM)
  } // end of for (; mySFS->MoreStartElements();...
}

//=======================================================================
// function: DoCorrections
// purpose: 
//=======================================================================
  void XBOP_SFSCorrector::DoCorrections()
{
  Standard_Boolean anIsRegular, anIsNothingToDo, anIsDone;
  TopoDS_Shell aShell;
  XBOP_ListIteratorOfListOfConnexityBlock aCBIt;
  
  //myNewSFS.Initialize(mySFS->Face());
  anIsDone=Standard_False;
  anIsNothingToDo=Standard_True;

  aCBIt.Initialize(myConnexityBlocks);
  for (; aCBIt.More(); aCBIt.Next()) {
    const XBOP_ConnexityBlock& aCB=aCBIt.Value();
    const TopTools_ListOfShape& aListOfEdges=aCB.Shapes();

    anIsRegular=aCB.IsRegular();

    if (anIsRegular) {
      MakeShell(aListOfEdges, aShell);
      myNewSFS.AddShape (aShell);
      continue;
    }
    //
    // Treatment of non regular Connexity Block
    XBOP_ShellSplitter aShellSplitter;
    aShellSplitter.DoWithListOfEdges(aListOfEdges);
    
    anIsDone=aShellSplitter.IsDone();
    anIsNothingToDo=aShellSplitter.IsNothingToDo();
    
    if (!anIsDone || anIsNothingToDo) {
      MakeShell(aListOfEdges, aShell);
      myNewSFS.AddShape (aShell);
      continue;
    }

    //
    const XBOPTColStd_ListOfListOfShape& aSSS=aShellSplitter.Shapes();
    
    XBOPTColStd_ListIteratorOfListOfListOfShape aShellIt(aSSS);
    for (; aShellIt.More(); aShellIt.Next()) {
      const TopTools_ListOfShape& aListF=aShellIt.Value();
      //
      MakeShell(aListF, aShell);
      myNewSFS.AddShape (aShell);
    }
  }
}

//=======================================================================
// function: MakeShell
// purpose: 
//=======================================================================
  void MakeShell(const TopTools_ListOfShape& aLE, 
		 TopoDS_Shell& newShell)
{
  BRep_Builder aBB;
  aBB.MakeShell(newShell);

  TopTools_ListIteratorOfListOfShape anIt(aLE);
  for (; anIt.More(); anIt.Next()){
    const TopoDS_Face& aF=TopoDS::Face(anIt.Value());
    aBB.Add(newShell, aF);
  }
}
/////////////////////////////////////////////////////////////////
//
// myErrorStatus:
//
// 1 - Nothing is done because only constructor has been called
//
/////////////////////////////////////////////////////////////////
