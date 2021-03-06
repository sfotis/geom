// This file is generated by WOK (CPPExt).
// Please do not edit this file; modify original file instead.
// The copyright and license terms as defined for the original file apply to 
// this header file considered to be the "object code" form of the original source.

#ifndef _XBOPTools_VSInterference_HeaderFile
#define _XBOPTools_VSInterference_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

#ifndef _Standard_Real_HeaderFile
#include <Standard_Real.hxx>
#endif
#ifndef _XBOPTools_ShapeShapeInterference_HeaderFile
#include <XBOPTools_ShapeShapeInterference.hxx>
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif



//!  Class for storing info about an Verex/Face interference <br>
//! <br>
class XBOPTools_VSInterference  : public XBOPTools_ShapeShapeInterference {
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
  Standard_EXPORT   XBOPTools_VSInterference();
  
//! Constructor <br>
//! anIndex1, <br>
//! anIndex2 see XBOPTools_ShapeShapeInterference for details <br>
//! U, V  -  values of parameters on the surface <br>
  Standard_EXPORT   XBOPTools_VSInterference(const Standard_Integer anIndex1,const Standard_Integer anIndex2,const Standard_Real U,const Standard_Real V);
  
//! Modifier <br>
  Standard_EXPORT     void SetUV(const Standard_Real U,const Standard_Real V) ;
  
//! Selector <br>
  Standard_EXPORT     void UV(Standard_Real& U,Standard_Real& V) const;





protected:





private:



Standard_Real myU;
Standard_Real myV;


};





// other Inline functions and methods (like "C++: function call" methods)


#endif
