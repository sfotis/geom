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
//NOTE: This is an intreface to a function for the vector creation.


#include "GEOM_Function.hxx"

#define VEC_ARG_DX   1
#define VEC_ARG_DY   2
#define VEC_ARG_DZ   3

#define VEC_ARG_PNT1 4
#define VEC_ARG_PNT2 5

#define VEC_ARG_CURVE 6
#define VEC_ARG_PARAM 7


class GEOMImpl_IVector
{
 public:

  GEOMImpl_IVector(Handle(GEOM_Function) theFunction): _func(theFunction) {}

  void SetDX(const TCollection_AsciiString& theDX) { _func->SetReal(VEC_ARG_DX, theDX); }
  void SetDY(const TCollection_AsciiString& theDY) { _func->SetReal(VEC_ARG_DY, theDY); }
  void SetDZ(const TCollection_AsciiString& theDZ) { _func->SetReal(VEC_ARG_DZ, theDZ); }

  void SetDX(const double& theDX) { _func->SetReal(VEC_ARG_DX, theDX); }
  void SetDY(const double& theDY) { _func->SetReal(VEC_ARG_DY, theDY); }
  void SetDZ(const double& theDZ) { _func->SetReal(VEC_ARG_DZ, theDZ); }

  double GetDX() { return _func->GetReal(VEC_ARG_DX); }
  double GetDY() { return _func->GetReal(VEC_ARG_DY); }
  double GetDZ() { return _func->GetReal(VEC_ARG_DZ); }

  void SetPoint1(Handle(GEOM_Function) theRef) { _func->SetReference(VEC_ARG_PNT1, theRef); }
  void SetPoint2(Handle(GEOM_Function) theRef) { _func->SetReference(VEC_ARG_PNT2, theRef); }

  Handle(GEOM_Function) GetPoint1() { return _func->GetReference(VEC_ARG_PNT1); }
  Handle(GEOM_Function) GetPoint2() { return _func->GetReference(VEC_ARG_PNT2); }

  void SetCurve(Handle(GEOM_Function) theRef) { _func->SetReference(VEC_ARG_CURVE, theRef); }

  Handle(GEOM_Function) GetCurve() { return _func->GetReference(VEC_ARG_CURVE); }

  void SetParameter(const TCollection_AsciiString& theParam) { _func->SetReal(VEC_ARG_PARAM, theParam); }

  double GetParameter() { return _func->GetReal(VEC_ARG_PARAM); }

 private:

  Handle(GEOM_Function) _func;
};
