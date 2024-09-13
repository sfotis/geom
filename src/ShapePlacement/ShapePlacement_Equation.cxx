// File:	ShapePlacement_Equation.cxx
// Created:	Wed Mar 27 10:39:22 1996
// Author:	Xavier BENVENISTE
//		<xab@mentox>

#include <ShapePlacement_TypeOfConstraint.hxx>
#include <ShapePlacement_TypeOfAxisConstraint.hxx>
#include <ShapePlacement_TypeOfEquation.hxx>
#include <ShapePlacement_Equation.ixx>
#include <Standard_ConstructionError.hxx>
#include <TopoDS.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Surface.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <TopAbs.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS_Face.hxx>
#include <gp_Ax3.hxx>
#include <Precision.hxx>
#include <gp_Pln.hxx>
#include <gp_Sphere.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Torus.hxx>
#include <gp_Cone.hxx>

// 
// Diagramme of the shape placement constraint to use when issuing constraints
// 
// 
//           Orientation Topologique de la face
//           Matiere simbolisee par des +++
// 
//             Fix        Moving
// 
//             +|           +|
//             +|           +|
//             +|           +|
// 
// 
//      Type     Fix      Moving       Distance     Effet  
// 
//   IN_ALIGN    Forward  Forward      None        translation de Moving
// 
//   IN_ALIGN    Forward  Reverse      None        translation avec retournement 
// 						de Moving
//                                                |
//   IN_ALIGN    Reverse  Forward      None        translation et retournement 
// 						de Moving
// 
//   IN_ALIGN    Reverse  Reverse      None        translation de Moving
//  
//   OUT_ALIGN   Forward  Forward      None        translation de Moving 
// 						avec retournement
// 
//   OUT_ALIGN   Forward  Reverse      None        translation de Moving 
//   
//   OUT_ALIGN   Reverse  Forward      None        translation de Moving
// 
//   OUT_ALIGN   Reverse  Reverse      None        translation de Moving 
// 						avec retournement
// 
// 
// 
//                                             Moving      Fix           Moving
//   IN_PARALLELE  Forward  Forward    > 0         	 +|  <----->  +|
//                                                          +|           +|
//                                                          +|   dist    +|
// 
//   IN_PARALLELE  Forward  Reverse    > 0                  +|  <----->   |+
//                                                          +|   dist     |+
//                                                          +|            |+
// 
//   IN_PARALLELE  Reverse Forward     > 0       +| <---->  +|  
//                                               +|  dist   +|  
//                                               +|         +|           
// 
//   IN_PARALLELE  Reverse Reverse     > 0       |+ <---->  +|
//                                               |+  dist   +|
//                                               |+         +|
// 
// 
//   OUT_PARALLELE Forward Forward     > 0       +| <---->  +|  
//                                               +|  dist   +|  
//                                               +|         +|           
// 
// 
//   OUT_PARALLELE Forward Reverse     > 0       |+ <---->  +|
//                                               |+  dist   +|
//                                               |+         +|
// 
//   OUT_PARALLELE  Reverse Forward    > 0    	     	 +|  <----->  +|
//                                                          +|           +|
//                                                          +|   dist    +|
// 
// 
//   
//   
//   IN_PARALLELE  Forward  Reverse    > 0                  +|  <----->   |+
//                                                          +|   dist     |+
//                                                          +|            |+
// 



//=======================================================================
//function : ShapePlacement_Equation
//purpose  : 
//=======================================================================

ShapePlacement_Equation::ShapePlacement_Equation() 
{} 

//=======================================================================
//function : ShapePlacement_Equation
//purpose  : 
//=======================================================================

ShapePlacement_Equation::ShapePlacement_Equation(
		      const ShapePlacement_TypeOfConstraint aKey, 
		      const TopoDS_Shape& aSubShape, 
		      const ShapePlacement_TypeOfAxisConstraint aSubAxis,
                      const TopoDS_Shape& aFixShape, 
		      const ShapePlacement_TypeOfAxisConstraint aFixAxis, 
		      const Standard_Real aValue, 
		      const Standard_Boolean IsaValue)
{
 Init( Precision::Angular(),
       aKey, 
       aSubShape, 
       aSubAxis,
       aFixShape, 
       aFixAxis, 
       aValue, 
       IsaValue) ;
}
//=======================================================================
//function : ShapePlacement_Equation
//purpose  : 
//=======================================================================

ShapePlacement_Equation::ShapePlacement_Equation(
		      const Standard_Real anAngularTolerance,
		      const ShapePlacement_TypeOfConstraint aKey, 
		      const TopoDS_Shape& aSubShape, 
		      const ShapePlacement_TypeOfAxisConstraint aSubAxis,
                      const TopoDS_Shape& aFixShape, 
		      const ShapePlacement_TypeOfAxisConstraint aFixAxis, 
		      const Standard_Real aValue, 
		      const Standard_Boolean IsaValue)
{
 Init( anAngularTolerance,
       aKey, 
       aSubShape, 
       aSubAxis,
       aFixShape, 
       aFixAxis, 
       aValue, 
       IsaValue) ;
}
//=======================================================================
//function : Init
//purpose  : 
//=======================================================================

void ShapePlacement_Equation::Init(
		      const Standard_Real  anAngularTolerance,
		      const ShapePlacement_TypeOfConstraint aKey, 
		      const TopoDS_Shape& aSubShape, 
		      const ShapePlacement_TypeOfAxisConstraint aSubAxis,
                      const TopoDS_Shape& aFixShape, 
		      const ShapePlacement_TypeOfAxisConstraint aFixAxis, 
		      const Standard_Real aValue, 
		      const Standard_Boolean IsaValue)
{
//
//  get the type of the shapes involved
//
 TopAbs_ShapeEnum a_moving_type,
 a_fixed_type ;

 a_moving_type =
 aSubShape.ShapeType() ;
 a_fixed_type =
 aFixShape.ShapeType() ;
 if (aSubAxis == ShapePlacement_AXIS) {
   a_moving_type = TopAbs_FACE ;
 }
 if (aFixAxis == ShapePlacement_AXIS) {
   a_fixed_type = TopAbs_FACE ;
 }
 if (a_moving_type == TopAbs_FACE && aSubAxis == ShapePlacement_NONE)  {
   const TopoDS_Face & a_moving_face = 
     TopoDS::Face(aSubShape) ;
   if (a_fixed_type == TopAbs_FACE && aFixAxis == ShapePlacement_NONE) {
     const TopoDS_Face & a_fixed_face =
       TopoDS::Face(aFixShape) ;
     InitFaceFace(anAngularTolerance,
		  a_moving_face,
		  a_fixed_face,
		  aKey,
		  aValue,
		  IsaValue) ;
   }
   if (a_fixed_type == TopAbs_FACE && aFixAxis == ShapePlacement_AXIS) {
     const TopoDS_Face & a_fixed_face =
       TopoDS::Face(aFixShape) ;
     InitFaceAxis(a_moving_face,
		  a_fixed_face,
		  aKey,
		  aValue,
		  IsaValue) ;
   }
 }
 else {
   if (a_moving_type == TopAbs_FACE && aSubAxis == ShapePlacement_AXIS)  {
     const TopoDS_Face & a_moving_face = 
       TopoDS::Face(aSubShape) ;
     if (a_fixed_type == TopAbs_FACE && aFixAxis == ShapePlacement_AXIS) {
       const TopoDS_Face & a_fixed_face =
	 TopoDS::Face(aFixShape) ;
       InitAxisAxis(a_moving_face,
		    a_fixed_face,
		    aKey,
		    aValue,
		    IsaValue) ;
     }
     if (a_fixed_type == TopAbs_FACE && aFixAxis == ShapePlacement_NONE) {
       const TopoDS_Face & a_fixed_face =
	 TopoDS::Face(aFixShape) ;
       InitAxisFace(a_moving_face,
		    a_fixed_face,
		    aKey,
		    aValue,
		    IsaValue) ;
     } 
   }
 }
}


//=======================================================================
//function : InitFaceFace
//purpose  : 
//=======================================================================

void ShapePlacement_Equation::InitFaceFace(
		      const Standard_Real      anAngularTolerance,
		      const TopoDS_Face&       aFace, 
                      const TopoDS_Face&       aFixShape, 
		      const ShapePlacement_TypeOfConstraint aKey, 
		      const Standard_Real aValue, 
		      const Standard_Boolean IsaValue)
{
  Standard_Integer ii ;
  TopLoc_Location moving_location,
  fixed_location ;
  Standard_Boolean Have_2_Surfaces_Ok = Standard_False;
  gp_Dir Dummy(0.0e0, 0.0e0, 1.0e0) ;  
  myFixedDirection  = Dummy ;
  myMovingDirection = Dummy ;
  myFixedNormal  = Dummy ;
  myMovingNormal = Dummy ;
  Handle(Geom_Surface) moving_surface_ptr = 
    BRep_Tool::Surface(aFace,
		       moving_location) ;
  Handle(Geom_Surface) fixed_surface_ptr =
     BRep_Tool::Surface(aFixShape,
			fixed_location) ;
  if (moving_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface)) {
    Handle(Geom_RectangularTrimmedSurface) trimmed_moving_ptr =
      Handle(Geom_RectangularTrimmedSurface)::DownCast(moving_surface_ptr); 
    moving_surface_ptr =
      trimmed_moving_ptr->BasisSurface() ;
  }
  if (fixed_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface))   {
    Handle(Geom_RectangularTrimmedSurface) trimmed_fixed_ptr =
      Handle(Geom_RectangularTrimmedSurface)::DownCast(fixed_surface_ptr);
    fixed_surface_ptr = 
      trimmed_fixed_ptr->BasisSurface() ;
  }


// 2 surfaces coniques de meme angle 
  gp_Ax3 moving_axis,fixed_axis;
  Standard_Real moving_angle,fixed_angle;
  if (moving_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_ConicalSurface)
                                        &&
       fixed_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_ConicalSurface)) {
    Handle(Geom_ConicalSurface) moving_conical_ptr = 
      Handle(Geom_ConicalSurface)::DownCast(moving_surface_ptr) ;
    Handle(Geom_ConicalSurface) fixed_conical_ptr =
      Handle(Geom_ConicalSurface)::DownCast(fixed_surface_ptr) ;
    moving_angle = moving_conical_ptr->Cone().SemiAngle();
    fixed_angle = fixed_conical_ptr->Cone().SemiAngle();
    if (Abs(Abs(moving_angle) - Abs(fixed_angle)) <= anAngularTolerance) {
      moving_axis = moving_conical_ptr->Cone().Position() ;
      fixed_axis = fixed_conical_ptr->Cone().Position() ;
      moving_axis.SetLocation(moving_conical_ptr->Cone().Apex());
      fixed_axis.SetLocation(fixed_conical_ptr->Cone().Apex());
      if ( Abs(moving_angle + fixed_angle) <= anAngularTolerance) //Precision_Angular)
	if (aKey == ShapePlacement_OUT_ALIGN || aKey == ShapePlacement_OUT_PARALLELE) 
	  fixed_axis.SetDirection(- fixed_axis.Direction());
	else if (aKey == ShapePlacement_IN_ALIGN || aKey == ShapePlacement_IN_PARALLELE)
	  fixed_axis.SetDirection(- fixed_axis.Direction());
      myKeyWord      = ShapePlacement_CONE_PLACEMENT ;
      Have_2_Surfaces_Ok = Standard_True;
    }
    
  } else if (moving_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_Plane) &&
	     fixed_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_Plane)) {
    Handle(Geom_Plane) moving_plane_ptr = 
      Handle(Geom_Plane)::DownCast(moving_surface_ptr) ;
    Handle(Geom_Plane) fixed_plane_ptr =
      Handle(Geom_Plane)::DownCast(fixed_surface_ptr) ;
    moving_axis = moving_plane_ptr->Pln().Position() ;
    fixed_axis = fixed_plane_ptr->Pln().Position() ;

      
    Have_2_Surfaces_Ok = Standard_True;
    myKeyWord      = ShapePlacement_PLANE_PLACEMENT ;
  }

//
//   transform the axis by the Location
// 
  if (Have_2_Surfaces_Ok) {
    moving_axis.Transform(moving_location.Transformation()) ;
    fixed_axis.Transform(fixed_location.Transformation()) ;
    myFixedPoint   = fixed_axis.Location()  ;
    myMovingPoint  = moving_axis.Location() ;
    myFixedNormal  = fixed_axis.Direction() ;
    myMovingNormal = moving_axis.Direction() ;

    if (myKeyWord == ShapePlacement_PLANE_PLACEMENT) {
      if (! moving_axis.Direct()) {
//
//  the orientation of the solid is regarding the Xdirection crossed with the Y direction
//  and not using the Direction !!
//
	for (ii = 1 ; ii <= 3 ; ii++) {
	  myMovingNormal.SetCoord(ii,- myMovingNormal.Coord(ii)) ;
	}
      }
      if (! fixed_axis.Direct() ) {
//
//  the orientation of the solid is regarding the Xdirection crossed with the Y direction
//  and not using the Direction !!
//
	for (ii = 1 ; ii <= 3 ; ii++) {
	  myFixedNormal.SetCoord(ii, - myFixedNormal.Coord(ii)) ;
	}
      }  
    }
    myDistance = 0.0e0 ;
    myAngle    = 0.0e0 ;
    switch (aKey) {
    case ShapePlacement_ALIGN:
    case ShapePlacement_IN_ALIGN:
      break ;
    case ShapePlacement_OUT_ALIGN:
      if (myKeyWord == ShapePlacement_PLANE_PLACEMENT)
	for (ii = 1 ; ii <= 3 ; ii++) {
	  myFixedNormal.SetCoord(ii, -myFixedNormal.Coord(ii)) ;
	}
      break ;
    case ShapePlacement_PARALLELE:
    case ShapePlacement_IN_PARALLELE:
      if (IsaValue) {
	myDistance = aValue ;
      }
      break ;
      
      
    case ShapePlacement_OUT_PARALLELE:
      if (myKeyWord == ShapePlacement_PLANE_PLACEMENT)
	for (ii = 1 ; ii <= 3 ; ii++) {
	  myFixedNormal.SetCoord(ii, -myFixedNormal.Coord(ii)) ;
	}
      if (IsaValue) {
	myDistance =  aValue ;
      }
      break ;
      
    case ShapePlacement_ANGLE:
    case ShapePlacement_IN_ANGLE:
    case ShapePlacement_OUT_ANGLE:
      if (IsaValue) {
	myAngle = aValue  ;
      }
      myKeyWord      = ShapePlacement_P_ANGULAR_PLACEMENT ; 
      break;
      default :
      Standard_ConstructionError::Raise() ;
      break ; 
    }
  }   
  
  else {
    Standard_ConstructionError::Raise() ;
  }
}
//=======================================================================
//function : InitAxisAxis
//purpose  : 
//=======================================================================

void ShapePlacement_Equation::InitAxisAxis(
		      const TopoDS_Face&                    aFace, 
                      const TopoDS_Face&                   aFixShape, 
		      const ShapePlacement_TypeOfConstraint aKey, 
		      const Standard_Real aValue, 
		      const Standard_Boolean IsaValue)
{
  Standard_Integer ii ;
  TopLoc_Location moving_location,
  fixed_location ;
  Standard_Boolean Cylinders_or_Cones = Standard_False, 
  Cylinder_or_Cone = Standard_False;
  gp_Dir Dummy(0.0e0, 0.0e0, 1.0e0) ;  
  myFixedDirection  = Dummy ;
  myMovingDirection = Dummy ;
  myFixedNormal  = Dummy ;
  myMovingNormal = Dummy ;
  Handle(Geom_Surface) moving_surface_ptr = 
    BRep_Tool::Surface(aFace,
		       moving_location) ;
  Handle(Geom_Surface) fixed_surface_ptr =
     BRep_Tool::Surface(aFixShape,
			fixed_location) ;
  if (moving_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface)) {
    Handle(Geom_RectangularTrimmedSurface) trimmed_moving_ptr =
      Handle(Geom_RectangularTrimmedSurface)::DownCast(moving_surface_ptr); 
    moving_surface_ptr =
      trimmed_moving_ptr->BasisSurface() ;
  }
  if (fixed_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface))   {
    Handle(Geom_RectangularTrimmedSurface) trimmed_fixed_ptr =
      Handle(Geom_RectangularTrimmedSurface)::DownCast(fixed_surface_ptr);
    fixed_surface_ptr = 
      trimmed_fixed_ptr->BasisSurface() ;
  }


//  if (moving_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_CylindricalSurface) 
//                     &&
//	   fixed_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_CylindricalSurface)) {
//    Handle(Geom_CylindricalSurface) moving_cylinder_ptr = 
//      Handle(Geom_CylindricalSurface)::DownCast(moving_surface_ptr) ;
//    Handle(Geom_CylindricalSurface) fixed_cylinder_ptr =
//      Handle(Geom_CylindricalSurface)::DownCast(fixed_surface_ptr) ;
//    gp_Ax3 moving_axis = moving_cylinder_ptr->Cylinder().Position() ;
//    gp_Ax3 fixed_axis = fixed_cylinder_ptr->Cylinder().Position() ;

  gp_Ax3 moving_axis,fixed_axis;

// moving_shape cylindre ou cone ? ou torus!!

  if (moving_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_CylindricalSurface)) {
    Handle(Geom_CylindricalSurface) moving_shape_ptr =
      Handle(Geom_CylindricalSurface)::DownCast(moving_surface_ptr) ;
    moving_axis = moving_shape_ptr->Cylinder().Position();
    Cylinder_or_Cone = Standard_True;
  } else if (moving_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_ConicalSurface)) {
    Handle(Geom_ConicalSurface) moving_shape_ptr =
      Handle(Geom_ConicalSurface)::DownCast(moving_surface_ptr) ;
    moving_axis = moving_shape_ptr->Cone().Position();
    Cylinder_or_Cone = Standard_True;
  } else if (moving_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_ToroidalSurface)) {
    Handle(Geom_ToroidalSurface) moving_shape_ptr =
      Handle(Geom_ToroidalSurface)::DownCast(moving_surface_ptr) ;
    moving_axis = moving_shape_ptr->Torus().Position();
    Cylinder_or_Cone = Standard_True;
  }

// fixed_shape cylindre ou cone ?

  if (fixed_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_CylindricalSurface) 
                                       &&
                                          Cylinder_or_Cone) {
    Handle(Geom_CylindricalSurface) fixed_shape_ptr =
      Handle(Geom_CylindricalSurface)::DownCast(fixed_surface_ptr) ;
    fixed_axis = fixed_shape_ptr->Cylinder().Position();
    Cylinders_or_Cones = Standard_True;
  } else if (fixed_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_ConicalSurface)
	     &&
	     Cylinder_or_Cone) {
    Handle(Geom_ConicalSurface) fixed_shape_ptr =
      Handle(Geom_ConicalSurface)::DownCast(fixed_surface_ptr) ;
    fixed_axis = fixed_shape_ptr->Cone().Position();
    Cylinders_or_Cones = Standard_True;
  } else  if (fixed_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_ToroidalSurface)
	      &&
	      Cylinder_or_Cone) {
    Handle(Geom_ToroidalSurface) fixed_shape_ptr =
      Handle(Geom_ToroidalSurface)::DownCast(fixed_surface_ptr) ;
    fixed_axis = fixed_shape_ptr->Torus().Position();
    Cylinders_or_Cones = Standard_True;
  }
  
// moving & fixed shape cylindre ou cone !! placement.

  if (Cylinders_or_Cones) {


//
//   transform the axis by the Location
//     
    moving_axis.Transform(moving_location.Transformation()) ;
    fixed_axis.Transform(fixed_location.Transformation()) ;
    myFixedPoint   = fixed_axis.Location()  ;
    myMovingPoint  = moving_axis.Location() ;
    myFixedDirection  = fixed_axis.Direction() ;
    myMovingDirection = moving_axis.Direction() ;
    myDistance = 0.0e0 ;
    myAngle    = 0.0e0 ;
    myKeyWord      = ShapePlacement_AXIS_AXIS_PLACEMENT ;
    switch (aKey) {
    case ShapePlacement_ALIGN:
    case ShapePlacement_IN_ALIGN:
      if (IsaValue) {
	myDistance = aValue ;
      }
      break ;
    case ShapePlacement_OUT_ALIGN:
      for (ii = 1 ; ii <= 3 ; ii++) {
	myFixedDirection.SetCoord(ii, -myFixedDirection.Coord(ii)) ;
      }
      if (IsaValue) {
	myDistance = aValue ;
      }
      break ;
    case ShapePlacement_PARALLELE:
    case ShapePlacement_IN_PARALLELE:
    case ShapePlacement_OUT_PARALLELE:
      if (IsaValue) {
	myDistance = aValue ;
      }
      if (aKey == ShapePlacement_OUT_PARALLELE) {
	for (ii = 1 ; ii <= 3 ; ii++) {
	  myFixedDirection.SetCoord(ii, -myFixedDirection.Coord(ii)) ;
	}
      }
      break ;
      
    case ShapePlacement_ANGLE:
    case ShapePlacement_IN_ANGLE:
    case ShapePlacement_OUT_ANGLE:
      if (IsaValue) {
	myAngle = aValue  ;
      }
      myMovingNormal = myMovingDirection;
      myFixedNormal = myFixedDirection;
      myKeyWord      = ShapePlacement_A_ANGULAR_PLACEMENT ; 
      break;
      default :
      Standard_ConstructionError::Raise() ;
      break ; 
    }
  }
  
  else {
    Standard_ConstructionError::Raise() ;
  }
}



//=======================================================================
//function : InitAxisFace 
//purpose  : 
//=======================================================================


void ShapePlacement_Equation::InitAxisFace(
		      const TopoDS_Face&                    aFace, 
                      const TopoDS_Face&                   aFixShape, 
		      const ShapePlacement_TypeOfConstraint aKey, 
		      const Standard_Real aValue, 
		      const Standard_Boolean IsaValue)
{
  Standard_Integer ii ;
  TopLoc_Location moving_location,
  fixed_location ;
  Standard_Boolean moving_ok = Standard_False;
  gp_Dir Dummy(0.0e0, 0.0e0, 1.0e0) ;  
  myFixedDirection  = Dummy ;
  myMovingDirection = Dummy ;
  myFixedNormal  = Dummy ;
  myMovingNormal = Dummy ;
  Handle(Geom_Surface) moving_surface_ptr = 
    BRep_Tool::Surface(aFace,
		       moving_location) ;
  Handle(Geom_Surface) fixed_surface_ptr =
     BRep_Tool::Surface(aFixShape,
			fixed_location) ;
  if (moving_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface)) {
    Handle(Geom_RectangularTrimmedSurface) trimmed_moving_ptr =
      Handle(Geom_RectangularTrimmedSurface)::DownCast(moving_surface_ptr); 
    moving_surface_ptr =
      trimmed_moving_ptr->BasisSurface() ;
  }
  if (fixed_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface))   {
    Handle(Geom_RectangularTrimmedSurface) trimmed_fixed_ptr =
      Handle(Geom_RectangularTrimmedSurface)::DownCast(fixed_surface_ptr);
    fixed_surface_ptr = 
      trimmed_fixed_ptr->BasisSurface() ;
  }


  gp_Ax3 moving_axis,fixed_axis;

  if (moving_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_CylindricalSurface)) {
    Handle(Geom_CylindricalSurface) moving_shape_ptr =
      Handle(Geom_CylindricalSurface)::DownCast(moving_surface_ptr) ;
    moving_axis = moving_shape_ptr->Cylinder().Position();
    moving_ok = Standard_True;
  } else if (moving_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_ConicalSurface)) {
    Handle(Geom_ConicalSurface) moving_shape_ptr =
      Handle(Geom_ConicalSurface)::DownCast(moving_surface_ptr) ;
    moving_axis = moving_shape_ptr->Cone().Position();
    moving_ok = Standard_True;
  }  else if (moving_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_ToroidalSurface)) {
    Handle(Geom_ToroidalSurface) moving_shape_ptr =
      Handle(Geom_ToroidalSurface)::DownCast(moving_surface_ptr) ;
    moving_axis = moving_shape_ptr->Torus().Position();
    moving_ok = Standard_True;
  }
  
  if (moving_ok && fixed_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_Plane)) {
    Handle(Geom_Plane) fixed_plane_ptr =
      Handle(Geom_Plane)::DownCast(fixed_surface_ptr) ;
    fixed_axis = fixed_plane_ptr->Pln().Position() ;
    

    //
    //   transform the axis by the Location
    //     


    moving_axis.Transform(moving_location.Transformation()) ;
    fixed_axis.Transform(fixed_location.Transformation()) ;
    myFixedPoint   = fixed_axis.Location()  ;
    myMovingPoint  = moving_axis.Location() ;
    myFixedNormal  = fixed_axis.Direction() ;
    myMovingDirection = moving_axis.Direction() ;
    myDistance = 0.0e0 ;
    myAngle    = 0.0e0 ;
    myKeyWord      = ShapePlacement_PLANE_AXIS_PLACEMENT ;
    switch (aKey) {
    case ShapePlacement_ALIGN:
    case ShapePlacement_IN_ALIGN:
      break ;
    case ShapePlacement_OUT_ALIGN:
      for (ii = 1 ; ii <= 3 ; ii++) {
	myFixedNormal.SetCoord(ii, -myFixedNormal.Coord(ii)) ;
      }
      break ;
    case ShapePlacement_PARALLELE:
      if (IsaValue) {
	myDistance = aValue ;
      }
    case ShapePlacement_IN_PARALLELE:
    case ShapePlacement_OUT_PARALLELE:
      if (IsaValue) {
	myDistance = aValue ;
      }
      if (aKey == ShapePlacement_OUT_PARALLELE) {
	for (ii = 1 ; ii <= 3 ; ii++) {
	  myFixedNormal.SetCoord(ii, -myFixedNormal.Coord(ii)) ;
	}
      }
      break ;
      
    case ShapePlacement_ANGLE:
    case ShapePlacement_IN_ANGLE:
    case ShapePlacement_OUT_ANGLE:
      if (IsaValue) {
	myAngle = aValue  ;
      }
      myMovingNormal = myMovingDirection;
      myKeyWord = ShapePlacement_A_ANGULAR_PLACEMENT ; 
      break;
      default :
      Standard_ConstructionError::Raise() ;
      break ; 
    }
  }
  
  else {
    Standard_ConstructionError::Raise() ;
  }
}

//=======================================================================
//function : InitFaceAxis 
//purpose  : 
//=======================================================================


void ShapePlacement_Equation::InitFaceAxis(
		      const TopoDS_Face&                    aFace, 
                      const TopoDS_Face&                   aFixShape, 
		      const ShapePlacement_TypeOfConstraint aKey, 
		      const Standard_Real aValue, 
		      const Standard_Boolean IsaValue)
{
  Standard_Integer ii ;
  TopLoc_Location moving_location,
  fixed_location ;
  Standard_Boolean fixed_ok = Standard_False;
  gp_Dir Dummy(0.0e0, 0.0e0, 1.0e0) ;  
  myFixedDirection  = Dummy ;
  myMovingDirection = Dummy ;
  myFixedNormal  = Dummy ;
  myMovingNormal = Dummy ;
  Handle(Geom_Surface) moving_surface_ptr = 
    BRep_Tool::Surface(aFace,
		       moving_location) ;
  Handle(Geom_Surface) fixed_surface_ptr =
     BRep_Tool::Surface(aFixShape,
			fixed_location) ;
  if (moving_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface)) {
    Handle(Geom_RectangularTrimmedSurface) trimmed_moving_ptr =
      Handle(Geom_RectangularTrimmedSurface)::DownCast(moving_surface_ptr); 
    moving_surface_ptr =
      trimmed_moving_ptr->BasisSurface() ;
  }
  if (fixed_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface))   {
    Handle(Geom_RectangularTrimmedSurface) trimmed_fixed_ptr =
      Handle(Geom_RectangularTrimmedSurface)::DownCast(fixed_surface_ptr);
    fixed_surface_ptr = 
      trimmed_fixed_ptr->BasisSurface() ;
  }


  gp_Ax3 moving_axis,fixed_axis;

  if (fixed_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_CylindricalSurface)) {
    Handle(Geom_CylindricalSurface) fixed_shape_ptr =
      Handle(Geom_CylindricalSurface)::DownCast(fixed_surface_ptr) ;
    fixed_axis = fixed_shape_ptr->Cylinder().Position();
    fixed_ok = Standard_True;
  } else  if (fixed_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_ConicalSurface)) {
    Handle(Geom_ConicalSurface) fixed_shape_ptr =
      Handle(Geom_ConicalSurface)::DownCast(fixed_surface_ptr) ;
    fixed_axis = fixed_shape_ptr->Cone().Position();
    fixed_ok = Standard_True;
  } else if (fixed_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_ToroidalSurface)) {
    Handle(Geom_ToroidalSurface) fixed_shape_ptr =
      Handle(Geom_ToroidalSurface)::DownCast(fixed_surface_ptr) ;
    fixed_axis = fixed_shape_ptr->Torus().Position();
    fixed_ok = Standard_True;
  }
  
  if (fixed_ok && moving_surface_ptr->DynamicType() == STANDARD_TYPE(Geom_Plane)) {
    Handle(Geom_Plane) moving_plane_ptr =
      Handle(Geom_Plane)::DownCast(moving_surface_ptr) ;
    moving_axis = moving_plane_ptr->Pln().Position() ;


    //
    //   transform the axis by the Location
    //     


    moving_axis.Transform(moving_location.Transformation()) ;
    fixed_axis.Transform(fixed_location.Transformation()) ;
    myFixedPoint   = fixed_axis.Location()  ;
    myMovingPoint  = moving_axis.Location() ;
    myFixedDirection  = fixed_axis.Direction() ;
    myMovingNormal = moving_axis.Direction() ;
    myDistance = 0.0e0 ;
    myAngle    = 0.0e0 ;
    myKeyWord      = ShapePlacement_PLANE_AXIS_PLACEMENT ;
    switch (aKey) {
    case ShapePlacement_ALIGN:
    case ShapePlacement_IN_ALIGN:
      break ;
    case ShapePlacement_OUT_ALIGN:
      for (ii = 1 ; ii <= 3 ; ii++) {
	myFixedNormal.SetCoord(ii, -myFixedNormal.Coord(ii)) ;
      }
      break ;
    case ShapePlacement_PARALLELE:
      if (IsaValue) {
	myDistance = aValue ;
      }
    case ShapePlacement_IN_PARALLELE:
    case ShapePlacement_OUT_PARALLELE:
      if (IsaValue) {
	myDistance = aValue ;
      }
      if (aKey == ShapePlacement_OUT_PARALLELE) {
	for (ii = 1 ; ii <= 3 ; ii++) {
	  myFixedNormal.SetCoord(ii, -myFixedNormal.Coord(ii)) ;
	}
      }
      break ;
      
    case ShapePlacement_ANGLE:
    case ShapePlacement_IN_ANGLE:
    case ShapePlacement_OUT_ANGLE:
      if (IsaValue) {
	myAngle = aValue  ;
      }
      myFixedNormal = myFixedDirection;
      myKeyWord = ShapePlacement_P_ANGULAR_PLACEMENT ; 
      break;
      default :
      Standard_ConstructionError::Raise() ;
      break ; 
    }
  }
  
  else {
    Standard_ConstructionError::Raise() ;
  }
}


gp_Pnt ShapePlacement_Equation::FixedPoint() const 
{ return myFixedPoint ; }
//=======================================================================
//function : MovingPoint
//purpose  : 
//=======================================================================

gp_Pnt ShapePlacement_Equation::MovingPoint() const 
{ return myMovingPoint ; }
//=======================================================================
//function : FixedNormal
//purpose  : 
//=======================================================================

gp_Dir ShapePlacement_Equation::FixedNormal() const
{ return myFixedNormal ; }

//=======================================================================
//function : MovingNormal
//purpose  : 
//=======================================================================

gp_Dir ShapePlacement_Equation::MovingNormal() const 
{ return myMovingNormal ; }
//=======================================================================
//function : FixedDirection
//purpose  : 
//=======================================================================

gp_Dir ShapePlacement_Equation::FixedDirection() const
{ return myFixedDirection ; } 
//=======================================================================
//function : MovingDirection
//purpose  : 
//=======================================================================

gp_Dir ShapePlacement_Equation::MovingDirection() const
{ return myMovingDirection ; } 

//=======================================================================
//function : SignedDistance
//purpose  : 
//=======================================================================

Standard_Real ShapePlacement_Equation::SignedDistance() const
{ return myDistance ; }

//=======================================================================
//function : SignedDistance
//purpose  : 
//=======================================================================

Standard_Real ShapePlacement_Equation::Angle() const
{ return myAngle ; }


//=======================================================================
//function : Type
//purpose  : 
//=======================================================================

ShapePlacement_TypeOfEquation ShapePlacement_Equation::Type() const
{ return myKeyWord ; }
//=======================================================================
//function : Print
//purpose  : 
//=======================================================================

void ShapePlacement_Equation::Print(Standard_OStream& S) const
{
  Standard_Integer ii ;
  switch (myKeyWord ) {
  case ShapePlacement_PLANE_PLACEMENT:
    S << "Type is PLANE_PLACEMENT" ;
    S << "Fixed point is : " ;
    for ( ii = 1 ; ii <= 3 ; ii++) {
      S << myFixedPoint.Coord(ii) << " " ;
    }
    S << endl ;
    S << "Fixed normal is : " ; 
    for ( ii = 1 ; ii <= 3 ; ii++) {
      S << myFixedNormal.Coord(ii) << " " ;
    }
    S << endl ; 
    S << "Moving point is : " ;
    for ( ii = 1 ; ii <= 3 ; ii++) {
      S << myMovingPoint.Coord(ii) << " " ;
    }
    S << endl ;
    S << "Moving normal is : " ; 
    for ( ii = 1 ; ii <= 3 ; ii++) {
      S << myMovingNormal.Coord(ii) << " " ;
    }
    S << endl ; 
    break ;
  case ShapePlacement_CONE_PLACEMENT:
    S << "Type is CONE_PLACEMENT" ;
    S << "Fixed point is : " ;
    for ( ii = 1 ; ii <= 3 ; ii++) {
      S << myFixedPoint.Coord(ii) << " " ;
    }
    S << endl ;
    S << "Fixed normal is : " ; 
    for ( ii = 1 ; ii <= 3 ; ii++) {
      S << myFixedNormal.Coord(ii) << " " ;
    }
    S << endl ; 
    S << "Moving point is : " ;
    for ( ii = 1 ; ii <= 3 ; ii++) {
      S << myMovingPoint.Coord(ii) << " " ;
    }
    S << endl ;
    S << "Moving normal is : " ; 
    for ( ii = 1 ; ii <= 3 ; ii++) {
      S << myMovingNormal.Coord(ii) << " " ;
    }
    S << endl ; 
    break ;
  case ShapePlacement_LINE_PLACEMENT: 
    S << "Type is LINE_PLACEMENT" ; 
    break ;
  case ShapePlacement_A_ANGULAR_PLACEMENT:
    S << "Tyoe is A_ANGULAR_PLACEMENT" ;
    break ;
  case ShapePlacement_P_ANGULAR_PLACEMENT:
    S << "Type is P_ANGULAR_PLACEMENT" ;
    break ;
    default :
    break ;
  }
}



