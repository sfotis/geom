// Created on: 2001-01-26
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



#include <XBOPTools_InterferencePool.ixx>

#include <BRep_Tool.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>

#include <XIntTools_Tools.hxx>
#include <XIntTools_EdgeEdge.hxx>
#include <XIntTools_SequenceOfCommonPrts.hxx>
#include <XIntTools_CommonPrt.hxx>

#include <XBOPTools_VEInterference.hxx>
#include <XBOPTools_VVInterference.hxx>
#include <XBOPTools_VSInterference.hxx>
#include <XBOPTools_EEInterference.hxx>
#include <XBOPTools_SSInterference.hxx>
#include <XBOPTools_InterferenceLine.hxx>
#include <XIntTools_EdgeFace.hxx>
#include <XBOPTools_ESInterference.hxx>


//=======================================================================
//function : XBOPTools_InterferencePool
//purpose  : 
//=======================================================================
XBOPTools_InterferencePool::XBOPTools_InterferencePool()
{
  myDS=NULL;
  myNbSourceShapes=0;
}
//=======================================================================
//function : XBOPTools_InterferencePool
//purpose  : 
//=======================================================================
  XBOPTools_InterferencePool::XBOPTools_InterferencePool (const XBooleanOperations_ShapesDataStructure& aDS) 
{
  SetDS(aDS);
}
//=======================================================================
//function : SetDS
//purpose  : 
//=======================================================================
  void XBOPTools_InterferencePool::SetDS(const XBooleanOperations_ShapesDataStructure& aDS) 
{
  void* p=(void*) &aDS;
  myDS=(XBooleanOperations_ShapesDataStructure*) p;
  myNbSourceShapes= myDS->NumberOfShapesOfTheObject()+myDS->NumberOfShapesOfTheTool();
  myInterferenceTable.Resize (myNbSourceShapes);
}
//=======================================================================
//function : DS
//purpose  : 
//=======================================================================
  XBooleanOperations_PShapesDataStructure XBOPTools_InterferencePool::DS() const
{
  return myDS;
}
//=======================================================================
//function : HasInterference
//purpose  : 
//=======================================================================
  Standard_Boolean XBOPTools_InterferencePool::HasInterference(const Standard_Integer anInd1)const
{
  const XBOPTools_InterferenceLine& aWhatLine=myInterferenceTable(anInd1);
  Standard_Boolean bFlag=aWhatLine.HasInterference();
  return bFlag;
}
//=======================================================================
//function : IsComputed
//purpose  : 
//=======================================================================
  Standard_Boolean XBOPTools_InterferencePool::IsComputed(const Standard_Integer anInd1,
							 const Standard_Integer anInd2) const
{
  XBooleanOperations_KindOfInterference theType;

  theType=InterferenceType(anInd1, anInd2);

  if (theType==XBooleanOperations_UnknownInterference) {
    return Standard_False;
  }

  const XBOPTools_InterferenceLine& aWhatLine=myInterferenceTable(anInd1);
  Standard_Boolean aFlag=aWhatLine.IsComputed(anInd2, theType);
  return aFlag;
}


//===========================================================================
//function : AddInterference
//purpose  : 
//===========================================================================
  void XBOPTools_InterferencePool::AddInterference (const Standard_Integer theWhat,
						   const Standard_Integer theWith,
						   const XBooleanOperations_KindOfInterference theType,
						   const Standard_Integer theIndexOfInterference)
{
  XBOPTools_InterferenceLine& aWhatLine=myInterferenceTable(theWhat);
  aWhatLine.AddInterference(theWith, theType, theIndexOfInterference);

  XBOPTools_InterferenceLine& aWithLine=myInterferenceTable(theWith);
  aWithLine.AddInterference(theWhat, theType, theIndexOfInterference);
}

//=======================================================================
//function : InterferenceType
//purpose  : 
//=======================================================================
  XBooleanOperations_KindOfInterference 
    XBOPTools_InterferencePool::InterferenceType(const Standard_Integer theWhat,
						const Standard_Integer theWith) const
{
  XBooleanOperations_KindOfInterference theType;
  Standard_Integer aWhat, aWith;
  TopAbs_ShapeEnum aType1, aType2;

  aWhat=theWhat;
  aWith=theWith;
  SortTypes(aWhat, aWith);

  aType1= myDS->GetShapeType(aWhat),
  aType2= myDS->GetShapeType(aWith);
  
  if (aType1==TopAbs_VERTEX && aType2==TopAbs_VERTEX) {
    theType=XBooleanOperations_VertexVertex;
  }
  else if  (aType1==TopAbs_VERTEX && aType2==TopAbs_EDGE) {
    theType=XBooleanOperations_VertexEdge;
  }
  else if  (aType1==TopAbs_VERTEX && aType2==TopAbs_FACE) {
    theType=XBooleanOperations_VertexSurface;
  }
  else if  (aType1==TopAbs_EDGE && aType2==TopAbs_EDGE) {
    theType=XBooleanOperations_EdgeEdge;
  }
  else if  (aType1==TopAbs_EDGE && aType2==TopAbs_FACE) {
    theType=XBooleanOperations_EdgeSurface;
  }
  else if  (aType1==TopAbs_FACE && aType2==TopAbs_FACE) {
    theType=XBooleanOperations_SurfaceSurface;
  }
  else {
    theType=XBooleanOperations_UnknownInterference;
  }

  return theType;
}

//=======================================================================
//function : SortTypes
//purpose  : 
//=======================================================================
  void XBOPTools_InterferencePool::SortTypes(Standard_Integer& theWhat,
					    Standard_Integer& theWith) const
{ 
  Standard_Boolean aReverseFlag=Standard_True;

  TopAbs_ShapeEnum aType1= myDS->GetShapeType(theWhat),
                   aType2= myDS->GetShapeType(theWith);
  
  if (aType1==aType2)
    return;
  
  if (aType1==TopAbs_EDGE && aType2==TopAbs_FACE){
    aReverseFlag=Standard_False;
  }

  if (aType1==TopAbs_VERTEX && 
      (aType2==TopAbs_FACE || aType2==TopAbs_EDGE)) {
    aReverseFlag=Standard_False;
  }
  
  Standard_Integer aWhat, aWith;
  aWhat=(aReverseFlag) ? theWith : theWhat;
  aWith=(aReverseFlag) ? theWhat : theWith;
  
  theWhat=aWhat;
  theWith=aWith;
}

//===========================================================================
//function : InterferenceTable
//purpose  : 
//===========================================================================
const XBOPTools_CArray1OfInterferenceLine&
   XBOPTools_InterferencePool::InterferenceTable()const
{
  return myInterferenceTable;
}

//===========================================================================
//function : SSInterferences
//purpose  : 
//===========================================================================
  XBOPTools_CArray1OfSSInterference&  XBOPTools_InterferencePool::SSInterferences()
{
  return mySSInterferences;
}
//===========================================================================
//function : ESInterferences
//purpose  : 
//===========================================================================
  XBOPTools_CArray1OfESInterference&  XBOPTools_InterferencePool::ESInterferences()
{
  return myESInterferences;
}
//===========================================================================
//function : VSInterferences
//purpose  : 
//===========================================================================
  XBOPTools_CArray1OfVSInterference&  XBOPTools_InterferencePool::VSInterferences()
{
  return myVSInterferences;
}
//===========================================================================
//function : EEInterferences
//purpose  : 
//===========================================================================
  XBOPTools_CArray1OfEEInterference&  XBOPTools_InterferencePool::EEInterferences()
{
  return myEEInterferences;
}
//===========================================================================
//function : VEInterferences
//purpose  : 
//===========================================================================
  XBOPTools_CArray1OfVEInterference&  XBOPTools_InterferencePool::VEInterferences()
{
  return myVEInterferences;
}

//===========================================================================
//function : VVInterferences
//purpose  : 
//===========================================================================
  XBOPTools_CArray1OfVVInterference&  XBOPTools_InterferencePool::VVInterferences()
{
  return myVVInterferences;
}
////////////////////////

//===========================================================================
//function : SSInterfs
//purpose  : 
//===========================================================================
  const XBOPTools_CArray1OfSSInterference&  XBOPTools_InterferencePool::SSInterfs()const
{
  return mySSInterferences;
}
//===========================================================================
//function : ESInterfs
//purpose  : 
//===========================================================================
  const XBOPTools_CArray1OfESInterference&  XBOPTools_InterferencePool::ESInterfs()const
{
  return myESInterferences;
}
//===========================================================================
//function : VSInterfs
//purpose  : 
//===========================================================================
  const XBOPTools_CArray1OfVSInterference&  XBOPTools_InterferencePool::VSInterfs()const
{
  return myVSInterferences;
}
//===========================================================================
//function : EEInterfs
//purpose  : 
//===========================================================================
  const XBOPTools_CArray1OfEEInterference&  XBOPTools_InterferencePool::EEInterfs()const
{
  return myEEInterferences;
}
//===========================================================================
//function : VEInterfs
//purpose  : 
//===========================================================================
  const XBOPTools_CArray1OfVEInterference&  XBOPTools_InterferencePool::VEInterfs()const
{
  return myVEInterferences;
}

//===========================================================================
//function : VVInterfs
//purpose  : 
//===========================================================================
  const XBOPTools_CArray1OfVVInterference&  XBOPTools_InterferencePool::VVInterfs()const
{
  return myVVInterferences;
}
//===========================================================================
//function : GetInterference
//purpose  : 
//===========================================================================
XBOPTools_PShapeShapeInterference
  XBOPTools_InterferencePool::GetInterference(const Standard_Integer anIndex,
					     const XBooleanOperations_KindOfInterference aType)const
{
  Standard_Integer aNb;
  XBOPTools_PShapeShapeInterference pI=NULL;

  switch (aType) {
    //
    case XBooleanOperations_SurfaceSurface:
      aNb=mySSInterferences.Extent();
      if (anIndex > 0 && anIndex <= aNb) {
	pI=(XBOPTools_PShapeShapeInterference)&mySSInterferences(anIndex);
      }
      break;
    //
    case XBooleanOperations_EdgeSurface:
      aNb=myESInterferences.Extent();
      if (anIndex > 0 && anIndex <= aNb) {
	pI=(XBOPTools_PShapeShapeInterference)&myESInterferences(anIndex);
      }
      break;
    //
    case XBooleanOperations_VertexSurface:
      aNb=myVSInterferences.Extent();
      if (anIndex > 0 && anIndex <= aNb) {
	pI=(XBOPTools_PShapeShapeInterference)&myVSInterferences(anIndex);
      }
      break;
    //
    case XBooleanOperations_EdgeEdge:
      aNb=myEEInterferences.Extent();
      if (anIndex > 0 && anIndex <= aNb) {
	pI=(XBOPTools_PShapeShapeInterference)&myEEInterferences(anIndex);
      }
      break;
    //  
    case XBooleanOperations_VertexEdge:
      aNb=myVEInterferences.Extent();
      if (anIndex > 0 && anIndex <= aNb) {
	pI=(XBOPTools_PShapeShapeInterference)&myVEInterferences(anIndex);
      }
      break;
    //
    case XBooleanOperations_VertexVertex:
      aNb=myVVInterferences.Extent();
      if (anIndex > 0 && anIndex <= aNb) {
	pI=(XBOPTools_PShapeShapeInterference)&myVVInterferences(anIndex);
      }
      break;
    //
    case XBooleanOperations_UnknownInterference:
    default:
      break;
  }
  return pI;
}

