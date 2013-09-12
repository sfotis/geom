// Created on: 2001-02-08
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



#include <XBOPTools_PaveSet.ixx>

#include <XBOPTools_Array1OfPave.hxx>
#include <XBOPTools_ListIteratorOfListOfPave.hxx>
#include <XBOPTools_QuickSortPave.hxx>
#include <XBOPTools_ComparePave.hxx>

#ifdef WNT
#pragma warning ( disable : 4101 )
#endif

//=======================================================================
// function: XBOPTools_PaveSet::XBOPTools_PaveSet
// purpose: 
//=======================================================================
  XBOPTools_PaveSet::XBOPTools_PaveSet() {}

//=======================================================================
// function: ChangeSet
// purpose: 
//=======================================================================
  XBOPTools_ListOfPave& XBOPTools_PaveSet::ChangeSet()
{
  return myPaveList;
}
//=======================================================================
// function: Set
// purpose: 
//=======================================================================
  const XBOPTools_ListOfPave& XBOPTools_PaveSet::Set() const
{
  return myPaveList;
}
//=======================================================================
// function: Append
// purpose: 
//=======================================================================
  void XBOPTools_PaveSet::Append(const XBOPTools_Pave& aPave)
{
  myPaveList.Append(aPave);
}
//=======================================================================
// function: SortSet
// purpose: 
//=======================================================================
  void XBOPTools_PaveSet::SortSet()
{
  // Not implemented yet
  Standard_Integer aNbPaves, i;
  aNbPaves=myPaveList.Extent();
  if (aNbPaves>1) {
    XBOPTools_Array1OfPave anArray1OfPave (1, aNbPaves);
    
    XBOPTools_ListIteratorOfListOfPave anIt(myPaveList);
    
    for (i=1; anIt.More(); anIt.Next(), i++) {
      const XBOPTools_Pave& aPave=anIt.Value();
      anArray1OfPave(i)=aPave;
    }
    
    XBOPTools_QuickSortPave aQuickSortPave;
    XBOPTools_ComparePave   aComparePave;
    aQuickSortPave.Sort (anArray1OfPave, aComparePave);
    
    myPaveList.Clear();
    for (i=1; i<=aNbPaves; i++){
      const XBOPTools_Pave& aPave=anArray1OfPave(i);
      myPaveList.Append (aPave);
    }
  }
}
#ifdef WNT
#pragma warning ( default : 4101 )
#endif
