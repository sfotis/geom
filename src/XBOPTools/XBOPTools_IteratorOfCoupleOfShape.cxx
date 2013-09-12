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


#include <XBOPTools_IteratorOfCoupleOfShape.ixx>

#include <XBooleanOperations_ShapesDataStructure.hxx>
#include <XBOPTools_RoughShapeIntersector.hxx>
#include <XBOPTools_CoupleOfInteger.hxx>

// ================================================================================================
// function: Constructor
// ================================================================================================
XBOPTools_IteratorOfCoupleOfShape::XBOPTools_IteratorOfCoupleOfShape() 
:
  myCurrentIndex1(-1),
  myCurrentIndex2(-1),
  myType1(TopAbs_SHAPE),
  myType2(TopAbs_SHAPE),
  myFirstLowerIndex(1),
  myFirstUpperIndex(0),
  mySecondLowerIndex(1),
  mySecondUpperIndex(0)
{
  myPDS = NULL;
}

// ================================================================================================
// function: Constructor
// ================================================================================================
  XBOPTools_IteratorOfCoupleOfShape::XBOPTools_IteratorOfCoupleOfShape(const XBooleanOperations_PShapesDataStructure& PDS,
								     const TopAbs_ShapeEnum Type1,
								     const TopAbs_ShapeEnum Type2)
 :
  myCurrentIndex1(-1),
  myCurrentIndex2(-1),
  myType1(TopAbs_SHAPE),
  myType2(TopAbs_SHAPE)
{
  SetDataStructure(PDS);

  Initialize(Type1, Type2);
}

// ================================================================================================
// function: virtual destructor
// ================================================================================================
void XBOPTools_IteratorOfCoupleOfShape::Destroy() 
{
}

// ================================================================================================
// function: SetDataStructure
// ================================================================================================
  void XBOPTools_IteratorOfCoupleOfShape::SetDataStructure(const XBooleanOperations_PShapesDataStructure& PDS) 
{
  if(PDS==NULL) {
    Standard_NoSuchObject::Raise("XBOPTools_IteratorOfCoupleOfShape::SetDataStructure: PDS==NULL");
  }

  myListOfCouple.Clear();

  myPDS = PDS;

  XBOPTools_RoughShapeIntersector aRoughIntersector(myPDS);
  aRoughIntersector.Perform();
  if(aRoughIntersector.IsDone()) {
    myTableOfStatus = aRoughIntersector.TableOfStatus();
  } else {
    Handle(XBOPTools_HArray2OfIntersectionStatus) anemptyhandle;
    myTableOfStatus = anemptyhandle;
  }
}


// ================================================================================================
// function: Initialize
// ================================================================================================
  void XBOPTools_IteratorOfCoupleOfShape::Initialize(const TopAbs_ShapeEnum Type1,
						    const TopAbs_ShapeEnum Type2) 
{

  if(myPDS==NULL) {
    Standard_NoSuchObject::Raise("XBOPTools_IteratorOfCoupleOfShape::Initialize: myPDS==NULL");
  }
  myType1 = Type1;
  myType2 = Type2;
  myCurrentIndex1 = -1;
  myCurrentIndex2 = -1;
  //
  myFirstLowerIndex=1;
  myFirstUpperIndex=myPDS->NumberOfShapesOfTheObject();
  mySecondLowerIndex=myFirstUpperIndex+1;
  mySecondUpperIndex=myFirstUpperIndex+myPDS->NumberOfShapesOfTheTool();
  
  NextP();

  Standard_Integer n1, n2;
  
  myListOfCouple.Clear();
  for (; MoreP(); NextP()) {
    CurrentP(n1, n2);
    XBOPTools_CoupleOfInteger aCouple(n1, n2);
    myListOfCouple.Append(aCouple);
  }
  myIterator.Initialize(myListOfCouple);
}

//=======================================================================
// function: More
// purpose: 
//=======================================================================
  Standard_Boolean XBOPTools_IteratorOfCoupleOfShape::More()const
{
  return myIterator.More();
} 
//=======================================================================
// function: Next
// purpose: 
//=======================================================================
  void XBOPTools_IteratorOfCoupleOfShape::Next()
{
  myIterator.Next();
} 
//=======================================================================
// function: Current
// purpose: 
//=======================================================================
  void XBOPTools_IteratorOfCoupleOfShape::Current(Standard_Integer& Index1,
						 Standard_Integer& Index2,
						 Standard_Boolean& WithSubShape) const
{
  WithSubShape = Standard_False;

  const XBOPTools_CoupleOfInteger& aCouple=myIterator.Value();
  aCouple.Couple(Index1, Index2);

  XBOPTools_IntersectionStatus aStatus = myTableOfStatus->Value(Index1, Index2);

  if(aStatus == XBOPTools_BOUNDINGBOXOFSUBSHAPESINTERSECTED) {
    WithSubShape = Standard_True;
  }
} 

//=======================================================================
// function: ListOfCouple
// purpose: 
//=======================================================================
  const XBOPTools_ListOfCoupleOfInteger& XBOPTools_IteratorOfCoupleOfShape::ListOfCouple() const
{
  return myListOfCouple;
}

// ================================================================================================
// function: NextP
// ================================================================================================
  void XBOPTools_IteratorOfCoupleOfShape::NextP() 
{
  if(myPDS==NULL) {
    myCurrentIndex1 = -1;
    myCurrentIndex2 = -1;
    return;
  } 
  
  Standard_Boolean couplefound, IsValidTableOfStatus = Standard_False;
  Standard_Integer stopedindex1, stopedindex2, starti, i, startj, j;
  TopAbs_ShapeEnum aTypei, aTypej;

  stopedindex1 = myCurrentIndex1;
  stopedindex2 = myCurrentIndex2;
  
  if(!myTableOfStatus.IsNull()) {
    IsValidTableOfStatus = Standard_True;
  }
  
  myCurrentIndex1 = -1;
  myCurrentIndex2 = -1;
  
  couplefound = Standard_False;
  starti = stopedindex1;
  if(starti < 0){
    starti = myFirstLowerIndex;
  }
  for(i = starti; !couplefound && i<=myFirstUpperIndex; i++) {
    startj = mySecondLowerIndex;
    if(i==stopedindex1 && (stopedindex2 >= 0)) {	
      startj = stopedindex2 + 1;
    }
    for(j = startj; !couplefound && j<=mySecondUpperIndex; j++) {
      aTypei=myPDS->GetShapeType(i);
      aTypej=myPDS->GetShapeType(j);
      
      if(((aTypei == myType1) && (aTypej == myType2)) ||
	 ((aTypei == myType2) && (aTypej == myType1))) {
	myCurrentIndex1 = i;
	myCurrentIndex2 = j;
	//
	couplefound = Standard_True;
	if(IsValidTableOfStatus) {
	  XBOPTools_IntersectionStatus aStatus = myTableOfStatus->Value(i, j);
	  if(aStatus==XBOPTools_INTERSECTED || aStatus==XBOPTools_NONINTERSECTED) {
	    myCurrentIndex1 = -1;
	    myCurrentIndex2 = -1;
	    couplefound = Standard_False;
	  }
	}
      }
    }
  }
  //}    
}

// ================================================================================================
// function: More
// ================================================================================================
  Standard_Boolean XBOPTools_IteratorOfCoupleOfShape::MoreP() const
{
  if((myCurrentIndex1 < 0) || (myCurrentIndex2 < 0))
    return Standard_False;
  return Standard_True;
}

// ================================================================================================
// function: Current
// ================================================================================================
  void XBOPTools_IteratorOfCoupleOfShape::CurrentP(Standard_Integer& Index1,
						  Standard_Integer& Index2) const
{
  if((myCurrentIndex1 < 0) || (myCurrentIndex2 < 0)) {
    Standard_NoSuchObject::Raise("XBOPTools_IteratorOfCoupleOfShape::Current");
  }
  Index1 = myCurrentIndex1;
  Index2 = myCurrentIndex2;
}


// ================================================================================================
// function: SetIntersectionStatus
// ================================================================================================
  void XBOPTools_IteratorOfCoupleOfShape::SetIntersectionStatus(const Standard_Integer Index1,
							       const Standard_Integer Index2,
							       const XBOPTools_IntersectionStatus theStatus) 
{
  if((Index1 >= myTableOfStatus->LowerRow()) &&
     (Index1 <= myTableOfStatus->UpperRow()) &&
     (Index2 >= myTableOfStatus->LowerCol()) &&
     (Index2 <= myTableOfStatus->UpperCol())) {
    myTableOfStatus->ChangeValue(Index1, Index2) = theStatus;
  }
}

// ================================================================================================
// function: GetTableOfIntersectionStatus
// ================================================================================================
  const Handle(XBOPTools_HArray2OfIntersectionStatus)& 
    XBOPTools_IteratorOfCoupleOfShape::GetTableOfIntersectionStatus() const
{
  return myTableOfStatus;
}

// ================================================================================================
// function: DumpTableOfIntersectionStatus
// ================================================================================================
  void XBOPTools_IteratorOfCoupleOfShape::DumpTableOfIntersectionStatus() const
{
  cout << "*XBOPTools_IteratorOfCoupleOfShape::DumpTableOfIntersectionStatus.BEGIN*"   << endl;
  cout << "myTableOfStatus.LowerRow="<< myTableOfStatus->LowerRow()   << endl;
  cout << "myTableOfStatus.UpperRow="<< myTableOfStatus->UpperRow()   << endl;
  cout << "myTableOfStatus.LowerCol()="<< myTableOfStatus->LowerCol()   << endl;
  cout << "myTableOfStatus.UpperCol()="<< myTableOfStatus->UpperCol()   << endl;
  for(Standard_Integer k=myTableOfStatus->LowerCol(); k<=myTableOfStatus->UpperCol(); k++) {
    cout << k << " ";
  }
  cout << endl;
  for(Standard_Integer i=myTableOfStatus->LowerRow(); i<=myTableOfStatus->UpperRow(); i++) {
    for(Standard_Integer j=myTableOfStatus->LowerCol(); j<=myTableOfStatus->UpperCol(); j++) {
      cout << myTableOfStatus->Value(i, j) << "  ";
    }
    cout << endl;
  }
  cout << "*XBOPTools_IteratorOfCoupleOfShape::DumpTableOfIntersectionStatus.END*"   << endl;
}
