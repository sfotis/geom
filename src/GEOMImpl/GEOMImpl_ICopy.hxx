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
//NOTE: This is an intreface to a function for the Copy operation.


#include "GEOM_Function.hxx"

#define COPY_ARG_REF  1

class GEOMImpl_ICopy
{
 public:

  GEOMImpl_ICopy(Handle(GEOM_Function) theFunction): _func(theFunction) {}

  void SetOriginal(Handle(GEOM_Function) theOriginal) { _func->SetReference(COPY_ARG_REF, theOriginal); }

  Handle(GEOM_Function) GetOriginal() { return _func->GetReference(COPY_ARG_REF); }

 private:

  Handle(GEOM_Function) _func;
};
