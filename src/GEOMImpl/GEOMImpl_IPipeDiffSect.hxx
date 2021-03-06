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
//NOTE: This is an intreface to a function for the Pipe creation.

#ifndef _GEOMImpl_IPIPEDIFFSECT_HXX_
#define _GEOMImpl_IPIPEDIFFSECT_HXX_

#include "GEOM_Function.hxx"

#ifndef _GEOMImpl_IPIPE_HXX_
#include "GEOMImpl_IPipe.hxx"
#endif

#define PIPEDS_LIST_BASES 3
#define PIPEDS_LIST_LOCATIONS 4
#define PIPEDS_ARG_WITHCONTACT 5
#define PIPEDS_ARG_WITHCORRECT 6


class GEOMImpl_IPipeDiffSect : public GEOMImpl_IPipe
{
 public:

  GEOMImpl_IPipeDiffSect(const Handle(GEOM_Function)& theFunction):GEOMImpl_IPipe(theFunction) {}
  
  void SetBases (const Handle(TColStd_HSequenceOfTransient)& theBases) 
  {
    _func->SetReferenceList(PIPEDS_LIST_BASES,theBases);
  }

  Handle(TColStd_HSequenceOfTransient) GetBases ()
  { 
    Handle(TColStd_HSequenceOfTransient) aBases = _func->GetReferenceList(PIPEDS_LIST_BASES);
    return aBases; 
  }

  void SetLocations (const Handle(TColStd_HSequenceOfTransient)& theLocations) 
  { _func->SetReferenceList(PIPEDS_LIST_LOCATIONS,theLocations); }

  Handle(TColStd_HSequenceOfTransient) GetLocations ()
  { 
    Handle(TColStd_HSequenceOfTransient) aLocs = _func->GetReferenceList(PIPEDS_LIST_LOCATIONS);
    return aLocs; 
  }

  //void SetPath (const Handle(GEOM_Function)& thePath) { _func->SetReference(PIPEDS_ARG_PATH, thePath); }

  //Handle(GEOM_Function) GetPath() { return _func->GetReference(PIPEDS_ARG_PATH); }

  void SetWithContactMode(int theWithContact)
  { _func->SetInteger(PIPEDS_ARG_WITHCONTACT,theWithContact); }

  int GetWithContactMode()
  { return _func->GetInteger(PIPEDS_ARG_WITHCONTACT); }

  void SetWithCorrectionMode(int theWithCorrection)
  { _func->SetInteger(PIPEDS_ARG_WITHCORRECT,theWithCorrection); }

  int GetWithCorrectionMode()
  { return _func->GetInteger(PIPEDS_ARG_WITHCORRECT); }

};

#endif
