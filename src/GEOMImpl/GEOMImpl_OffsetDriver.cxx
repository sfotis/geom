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

#include <Standard_Stream.hxx>

#include <GEOMImpl_OffsetDriver.hxx>
#include <GEOMImpl_IOffset.hxx>
#include <GEOMImpl_Types.hxx>
#include <GEOM_Function.hxx>

#include <BRepOffsetAPI_MakeOffsetShape.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopAbs.hxx>
#include <TopExp.hxx>

#include <BRepClass3d_SolidClassifier.hxx>

#include <Precision.hxx>
#include <gp_Pnt.hxx>

#include <BRepCheck_Analyzer.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <ShapeFix_Shape.hxx>

#include <Standard_ConstructionError.hxx>
#include <StdFail_NotDone.hxx>

#include "utilities.h"

//=======================================================================
//function : GetID
//purpose  :
//=======================================================================
const Standard_GUID& GEOMImpl_OffsetDriver::GetID()
{
  static Standard_GUID aOffsetDriver("FF1BBB51-5D14-4df2-980B-3A668264EA16");
  return aOffsetDriver;
}


//=======================================================================
//function : GEOMImpl_OffsetDriver
//purpose  :
//=======================================================================
GEOMImpl_OffsetDriver::GEOMImpl_OffsetDriver()
{
}

//=======================================================================
//function : Execute
//purpose  :
//=======================================================================
Standard_Integer GEOMImpl_OffsetDriver::Execute(TFunction_Logbook& log) const
{
  if (Label().IsNull()) return 0;
  Handle(GEOM_Function) aFunction = GEOM_Function::GetFunction(Label());

  GEOMImpl_IOffset aCI (aFunction);
  Standard_Integer aType = aFunction->GetType();

  TopoDS_Shape aShape;

	Handle(GEOM_Function) aRefShape = aCI.GetShape();
	TopoDS_Shape aShapeBase = aRefShape->GetValue();
	Standard_Real anOffset = aCI.GetValue();
	Standard_Real aTol = Precision::Confusion();

	if (Abs(anOffset) < aTol) {
	  TCollection_AsciiString aMsg ("Absolute value of offset can not be less than the tolerance value (");
	  aMsg += TCollection_AsciiString(aTol);
	  aMsg += ")";
	  StdFail_NotDone::Raise(aMsg.ToCString());
	}

  if (aType == OFFSET_SHAPE || aType == OFFSET_SHAPE_COPY) {
	BRepOffsetAPI_MakeOffsetShape MO (aShapeBase,
									  aCI.GetValue(),
									  aTol);
    if (MO.IsDone()) {
	  aShape = MO.Shape();
	}
    else {
      StdFail_NotDone::Raise("Offset construction failed");
    }
  }
  else if (aType == OFFSET_THICKENING || aType == OFFSET_THICKENING_COPY)
  {
    BRepClass3d_SolidClassifier aClassifier = BRepClass3d_SolidClassifier(aShapeBase);
    aClassifier.PerformInfinitePoint(Precision::Confusion());
    if (aClassifier.State()==TopAbs_IN)
    {
      // If the generated pipe faces normals are oriented towards the inside, the offset is negative
      // so that the thickening is still towards outside
      anOffset=-anOffset;
    }

    BRepOffset_MakeOffset myOffsetShape(aShapeBase, anOffset, aTol, BRepOffset_Skin,
                                        Standard_False, Standard_False, GeomAbs_Intersection, Standard_True);

    if (!myOffsetShape.IsDone())
    {
      StdFail_NotDone::Raise("Thickening construction failed");
    }
    aShape = myOffsetShape.Shape();

    // Control the solid orientation. This is mostly done to fix a bug in case of extrusion
    // of a circle. The built solid is then badly oriented
    BRepClass3d_SolidClassifier anotherClassifier = BRepClass3d_SolidClassifier(aShape);
    anotherClassifier.PerformInfinitePoint(Precision::Confusion());
    if (anotherClassifier.State()==TopAbs_IN)
    {
      aShape.Reverse();
    }
  }

  else if (aType == OFFSET_SHAPE_PLANAR || aType == OFFSET_SHAPE_COPY_PLANAR) {
	Handle(GEOM_Function) aRefShape = aCI.GetShape();
	TopoDS_Shape aShapeBase = aRefShape->GetValue();
	Standard_Real anOffset = aCI.GetValue();
	Standard_Real anAltValue = aCI.GetAltValue();

	if (Abs(anOffset) < Precision::Confusion()) {
	  TCollection_AsciiString aMsg ("Offset value can not be less than the Confusion tolerance value (");
	  StdFail_NotDone::Raise(aMsg.ToCString());
	}

	BRepOffsetAPI_MakeOffset MO;
	bool isFace = false;

	if (aShapeBase.ShapeType() == TopAbs_FACE)
	{
		TopoDS_Face aFace = TopoDS::Face(aShapeBase);
		MO.Init(aFace, GeomAbs_Arc);
		isFace = true;
	}
	else if (aShapeBase.ShapeType() == TopAbs_WIRE)
	{
		TopoDS_Wire aWire = TopoDS::Wire(aShapeBase);
		MO.Init(GeomAbs_Arc);
		MO.AddWire(aWire);
	}
	else if (aShapeBase.ShapeType() == TopAbs_EDGE) 
	{
	  BRepBuilderAPI_MakeWire MW;
	  MW.Add(TopoDS::Edge(aShapeBase));
      if (!MW.IsDone()) {
        Standard_ConstructionError::Raise("Wire construction from edge failed");
	  }
	  MO.Init(GeomAbs_Arc);
	  MO.AddWire(MW.Wire());
	}
	else
	{
		StdFail_NotDone::Raise("Shape is neither a face , a wire or a planar edge");
	}

	MO.Perform(anOffset, anAltValue);

	if (MO.IsDone()) {
	  TopoDS_Shape aWireShape;
	  aWireShape = MO.Shape();
	  if (isFace)
	  {
		//In case we have a face then the result should be again a face.
		//Since the result of BRepOffsetAPI_MakeOffset is always a wire
		//then we will try to convert the wire to a face.
		//NOTE : A face gives always a closed wire !!
		TopoDS_Wire aWire = TopoDS::Wire(aWireShape);
		GEOMImpl_Block6Explorer::MakeFace(aWire, true, aShape);
	  }
	  else													  
	  {
		 aShape = aWireShape;
	  }
	} else {
	  StdFail_NotDone::Raise("Offset construction failed");
	}
  }
  else {
  }

  if (aShape.IsNull()) return 0;

  // 23.04.2010 skl for bug 21699 from Mantis
  BRepCheck_Analyzer ana (aShape, Standard_True);
  ana.Init(aShape);
  if (!ana.IsValid()) {
    ShapeFix_ShapeTolerance aSFT;
    aSFT.LimitTolerance(aShape, Precision::Confusion(),
                        Precision::Confusion(), TopAbs_SHAPE);
    Handle(ShapeFix_Shape) aSfs = new ShapeFix_Shape(aShape);
    aSfs->Perform();
    aShape = aSfs->Shape();
    ana.Init(aShape);
    if (!ana.IsValid())
      Standard_ConstructionError::Raise("Offset construction failed : non valid shape result");
  }

  aFunction->SetValue(aShape);

  log.SetTouched(Label());

  return 1;
}

//================================================================================
/*!
 * \brief Returns a name of creation operation and names and values of creation parameters
 */
//================================================================================

bool GEOMImpl_OffsetDriver::
GetCreationInformation(std::string&             theOperationName,
                       std::vector<GEOM_Param>& theParams)
{
  if (Label().IsNull()) return 0;
  Handle(GEOM_Function) function = GEOM_Function::GetFunction(Label());

  GEOMImpl_IOffset aCI( function );
  Standard_Integer aType = function->GetType();

  switch ( aType ) {
  case OFFSET_SHAPE:
  case OFFSET_SHAPE_COPY:
    theOperationName = "OFFSET";
    AddParam( theParams, "Object", aCI.GetShape() );
    AddParam( theParams, "Offset", aCI.GetValue() );
    break;
  case OFFSET_THICKENING:
  case OFFSET_THICKENING_COPY:
    theOperationName = "MakeThickening";
    AddParam( theParams, "Object", aCI.GetShape() );
    AddParam( theParams, "Offset", aCI.GetValue() );
    break;
  default:
    return false;
  }
  
  return true;
}

IMPLEMENT_STANDARD_HANDLE (GEOMImpl_OffsetDriver,GEOM_BaseDriver);
IMPLEMENT_STANDARD_RTTIEXT (GEOMImpl_OffsetDriver,GEOM_BaseDriver);
