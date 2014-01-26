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

//NOTE: This is an intreface to a function for the Sphere creation.
//
#include "GEOM_Function.hxx"

#define SPH_ARG_R   1
#define SPH_ARG_PNT 4
#define SPH_ARG_ANG 5
#define SPH_ARG_VMIN 6
#define SPH_ARG_VMAX 7

class GEOMImpl_ISphere
{
 public:

  GEOMImpl_ISphere(Handle(GEOM_Function) theFunction): _func(theFunction) {}

  void SetR(double theR) { _func->SetReal(SPH_ARG_R, theR); }

  double GetR() { return _func->GetReal(SPH_ARG_R); }

  void SetPoint(Handle(GEOM_Function) theRefPoint) { _func->SetReference(SPH_ARG_PNT, theRefPoint); }

  Handle(GEOM_Function) GetPoint() { return _func->GetReference(SPH_ARG_PNT); }

  void SetR(const TCollection_AsciiString& theR) { _func->SetReal(SPH_ARG_R, theR); }

  void SetAngle(const TCollection_AsciiString& theAngle) { _func->SetReal(SPH_ARG_ANG, theAngle); }
  void SetAngle(const double& theAngle) { _func->SetReal(SPH_ARG_ANG, theAngle); }

  double GetAngle() { return _func->GetReal(SPH_ARG_ANG); }

  void SetVCoordStart(const TCollection_AsciiString& theAngle) { _func->SetReal(SPH_ARG_VMIN, theAngle); }
  void SetVCoordStart(const double& theAngle) { _func->SetReal(SPH_ARG_VMIN, theAngle); }

  double GetVCoordStart() { return _func->GetReal(SPH_ARG_VMIN); }

  void SetVCoordEnd(const TCollection_AsciiString& theAngle) { _func->SetReal(SPH_ARG_VMAX, theAngle); }
  void SetVCoordEnd(const double& theAngle) { _func->SetReal(SPH_ARG_VMAX, theAngle); }

  double GetVCoordEnd() { return _func->GetReal(SPH_ARG_VMAX); }

 private:

  Handle(GEOM_Function) _func;
};
