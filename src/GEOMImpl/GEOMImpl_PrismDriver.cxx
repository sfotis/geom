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

#include <GEOMImpl_PrismDriver.hxx>

#include <GEOMImpl_IPrism.hxx>
#include <GEOMImpl_IShapesOperations.hxx>
#include <GEOMImpl_IMeasureOperations.hxx>
#include <GEOMImpl_GlueDriver.hxx>
#include <GEOMImpl_PipeDriver.hxx>
#include <GEOMImpl_Types.hxx>
#include <GEOM_Function.hxx>

#include <BRepPrimAPI_MakePrism.hxx>

#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepOffsetAPI_MakeDraft.hxx>
#include <BRepCheck_Shell.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRep_Tool.hxx>
#include <TopAbs.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>

#include <Precision.hxx>
#include <gp_Ax3.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Trsf.hxx>

#include <Standard_Stream.hxx>

#include <Standard_ConstructionError.hxx>

//=======================================================================
//function : GetID
//purpose  :
//======================================================================= 
const Standard_GUID& GEOMImpl_PrismDriver::GetID()
{
  static Standard_GUID aPrismDriver("FF1BBB17-5D14-4df2-980B-3A668264EA16");
  return aPrismDriver; 
}


//=======================================================================
//function : GEOMImpl_PrismDriver
//purpose  : 
//=======================================================================
GEOMImpl_PrismDriver::GEOMImpl_PrismDriver() 
{
}

//=======================================================================
//function : Execute
//purpose  :
//======================================================================= 
Standard_Integer GEOMImpl_PrismDriver::Execute(TFunction_Logbook& log) const
{
  if (Label().IsNull()) return 0;    
  Handle(GEOM_Function) aFunction = GEOM_Function::GetFunction(Label());

  GEOMImpl_IPrism aCI (aFunction);
  Standard_Integer aType = aFunction->GetType();

  TopoDS_Shape aShape;

  if (aType == PRISM_BASE_VEC_H || aType == PRISM_BASE_VEC_H_2WAYS) {
	Handle(GEOM_Function) aRefBase = aCI.GetBase();
	Handle(GEOM_Function) aRefVector = aCI.GetVector();
	TopoDS_Shape aShapeBase = aRefBase->GetValue();
	TopoDS_Shape aShapeVec = aRefVector->GetValue();
	if (aShapeVec.ShapeType() == TopAbs_EDGE) {
	  TopoDS_Edge anE = TopoDS::Edge(aShapeVec);
	  TopoDS_Vertex V1, V2;
	  TopExp::Vertices(anE, V1, V2, Standard_True);
	  if (!V1.IsNull() && !V2.IsNull()) {
		gp_Vec aV (BRep_Tool::Pnt(V1), BRep_Tool::Pnt(V2));
		if (Abs(aCI.GetH()) < Precision::Confusion()) {
		  Standard_ConstructionError::Raise("Absolute value of prism height is too small");
		}
		if (aV.Magnitude() > Precision::Confusion()) {
		  aV.Normalize();
          if (aType != PRISM_BASE_DXDYDZ_2WAYS && aCI.GetScale() > Precision::Confusion()) {
            aShape = MakeScaledPrism(aShapeBase, aV * aCI.GetH(), aCI.GetScale());
          }
          else {
		  if (aType == PRISM_BASE_VEC_H_2WAYS) {
			gp_Trsf aTrsf;
			aTrsf.SetTranslation( (-aV) * aCI.GetH() );
			BRepBuilderAPI_Transform aTransformation(aShapeBase, aTrsf, Standard_False);
			aShapeBase = aTransformation.Shape();
			aCI.SetH( aCI.GetH()*2 );
		  }
		  aShape = BRepPrimAPI_MakePrism(aShapeBase, aV * aCI.GetH(), Standard_False).Shape();
		}
	  }
	}
    }
  } else if (aType == PRISM_BASE_TWO_PNT || aType == PRISM_BASE_TWO_PNT_2WAYS) {
	Handle(GEOM_Function) aRefBase = aCI.GetBase();
	Handle(GEOM_Function) aRefPnt1 = aCI.GetFirstPoint();
	Handle(GEOM_Function) aRefPnt2 = aCI.GetLastPoint();
	TopoDS_Shape aShapeBase = aRefBase->GetValue();
	TopoDS_Shape aShapePnt1 = aRefPnt1->GetValue();
	TopoDS_Shape aShapePnt2 = aRefPnt2->GetValue();
	if (aShapePnt1.ShapeType() == TopAbs_VERTEX &&
		aShapePnt2.ShapeType() == TopAbs_VERTEX) {
	  TopoDS_Vertex V1 = TopoDS::Vertex(aShapePnt1);
	  TopoDS_Vertex V2 = TopoDS::Vertex(aShapePnt2);
	  if (!V1.IsNull() && !V2.IsNull()) {
		gp_Vec aV (BRep_Tool::Pnt(V1), BRep_Tool::Pnt(V2));
		if (aV.Magnitude() > gp::Resolution()) {
          if (aType != PRISM_BASE_DXDYDZ_2WAYS && aCI.GetScale() > Precision::Confusion()) {
            aShape = MakeScaledPrism(aShapeBase, aV, aCI.GetScale());
          }
          else {
		  if (aType == PRISM_BASE_TWO_PNT_2WAYS) {
			gp_Trsf aTrsf;
			aTrsf.SetTranslation(-aV);
			BRepBuilderAPI_Transform aTransformation(aShapeBase, aTrsf, Standard_False);
			aShapeBase = aTransformation.Shape();
			aV = aV * 2;
		  }
		  aShape = BRepPrimAPI_MakePrism(aShapeBase, aV, Standard_False).Shape();
		}
	  }
	}
  }
  }
  else if (aType == PRISM_BASE_VEC_H_ANG) {
	Handle(GEOM_Function) aRefBase = aCI.GetBase();
	Handle(GEOM_Function) aRefVector = aCI.GetVector();
	TopoDS_Shape aShapeBase = aRefBase->GetValue();
	TopoDS_Shape aShapeVec = aRefVector->GetValue();
	if (aShapeVec.ShapeType() == TopAbs_EDGE) {
	  TopoDS_Edge anE = TopoDS::Edge(aShapeVec);
	  TopoDS_Vertex V1, V2;
	  TopExp::Vertices(anE, V1, V2, Standard_True);
	  if (!V1.IsNull() && !V2.IsNull()) {
		gp_Vec aV (BRep_Tool::Pnt(V1), BRep_Tool::Pnt(V2));
		if (Abs(aCI.GetH()) < Precision::Confusion()) {
		  Standard_ConstructionError::Raise("Absolute value of prism height is too small");
		}
		if (aV.Magnitude() > Precision::Confusion()) {
		  aV.Normalize();
		  TopAbs_ShapeEnum myBaseType = aShapeBase.ShapeType();

		  if (  (myBaseType != TopAbs_EDGE) && (myBaseType != TopAbs_WIRE) &&
				(myBaseType != TopAbs_FACE) && (myBaseType != TopAbs_SHELL) )
			Standard_ConstructionError::Raise("Base shape type is not of the requested type");

		  if (myBaseType == TopAbs_EDGE)
			aShapeBase = BRepBuilderAPI_MakeWire(TopoDS::Edge(aShapeBase));

		  BRepOffsetAPI_MakeDraft aDraft(aShapeBase, aV, aCI.GetAngle());

		  if (aCI.GetAngle() < 0.)
		    aDraft.SetDraft(Standard_True);

		  aDraft.Perform(aCI.GetH());
		  aShape = aDraft.Shape();
		}
	  }
	}
  } else if (aType == PRISM_BASE_DXDYDZ || aType == PRISM_BASE_DXDYDZ_2WAYS) {
    Handle(GEOM_Function) aRefBase = aCI.GetBase();
    TopoDS_Shape aShapeBase = aRefBase->GetValue();
    gp_Vec aV (aCI.GetDX(), aCI.GetDY(), aCI.GetDZ());
    if (aV.Magnitude() > gp::Resolution()) {
      if (aType != PRISM_BASE_DXDYDZ_2WAYS && aCI.GetScale() > Precision::Confusion()) {
        aShape = MakeScaledPrism(aShapeBase, aV, aCI.GetScale());
      }
      else {
        if (aType == PRISM_BASE_DXDYDZ_2WAYS) {
	  gp_Trsf aTrsf;
	  aTrsf.SetTranslation(-aV);
	  BRepBuilderAPI_Transform aTransformation(aShapeBase, aTrsf, Standard_False);
	  aShapeBase = aTransformation.Shape();
	  aV = aV * 2;
	}
      aShape = BRepPrimAPI_MakePrism(aShapeBase, aV, Standard_False).Shape();
    }
    }
  }

  if (aShape.IsNull()) return 0;

  TopoDS_Shape aRes = GEOMImpl_IShapesOperations::CompsolidToCompound(aShape);
  aFunction->SetValue(aRes);

  log.SetTouched(Label()); 

  return 1;    
}

//=======================================================================
//function : MakeScaledPrism
//purpose  :
//=======================================================================
TopoDS_Shape GEOMImpl_PrismDriver::MakeScaledPrism (const TopoDS_Shape& theShapeBase,
                                                    const gp_Vec&       theVector,
                                                    const Standard_Real theScaleFactor,
                                                    const gp_Pnt&       theCDG,
                                                    bool                isCDG)
{
  TopoDS_Shape aShape;
  BRep_Builder B;

  // 1. aCDG = geompy.MakeCDG(theBase)
  gp_Pnt aCDG = theCDG;
  if (!isCDG) {
    gp_Ax3 aPos = GEOMImpl_IMeasureOperations::GetPosition(theShapeBase);
    aCDG = aPos.Location();
  }
  TopoDS_Shape aShapeCDG_1 = BRepBuilderAPI_MakeVertex(aCDG).Shape();

  // Process case of several given shapes
  if (theShapeBase.ShapeType() == TopAbs_COMPOUND ||
      theShapeBase.ShapeType() == TopAbs_SHELL) {
    int nbSub = 0;
    TopoDS_Shape aShapeI;
    TopoDS_Compound aCompound;
    B.MakeCompound(aCompound);
    TopoDS_Iterator It (theShapeBase, Standard_True, Standard_True);
    for (; It.More(); It.Next()) {
      nbSub++;
      aShapeI = MakeScaledPrism(It.Value(), theVector, theScaleFactor, aCDG, true);
      B.Add(aCompound, aShapeI);
    }
    if (nbSub == 1)
      aShape = aShapeI;
    else if (nbSub > 1)
      aShape = GEOMImpl_GlueDriver::GlueFaces(aCompound, Precision::Confusion(), Standard_True);
    return aShape;
  }

  // 2. Scale = geompy.MakeScaleTransform(theBase, aCDG, theScaleFactor)

  // Bug 6839: Check for standalone (not included in faces) degenerated edges
  TopTools_IndexedDataMapOfShapeListOfShape aEFMap;
  TopExp::MapShapesAndAncestors(theShapeBase, TopAbs_EDGE, TopAbs_FACE, aEFMap);
  Standard_Integer i, nbE = aEFMap.Extent();
  for (i = 1; i <= nbE; i++) {
    TopoDS_Shape anEdgeSh = aEFMap.FindKey(i);
    if (BRep_Tool::Degenerated(TopoDS::Edge(anEdgeSh))) {
      const TopTools_ListOfShape& aFaces = aEFMap.FindFromIndex(i);
      if (aFaces.IsEmpty())
        Standard_ConstructionError::Raise
          ("Scaling aborted : cannot scale standalone degenerated edge");
    }
  }

  // Perform Scaling
  gp_Trsf aTrsf;
  aTrsf.SetScale(aCDG, theScaleFactor);
  BRepBuilderAPI_Transform aBRepTrsf (theShapeBase, aTrsf, Standard_False);
  TopoDS_Shape aScale = aBRepTrsf.Shape();

  // 3. aBase2 = geompy.MakeTranslationVectorDistance(Scale, theVec, theH)
  gp_Trsf aTrsf3;
  aTrsf3.SetTranslation(theVector);
  TopLoc_Location aLocOrig = aScale.Location();
  gp_Trsf aTrsfOrig = aLocOrig.Transformation();
  TopLoc_Location aLocRes (aTrsf3 * aTrsfOrig);
  TopoDS_Shape aBase2 = aScale.Located(aLocRes);

  // 4. aCDG_2 = geompy.MakeTranslationVectorDistance(aCDG, theVec, theH)
  gp_Pnt aCDG_2 = aCDG.Translated(theVector);
  TopoDS_Shape aShapeCDG_2 = BRepBuilderAPI_MakeVertex(aCDG_2).Shape();

  // 5. Vector = geompy.MakeVector(aCDG, aCDG_2)
  TopoDS_Shape aShapeVec = BRepBuilderAPI_MakeEdge(aCDG, aCDG_2).Shape();
  TopoDS_Edge anEdge = TopoDS::Edge(aShapeVec);
  TopoDS_Wire aWirePath = BRepBuilderAPI_MakeWire(anEdge).Wire();

  // 6. aPrism = geompy.MakePipeWithDifferentSections([theBase, aBase2], [aCDG, aCDG_2], Vector, False, False)
  Handle(TopTools_HSequenceOfShape) aBases = new TopTools_HSequenceOfShape;
  aBases->Append(theShapeBase);
  aBases->Append(aBase2);

  Handle(TopTools_HSequenceOfShape) aLocs = new TopTools_HSequenceOfShape;
  aLocs->Append(aShapeCDG_1);
  aLocs->Append(aShapeCDG_2);

  aShape = GEOMImpl_PipeDriver::CreatePipeWithDifferentSections(aWirePath, aBases, aLocs, false, false);

  // 7. Make a solid, if possible
  if (theShapeBase.ShapeType() == TopAbs_FACE) {
    BRepBuilderAPI_Sewing aSewing (Precision::Confusion()*10.0);
    TopExp_Explorer expF (aShape, TopAbs_FACE);
    Standard_Integer ifa = 0;
    for (; expF.More(); expF.Next()) {
      aSewing.Add(expF.Current());
      ifa++;
    }
    if (ifa > 0) {
      aSewing.Perform();
      TopoDS_Shape aShell;

      TopoDS_Shape sh = aSewing.SewedShape();
      if (sh.ShapeType() == TopAbs_FACE && ifa == 1) {
        // case for creation of shell from one face
        TopoDS_Shell ss;
        B.MakeShell(ss);
        B.Add(ss,sh);
        aShell = ss;
      }
      else {
        TopExp_Explorer exp (sh, TopAbs_SHELL);
        Standard_Integer ish = 0;
        for (; exp.More(); exp.Next()) {
          aShell = exp.Current();
          ish++;
        }
        if (ish != 1)
          aShell = sh;
      }
      BRepCheck_Shell chkShell (TopoDS::Shell(aShell));
      if (chkShell.Closed() == BRepCheck_NoError) {
        TopoDS_Solid Sol;
        B.MakeSolid(Sol);
        B.Add(Sol, aShell);
        BRepClass3d_SolidClassifier SC (Sol);
        SC.PerformInfinitePoint(Precision::Confusion());
        if (SC.State() == TopAbs_IN) {
          B.MakeSolid(Sol);
          B.Add(Sol, aShell.Reversed());
        }
        aShape = Sol;
      }
    }
  }

  return aShape;
}

//=======================================================================
//function :  GEOMImpl_PrismDriver_Type_
//purpose  :
//======================================================================= 
Standard_EXPORT Handle_Standard_Type& GEOMImpl_PrismDriver_Type_()
{

  static Handle_Standard_Type aType1 = STANDARD_TYPE(TFunction_Driver);
  if ( aType1.IsNull()) aType1 = STANDARD_TYPE(TFunction_Driver);
  static Handle_Standard_Type aType2 = STANDARD_TYPE(MMgt_TShared);
  if ( aType2.IsNull()) aType2 = STANDARD_TYPE(MMgt_TShared); 
  static Handle_Standard_Type aType3 = STANDARD_TYPE(Standard_Transient);
  if ( aType3.IsNull()) aType3 = STANDARD_TYPE(Standard_Transient);
 

  static Handle_Standard_Transient _Ancestors[]= {aType1,aType2,aType3,NULL};
  static Handle_Standard_Type _aType = new Standard_Type("GEOMImpl_PrismDriver",
			                                 sizeof(GEOMImpl_PrismDriver),
			                                 1,
			                                 (Standard_Address)_Ancestors,
			                                 (Standard_Address)NULL);

  return _aType;
}

//=======================================================================
//function : DownCast
//purpose  :
//======================================================================= 
const Handle(GEOMImpl_PrismDriver) Handle(GEOMImpl_PrismDriver)::DownCast(const Handle(Standard_Transient)& AnObject)
{
  Handle(GEOMImpl_PrismDriver) _anOtherObject;

  if (!AnObject.IsNull()) {
     if (AnObject->IsKind(STANDARD_TYPE(GEOMImpl_PrismDriver))) {
       _anOtherObject = Handle(GEOMImpl_PrismDriver)((Handle(GEOMImpl_PrismDriver)&)AnObject);
     }
  }

  return _anOtherObject ;
}
