// This file is generated by WOK (CPPExt).
// Please do not edit this file; modify original file instead.
// The copyright and license terms as defined for the original file apply to 
// this header file considered to be the "object code" form of the original source.

#ifndef _XBOPTools_IteratorOfCoupleOfShape_HeaderFile
#define _XBOPTools_IteratorOfCoupleOfShape_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

#ifndef _XBooleanOperations_PShapesDataStructure_HeaderFile
#include <XBooleanOperations_PShapesDataStructure.hxx>
#endif
#ifndef _Handle_XBOPTools_HArray2OfIntersectionStatus_HeaderFile
#include <Handle_XBOPTools_HArray2OfIntersectionStatus.hxx>
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _TopAbs_ShapeEnum_HeaderFile
#include <TopAbs_ShapeEnum.hxx>
#endif
#ifndef _XBOPTools_ListOfCoupleOfInteger_HeaderFile
#include <XBOPTools_ListOfCoupleOfInteger.hxx>
#endif
#ifndef _XBOPTools_ListIteratorOfListOfCoupleOfInteger_HeaderFile
#include <XBOPTools_ListIteratorOfListOfCoupleOfInteger.hxx>
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
#ifndef _XBOPTools_IntersectionStatus_HeaderFile
#include <XBOPTools_IntersectionStatus.hxx>
#endif
class XBOPTools_HArray2OfIntersectionStatus;
class Standard_NoSuchObject;
class XBOPTools_ListOfCoupleOfInteger;


//! The class IteratorOfCoupleOfShape provides the iteration <br>
//!         on the couples of shapes stored in ShapesDataStructure <br>
//!         according to the given types of shapes and <br>
//!         status of their intersection. <br>
//!         The statuses are stored in 2 dimension array. <br>
class XBOPTools_IteratorOfCoupleOfShape  {
public:

  void* operator new(size_t,void* anAddress) 
  {
    return anAddress;
  }
  void* operator new(size_t size) 
  {
    return Standard::Allocate(size); 
  }
  void  operator delete(void *anAddress) 
  {
    if (anAddress) Standard::Free((Standard_Address&)anAddress); 
  }

  
//! Empty Constructor <br>
  Standard_EXPORT   XBOPTools_IteratorOfCoupleOfShape();
  
//! Initializes iterator by ShapesDataStructure and <br>
//! shape types <br>
  Standard_EXPORT   XBOPTools_IteratorOfCoupleOfShape(const XBooleanOperations_PShapesDataStructure& PDS,const TopAbs_ShapeEnum Type1,const TopAbs_ShapeEnum Type2);
  
  Standard_EXPORT   virtual  void Destroy() ;
Standard_EXPORT virtual ~XBOPTools_IteratorOfCoupleOfShape(){Destroy();}
  
//! Initializes iterator with shape types. <br>
//! The iterator uses PDS assigned in constructor or in SetDataStructure(). <br>
//! Raises the exception if myPDS is null. <br>
  Standard_EXPORT   virtual  void Initialize(const TopAbs_ShapeEnum Type1,const TopAbs_ShapeEnum Type2) ;
  
//! Initialize iterator with ShapeDataStructure. <br>
  Standard_EXPORT     void SetDataStructure(const XBooleanOperations_PShapesDataStructure& PDS) ;
  
//! Returns True if there are still not <br>
//! treated couples of shapes <br>
  Standard_EXPORT   virtual  Standard_Boolean More() const;
  
//! Moves to the next couple of iteration <br>
  Standard_EXPORT   virtual  void Next() ;
  
//! Returns current couple of indices and <br>
//! flag WithSubShape which is true <br>
//! if bounding boxes of subshapes <br>
//! are intersected <br>
  Standard_EXPORT   virtual  void Current(Standard_Integer& Index1,Standard_Integer& Index2,Standard_Boolean& WithSubShape) const;
  
//! Returns a list of couples of shape indices <br>
//! according to shape types by which <br>
//! the iterator was initialized <br>
  Standard_EXPORT    const XBOPTools_ListOfCoupleOfInteger& ListOfCouple() const;
  
//! Sets status to array according to Index1 and Index2 <br>
  Standard_EXPORT     void SetIntersectionStatus(const Standard_Integer Index1,const Standard_Integer Index2,const XBOPTools_IntersectionStatus theStatus) ;
  
//! Returns 2 dimension array of intersection statuses <br>
  Standard_EXPORT    const Handle_XBOPTools_HArray2OfIntersectionStatus& GetTableOfIntersectionStatus() const;
  
//! For internal use <br>
  Standard_EXPORT     void DumpTableOfIntersectionStatus() const;





protected:

  
  Standard_EXPORT     Standard_Boolean MoreP() const;
  
  Standard_EXPORT     void NextP() ;
  
  Standard_EXPORT     void CurrentP(Standard_Integer& Index1,Standard_Integer& Index2) const;


XBooleanOperations_PShapesDataStructure myPDS;
Handle_XBOPTools_HArray2OfIntersectionStatus myTableOfStatus;
Standard_Integer myCurrentIndex1;
Standard_Integer myCurrentIndex2;
TopAbs_ShapeEnum myType1;
TopAbs_ShapeEnum myType2;
Standard_Integer myFirstLowerIndex;
Standard_Integer myFirstUpperIndex;
Standard_Integer mySecondLowerIndex;
Standard_Integer mySecondUpperIndex;
XBOPTools_ListOfCoupleOfInteger myListOfCouple;
XBOPTools_ListIteratorOfListOfCoupleOfInteger myIterator;


private:





};





// other Inline functions and methods (like "C++: function call" methods)


#endif
