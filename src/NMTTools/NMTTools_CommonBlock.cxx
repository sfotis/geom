// Copyright (C) 2007-2013  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// File:        NMTTools_CommonBlock.cxx
// Created:     Tue Dec  9 12:23:29 2003
// Author:      Peter KURNEV
//              <pkv@irinox>
//
#include <NMTTools_CommonBlock.hxx>

#include <XBOPTools_ListIteratorOfListOfPaveBlock.hxx>
#include <TColStd_ListIteratorOfListOfInteger.hxx>

//=======================================================================
// function:  NMTTools_CommonBlock::NMTTools_CommonBlock()
// purpose:
//=======================================================================
  NMTTools_CommonBlock::NMTTools_CommonBlock()
{
}
//=======================================================================
// function:  AddPaveBlock
// purpose:
//=======================================================================
  void NMTTools_CommonBlock::AddPaveBlock(const XBOPTools_PaveBlock& aPB)
{
  myPaveBlocks.Append(aPB);
}
//=======================================================================
// function:  AddFace
// purpose:
//=======================================================================
  void NMTTools_CommonBlock::AddFace(const Standard_Integer aF)
{
  myFaces.Append(aF);
}
//=======================================================================
// function:  AddFaces
// purpose:
//=======================================================================
  void NMTTools_CommonBlock::AddFaces(const TColStd_ListOfInteger& aLF)
{
  TColStd_ListIteratorOfListOfInteger aIt(aLF);
  //
  for (; aIt.More(); aIt.Next()) {
    myFaces.Append(aIt.Value());
  }
}
//=======================================================================
// function:  PaveBlocks
// purpose:
//=======================================================================
  const XBOPTools_ListOfPaveBlock& NMTTools_CommonBlock::PaveBlocks()const
{
  return myPaveBlocks;
}
//=======================================================================
// function:  Faces
// purpose:
//=======================================================================
  const TColStd_ListOfInteger& NMTTools_CommonBlock::Faces()const
{
  return myFaces;
}
//=======================================================================
// function:  PaveBlock1
// purpose:
//=======================================================================
  const XBOPTools_PaveBlock& NMTTools_CommonBlock::PaveBlock1()const
{
  return myPaveBlocks.First();
}
//=======================================================================
// function:  PaveBlock1
// purpose:
//=======================================================================
   XBOPTools_PaveBlock& NMTTools_CommonBlock::PaveBlock1(const Standard_Integer aIx)
{
  return PaveBlockOnEdge(aIx);
}
//=======================================================================
// function:  PaveBlockOnEdge
// purpose:
//=======================================================================
   XBOPTools_PaveBlock& NMTTools_CommonBlock::PaveBlockOnEdge(const Standard_Integer aIx)
{
  static XBOPTools_PaveBlock aPBs;
  Standard_Integer aIOr;
  XBOPTools_ListIteratorOfListOfPaveBlock anIt(myPaveBlocks);
  //
  for (; anIt.More(); anIt.Next()) {
    XBOPTools_PaveBlock& aPB=anIt.Value();
    aIOr=aPB.OriginalEdge();
    if (aIOr==aIx){
      return aPB;
    }
  }
  return aPBs;
}
//=======================================================================
// function:  IsPaveBlockOnFace
// purpose:
//=======================================================================
  Standard_Boolean NMTTools_CommonBlock::IsPaveBlockOnFace(const Standard_Integer aIx)const
{
  Standard_Boolean bFound=Standard_False;
  Standard_Integer nF;
  TColStd_ListIteratorOfListOfInteger anIt(myFaces);
  //
  for (; anIt.More(); anIt.Next()) {
    nF=anIt.Value();
    if (nF==aIx){
      return !bFound;
    }
  }
  return bFound;
}
//=======================================================================
// function:  IsPaveBlockOnEdge
// purpose:
//=======================================================================
  Standard_Boolean NMTTools_CommonBlock::IsPaveBlockOnEdge(const Standard_Integer aIx)const
{
  Standard_Boolean bFound=Standard_False;
  Standard_Integer aIOr;
  XBOPTools_ListIteratorOfListOfPaveBlock anIt(myPaveBlocks);
  //
  for (; anIt.More(); anIt.Next()) {
    const XBOPTools_PaveBlock& aPB=anIt.Value();
    aIOr=aPB.OriginalEdge();
    if (aIOr==aIx){
      return !bFound;
    }
  }
  return bFound;
}
//=======================================================================
// function:  IsEqual
// purpose:
//=======================================================================
  Standard_Boolean NMTTools_CommonBlock::IsEqual(const NMTTools_CommonBlock& aOther)const
{
  Standard_Boolean bFound=Standard_True;
  Standard_Integer aNb1, aNb2;
  XBOPTools_ListIteratorOfListOfPaveBlock anIt;
  //
  aNb1=myPaveBlocks.Extent();
  aNb2=aOther.myPaveBlocks.Extent();
  //
  if (!aNb1 && !aNb2) {
    return bFound;
  }
  if (!aNb1) {
    return !bFound;
  }
  if (!aNb2) {
    return !bFound;
  }
  //
  const XBOPTools_PaveBlock& aPB=PaveBlock1();
  //
  anIt.Initialize(aOther.myPaveBlocks);
  for (; anIt.More(); anIt.Next()) {
    const XBOPTools_PaveBlock& aPBOther=anIt.Value();
    if (aPB.IsEqual(aPBOther)){
      return bFound;
    }
  }
  return !bFound;
}
//=======================================================================
// function:  Contains
// purpose:
//=======================================================================
  Standard_Boolean NMTTools_CommonBlock::Contains(const XBOPTools_PaveBlock& aPBx)const
{
  Standard_Boolean bFound=Standard_False;
  Standard_Integer aNb1;
  XBOPTools_ListIteratorOfListOfPaveBlock anIt;
  //
  aNb1=myPaveBlocks.Extent();
  //
  if (!aNb1) {
    return bFound;
  }
  //
  anIt.Initialize(myPaveBlocks);
  for (; anIt.More(); anIt.Next()) {
    const XBOPTools_PaveBlock& aPB=anIt.Value();
    if (aPB.IsEqual(aPBx)) {
      return !bFound;
    }
  }
  return bFound;
}
/*
//=======================================================================
// function:  SetEdge
// purpose:
//=======================================================================
  void NMTTools_CommonBlock::SetEdge(const Standard_Integer anEdge)
{
  XBOPTools_ListIteratorOfListOfPaveBlock anIt;
  //
  anIt.Initialize(myPaveBlocks);
  for (; anIt.More(); anIt.Next()) {
    XBOPTools_PaveBlock& aPB=anIt.Value();
    aPB.SetEdge(anEdge);
  }
}
//=======================================================================
// function:  Edge
// purpose:
//=======================================================================
  Standard_Integer NMTTools_CommonBlock::Edge()const
{
  Standard_Integer aNb;
  //
  aNb=myPaveBlocks.Extent();
  //
  if (!aNb) {
    return aNb;
  }
  //
  const XBOPTools_PaveBlock& aPB=PaveBlock1();
  aNb=aPB.Edge();
  return aNb;
}
*/
