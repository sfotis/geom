// Copyright (C) 2007-2013  CEA/DEN, EDF R&D, OPEN CASCADE
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

#include <GEOMImpl_FaceDriver.hxx>

#include <GEOMImpl_IFace.hxx>
#include <GEOMImpl_Types.hxx>

#include <GEOM_Function.hxx>

#include <GEOMUtils.hxx>

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRep_Tool.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pln.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopAbs.hxx>
#include <TopExp.hxx>

#include <Standard_Stream.hxx>
#include <BRepBuilderaFI_MakeWire.hxx>
#include <BRepBuilderaFI_MakeEdge.hxx>
#include <TopoDS_Wire.hxx>
#include <Precision.hxx>
#include <Standard_ConstructionError.hxx>
#include <Standard_TypeMismatch.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <Geom_BezierSurface.hxx>

#include <StdFail_NotDone.hxx>

//=======================================================================
//function : GetID
//purpose  :
//=======================================================================
const Standard_GUID& GEOMImpl_FaceDriver::GetID()
{
  static Standard_GUID aFaceDriver("FF1BBB74-5D14-4df2-980B-3A668264EA16");
  return aFaceDriver;
}


//=======================================================================
//function : GEOMImpl_FaceDriver
//purpose  :
//=======================================================================
GEOMImpl_FaceDriver::GEOMImpl_FaceDriver()
{
}

//=======================================================================
//function : Execute
//purpose  :
//=======================================================================
Standard_Integer GEOMImpl_FaceDriver::Execute(TFunction_Logbook& log) const
{
  if (Label().IsNull())  return 0;
  Handle(GEOM_Function) aFunction = GEOM_Function::GetFunction(Label());

  GEOMImpl_IFace aFI (aFunction);
  Standard_Integer aType = aFunction->GetType();

  TopoDS_Shape aShape;

  if (aType == FACE_THREE_PNT) {
    Handle(GEOM_Function) aRefPnt1 = aFI.GetPoint1();
    Handle(GEOM_Function) aRefPnt2 = aFI.GetPoint2();
    Handle(GEOM_Function) aRefPnt3 = aFI.GetPoint3();
    TopoDS_Shape aShape1 = aRefPnt1->GetValue();
    TopoDS_Shape aShape2 = aRefPnt2->GetValue();
    TopoDS_Shape aShape3 = aRefPnt3->GetValue();
    if (aShape1.ShapeType() != TopAbs_VERTEX ||
        aShape2.ShapeType() != TopAbs_VERTEX ||
        aShape3.ShapeType() != TopAbs_VERTEX) return 0;
    gp_Pnt aP1 = BRep_Tool::Pnt(TopoDS::Vertex(aShape1));
    gp_Pnt aP2 = BRep_Tool::Pnt(TopoDS::Vertex(aShape2));
    gp_Pnt aP3 = BRep_Tool::Pnt(TopoDS::Vertex(aShape3));
    if (aP1.Distance(aP2) < gp::Resolution() ||
        aP1.Distance(aP3) < gp::Resolution() ||
        aP2.Distance(aP3) < gp::Resolution())
      Standard_ConstructionError::Raise("Face creation aborted: coincident points given");
    if (gp_Vec(aP1, aP2).IsParallel(gp_Vec(aP1, aP3), Precision::Angular()))
      Standard_ConstructionError::Raise("Face creation aborted: points lay on one line");
	TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge( aP1, aP2 );
	TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge( aP2, aP3 );
	TopoDS_Edge e3 = BRepBuilderAPI_MakeEdge( aP3, aP1 );

	TopoDS_Wire w = BRepBuilderaFI_MakeWire( e1, e2, e3 );

    aShape = BRepBuilderaFI_MakeFace(w).Shape();
  }
  else if (aType == FACE_FOUR_PNT) {
	Handle(GEOM_Function) aRefPnt1 = aFI.GetPoint1();
	Handle(GEOM_Function) aRefPnt2 = aFI.GetPoint2();
	Handle(GEOM_Function) aRefPnt3 = aFI.GetPoint3();
	Handle(GEOM_Function) aRefPnt4 = aFI.GetPoint4();
	TopoDS_Shape aShape1 = aRefPnt1->GetValue();
	TopoDS_Shape aShape2 = aRefPnt2->GetValue();
	TopoDS_Shape aShape3 = aRefPnt3->GetValue();
	TopoDS_Shape aShape4 = aRefPnt4->GetValue();
	if (aShape1.ShapeType() != TopAbs_VERTEX ||
			aShape2.ShapeType() != TopAbs_VERTEX ||
			aShape3.ShapeType() != TopAbs_VERTEX ||
			aShape4.ShapeType() != TopAbs_VERTEX) return 0;
	gp_Pnt aP1 = BRep_Tool::Pnt(TopoDS::Vertex(aShape1));
	gp_Pnt aP2 = BRep_Tool::Pnt(TopoDS::Vertex(aShape2));
	gp_Pnt aP3 = BRep_Tool::Pnt(TopoDS::Vertex(aShape3));
	gp_Pnt aP4 = BRep_Tool::Pnt(TopoDS::Vertex(aShape4));
	if (aP1.Distance(aP2) < gp::Resolution() ||
			aP1.Distance(aP3) < gp::Resolution() ||
			aP1.Distance(aP4) < gp::Resolution() ||
			aP2.Distance(aP3) < gp::Resolution() ||
			aP2.Distance(aP4) < gp::Resolution() ||
			aP3.Distance(aP4) < gp::Resolution())
		Standard_ConstructionError::Raise("Face creation aborted: coincident points given");
    //still todo: order the points, check for intersecting edges
	gp_Vec v12( aP1, aP2 );
	gp_Vec v13( aP1, aP3 );
	gp_Pln plane( aP1, gp_Dir( v12.Crossed( v13 ) ) );
	if( plane.Contains( aP4, Precision::Confusion() ) )
	{
	  //the four points descibe a plane
		TopoDS_Edge e12 = BRepBuilderaFI_MakeEdge( aP1, aP2 ).Edge();
		TopoDS_Edge e23 = BRepBuilderaFI_MakeEdge( aP2, aP3 ).Edge();
		TopoDS_Edge e34 = BRepBuilderaFI_MakeEdge( aP3, aP4 ).Edge();
		TopoDS_Edge e41 = BRepBuilderaFI_MakeEdge( aP4, aP1 ).Edge();
		
		TopoDS_Wire w = BRepBuilderaFI_MakeWire( e12, e23, e34, e41 );
		aShape = BRepBuilderaFI_MakeFace( w );
	}
	else
	{
	  TColgp_Array2OfPnt   points(1, 2, 1, 2);
	  points.SetValue( 1, 1, aP1 );
	  points.SetValue( 1, 2, aP2 );
	  points.SetValue( 2, 1, aP4 );
	  points.SetValue( 2, 2, aP3 );
	
	  Handle_Geom_BezierSurface bez = new Geom_BezierSurface(points);
	  aShape = BRepBuilderAPI_MakeFace(bez, Precision::Confusion()).Shape();
	}
  }
  else if (aType == FACE_OBJ_H_W) {
    Handle(GEOM_Function) aRefFunct = aFI.GetRef1();
    TopoDS_Shape aRefShape = aRefFunct->GetValue();
    if (aRefShape.ShapeType() == TopAbs_EDGE) {
      TopoDS_Edge anEdge = TopoDS::Edge(aRefShape);
      double aH = aFI.GetH() / 2.0;
      double aW = aFI.GetW() / 2.0;
      TopoDS_Vertex V1, V2;
      TopExp::Vertices(anEdge, V1, V2, Standard_True);
      gp_Pnt aP (BRep_Tool::Pnt(V1));
      gp_Vec aV (BRep_Tool::Pnt(V1), BRep_Tool::Pnt(V2));
      gp_Pln aPlane (aP, aV);
      aShape = BRepBuilderAPI_MakeFace(aPlane, -aH, +aH, -aW, +aW).Shape();
    } else if (aRefShape.ShapeType() == TopAbs_FACE) {
      double aH = aFI.GetH() / 2.0;
      double aW = aFI.GetW() / 2.0;
      gp_Ax3 anAx = GEOMUtils::GetPosition(aRefShape);
      gp_Pln aPln (anAx);
      aShape = BRepBuilderAPI_MakeFace(aPln, -aH, +aH, -aW, +aW).Shape();
    }
  }
  else if (aType == FACE_H_W) {
    double aH = aFI.GetH() / 2.0;
    double aW = aFI.GetW() / 2.0;
    TopoDS_Vertex V1, V2;
    int anOrient = aFI.GetOrientation();
    gp_Pnt aP = gp::Origin();
    gp_Vec aV;
    if (anOrient == 1)
      aV = gp::DZ();
    else if (anOrient == 2)
      aV = gp::DX();
    else if (anOrient == 3)
      aV = gp::DY();

    gp_Pln aPlane (aP, aV);
    aShape = BRepBuilderAPI_MakeFace(aPlane, -aH, +aH, -aW, +aW).Shape();
  }
  else {
  }

  if (aShape.IsNull()) return 0;

  aFunction->SetValue(aShape);

  log.SetTouched(Label());

  return 1;
}

//================================================================================
/*!
 * \brief Returns a name of creation operation and names and values of creation parameters
 */
//================================================================================

bool GEOMImpl_FaceDriver::
GetCreationInformation(std::string&             theOperationName,
                       std::vector<GEOM_Param>& theParams)
{
  if (Label().IsNull()) return 0;
  Handle(GEOM_Function) function = GEOM_Function::GetFunction(Label());

  GEOMImpl_IFace aCI( function );
  Standard_Integer aType = function->GetType();

  theOperationName = "RECTANGLE";

  switch ( aType ) {
  case FACE_OBJ_H_W:
{
    Handle(GEOM_Function) aRefFunct = aCI.GetRef1();
    TopoDS_Shape aRefShape = aRefFunct->GetValue();
    if (aRefShape.ShapeType() == TopAbs_EDGE)
      AddParam( theParams, "Edge", aRefFunct );
    else
      AddParam( theParams, "Face", aRefFunct );
    AddParam( theParams, "Height", aCI.GetH() );
    AddParam( theParams, "Width", aCI.GetW() );
    break;
  }
  case FACE_H_W:
    AddParam( theParams, "Height", aCI.GetH() );
    AddParam( theParams, "Width", aCI.GetW() );
    AddParam( theParams, "Orientation", aCI.GetOrientation() );
    break;
  default:
    return false;
  }

  return true;
}

IMPLEMENT_STANDARD_HANDLE (GEOMImpl_FaceDriver,GEOM_BaseDriver);
IMPLEMENT_STANDARD_RTTIEXT (GEOMImpl_FaceDriver,GEOM_BaseDriver);
