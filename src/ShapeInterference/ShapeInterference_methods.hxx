// File:	ShapeInterference_methods.hxx
// Created:	Wed Jan 29 14:32:11 1997
// Author:	Prestataire Xuan PHAM PHU
//		<xpu@salgox.paris1.matra-dtv.fr>


#ifndef _ShapeInterference_methods_HeaderFile
#define _ShapeInterference_methods_HeaderFile

#include <gp_Pnt2d.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <Bnd_Box.hxx>
#include <TopOpeBRepBuild_HBuilder.hxx>
#include <TopTools_Array1OfShape.hxx>
#include <TopTools_Array1OfListOfShape.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_IndexedDataMapOfShapeShape.hxx>
#include <TopTools_DataMapOfIntegerListOfShape.hxx>
#include <TopOpeBRepDS_HDataStructure.hxx>
#include <ShapeInterference_Interference.hxx>


Standard_Boolean ShapeInterference_TestOnVertex
(const TopoDS_Shape& aShape1,
 const TopoDS_Shape& aShape2,
 Standard_Boolean& allvof1ON2);

Standard_Boolean ShapeInterference_IsInBndBox
(const Bnd_Box& aBndBox1,
 const Bnd_Box& aBndBox2,
 Standard_Boolean& OneIsInTwo);

/*
void ShapeInterference_AddToMap(TopTools_DataMapOfShapeListOfShape& mapoftgcies,
				const TopoDS_Shape& shapeOn1,
				const TopoDS_Shape& shapeOn2,
				const TopoDS_Shape& anEdge);

void ShapeInterference_MapOfTgciesToInterf
(const TopTools_DataMapOfShapeListOfShape& mapoftgcies,
 ShapeInterference_Interference& theInterf);

Standard_Boolean  ShapeInterference_GetAncestorsFromEdge
(const TopoDS_Shape& theEdge,
 const Handle(TopOpeBRepBuild_HBuilder)& HB,
 const Standard_Boolean& isaCurveEdge,
 const TopTools_Array1OfShape& Shape,
 TopTools_Array1OfShape& AncestorToEdge,
 Standard_Boolean& twocornercase);

Standard_Boolean ShapeInterference_SameDomainandDiffOri
(const TopoDS_Shape& aShape1,
 const TopoDS_Shape& aShape2,
 const Handle(TopOpeBRepDS_HDataStructure)& HDS);

Standard_Boolean ShapeInterference_HasTgtFaces
(const Handle( TopOpeBRepDS_HDataStructure )& HDS,
 TopTools_Array1OfListOfShape& theConnexedFaces,
 TopTools_Array1OfListOfShape& TgtFaces);

Standard_Boolean ShapeInterference_HasTgtFaces
(const Handle( TopOpeBRepDS_HDataStructure )& HDS,
 TopTools_Array1OfListOfShape& theConnexedFaces,
 TopTools_Array1OfListOfShape& TgtFaces);
*/

Standard_Boolean ShapeInterference_GetConnexedFaces
(const TopTools_Array1OfShape& theShape,
 const TopTools_Array1OfShape& theAncestors,
 TopTools_Array1OfListOfShape& ConnexedFaces);

 // xpu : +16-07-97
Standard_Boolean ShapeInterference_SameDomain
(const TopoDS_Shape& aShape1,
 const TopoDS_Shape& aShape2,
 const Handle(TopOpeBRepDS_HDataStructure)& HDS);

Standard_Boolean ShapeInterference_DiffOri
(const TopoDS_Shape& aShape1,
 const TopoDS_Shape& aShape2,
 const Handle(TopOpeBRepDS_HDataStructure)& HDS);

Standard_Boolean ShapeInterference_HasSdmFaces
(const Handle( TopOpeBRepDS_HDataStructure )& HDS,
 TopTools_Array1OfListOfShape& theConnexedFaces,
 TopTools_DataMapOfShapeShape& SdmF);

Standard_Boolean  ShapeInterference_PInFclosetoE (const TopoDS_Shape& theEdge,
						  const TopoDS_Shape& theFace,
						  gp_Pnt& thePoint);

Standard_Boolean ShapeInterference_ShareSameDomain
(const TopoDS_Shape& ed,
 const TopoDS_Shape& f1,
 const TopoDS_Shape& f2);
 // xpu : +16-07-97

Standard_Boolean ShapeInterference_FaceIsInSh(const TopoDS_Shape& theEdge,
					      const TopoDS_Shape& theFace,
					      const TopoDS_Shape& theShape);

Standard_Boolean ShapeInterference_CollisionOnSectionEdge
(const Handle(TopOpeBRepBuild_HBuilder)& HB,
 const Handle( TopOpeBRepDS_HDataStructure )& HDS,
 const TopoDS_Shape& theEdge,
 const TopTools_Array1OfShape& Shape,
// const TopTools_Array1OfShape& AncestorToEdge,   // xpu : - 15-07-97
// const Standard_Boolean& twocornercase,          // xpu : - 15-07-97
// TopTools_Array1OfListOfShape& TangentAncestors, // xpu : - 15-07-97
 TopTools_Array1OfShape& AncestorToEdge,
 TopTools_DataMapOfShapeShape& TangentAncestors,
 Standard_Boolean& hascollision);


 // xpu : +16-07-97
Standard_Boolean ShapeInterference_parVonF
(const TopoDS_Vertex& v,
 const TopoDS_Face& F,
 gp_Pnt2d& p2d);

Standard_Boolean ShapeInterference_getnormal
(const TopoDS_Vertex& v,
 const TopoDS_Face& F,
 gp_Vec normt);

Standard_Boolean ShapeInterference_IsAnInclusion
(const ShapeInterference_Interference& theInterf,
 const Standard_Boolean OneIsInTwo);

void ShapeInterference_addToItem
(TopTools_DataMapOfIntegerListOfShape& mapTgcies,
 const Standard_Integer& Ise,
 const TopoDS_Shape& fa);

void ShapeInterference_TgciesToInterf
(const TopTools_IndexedDataMapOfShapeShape& mapf1tgTof2,
 const TopTools_DataMapOfIntegerListOfShape& mapTgcies,
 ShapeInterference_Interference& theInterf);
 // xpu : +16-07-97

#endif
