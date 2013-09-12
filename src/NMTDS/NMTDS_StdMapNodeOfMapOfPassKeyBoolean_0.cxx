//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
#include <NMTDS_StdMapNodeOfMapOfPassKeyBoolean.hxx>

#ifndef _Standard_TypeMismatch_HeaderFile
#include <Standard_TypeMismatch.hxx>
#endif

#ifndef _NMTDS_PassKeyBoolean_HeaderFile
#include <NMTDS_PassKeyBoolean.hxx>
#endif
#ifndef _NMTDS_PassKeyMapHasher_HeaderFile
#include <NMTDS_PassKeyMapHasher.hxx>
#endif
#ifndef _NMTDS_MapOfPassKeyBoolean_HeaderFile
#include <NMTDS_MapOfPassKeyBoolean.hxx>
#endif
#ifndef _NMTDS_MapIteratorOfMapOfPassKeyBoolean_HeaderFile
#include <NMTDS_MapIteratorOfMapOfPassKeyBoolean.hxx>
#endif
//NMTDS_StdMapNodeOfMapOfPassKeyBoolean::~NMTDS_StdMapNodeOfMapOfPassKeyBoolean() {}
 


Standard_EXPORT Handle_Standard_Type& NMTDS_StdMapNodeOfMapOfPassKeyBoolean_Type_()
{

    static Handle_Standard_Type aType1 = STANDARD_TYPE(TCollection_MapNode);
  static Handle_Standard_Type aType2 = STANDARD_TYPE(MMgt_TShared);
  static Handle_Standard_Type aType3 = STANDARD_TYPE(Standard_Transient);
 

  static Handle_Standard_Transient _Ancestors[]= {aType1,aType2,aType3,NULL};
  static Handle_Standard_Type _aType = new Standard_Type("NMTDS_StdMapNodeOfMapOfPassKeyBoolean",
			                                 sizeof(NMTDS_StdMapNodeOfMapOfPassKeyBoolean),
			                                 1,
			                                 (Standard_Address)_Ancestors,
			                                 (Standard_Address)NULL);

  return _aType;
}


// DownCast method
//   allow safe downcasting
//
const Handle(NMTDS_StdMapNodeOfMapOfPassKeyBoolean) Handle(NMTDS_StdMapNodeOfMapOfPassKeyBoolean)::DownCast(const Handle(Standard_Transient)& AnObject) 
{
  Handle(NMTDS_StdMapNodeOfMapOfPassKeyBoolean) _anOtherObject;

  if (!AnObject.IsNull()) {
     if (AnObject->IsKind(STANDARD_TYPE(NMTDS_StdMapNodeOfMapOfPassKeyBoolean))) {
       _anOtherObject = Handle(NMTDS_StdMapNodeOfMapOfPassKeyBoolean)((Handle(NMTDS_StdMapNodeOfMapOfPassKeyBoolean)&)AnObject);
     }
  }

  return _anOtherObject ;
}
const Handle(Standard_Type)& NMTDS_StdMapNodeOfMapOfPassKeyBoolean::DynamicType() const 
{ 
  return STANDARD_TYPE(NMTDS_StdMapNodeOfMapOfPassKeyBoolean) ; 
}
//Standard_Boolean NMTDS_StdMapNodeOfMapOfPassKeyBoolean::IsKind(const Handle(Standard_Type)& AType) const 
//{ 
//  return (STANDARD_TYPE(NMTDS_StdMapNodeOfMapOfPassKeyBoolean) == AType || TCollection_MapNode::IsKind(AType)); 
//}
//Handle_NMTDS_StdMapNodeOfMapOfPassKeyBoolean::~Handle_NMTDS_StdMapNodeOfMapOfPassKeyBoolean() {}
#define TheKey NMTDS_PassKeyBoolean
#define TheKey_hxx <NMTDS_PassKeyBoolean.hxx>
#define Hasher NMTDS_PassKeyMapHasher
#define Hasher_hxx <NMTDS_PassKeyMapHasher.hxx>
#define TCollection_MapIterator NMTDS_MapIteratorOfMapOfPassKeyBoolean
#define TCollection_MapIterator_hxx <NMTDS_MapIteratorOfMapOfPassKeyBoolean.hxx>
#define TCollection_StdMapNode NMTDS_StdMapNodeOfMapOfPassKeyBoolean
#define TCollection_StdMapNode_hxx <NMTDS_StdMapNodeOfMapOfPassKeyBoolean.hxx>
#define Handle_TCollection_StdMapNode Handle_NMTDS_StdMapNodeOfMapOfPassKeyBoolean
#define TCollection_StdMapNode_Type_() NMTDS_StdMapNodeOfMapOfPassKeyBoolean_Type_()
#define TCollection_Map NMTDS_MapOfPassKeyBoolean
#define TCollection_Map_hxx <NMTDS_MapOfPassKeyBoolean.hxx>
#include <TCollection_StdMapNode.gxx>

