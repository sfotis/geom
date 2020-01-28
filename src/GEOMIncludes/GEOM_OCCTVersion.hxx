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

// File   : GEOM_OCCTVersion.hxx
// Author : Julia DOROVSKIKH, Open CASCADE S.A.S (julia.dorovskikh@opencascade.com)

#ifndef GEOM_OCCTVERSION_HXX
#define GEOM_OCCTVERSION_HXX

#include <Standard_Version.hxx>

//
// NOTE: CAS_VERSION_DEVELOPMENT macro is defined via command line in contrast to OCC_VERSION_DEVELOPMENT
//       which is specified in the Standard_Version.hxx
//

#ifdef OCC_VERSION_SERVICEPACK
#  define OCC_VERSION_LARGE (OCC_VERSION_MAJOR << 24 | OCC_VERSION_MINOR << 16 | OCC_VERSION_MAINTENANCE << 8 | OCC_VERSION_SERVICEPACK)
#else
#  ifdef CAS_VERSION_DEVELOPMENT
#    define OCC_VERSION_LARGE (OCC_VERSION_MAJOR << 24 | OCC_VERSION_MINOR << 16 | OCC_VERSION_MAINTENANCE << 8 | 1)
#  else
#    define OCC_VERSION_LARGE (OCC_VERSION_MAJOR << 24 | OCC_VERSION_MINOR << 16 | OCC_VERSION_MAINTENANCE << 8)
#  endif
#endif

#endif // GEOM_OCCTVERSION_HXX
