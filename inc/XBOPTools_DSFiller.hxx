// This file is generated by WOK (CPPExt).
// Please do not edit this file; modify original file instead.
// The copyright and license terms as defined for the original file apply to 
// this header file considered to be the "object code" form of the original source.

#ifndef _XBOPTools_DSFiller_HeaderFile
#define _XBOPTools_DSFiller_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

#ifndef _TopoDS_Shape_HeaderFile
#include <TopoDS_Shape.hxx>
#endif
#ifndef _XBooleanOperations_PShapesDataStructure_HeaderFile
#include <XBooleanOperations_PShapesDataStructure.hxx>
#endif
#ifndef _XBOPTools_PInterferencePool_HeaderFile
#include <XBOPTools_PInterferencePool.hxx>
#endif
#ifndef _XBOPTools_PPaveFiller_HeaderFile
#include <XBOPTools_PPaveFiller.hxx>
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
#ifndef _TColStd_DataMapOfIntegerListOfInteger_HeaderFile
#include <TColStd_DataMapOfIntegerListOfInteger.hxx>
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
class TopoDS_Shape;
class TColStd_SetOfInteger;
class XBOPTools_SSIntersectionAttribute;
class XBooleanOperations_ShapesDataStructure;
class XBOPTools_InterferencePool;
class XBOPTools_PavePool;
class XBOPTools_CommonBlockPool;
class XBOPTools_SplitShapesPool;
class XBOPTools_PaveFiller;
class TColStd_DataMapOfIntegerListOfInteger;



//!  class that provides <br>
//!  1. creation of the data structure (DS) <br>
//!  2. creation of the interferences' pool <br>
//!  3. invokation of PaveFiller->Perform() to fill the DS <br>
class XBOPTools_DSFiller  {
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

  
//! Empty constructor <br>
  Standard_EXPORT   XBOPTools_DSFiller();
  //! Destructor <br>
  Standard_EXPORT     void Destroy() ;
~XBOPTools_DSFiller()
{
  Destroy();
}
  
//! Modifier <br>
//! Sets the arguments of boolean operation <br>
  Standard_EXPORT     void SetShapes(const TopoDS_Shape& aS1,const TopoDS_Shape& aS2) ;
  
//! Selector <br>
  Standard_EXPORT    const TopoDS_Shape& Shape1() const;
  
//! Selector <br>
  Standard_EXPORT    const TopoDS_Shape& Shape2() const;
  
//! Performs the filling of the DS <br>
//! <br>
  Standard_EXPORT     void Perform() ;
  
  Standard_EXPORT     void InitFillersAndPools() ;
  
  Standard_EXPORT     void PartialPerform(const TColStd_SetOfInteger& anObjSubSet,const TColStd_SetOfInteger& aToolSubSet) ;
  
  Standard_EXPORT     void ToCompletePerform() ;
  
//! Performs the filling of the DS <br>
  Standard_EXPORT     void Perform(const XBOPTools_SSIntersectionAttribute& theSectionAttribute) ;
  
//! Selector <br>
  Standard_EXPORT    const XBooleanOperations_ShapesDataStructure& DS() const;
  
//! Selector <br>
  Standard_EXPORT    const XBOPTools_InterferencePool& InterfPool() const;
  
//! Selector <br>
  Standard_EXPORT    const XBOPTools_PavePool& PavePool() const;
  
//! Selector <br>
  Standard_EXPORT    const XBOPTools_CommonBlockPool& CommonBlockPool() const;
  
//! Selector <br>
  Standard_EXPORT    const XBOPTools_SplitShapesPool& SplitShapesPool() const;
  
//! Selector <br>
  Standard_EXPORT    const XBOPTools_PaveFiller& PaveFiller() const;
  
//! Returns TRUE if new DS and the interferences' pool has been created <br>
  Standard_EXPORT     Standard_Boolean IsNewFiller() const;
  
//! Modifier <br>
  Standard_EXPORT     void SetNewFiller(const Standard_Boolean aFlag) const;
  
//! Selector <br>
  Standard_EXPORT     Standard_Boolean IsDone() const;
  
       const TColStd_DataMapOfIntegerListOfInteger& SplitFacePool() const;
  
        TColStd_DataMapOfIntegerListOfInteger& ChangeSplitFacePool() ;
  //! Finds sub-shapes of theShape having equal type <br>
//!          and store them in theShapeResult. <br>
//!          Returns the following status codes: <br>
//!          0 - OK <br>
//!          1 - Error: theShape is a COMPSOLID <br>
//!          2 - Error: theShape is not a COMPOUND <br>
//!          3 - Error: theShape contains shapes of COMPSOLID type <br>
//!          4 - Error: Subshape of theShape have unkown type <br>
//!          5 - Error: theShape contains shapes of different type <br>
  Standard_EXPORT   static  Standard_Integer TreatCompound(const TopoDS_Shape& theShape,TopoDS_Shape& theShapeResult) ;





protected:





private:

  
//! Clear contents of the DS and the interferences' pool <br>
  Standard_EXPORT     void Clear() ;


TopoDS_Shape myShape1;
TopoDS_Shape myShape2;
XBooleanOperations_PShapesDataStructure myDS;
XBOPTools_PInterferencePool myInterfPool;
XBOPTools_PPaveFiller myPaveFiller;
Standard_Boolean myIsDone;
TColStd_DataMapOfIntegerListOfInteger mySplitFacePool;
Standard_Boolean myNewFiller;


};


#include <XBOPTools_DSFiller.lxx>



// other Inline functions and methods (like "C++: function call" methods)


#endif