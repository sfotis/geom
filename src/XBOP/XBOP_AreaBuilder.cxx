// Created on: 1995-12-21
// Created by: Jean Yves LEBEY
// Copyright (c) 1995-1999 Matra Datavision
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


// " Voyager, c'est bien utile, ca fait travailler l'imagination.
//   Tout le reste n'est que deceptions et fatigues. Notre voyage 
//   a nous est entierement imaginaire. Voila sa force. "
//                         Celine
//                         Voyage au bout de la nuit

#include <XBOP_AreaBuilder.ixx>

#include <Standard_DomainError.hxx>
#include <TopAbs.hxx>

//=======================================================================
//function : XBOP_AreaBuilder::XBOP_AreaBuilder
//purpose  : 
//=======================================================================
XBOP_AreaBuilder::XBOP_AreaBuilder() 
:
  myUNKNOWNRaise(Standard_False) 
{
}

//=======================================================================
//function : TopOpeBRepBuild_AreaBuilder
//purpose  : 
//=======================================================================
  XBOP_AreaBuilder::XBOP_AreaBuilder (XBOP_LoopSet&        LS,
				    XBOP_LoopClassifier& LC,
				    const Standard_Boolean ForceClass)
:
  myUNKNOWNRaise(Standard_False) 
{
  InitAreaBuilder(LS, LC, ForceClass);
}
//=======================================================================
//function : Delete
//purpose  : 
//=======================================================================
  void XBOP_AreaBuilder::Delete()
{}

//=======================================================================
//function : CompareLoopWithListOfLoop
//purpose  : Compare position of the Loop <L> with the Area <LOL>
//           using the Loop Classifier <LC>.
//           According to <whattotest>, Loops of <LOL> are selected or not
//           during <LOL> exploration.
//result   : TopAbs_OUT if <LOL> is empty
//           TopAbs_UNKNOWN if position undefined
//           TopAbs_IN  if <L> is inside all the selected Loops of <LOL>
//           TopAbs_OUT if <L> is outside one of the selected Loops of <LOL>
//           TopAbs_ON  if <L> is on one of the selected Loops of <LOL>
//=======================================================================
  TopAbs_State XBOP_AreaBuilder::CompareLoopWithListOfLoop (XBOP_LoopClassifier &LC,
							   const Handle(XBOP_Loop)& L,
							   const XBOP_ListOfLoop &LOL,
							   const XBOP_LoopEnum   what) const
{
  TopAbs_State                 state = TopAbs_UNKNOWN;
  Standard_Boolean             totest; // L must or not be tested
  XBOP_ListIteratorOfListOfLoop LoopIter;  
  
  if ( LOL.IsEmpty() ) {
    return TopAbs_OUT;
  }

  LoopIter.Initialize(LOL);
  for (; LoopIter.More(); LoopIter.Next() ) {
    const Handle(XBOP_Loop)& curL = LoopIter.Value();
    switch ( what ) { 
    case XBOP_ANYLOOP  : 
      totest = Standard_True;
      break;
    case XBOP_BOUNDARY : 
      totest =  curL->IsShape();
      break;
    case XBOP_BLOCK    : 
      totest = !curL->IsShape();
      break;
    default:
      totest = Standard_False;
      break;
    }
    if ( totest ) {
      state = LC.Compare(L,curL);
      if (state == TopAbs_OUT) 
	// <L> is out of at least one Loop 
	//of <LOL> : stop to explore
	break;  
    }
  }
  return state;
}

//=======================================================================
//function : Atomize
//purpose  : 
//=======================================================================
  void XBOP_AreaBuilder::Atomize(TopAbs_State& state, 
				const TopAbs_State newstate) const
{
  if (myUNKNOWNRaise) {
    Standard_DomainError_Raise_if((state == TopAbs_UNKNOWN),
				  "AreaBuilder : Position Unknown");
  }
  else {
    state = newstate;
  }
}


//=======================================================================
//function : InitAreaBuilder
//purpose  : 
//=======================================================================
  void XBOP_AreaBuilder::InitAreaBuilder(XBOP_LoopSet& LS,
					XBOP_LoopClassifier& LC,
					const Standard_Boolean ForceClass)
{
  TopAbs_State     state;
  Standard_Boolean Loopinside;
  Standard_Boolean loopoutside;
  
  XBOP_ListIteratorOfListOfListOfLoop AreaIter;
  XBOP_ListIteratorOfListOfLoop       LoopIter;
  // boundaryloops : list of boundary loops out of the areas.
  XBOP_ListOfLoop                     boundaryloops; 
  
  myArea.Clear();          // Clear the list of Area to be built
  
  for (LS.InitLoop(); LS.MoreLoop(); LS.NextLoop()) {
    
    // process a new loop : L is the new current Loop
    const Handle(XBOP_Loop)& L = LS.Loop();
    Standard_Boolean boundaryL = L->IsShape();
    
    // L = Shape et ForceClass  : on traite L comme un block
    // L = Shape et !ForceClass : on traite L comme un pur Shape
    // L = !Shape               : on traite L comme un block
    Standard_Boolean traitercommeblock = (!boundaryL) || ForceClass;
    if ( ! traitercommeblock ) {

      // the loop L is a boundary loop : 
      // - try to insert it in an existing area, such as L is inside all 
      //   the block loops. Only block loops of the area are compared. 
      // - if L could not be inserted, store it in list of boundary loops.

      Loopinside = Standard_False; 
      for (AreaIter.Initialize(myArea); AreaIter.More(); AreaIter.Next()) {
	XBOP_ListOfLoop& aArea = AreaIter.Value();
	if ( aArea.IsEmpty() ) continue;
	state = CompareLoopWithListOfLoop(LC,L,aArea,XBOP_BLOCK );
	if (state == TopAbs_UNKNOWN) {
	  Atomize(state,TopAbs_IN);
	}	
	Loopinside = ( state == TopAbs_IN);
	if ( Loopinside ) {
	  break;
	}
      } // end of Area scan

      if ( Loopinside ) {
	XBOP_ListOfLoop& aArea = AreaIter.Value();
	ADD_Loop_TO_LISTOFLoop(L,aArea);
      }
      else if ( ! Loopinside ) {
	ADD_Loop_TO_LISTOFLoop(L,boundaryloops);
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
	if ( aArea.IsEmpty() ) {
	  continue;
	} 	
	state = CompareLoopWithListOfLoop(LC,L,aArea,XBOP_ANYLOOP);
	if (state == TopAbs_UNKNOWN) Atomize(state,TopAbs_IN);
 	Loopinside = (state == TopAbs_IN);
	if ( Loopinside ) {
	  break;
	}
      } // end of Area scan
      
      if ( Loopinside) {
	XBOP_ListOfLoop& aArea = AreaIter.Value();
	Standard_Boolean allShape = Standard_True;
	XBOP_ListOfLoop removedLoops;
	
	LoopIter.Initialize(aArea);
	while (LoopIter.More()) {
	  state = LC.Compare(LoopIter.Value(),L);
	  if (state == TopAbs_UNKNOWN){
	    Atomize(state,TopAbs_IN); // not OUT
	  }
	  
	  loopoutside = ( state == TopAbs_OUT );
	  
	  if ( loopoutside ) {
	    const Handle(XBOP_Loop)& curL = LoopIter.Value();
	    // remove the loop from the area
	    ADD_Loop_TO_LISTOFLoop (curL,removedLoops);
	    
	    allShape = allShape && curL->IsShape();
	    REM_Loop_FROM_LISTOFLoop(LoopIter,AreaIter.Value());
	  }
	  else {
	    LoopIter.Next();
	  }
	}
	// insert the loop in the area
	ADD_Loop_TO_LISTOFLoop(L,aArea);
	if ( ! removedLoops.IsEmpty() ) {
	  if ( allShape ) {
	    ADD_LISTOFLoop_TO_LISTOFLoop(removedLoops,boundaryloops);
	  }
	  else {
	    // make a new area with the removed loops
            XBOP_ListOfLoop thelistofloop;
	    myArea.Append(thelistofloop);
	    ADD_LISTOFLoop_TO_LISTOFLoop (removedLoops,myArea.Last());
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
	  state = LC.Compare(LoopIter.Value(),L);
	  if (state == TopAbs_UNKNOWN) {
	    Atomize(state,TopAbs_IN);
          }
	 
	  ashapeinside = (state == TopAbs_IN);
          if (ashapeinside) {
	    state = LC.Compare(L,LoopIter.Value());
	    if (state == TopAbs_UNKNOWN){
	      Atomize(state,TopAbs_IN);
	    }	    
	    ablockinside = (state == TopAbs_IN);
	  }

	  if ( ashapeinside && ablockinside ) {
	    const Handle(XBOP_Loop)& curL = LoopIter.Value();
	    ADD_Loop_TO_LISTOFLoop(curL, newArea0);

	    REM_Loop_FROM_LISTOFLoop(LoopIter,boundaryloops);
	  }
          else { 
	    LoopIter.Next();
	  }
	} // end of boundaryloops scan
      } // Loopinside == False
    } // end of block loop
  } // end of LoopSet LS scan
  
  InitArea();
}

//=======================================================================
//function : InitArea
//purpose  : 
//=======================================================================
  Standard_Integer XBOP_AreaBuilder::InitArea()
{
  myAreaIterator.Initialize(myArea);
  InitLoop();
  Standard_Integer n = myArea.Extent();
  return n;
}

//=======================================================================
//function : MoreArea
//purpose  : 
//=======================================================================
  Standard_Boolean XBOP_AreaBuilder::MoreArea() const
{
  Standard_Boolean b = myAreaIterator.More();
  return b;
}

//=======================================================================
//Function : NextArea
//Purpose  : 
//=======================================================================
  void XBOP_AreaBuilder::NextArea()
{
  myAreaIterator.Next();
  InitLoop();
}

//=======================================================================
//function : InitLoop
//purpose  : 
//=======================================================================
  Standard_Integer XBOP_AreaBuilder::InitLoop()
{
  Standard_Integer n = 0;
  if (myAreaIterator.More()) {
    const XBOP_ListOfLoop& LAL = myAreaIterator.Value();
    myLoopIterator.Initialize(LAL);
    n = LAL.Extent();
  }
  else { // Create an empty ListIteratorOfListOfLoop
    myLoopIterator = XBOP_ListIteratorOfListOfLoop();  
  }
  return n;
}

//=======================================================================
//function : MoreLoop
//purpose  : 
//=======================================================================
  Standard_Boolean XBOP_AreaBuilder::MoreLoop() const
{
  Standard_Boolean b = myLoopIterator.More();
  return b;
}

//=======================================================================
//function : NextLoop
//purpose  : 
//=======================================================================
  void XBOP_AreaBuilder::NextLoop()
{
  myLoopIterator.Next();
}

//=======================================================================
//function : Loop
//purpose  : 
//=======================================================================
  const Handle(XBOP_Loop)& XBOP_AreaBuilder::Loop() const
{
  const Handle(XBOP_Loop)& L = myLoopIterator.Value();
  return L;
}

//=======================================================================
//function : ADD_Loop_TO_LISTOFLoop
//purpose  : 
//=======================================================================
  void XBOP_AreaBuilder::ADD_Loop_TO_LISTOFLoop(const Handle(XBOP_Loop)& L,
					       XBOP_ListOfLoop& LOL) const
{
  LOL.Append(L);
}

//=======================================================================
//function : REM_Loop_FROM_LISTOFLoop
//purpose  : 
//=======================================================================
  void XBOP_AreaBuilder::REM_Loop_FROM_LISTOFLoop(XBOP_ListIteratorOfListOfLoop& ITA,
						 XBOP_ListOfLoop& A) const
{
  A.Remove(ITA);
}

//=======================================================================
//function : ADD_LISTOFLoop_TO_LISTOFLoop
//purpose  : 
//=======================================================================
  void XBOP_AreaBuilder::ADD_LISTOFLoop_TO_LISTOFLoop(XBOP_ListOfLoop& A1,
						     XBOP_ListOfLoop& A2) const
						     
{
  A2.Append(A1);
}
