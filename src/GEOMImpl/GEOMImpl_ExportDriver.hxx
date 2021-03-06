// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
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
//  File   : GEOMImpl_ExportDriver.ixx
//  Module : GEOMImpl

#ifndef _GEOMImpl_ExportDriver_HeaderFile
#define _GEOMImpl_ExportDriver_HeaderFile

#ifndef _TColStd_SequenceOfExtendedString_HeaderFile
#include <TColStd_SequenceOfExtendedString.hxx>
#endif
#ifndef _Standard_TypeMismatch_HeaderFile
#include <Standard_TypeMismatch.hxx>
#endif

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif

#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif
#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_GUID_HeaderFile
#include <Standard_GUID.hxx>
#endif 

#ifndef _Handle_TFunction_Driver_HeaderFile
#include <Handle_TFunction_Driver.hxx>
#endif

class Standard_Transient;
class Handle_Standard_Type;
class Handle(TFunction_Driver);
class GEOMImpl_ExportDriver;

Standard_EXPORT Handle_Standard_Type& STANDARD_TYPE(GEOMImpl_ExportDriver);

class Handle(GEOMImpl_ExportDriver) : public Handle(TFunction_Driver) {
  public:
    inline void* operator new(size_t,void* anAddress) 
      {
        return anAddress;
      }
    inline void* operator new(size_t size) 
      { 
        return Standard::Allocate(size); 
      }
    inline void  operator delete(void *anAddress) 
      { 
        if (anAddress) Standard::Free((Standard_Address&)anAddress); 
      }

    Handle(GEOMImpl_ExportDriver)():Handle(TFunction_Driver)() {} 
    Handle(GEOMImpl_ExportDriver)(const Handle(GEOMImpl_ExportDriver)& aHandle) : Handle(TFunction_Driver)(aHandle) 
     {
     }

    Handle(GEOMImpl_ExportDriver)(const GEOMImpl_ExportDriver* anItem) : Handle(TFunction_Driver)((TFunction_Driver *)anItem) 
     {
     }

    Handle(GEOMImpl_ExportDriver)& operator=(const Handle(GEOMImpl_ExportDriver)& aHandle)
     {
      Assign(aHandle.Access());
      return *this;
     }

    Handle(GEOMImpl_ExportDriver)& operator=(const GEOMImpl_ExportDriver* anItem)
     {
      Assign((Standard_Transient *)anItem);
      return *this;
     }

    GEOMImpl_ExportDriver* operator->() 
     {
      return (GEOMImpl_ExportDriver *)ControlAccess();
     }

    GEOMImpl_ExportDriver* operator->() const 
     {
      return (GEOMImpl_ExportDriver *)ControlAccess();
     }

   Standard_EXPORT ~Handle(GEOMImpl_ExportDriver)() {};
 
   Standard_EXPORT static const Handle(GEOMImpl_ExportDriver) DownCast(const Handle(Standard_Transient)& AnObject);
};

#ifndef _TFunction_Driver_HeaderFile
#include <TFunction_Driver.hxx>
#endif
#ifndef _TFunction_Logbook_HeaderFile
#include <TFunction_Logbook.hxx>
#endif
#ifndef _Standard_CString_HeaderFile
#include <Standard_CString.hxx>
#endif

class TColStd_SequenceOfExtendedString;


class GEOMImpl_ExportDriver : public TFunction_Driver {

public:

    inline void* operator new(size_t,void* anAddress) 
      {
        return anAddress;
      }
    inline void* operator new(size_t size) 
      { 
        return Standard::Allocate(size); 
      }
    inline void  operator delete(void *anAddress) 
      { 
        if (anAddress) Standard::Free((Standard_Address&)anAddress); 
      }

 // Methods PUBLIC
 // 
Standard_EXPORT GEOMImpl_ExportDriver();
Standard_EXPORT virtual  Standard_Integer Execute(TFunction_Logbook& log) const; 
Standard_EXPORT virtual void Validate(TFunction_Logbook&) const {}
Standard_EXPORT Standard_Boolean MustExecute(const TFunction_Logbook&) const { return Standard_True; }
Standard_EXPORT static const Standard_GUID& GetID();
Standard_EXPORT ~GEOMImpl_ExportDriver() {};


 // Type management
 //
Standard_EXPORT friend Handle_Standard_Type& GEOMImpl_ExportDriver_Type_();
Standard_EXPORT const Handle(Standard_Type)& DynamicType() const  { return STANDARD_TYPE(GEOMImpl_ExportDriver) ; }
Standard_EXPORT Standard_Boolean IsKind(const Handle(Standard_Type)& AType) const { return (STANDARD_TYPE(GEOMImpl_ExportDriver) == AType || TFunction_Driver::IsKind(AType)); } 


};

#endif
