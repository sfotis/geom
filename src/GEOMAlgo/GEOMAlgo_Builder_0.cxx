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

#include <GEOMAlgo_Builder.hxx>

#include <NMTDS_ShapesDataStructure.hxx>
//#include <NMTTools_DSFiller.hxx>
#include <NMTTools_PaveFiller.hxx>
#include <XIntTools_Context.hxx>
#include <TopoDS_Shape.hxx>
#include <XBOPTools_ListIteratorOfListOfPaveBlock.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <XBOPTools_ListOfPaveBlock.hxx>
#include <XBOPTools_PaveBlock.hxx>

//=======================================================================
//function : Shapes1
//purpose  :
//=======================================================================
  const TopTools_ListOfShape& GEOMAlgo_Builder::Shapes1(const Standard_Integer theType)const
{
  return myShapes1[theType];
}
//=======================================================================
//function : Images
//purpose  :
//=======================================================================
  const BRepAlgo_Image& GEOMAlgo_Builder::Images()const
{
  return myImages;
}
//=======================================================================
//function : InParts
//purpose  :
//=======================================================================
  const TopTools_ListOfShape& GEOMAlgo_Builder::InParts(const TopoDS_Shape& theS)const
{
  static TopTools_ListOfShape sLS;
  //
  if (myInParts.Contains(theS)) {
    return myInParts.FindFromKey(theS);
  }
  return sLS;
}
