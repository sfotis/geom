// Copyright (c) 1999-2012 OPEN CASCADE SAS
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

#include <XBOP_Area3dBuilder.ixx>

#include <XBOP_Loop.hxx>

// ==================================================================================
// function: XBOP_Area3dBuilder::XBOP_Area3dBuilder
// purpose: 
// ==================================================================================
XBOP_Area3dBuilder::XBOP_Area3dBuilder()
{
}

// ==================================================================================
// function: XBOP_Area3dBuilder::XBOP_Area3dBuilder
// purpose: 
// ==================================================================================
  XBOP_Area3dBuilder::XBOP_Area3dBuilder(XBOP_LoopSet&           theLS,
				       XBOP_LoopClassifier&    theLC,
				       const Standard_Boolean theForceClassFlag)
{
  InitAreaBuilder(theLS, theLC, theForceClassFlag);
}

// ==================================================================================
// function: InitAreaBuilder
// purpose: 
// ==================================================================================
  void XBOP_Area3dBuilder::InitAreaBuilder(XBOP_LoopSet&           theLS,
					  XBOP_LoopClassifier&    theLC,
					  const Standard_Boolean theForceClassFlag) 
{
  TopAbs_State     state;
  Standard_Boolean Loopinside;
  Standard_Boolean loopoutside;
  
  XBOP_ListIteratorOfListOfListOfLoop AreaIter;
  XBOP_ListIteratorOfListOfLoop       LoopIter;
  // boundaryloops : list of boundary loops out of the areas.
  XBOP_ListOfLoop                     boundaryloops; 
  
  myArea.Clear();          // Clear the list of Area to be built
  
  for (theLS.InitLoop(); theLS.MoreLoop(); theLS.NextLoop()) {
    
    // process a new loop : L is the new current Loop
    const Handle(XBOP_Loop)& L = theLS.Loop();
    Standard_Boolean boundaryL = L->IsShape();
    
    // L = shape et theForceClassFlag  : on traite L comme un block
    // L = shape et !theForceClassFlag : on traite L comme un pur shape
    // L = !shape               : on traite L comme un block
    Standard_Boolean traitercommeblock = (!boundaryL) || theForceClassFlag;
    if ( ! traitercommeblock ) {

      // the loop L is a boundary loop : 
      // - try to insert it in an existing area, such as L is inside all 
      //   the block loops. Only block loops of the area are compared. 
      // - if L could not be inserted, store it in list of boundary loops.

      Loopinside = Standard_False; 
      for (AreaIter.Initialize(myArea); AreaIter.More(); AreaIter.Next()) {
	XBOP_ListOfLoop& aArea = AreaIter.Value();
	if ( aArea.IsEmpty() ) continue;
	state = CompareLoopWithListOfLoop(theLC,L,aArea,XBOP_BLOCK );
	if (state == TopAbs_UNKNOWN) Atomize(state,TopAbs_IN);
	Loopinside = ( state == TopAbs_IN);
	if ( Loopinside ) break;
      } // end of Area scan

      if ( Loopinside ) {
	XBOP_ListOfLoop& aArea = AreaIter.Value();
	ADD_Loop_TO_LISTOFLoop(L,aArea);
      }
      else if ( ! Loopinside ) {
	ADD_Loop_TO_LISTOFLoop(L, boundaryloops);
      }

    } // end of boundary loop
    
    else { 
      // the loop L is a block loop
      // if L is IN theArea :
      //   - stop area scan, insert L in theArea.
      //   - remove from the area all the loops outside L
      //   - make a new area with them, unless they are all boundary
      //   - if they are all boundary put them back in boundaryLoops
      // else :
      //   - create a new area with L.
      //   - insert boundary loops that are IN the new area
      //     (and remove them from 'boundaryloops')
      
      Loopinside = Standard_False;
      for (AreaIter.Initialize(myArea); AreaIter.More(); AreaIter.Next() ) {
	XBOP_ListOfLoop& aArea = AreaIter.Value();
	if ( aArea.IsEmpty() ) continue;
 	state = CompareLoopWithListOfLoop(theLC,L,aArea,XBOP_ANYLOOP);
	if (state == TopAbs_UNKNOWN) Atomize(state,TopAbs_IN);
 	Loopinside = (state == TopAbs_IN);
	if ( Loopinside ) break;
      } // end of Area scan
      
      if ( Loopinside) {
	XBOP_ListOfLoop& aArea = AreaIter.Value();
	Standard_Boolean allShape = Standard_True;
	XBOP_ListOfLoop removedLoops;
	LoopIter.Initialize(aArea);
	while (LoopIter.More()) {
	  const Handle(XBOP_Loop)& LLI1 = LoopIter.Value();
	  state = theLC.Compare(LLI1,L);
	  if (state == TopAbs_UNKNOWN) Atomize(state,TopAbs_IN); // not OUT
	  loopoutside = ( state == TopAbs_OUT );
	  if ( loopoutside ) {
	    const Handle(XBOP_Loop)& curL = LoopIter.Value();
	    // remove the loop from the area
	    ADD_Loop_TO_LISTOFLoop
	      (curL,removedLoops);
	    
	    allShape = allShape && curL->IsShape();
	    REM_Loop_FROM_LISTOFLoop
	      (LoopIter, AreaIter.Value());
	  }
	  else {
	    LoopIter.Next();
	  }
	}
	// insert the loop in the area
	ADD_Loop_TO_LISTOFLoop(L,aArea);
	if ( ! removedLoops.IsEmpty() ) {
	  if ( allShape ) {
	    ADD_LISTOFLoop_TO_LISTOFLoop(removedLoops, boundaryloops);
	  }
	  else {
	    // make a new area with the removed loops
            XBOP_ListOfLoop thelistofloop;
	    myArea.Append(thelistofloop);
	    ADD_LISTOFLoop_TO_LISTOFLoop(removedLoops, myArea.Last());
	  }
	}
      } // Loopinside == True
      
      else {
        Standard_Integer ashapeinside,ablockinside;
	XBOP_ListOfLoop thelistofloop1;
	myArea.Append(thelistofloop1);
	XBOP_ListOfLoop& newArea0 = myArea.Last();
	ADD_Loop_TO_LISTOFLoop(L, newArea0);
	
        LoopIter.Initialize(boundaryloops);
        while ( LoopIter.More() ) {
          ashapeinside = ablockinside = Standard_False;
	  const Handle(XBOP_Loop)& LLI2 = LoopIter.Value();
	  state = theLC.Compare(LLI2,L);
	  if (state == TopAbs_UNKNOWN) Atomize(state,TopAbs_IN);
          ashapeinside = (state == TopAbs_IN);
          if (ashapeinside) {
	    const Handle(XBOP_Loop)& LLI3 = LoopIter.Value();
	    state = theLC.Compare(L,LLI3);
	    if (state == TopAbs_UNKNOWN) Atomize(state,TopAbs_IN);
	    ablockinside = (state == TopAbs_IN);
	  }
	  if ( ashapeinside && ablockinside ) {
	    const Handle(XBOP_Loop)& curL = LoopIter.Value();
	    ADD_Loop_TO_LISTOFLoop(curL, newArea0);

	    REM_Loop_FROM_LISTOFLoop(LoopIter, boundaryloops);
	  }
          else { 
	    LoopIter.Next();
	  }
	} // end of boundaryloops scan
      } // Loopinside == False
    } // end of block loop
  } // end of LoopSet theLS scan
  
  InitArea();
}

