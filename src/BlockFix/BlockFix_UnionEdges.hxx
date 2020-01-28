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

#ifndef _BlockFix_UnionEdges_HeaderFile
#define _BlockFix_UnionEdges_HeaderFile

#include <Standard_Real.hxx>

#include <ShapeBuild_ReShape.hxx>

#include <Standard.hxx>
#include <Standard_Macro.hxx>

class TopoDS_Shape;

class BlockFix_UnionEdges {

public:
  Standard_EXPORT BlockFix_UnionEdges();

  Standard_EXPORT TopoDS_Shape Perform (const TopoDS_Shape& Shape,const Standard_Real Tol);

private:
  Standard_Real myTolerance;
  Handle_ShapeBuild_ReShape myContext;

};

#endif
