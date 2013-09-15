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
// Original work from OpenCascade Sketcher 
// (http://sourceforge.net/projects/occsketcher/)
//
// Modifications for GEOM and OCAF
// Authored by : Sioutis Fotios (sfotis@gmail.com)

//------------------------------------------------------------------------------
#ifndef Sketcher_SnapCENTER_H
#define Sketcher_SnapCENTER_H
//------------------------------------------------------------------------------
#include "Sketcher_Snap.hxx"
//------------------------------------------------------------------------------
#include <Handle_Geom2d_Circle.hxx>
//------------------------------------------------------------------------------
DEFINE_STANDARD_HANDLE(Sketcher_SnapCenter,Sketcher_Snap)
//------------------------------------------------------------------------------
class Sketcher_SnapCenter : public Sketcher_Snap
{
  public:
    DEFINE_STANDARD_RTTI(Sketcher_SnapCenter)

    Standard_EXPORT Sketcher_SnapCenter();
    Standard_EXPORT ~Sketcher_SnapCenter();

    Standard_EXPORT void SelectEvent();
    Standard_EXPORT Sketcher_SnapType GetSnapType();

  private:
    Handle(Geom2d_Circle) curGeom2d_Circle;
};
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
