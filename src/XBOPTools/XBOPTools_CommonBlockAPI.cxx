// Created on: 2001-03-14
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



#include <XBOPTools_CommonBlockAPI.ixx>

#include <XBOPTools_CommonBlock.hxx>
#include <XBOPTools_PaveBlock.hxx>
#include <XBOPTools_ListOfPaveBlock.hxx>
#include <XBOPTools_ListOfCommonBlock.hxx>
#include <XBOPTools_ListIteratorOfListOfCommonBlock.hxx>
#include <XBOPTools_ListIteratorOfListOfPaveBlock.hxx>


//=======================================================================
// function:  XBOPTools_CommonBlockAPI::XBOPTools_CommonBlockAPI
// purpose: 
//=======================================================================
XBOPTools_CommonBlockAPI::XBOPTools_CommonBlockAPI
     (const XBOPTools_ListOfCommonBlock& aLCB)
{
  myListOfCommonBlock=(void *)&aLCB;
}
//=======================================================================
// function:  List
// purpose: 
//=======================================================================
  const XBOPTools_ListOfCommonBlock& XBOPTools_CommonBlockAPI::List () const
{
  XBOPTools_ListOfCommonBlock* pListOfCommonBlock=
    (XBOPTools_ListOfCommonBlock*)myListOfCommonBlock;
  return *pListOfCommonBlock;
}
//=======================================================================
// function:  CommonPaveBlocks
// purpose:   get all CommonPaveBlocks
//=======================================================================
  const XBOPTools_ListOfPaveBlock&
    XBOPTools_CommonBlockAPI::CommonPaveBlocks
      (const Standard_Integer anE) const
{
  Standard_Integer anECurrent, i;

  XBOPTools_ListOfPaveBlock* pmyListOfPaveBlock=
    (XBOPTools_ListOfPaveBlock*) &myListOfPaveBlock;
  pmyListOfPaveBlock->Clear();

  XBOPTools_ListOfCommonBlock* pListOfCommonBlock=
    (XBOPTools_ListOfCommonBlock*)myListOfCommonBlock;

  XBOPTools_ListIteratorOfListOfCommonBlock anIt(*pListOfCommonBlock);
  for (; anIt.More(); anIt.Next()) {
    const XBOPTools_CommonBlock& aCB=anIt.Value();
    
    for (i=0; i<2; i++) {
      const XBOPTools_PaveBlock& aPB=(!i) ? aCB.PaveBlock1() : aCB.PaveBlock2();
      anECurrent=aPB.OriginalEdge();
      if (anECurrent==anE) {
	pmyListOfPaveBlock->Append(aPB);
	break;
      }
    }
  }

  return myListOfPaveBlock;
}
//=======================================================================
// function:  IsCommonBlock
// purpose: 
//=======================================================================
  Standard_Boolean XBOPTools_CommonBlockAPI::IsCommonBlock 
    (const XBOPTools_PaveBlock& aPB) const
{
  Standard_Integer anE;

  anE=aPB.OriginalEdge();

  CommonPaveBlocks(anE);
  
  XBOPTools_ListIteratorOfListOfPaveBlock anIt(myListOfPaveBlock);
  for (; anIt.More(); anIt.Next()) {
    const XBOPTools_PaveBlock& aPB1=anIt.Value();
    if (aPB1.IsEqual(aPB)) {
      return Standard_True;
    }
  }
  return Standard_False;
}

