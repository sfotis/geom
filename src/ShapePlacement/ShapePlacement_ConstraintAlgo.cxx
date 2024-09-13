#include <gp_Trsf.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax3.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2d.hxx>
#include <gp_Ax22d.hxx>
#include <gp_Vec2d.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Lin2d.hxx>
#include <gp_Lin.hxx>
#include <gp_Cone.hxx>
#include <gp_Parab.hxx>
#include <gp_Pln.hxx>
#include <gp_Cone.hxx>
#include <gp_Parab2d.hxx>
#include <gp_Elips.hxx>
#include <gp_Elips2d.hxx>
#include <gp_Circ.hxx>
#include <gp_Circ2d.hxx>
#include <gp_Hypr.hxx>
#include <gp_Hypr2d.hxx>
#include <Precision.hxx>
#include <TColgp_Array1OfVec.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <ShapePlacement_ConstraintAlgo.ixx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools.hxx>
#include <ShapePlacement_Equation.hxx>
#include <ShapePlacement_Constraint.hxx>
#include <ShapePlacement_TypeOfConstraint.hxx>
#include <ShapePlacement_TypeOfEquation.hxx>
#include <ShapePlacement_TypeOfAxisConstraint.hxx>
#include <ShapePlacement_ListIteratorOfListOfConstraint.hxx>
#include <Standard_ConstructionError.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Vertex.hxx>
#include <BRep_Tool.hxx>
#include <IntAna_QuadQuadGeo.hxx>
#include <IntAna2d_AnaIntersection.hxx>
#include <IntAna_ResultType.hxx>
#include <IntAna2d_Conic.hxx>

typedef enum {
  NotInitialized                 =-1,
  Compatible                     = 0,
  NotCompatible                  = 1

  } EquationCompatibility ;
typedef enum {
  CanFlip               = 0,
  Reverse               = 1,
  Forward               = 2
  } FlipCondition ;
  
//=======================================================================
//function : ShapePlacement_ConstraintAlgo
//purpose  : 
//=======================================================================

ShapePlacement_ConstraintAlgo::ShapePlacement_ConstraintAlgo(
                   const TopoDS_Shape& aShape,
		   const Standard_Real AngularTolerance)
: myHasSolution(Standard_False),
  myAngularTolerance(AngularTolerance), 
  myIsOverConstrained(Standard_False)
{
  mySubShapes.Clear();
  myShapeToPosition = aShape;

}

//=======================================================================
//function : SetTolerance
//purpose  : 
//=======================================================================

void ShapePlacement_ConstraintAlgo::SetTolerance(const Standard_Real AngularTolerance) 
{
 myAngularTolerance = AngularTolerance ;
}
//=======================================================================
//function : ShapePlacement_ConstraintAlgo
//purpose  : 
//=======================================================================

ShapePlacement_ConstraintAlgo::ShapePlacement_ConstraintAlgo(
						const Standard_Real AngularTolerance )
:myHasSolution(Standard_False) ,
 myAngularTolerance(AngularTolerance)
 
{
}

//=======================================================================
//function : Load
//purpose : used only for tests with Draw
//          Set the fiel myShapeToPosition  to aShape
//=======================================================================


void ShapePlacement_ConstraintAlgo::Load(const TopoDS_Shape& aShape)
{
  mySubShapes.Clear();
  myShapeToPosition = aShape;
}





//=======================================================================
//function : HasSolution
//purpose  : 
//=======================================================================

Standard_Boolean ShapePlacement_ConstraintAlgo::HasSolution() const 
{
  return myHasSolution ;
}

//=======================================================================
//function : GetTrsf
//purpose  : 
//=======================================================================

gp_Trsf ShapePlacement_ConstraintAlgo::GetTrsf() const 
{
  return myTrsf ;
}
//=======================================================================
//function : CheckCompatibility
//purpose  : 
//=======================================================================

void CheckCompatibility(const TColgp_Array1OfVec&  FixedArray,
			const TColgp_Array1OfVec&  MovingArray,
			TColStd_Array1OfInteger&   FlipArray,
			const Standard_Integer     NumVectors,
			const Standard_Real        AngularTolerance,
			gp_Vec *                   LinearIdpndtMovingArray,
			Standard_Integer&          Dimension,
			EquationCompatibility&     Result)


// this will make the dot product FixedArray(ii).FixedArray(NumVectors)
// for ii = 1 to NumVectors -1 and compare those with 
// MovingArray(ii).MovingArray(NumVectors) 
// if FlipArray(ii) is Reverse  a the previous quantity will be 
// set to its opposite
// if all agree up to tolerance the vector will be named compatible
// otherwise it will be marked not compatible. It will also set the 
// FlipArray(NumVector) value if the vector is called compatible
//
{

 gp_Vec a_vector ;
 Standard_Integer  ii,
 not_done;

 Standard_Real tolerance_squared,
 dot_product[2],
 abs_dot_product[2],
 sign,
 a_sine_squared ;

 not_done = 1 ;
 Result = NotInitialized ;
 tolerance_squared = AngularTolerance * AngularTolerance ;
 if (NumVectors == 2) {
   a_sine_squared =
     MovingArray.Value(2).CrossSquareMagnitude(MovingArray.Value(1)) ;
   if (a_sine_squared <= tolerance_squared) {
     a_sine_squared =
     FixedArray.Value(2).CrossSquareMagnitude(FixedArray.Value(1)) ;
     if (a_sine_squared <= tolerance_squared) { 
        dot_product[0] = FixedArray.Value(2).Dot(FixedArray.Value(1)) ;
        dot_product[1] = MovingArray.Value(2).Dot(MovingArray.Value(1)) ;
	sign = 1.0e0 ;
        if (FlipArray(1) == Reverse) {
	  sign = -1.0e0;
        }
	if (sign * dot_product[0] * dot_product[1] > 0.0e0) {
	  Result = Compatible ;
        }
        else  if (FlipArray(2) == CanFlip) {
	  FlipArray(2) = Reverse ;
	  Result = Compatible ;
	}
	else {
	      Result = NotCompatible ;
         }
     }
     else {
       not_done = 0 ;
     }
   }
 }
 if (not_done) {
   Result = Compatible ;
   for (ii = 1 ; Result != NotCompatible && ii < NumVectors ; ii++) {
     dot_product[0] =
       (MovingArray.Value(ii)).Dot(MovingArray.Value(NumVectors)) ;
     abs_dot_product[0] = Abs(dot_product[0]) ;
     dot_product[1] =
       (FixedArray.Value(ii)).Dot(FixedArray.Value(NumVectors)) ; 
     abs_dot_product[1] = Abs(dot_product[1]) ;
     sign = 1.0e0;
     switch (FlipArray.Value(ii)) {
     case Forward: 
       switch (FlipArray(NumVectors)) {
       case Reverse:
	 sign = -1.0e0;
       case Forward:
         if (Abs(dot_product[0] - sign * dot_product[1]) 
	     >  AngularTolerance) {
	   Result = NotCompatible ;
	 }
	 break ;
       case CanFlip:
	 //
	 //  the size of the dot products must be meaningfull before
	 //  we decide something 
	 //
         if (abs_dot_product[0] > AngularTolerance || 
	     abs_dot_product[1] > AngularTolerance) {
	   if (Abs(abs_dot_product[0] -abs_dot_product[1]) < AngularTolerance) {
	     if (dot_product[0] * dot_product[1] > 0.0e0) {
	       FlipArray(NumVectors) = Forward ;
	     }
	     else {
	       FlipArray(NumVectors) = Reverse ;
	     }
	   }
	   else {
	     Result = NotCompatible ;
	   }
         }
	 break ;
       default:
	 break ;
       }
       break ;
     case Reverse: 
       switch (FlipArray(NumVectors)) {
       case Forward:
	 sign = -1.0e0;
       case Reverse:
         if (Abs(dot_product[0] - sign * dot_product[1]) 
	     >  AngularTolerance) {
	   Result = NotCompatible ;
	 }
	 break ;
       case CanFlip:
	 if (Abs(abs_dot_product[0] -abs_dot_product[1]) < AngularTolerance) {
	   if (dot_product[0] * dot_product[1] > 0.0e0) {
	     FlipArray(NumVectors) = Reverse ;
           }
	   else {
	     FlipArray(NumVectors) = Forward ;
           }
         }
	 else {
	   Result = NotCompatible ;
	 }
	 break ;
       default:
	 break ;
       }
       break ;
     case CanFlip: 
       switch (FlipArray(NumVectors)) {
       case Forward:
	 if (Abs(abs_dot_product[0] -abs_dot_product[1]) < AngularTolerance) {
	   if (dot_product[0] * dot_product[1] > 0.0e0) {
	     FlipArray(ii) = Forward ;
           }
	   else {
	     FlipArray(ii) = Reverse ;
           }
         }
	 else {
	   Result = NotCompatible ;
	 }
	 break ;
	
       case Reverse:
	 if (Abs(abs_dot_product[0] -abs_dot_product[1]) < AngularTolerance) {
	   if (dot_product[0] * dot_product[1] > 0.0e0) {
	     FlipArray(ii) = Reverse ;
           }
	   else {
	     FlipArray(ii) = Forward ;
           }
         }
	 else {
	   Result = NotCompatible ;
	 }
	 break ;

       case CanFlip:
	 if (Abs(abs_dot_product[0] -abs_dot_product[1]) < AngularTolerance) {
	   if (dot_product[0] * dot_product[1] > 0.0e0) {
	     FlipArray(ii) = Forward ;
	     FlipArray(NumVectors) = Forward ;
           }
	   else {
	     FlipArray(ii) = Forward ;
	     FlipArray(NumVectors) = Reverse ;
           }
         }
	 else {
	   Result = NotCompatible ;
	 }
	 break ;
       default:
	 break ;
       }
       break ;
	 
     }
   }
	 
   if (Result == Compatible) {
// 
// worth checking linearly indenpendance from the NumVectors-1 
// vectors in    MovingArray
// 
     switch (Dimension) {
     case 1:
       a_sine_squared =
	 LinearIdpndtMovingArray[0].
	   CrossSquareMagnitude(MovingArray.Value(NumVectors)) ; 
       if (a_sine_squared > tolerance_squared) {
	 Dimension = 2 ;
	 LinearIdpndtMovingArray[1] = MovingArray.Value(NumVectors) ;
	 LinearIdpndtMovingArray[1].Normalize() ;
       }
       break ;
     case 2:
       a_vector =
	 LinearIdpndtMovingArray[0].Crossed(LinearIdpndtMovingArray[1]) ;
       a_vector.Normalize() ;
       dot_product[0] = a_vector.Dot(MovingArray.Value(NumVectors)) ;
       if (Abs(dot_product[0]) > AngularTolerance) {
	 Dimension = 3 ;
	 LinearIdpndtMovingArray[2] = MovingArray.Value(NumVectors) ;
	 LinearIdpndtMovingArray[2].Normalize() ;
       }
       break ;
     case 3:
       break ;
     }
     
   }
 }
}
 
//=======================================================================
//function : CheckWhichCase
//purpose  : 
// a le bon gout de remplir myEquationStatus pour dire quelles sont
// les equations qui vont etre utilisees
//=======================================================================

Standard_Integer  ShapePlacement_ConstraintAlgo::CheckWhichCase() 
{ 
  Standard_Integer equation_index,
  ii,
  jj,
  Result,
  dimension,
  save_dimension,
  index = 1 ;
  gp_Vec linearly_independant_vectors[3] ;
  dimension = 0 ;  
  Result = 0 ;
  TColgp_Array1OfVec       fixed_vector_array(1,myConstraints.Extent()) ;
  TColgp_Array1OfVec       moving_vector_array(1,myConstraints.Extent()) ;
  TColStd_Array1OfInteger  flip_vector_array(1,myConstraints.Extent()) ;
  ShapePlacement_ListOfConstraint   selected_constraints ;
  
  myEquationStatus = 
    new TColStd_HArray1OfInteger(1,myConstraints.Extent()) ;
  myEquationOrientation = 
    new TColStd_HArray1OfInteger(1,myConstraints.Extent()) ;
  equation_index = 1 ;
  for (ii = 1 ; ii <= myConstraints.Extent() ; ii++) {
    myEquationStatus->ChangeArray1()(ii) = NotCompatible ;
    myEquationOrientation ->ChangeArray1()(ii) = Forward ; 
    flip_vector_array(ii) = Forward ;
  }
  ShapePlacement_ListIteratorOfListOfConstraint   anIterator ;
  //
  // do first the PLANE_PLACEMENT because of flipping of axis constraints
  //
  for (anIterator.Initialize(myConstraints) ; 
       anIterator.More() ; anIterator.Next()){
    ShapePlacement_Constraint & a_constraint = 
      anIterator.Value() ;
    const  ShapePlacement_Equation & an_equation =
      a_constraint.Equation() ;
    if (an_equation.Type() == ShapePlacement_PLANE_PLACEMENT || 
	an_equation.Type() == ShapePlacement_CONE_PLACEMENT )  {
      
      fixed_vector_array(index) = 
	an_equation.FixedNormal()  ;
      moving_vector_array(index) =
	an_equation.MovingNormal() ;
      if (index >= 2) {
	EquationCompatibility  compatibility ; 
	save_dimension = dimension ;
	CheckCompatibility(fixed_vector_array,
			   moving_vector_array,
			   flip_vector_array,
			   index,
			   myAngularTolerance,
			   linearly_independant_vectors,
			   dimension,
			   compatibility) ;
	
	switch (compatibility) {
	  
	case  Compatible:
	  
	  myEquationStatus->ChangeArray1()(equation_index) = compatibility ;
	  for (jj = 1 ; jj <= equation_index ; jj++) {
	    if (flip_vector_array(jj) == Reverse) {
	      myEquationOrientation->ChangeArray1()(equation_index) = Reverse ;
	    }
	  }
	  //
	  // the vectors are compatible but are colinear so do not add them to 
	  // to the definition
	  
	  if (save_dimension == dimension &&
	      an_equation.Type() == ShapePlacement_AXIS_AXIS_PLACEMENT) {
	    
	    //
	    // check if we are having at least two cylinders with
	    // which are not aligned 
	    //

	    if (! AxisAlignedWithPrevious(selected_constraints,
					  a_constraint) ) {
	      
	      selected_constraints.Append(a_constraint);		 
	      index += 1 ;
	    }
	    
	  }
	  else { 


	  selected_constraints.Append(a_constraint);	
	  index += 1 ;

	   }
	  break ;
	case NotInitialized:
	case NotCompatible:
	  //
	  //  the vector are not compatible
	  break ;
	}
      }
      else {
	//
	// first usefull constraint
	//
	myEquationStatus->ChangeArray1()(equation_index) = Compatible ;
	dimension = 1 ;
	linearly_independant_vectors[0] = moving_vector_array(index) ;
	selected_constraints.Append(a_constraint);	
	index += 1 ;
	
      }
   
    }
    equation_index += 1 ;  
  }
  //
  //  do the AXIS_AXIS_PLACEMENT doing the relevant flips
  //
  equation_index = 1 ;
  for (anIterator.Initialize(myConstraints) ; 
       anIterator.More() ; anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
      anIterator.Value() ;
    const  ShapePlacement_Equation & an_equation =
      a_constraint.Equation() ;
    if (an_equation.Type() == ShapePlacement_AXIS_AXIS_PLACEMENT) {
      flip_vector_array(index) = CanFlip ;
      fixed_vector_array(index) = 
	an_equation.FixedDirection()  ;
      moving_vector_array(index) =
	an_equation.MovingDirection() ;
      if (index >= 2) {
	EquationCompatibility  compatibility ; 
	save_dimension = dimension ;
	CheckCompatibility(fixed_vector_array,
			   moving_vector_array,
			   flip_vector_array,
			   index,
			   myAngularTolerance,
			   linearly_independant_vectors,
			   dimension,
			   compatibility) ;
	
	switch (compatibility) {
	  
	case  Compatible:
	  
	  myEquationStatus->ChangeArray1()(equation_index) = compatibility ;
	  if (flip_vector_array(index) == Reverse) {
	      myEquationOrientation->ChangeArray1()(equation_index) = Reverse ;
	    }
	  
	  //
	  // the vectors are compatible but are colinear so do not add them to 
	  // to the definition
	  //
	  if (save_dimension == dimension &&
	      an_equation.Type() == ShapePlacement_AXIS_AXIS_PLACEMENT) {
	    //
	    // check if we are having at least two cylinders with
	    // which are not aligned 
	    //
	    if (! AxisAlignedWithPrevious(selected_constraints,
					  a_constraint) ) {
	      
	      selected_constraints.Append(a_constraint);		 
	      index += 1 ;
	    }
	    
	  }
	  else {
	    selected_constraints.Append(a_constraint);	
	    index += 1 ;
	  }
	  break ;
	case NotInitialized:
	case NotCompatible:
	  //
	  //  the vector are not compatible
	  break ;
	}
      }
      else {
	//
	// first usefull constraint
	//
	myEquationStatus->ChangeArray1()(equation_index) = Compatible ;
	dimension = 1 ;
	linearly_independant_vectors[0] = moving_vector_array(index) ;
	selected_constraints.Append(a_constraint);	
	index += 1 ;
	
      }
     
    }
    equation_index += 1 ;  
  }




  //
  //  do the ANGULAR_PLACEMENT doing the relevant flips
  //
  equation_index = 1 ;
  for (anIterator.Initialize(myConstraints) ; 
       anIterator.More() ; anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
      anIterator.Value() ;
    const  ShapePlacement_Equation & an_equation =
      a_constraint.Equation() ;
    if (an_equation.Type() == ShapePlacement_P_ANGULAR_PLACEMENT) {
      flip_vector_array(index) = Forward ;
      fixed_vector_array(index) = 
	an_equation.FixedNormal()  ;
      moving_vector_array(index) =
	an_equation.MovingNormal() ;
      if (index >= 2) {
	EquationCompatibility  compatibility ; 
	save_dimension = dimension ;
	CheckCompatibility(fixed_vector_array,
			   moving_vector_array,
			   flip_vector_array,
			   index,
			   myAngularTolerance,
			   linearly_independant_vectors,
			   dimension,
			   compatibility) ;
	
	switch (compatibility) {
	  
	case  Compatible:
	  
	  myEquationStatus->ChangeArray1()(equation_index) = compatibility ;
	  if (flip_vector_array(index) == Reverse) {
	    myEquationOrientation->ChangeArray1()(equation_index) = Reverse ;
	  }
	  selected_constraints.Append(a_constraint);	
	  index += 1 ;
	
	  break ;
	case NotInitialized:
	case NotCompatible:
	  //
	  //  the vector are not compatible
	  break ;
	}
      }
      else {
	//
	// first usefull constraint
	//
	myEquationStatus->ChangeArray1()(equation_index) = Compatible ;
	dimension = 1 ;
	linearly_independant_vectors[0] = moving_vector_array(index) ;
	selected_constraints.Append(a_constraint);	
	index += 1 ;
	
      }
      
    }
    equation_index += 1 ;  
  }

  equation_index = 1 ;
  for (anIterator.Initialize(myConstraints) ; 
       anIterator.More() ; anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
      anIterator.Value() ;
    const  ShapePlacement_Equation & an_equation =
      a_constraint.Equation() ;
    if (an_equation.Type() == ShapePlacement_A_ANGULAR_PLACEMENT) {
      flip_vector_array(index) = CanFlip ;
      fixed_vector_array(index) = 
	an_equation.FixedNormal()  ;
      moving_vector_array(index) =
	an_equation.MovingNormal() ;
      if (index >= 2) {
	EquationCompatibility  compatibility ; 
	save_dimension = dimension ;
	CheckCompatibility(fixed_vector_array,
			   moving_vector_array,
			   flip_vector_array,
			   index,
			   myAngularTolerance,
			   linearly_independant_vectors,
			   dimension,
			   compatibility) ;
	
	switch (compatibility) {
	  
	case  Compatible:
	  
	  myEquationStatus->ChangeArray1()(equation_index) = compatibility ;
	  if (flip_vector_array(index) == Reverse) {
	    myEquationOrientation->ChangeArray1()(equation_index) = Reverse ;
	  }
	  selected_constraints.Append(a_constraint);	
	  index += 1 ;
	
	  break ;
	case NotInitialized:
	case NotCompatible:
	  //
	  //  the vector are not compatible
	  break ;
	}
      }
      else {
	//
	// first usefull constraint
	//
	myEquationStatus->ChangeArray1()(equation_index) = Compatible ;
	dimension = 1 ;
	linearly_independant_vectors[0] = moving_vector_array(index) ;
	selected_constraints.Append(a_constraint);	
	index += 1 ;
	
      }
      
    }
    equation_index += 1 ;  
  }

  //
  // PLANE_AXIS_PLACEMENT Constraints 
  // There is no compatibility's condition on this type of contraints 
  //

  equation_index = 1 ;
  for (anIterator.Initialize(myConstraints) ; 
       anIterator.More() ; anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
      anIterator.Value() ;
    const  ShapePlacement_Equation & an_equation =
      a_constraint.Equation() ;
    if (an_equation.Type() == ShapePlacement_PLANE_AXIS_PLACEMENT) {
      myEquationStatus->ChangeArray1()(equation_index) = Compatible ;
      selected_constraints.Append(a_constraint);	
      index += 1 ;
    }
    equation_index += 1 ;
  }
  ComputeWhichCase(Result) ;
  return Result ;
}


//=======================================================================
//function : AxisAlignedWithPrevious
//purpose  : 
//=======================================================================

Standard_Boolean  ShapePlacement_ConstraintAlgo::
   AxisAlignedWithPrevious(const ShapePlacement_ListOfConstraint&   ListOfReference,
			   const ShapePlacement_Constraint&         AConstraint) 
//
//   the ReferenceList tells 
//   which constrain from the  where picked from
//   the List of Constraints in myConstraints
//   AConstraint is the Constraints which is an AXIS AXIS 
//   Placement constraint  in the List of Constraints 
//   that has to be checked against all previous constraints to see
//   if it is aligned with an other AXIS AXIS placement constraints
//  
{
  Standard_Boolean  Result ;
  Standard_Integer ii ;
  Standard_Real a_sine_squared,
  tolerance_squared ;
  Result = Standard_False ;
  gp_Vec test_vector,
  ref_vector ;

  tolerance_squared = myAngularTolerance * myAngularTolerance ;
  const  ShapePlacement_Equation & ref_equation =
    AConstraint.Equation() ;
  if (ref_equation.Type() == ShapePlacement_AXIS_AXIS_PLACEMENT) {
    ref_vector = ref_equation.MovingDirection() ;
    ShapePlacement_ListIteratorOfListOfConstraint   anIterator ;
    for (anIterator.Initialize(ListOfReference) ; 
	 anIterator.More() && ! Result  ; anIterator.Next()){
      ShapePlacement_Constraint & a_constraint = 
	anIterator.Value() ; 
      const  ShapePlacement_Equation & an_equation =
	a_constraint.Equation() ;
      
      if (an_equation.Type() == ShapePlacement_AXIS_AXIS_PLACEMENT) {
	test_vector = an_equation.MovingDirection() ;
	a_sine_squared =
	test_vector.CrossSquareMagnitude(ref_vector) ;
	if (a_sine_squared <= tolerance_squared) {
	  for (ii = 1 ; ii <= 3 ; ii++) {
	    test_vector.SetCoord(ii,
				 an_equation.MovingPoint().Coord(ii)
				 - ref_equation.MovingPoint().Coord(ii)) ;
	  }
	  if (test_vector.SquareMagnitude() >= tolerance_squared) {
	    test_vector.Normalize() ;
	    a_sine_squared =
	      test_vector.CrossSquareMagnitude(ref_vector) ;
	    if (a_sine_squared <= tolerance_squared) {
	      Result = Standard_True ;
	    }
          } 
	  else {
	    Result = Standard_True ;
          }
	}
      }
    }
  }
	  
 return Result ;
}

							

	                   
//=======================================================================
//function : ComputeWhichCase
//purpose  : 
//=======================================================================

void ShapePlacement_ConstraintAlgo::ComputeWhichCase(
                     Standard_Integer&  Result)  const 
{
  Standard_Integer equation_index = 1,
  axis_placement,
  not_done,
  plane_placement,
  plane_axis_placement,
  angular_placement;
  not_done = 1 ; 
  axis_placement =
  plane_placement = 
  plane_axis_placement =
  angular_placement = 0 ;            
  ShapePlacement_ListIteratorOfListOfConstraint   anIterator ;
  for (anIterator.Initialize(myConstraints) ; 
     anIterator.More() && not_done  ; anIterator.Next()){
    ShapePlacement_Constraint & a_constraint = 
      anIterator.Value() ;
    const  ShapePlacement_Equation & an_equation =
      a_constraint.Equation() ;
    if (myEquationStatus->Array1()(equation_index) != NotCompatible) {
      if (an_equation.Type() == ShapePlacement_PLANE_PLACEMENT ||
	  an_equation.Type() == ShapePlacement_CONE_PLACEMENT )  {
	plane_placement += 1 ;
      } 
      else if (an_equation.Type() == ShapePlacement_AXIS_AXIS_PLACEMENT) {
	axis_placement += 1 ;
      }
      else if (an_equation.Type() == ShapePlacement_PLANE_AXIS_PLACEMENT) {
	plane_axis_placement += 1;
      }
      else if (an_equation.Type() == ShapePlacement_A_ANGULAR_PLACEMENT 
	       || an_equation.Type() == ShapePlacement_P_ANGULAR_PLACEMENT) {
	angular_placement += 1;
      }
    }
    else if (an_equation.Type() == ShapePlacement_PLANE_AXIS_PLACEMENT) {
      plane_axis_placement += 1;
    }
    else if (an_equation.Type() == ShapePlacement_A_ANGULAR_PLACEMENT 
	     || an_equation.Type() == ShapePlacement_P_ANGULAR_PLACEMENT) {
      angular_placement += 1;
    }
   

    if (plane_placement == 1 && 
	axis_placement == 0 && 
	plane_axis_placement == 0 &&
	angular_placement == 0) {
      Result = 1 ;
    }
    else if (plane_placement == 2 && axis_placement == 0) {
      Result = 1 ;
      not_done = 0 ;
    }
    else if (plane_placement == 2 && axis_placement == 1) {
      Result = 1;
      not_done = 0;
    }
    else if (axis_placement == 1 
	     && plane_placement == 0 
	     && plane_axis_placement == 0
	     && angular_placement == 0) {
      Result = 2 ;
    }
    else if (axis_placement == 1 && plane_placement == 1) {
      Result = 2 ;
    }
    else if (axis_placement == 2 && plane_placement == 1) {
      Result = 4 ;
      not_done = 0 ;
    }
    else if (axis_placement == 2 && plane_placement == 0) {
      Result = 3 ;
    }
    else if (axis_placement == 2 && plane_axis_placement == 1) {
      Result = 3;
      not_done = 0 ;
    }
    else if (plane_axis_placement == 1 && plane_placement == 1) {
      Result = 8 ;
    }
    else if (axis_placement == 1 && plane_axis_placement == 1) {
      Result = 8 ;
    }
    else if (plane_axis_placement == 1 && plane_placement == 2) {
      Result = 1 ;
      not_done = 0;
    }
    else if (plane_axis_placement >= 1 && angular_placement == 0) {
      Result = 7 ;
    }
    else if (plane_placement == 1 && angular_placement == 1) {
      Result = 10;
    }
    else if (axis_placement == 1 && angular_placement == 1) {
      Result = 10;
    }
    else if (angular_placement == 1 && plane_axis_placement == 1) {
      Result = 11;
    }
    else if (angular_placement>= 1) {
      Result = 9 ;
    }
    
    equation_index += 1 ;
  }
}


//=======================================================================
//function : Solve
//purpose  : 
//=======================================================================

void ShapePlacement_ConstraintAlgo::Solve() 
{
  myIsOverConstrained = Standard_False; 
  switch (CheckWhichCase()) {
  case 0: 
    Standard_ConstructionError::Raise() ;
    break ;
  case 1:
    SolveCase1() ;
    break ;
  case 2:
    SolveCase2() ;
    break ;
  case 3:
    SolveCase3() ;
    break ;
  case 4:
    SolveCase4() ;
    break ;
  case 5:
    SolveCase5() ;
    break ;
  case 6:
    SolveCase6() ;
    break ;
  case 7:
    SolveCase7() ;
    break;
  case 8:
    SolveCase8() ;
    break ;
  case 9:
    SolveCase9() ;
    break ;
  case 10:
    SolveCase10() ;
    break ;
  case 11:
    SolveCase11() ;
    break ;
  }
}

//=======================================================================
//function : IsoBarycentre
//purpose : compute the isobarycentre of all vertexes of a shape
//=======================================================================

gp_Pnt ShapePlacement_ConstraintAlgo::IsoBarycentre(const TopoDS_Shape& aShape) const {
  
  Standard_Integer ii = 0,jj;
  gp_Pnt barycentre(0.0e0,0.0e0,0.0e0);
  TopExp_Explorer Explore(aShape, TopAbs_VERTEX);

  while (Explore.More()) {
    TopoDS_Vertex vertex = TopoDS::Vertex(Explore.Current());

    gp_Pnt pnt = BRep_Tool::Pnt(vertex);

    for (jj = 1 ; jj <=3; jj++)
      barycentre.SetCoord(jj, barycentre.Coord(jj) + pnt.Coord(jj));
    ii ++;
    Explore.Next();
  }
  for (jj = 1 ; jj <=3; jj++)
    barycentre.SetCoord(jj, barycentre.Coord(jj) / ii);
  return barycentre;
}


    
 


//=======================================================================
//function : Solve
//purpose  : there are only PLAN_PLACEMENT equation
//=======================================================================

void ShapePlacement_ConstraintAlgo::SolveCase1() 
{
  Standard_Integer 
    ii,
    jj,
    kk,
    store_this_one,
    num_equations,
    index ;
  gp_Vec   matrix_vector[2][3],
  vector[2][3],
  a_vector ;
  gp_Pnt   origin[2][3] ;
  Standard_Real distance[3],
  tolerance_squared,
  a_sine_squared,
  value  ;
//
//    seul cas traiter pour l instant
//    search for linearly independant conditions
//
  tolerance_squared = myAngularTolerance * myAngularTolerance ;
  for (ii = 0 ; ii < 2 ; ii++) {
    for (jj = 0 ; jj < 3 ; jj++) {
      for (kk = 1 ; kk <= 3 ; kk++) {
	origin[ii][jj].SetCoord(kk, 1.0e0) ;
	vector[ii][jj].SetCoord(kk,1.0e0) ;
      }
    }
  }
  for (ii = 0 ; ii < 3 ; ii++) {
    distance[ii] = 0.0e0 ;
  }
  num_equations =
    myEquationStatus->Length() ;
  index = 0 ;
  ShapePlacement_ListIteratorOfListOfConstraint   anIterator ;
  store_this_one = 1 ;

  for (ii = 1,
       anIterator.Initialize(myConstraints); 
       index < 3 && ii <= num_equations ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
     anIterator.Value() ;
    const ShapePlacement_Equation & an_equation =
     a_constraint.Equation() ;
    if (myEquationStatus->Value(ii) == Compatible 
	&& 
	(an_equation.Type() == ShapePlacement_PLANE_PLACEMENT || an_equation.Type() == ShapePlacement_CONE_PLACEMENT)) {
      if (index == 1) {
	a_sine_squared =
	  matrix_vector[1][0].CrossSquareMagnitude(an_equation.MovingNormal()) ;
	store_this_one = (a_sine_squared > tolerance_squared) ;
      } 
      else if (index == 2) {
	a_vector = matrix_vector[1][0].Crossed(matrix_vector[1][1]) ;
	a_vector.Normalize() ;
	store_this_one = (Abs(a_vector.Dot(an_equation.MovingNormal()))
			  > myAngularTolerance) ;
      }  
      if (store_this_one) {
//
//  pick up the linearly independant 	
//
	matrix_vector[0][index] = an_equation.FixedNormal()  ;
	matrix_vector[1][index] = an_equation.MovingNormal() ;
	origin[0][index] = an_equation.FixedPoint()  ;
	origin[1][index] = an_equation.MovingPoint() ;
	distance[index] = an_equation.SignedDistance() ;
	index += 1 ;
      }
    }
  }

  //
  // build 2 Ax3 to define the Trsf 
  // the vector defining the Ax3 are 
  //   for the fixed Ax3             vector[0][0], 
  //                                 vector[0][1], 
  //                                 vector[0][2] 
  //
  //   for the moving Ax3            vector[1][0], 
  //                                 vector[1][1], 
  //                                 vector[1][2] 
  //
  

  // Isobarycentre of all vertexes of  moving_shape

  gp_Pnt a_point = IsoBarycentre(myShapeToPosition);



  if (index >= 2) {
    a_sine_squared = 
    matrix_vector[1][0].CrossSquareMagnitude(matrix_vector[1][1]) ;
  }
  else { 
    a_sine_squared = 1.0e0 ;
  }
  if (index >= 2 && a_sine_squared > tolerance_squared) {
    for (ii = 0 ; ii < 2 ; ii++) {
      vector[ii][0] = matrix_vector[ii][0] ;
      vector[ii][2] = matrix_vector[ii][0].Crossed(matrix_vector[ii][1]) ;
      vector[ii][1] = vector[ii][2].Crossed(matrix_vector[ii][0]) ;
    }
  }
  else {
    
//    gp_Pnt temporary_point(0.0e0,
//			   0.0e0,
//			   0.0e0) ;
//    gp_Dir temporary_dir0(matrix_vector[0][0]) ;
//    gp_Dir temporary_dir1(matrix_vector[1][0]) ;
//    gp_Ax3 temporary_axis0(temporary_point,
//			   temporary_dir0) ;
//    gp_Ax3 temporary_axis1(temporary_point,
//			   temporary_dir1) ;
//
    gp_Dir temporary_dir0(matrix_vector[0][0]) ;
    gp_Dir temporary_dir1(matrix_vector[1][0]) ;
    gp_Ax3 temporary_axis0(a_point,
			   temporary_dir0) ;
    gp_Ax3 temporary_axis1(a_point,
			   temporary_dir1) ;
//
// still the Z axis should be the first slot of the
// frame vector[0] and vector[1] 
//
    for (ii = 1 ; ii <= 3 ; ii++) {
      vector[0][0].SetCoord(ii, temporary_axis0.Direction().Coord(ii)) ;
      vector[0][1].SetCoord(ii, temporary_axis0.XDirection().Coord(ii)) ;
      vector[0][2].SetCoord(ii, temporary_axis0.YDirection().Coord(ii)) ;
      vector[1][0].SetCoord(ii, temporary_axis1.Direction().Coord(ii)) ;
      vector[1][1].SetCoord(ii, temporary_axis1.XDirection().Coord(ii)) ;
      vector[1][2].SetCoord(ii, temporary_axis1.YDirection().Coord(ii)) ;
    }
  }
//
// for the time being the Trsf will map (0,0,0) to (0,0,0) since
// I need the vectorial part of the transformation
//
  
// gp_Pnt  a_point(0.0e0, 
//	   	   0.0e0,
//	           0.0e0) ;

// gp_Vec  the_image(0.0e0,
//		    0.0e0,
//		    1.0e0) ;
//  a_vector.SetCoord(1,0.0e0) ;
//  a_vector.SetCoord(2,0.0e0) ;
//  a_vector.SetCoord(3,0.0e0) ;
//  a_vector.SetCoord(3,1.0e0) ;
  
          
//
//  this is the matrix such that we have 
//
//    matrix_vector[0][i] =  SUM   matrix[i][j] * vector[0][j] 
//                          j=1,3
//
  Standard_Real matrix[3][3],
  solution[3]  ;
  for (ii = 0 ; ii < 3 ; ii++) {
    for (jj = 0 ; jj < 3 ; jj++) {
      matrix[ii][jj] = 0.0e0 ; 
    }
  }
  matrix[0][0] = 1.0e0 ;
  if (index >= 2) {
    matrix[1][0] = vector[0][0].Dot(matrix_vector[0][1]) ;
    matrix[1][1] = vector[0][1].Dot(matrix_vector[0][1]) ;
    if (index >= 3) {
      matrix[2][0] = vector[0][0].Dot(matrix_vector[0][2]) ;
      matrix[2][1] = vector[0][1].Dot(matrix_vector[0][2]) ;
      matrix[2][2] = vector[0][2].Dot(matrix_vector[0][2]) ;
    }
  }

  gp_Ax3   fixed_frame(a_point,
		       vector[0][2],
		       vector[0][0]) ;
  gp_Ax3   moving_frame(a_point,
		       vector[1][2],
		       vector[1][0]) ;
  



  gp_Trsf  vectorial_part;
  vectorial_part.SetDisplacement(moving_frame,
				 fixed_frame) ;

//
// we are going to solve 
// 
//   X.matrix_vector[0][ii] = 
// (origin[0][ii] - v(origin[1][ii])). matrix_vector[0][ii] + distance[ii] 
//    for ii = 1,...,index 
//  
//  were v is the vectorial_part transformation
//
//  we write X =  SUM   solution[i] * vector[0][i] 
//               i=1,3
//

  for (jj = 0 ; jj < index ; jj++) {
    a_point = origin[1][jj].Transformed(vectorial_part) ;
    for (ii = 1 ; ii <= 3 ; ii++) {
      a_vector.SetCoord(ii, origin[0][jj].Coord(ii) - a_point.Coord(ii)) ;
    }
    solution[jj] = a_vector.Dot(matrix_vector[0][jj]) + distance[jj] ;
  }
  for (ii = index ; ii < 3 ; ii++) {
    solution[ii] = 0.0e0 ;
  }  

  if (index >= 2) {
    solution[1] -= matrix[1][0] * solution[0] ;
    solution[1] /= matrix[1][1] ;
    if (index == 3) {
      solution[2] -= (matrix[2][0] * solution[0] + matrix[2][1] * solution[1]) ;
      solution[2] /= matrix[2][2] ;
    }
  }
  for (ii = 0 ; ii < 3 ; ii++) {
    value = 0.0e0 ;
    for (jj = 0 ; jj < 3 ; jj++) {
      value += solution[jj] * vector[0][jj].Coord(ii + 1) ;
    }
    a_vector.SetCoord(ii + 1,value) ;
  }

// add the translation of vectorial part of Trsf to just-computed translation

  a_vector.Add(vectorial_part.TranslationPart());
  vectorial_part.SetTranslationPart(a_vector);


  //
  // case : conical surfaces
  // The translation part is computed in the way to send
  // the apex of the moving shape on the apex of the
  // fixed shape.

  gp_Trsf t;
  Standard_Boolean NotFound = Standard_True;
  for (ii = 1,anIterator.Initialize(myConstraints);
       index < 3 && ii <= num_equations && NotFound ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
      anIterator.Value() ;
    const ShapePlacement_Equation & an_equation =
      a_constraint.Equation() ;
    if (myEquationStatus->Value(ii) == Compatible 
	                            && 
	an_equation.Type() == ShapePlacement_CONE_PLACEMENT) {
      t.SetTranslation(an_equation.MovingPoint().Transformed(vectorial_part),
		       an_equation.FixedPoint());
      a_vector.Add(t.TranslationPart());
      NotFound = Standard_False;
    }
  }
  

  //
  // 1 AXIS_PLACEMENT constraint with 2 PLANE_PLACEMENT
  // The 2 axes are already parallel but we must compute
  // the tranlsation to make them aligned or to fit the offset
  //

  NotFound = Standard_True;
  for (ii = 1,anIterator.Initialize(myConstraints);
       index < 3 && ii <= num_equations && NotFound ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
      anIterator.Value() ;
    const ShapePlacement_Equation & an_equation =
      a_constraint.Equation() ;
    if (myEquationStatus->Value(ii) == Compatible 
	&& 
	an_equation.Type() == ShapePlacement_AXIS_AXIS_PLACEMENT) {
      gp_Vec vector1(an_equation.MovingPoint().Transformed(vectorial_part),
		     an_equation.FixedPoint()),
      vector2 = an_equation.FixedDirection();
      a_vector.Add(vector1 - vector1.Dot(vector2) * vector2);
      //
      // fit the offset if it can be done !!
      //
      if (vector[0][2].CrossSquareMagnitude(vector2) >= tolerance_squared)
	if (Abs(vector2.Dot(vector[0][2])) >= Precision::Angular()) 
	  a_vector.Add((an_equation.SignedDistance() / vector2.Dot(vector[0][2])) * vector[0][2]);
	else
	  a_vector.Add(an_equation.SignedDistance() * vector[0][2]);
      
      NotFound = Standard_False;
    
    }
  }
  
  
  //
  // 1 PLANE_AXIS_PLACEMENT constraint with 2 PLANE_PLACEMENT
  // The axis is already parallel to the plane but we must compute
  // the tranlsation to fit the offset
  //

  NotFound = Standard_True;
  for (ii = 1,anIterator.Initialize(myConstraints);
       index < 3 && ii <= num_equations && NotFound ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
      anIterator.Value() ;
    const ShapePlacement_Equation & an_equation =
      a_constraint.Equation() ;
    if (an_equation.Type() == ShapePlacement_PLANE_AXIS_PLACEMENT) {
      
      if (index == 2) {
	//
	// this should be true if we are here !! 
	//
	if (matrix_vector[0][0].CrossSquareMagnitude(matrix_vector[0][1])
	    >= tolerance_squared) {
	  gp_Vec vector1 = matrix_vector[0][0].Crossed(matrix_vector[0][1]);
	  if (vector1.CrossSquareMagnitude(an_equation.MovingDirection())
	      >= tolerance_squared) {
	    gp_Vec vector2(an_equation.MovingPoint().Transformed(vectorial_part),
			   an_equation.FixedPoint());
	    Standard_Real k3 = (vector2.Dot(an_equation.FixedNormal()) 
	                        + an_equation.SignedDistance())
	                        / vector1.Dot(an_equation.FixedNormal());
	    a_vector.Add(k3 * vector1);
	  }
	}
      }    
      NotFound = Standard_False;
    }
  }




  //
  // update Trsf
  // end solve_case1
  //
  vectorial_part.SetTranslationPart(a_vector) ;

  myHasSolution = Standard_True ;
  myTrsf = vectorial_part;
}

//=======================================================================
//function : Solve
//purpose  : There is exactly one AXIS_AXIS_PLACEMENT equation
//=======================================================================

void ShapePlacement_ConstraintAlgo::SolveCase2() 
{
  Standard_Integer 
  ii,
  definition_does_not_exists,
  num_equations,
  index ;
  gp_Vec   matrix_vector[2][3],
  vector[2][3],
  a_vector,
  a_vector1(0.0e0,0.0e0,0.0e0)  ;

  gp_Pnt   origin[2][3] ;
  Standard_Real distance[3],
    dot_product,
  tolerance_squared,
  a_sine_squared,
  solution;
  Standard_Boolean cone = Standard_False;

//
//    seul cas traiter pour l instant
//    search for linearly independant conditions
//
  tolerance_squared = myAngularTolerance * myAngularTolerance ;
  num_equations =
    myEquationStatus->Length() ;
  index = 0 ;
  ShapePlacement_ListIteratorOfListOfConstraint   anIterator ;
  for (ii = 1,
       anIterator.Initialize(myConstraints); 
       index < 3 && ii <= num_equations ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
     anIterator.Value() ;
    const ShapePlacement_Equation & an_equation =
     a_constraint.Equation() ;
    if (myEquationStatus->Value(ii) == Compatible &&
	an_equation.Type() == ShapePlacement_AXIS_AXIS_PLACEMENT) {
      matrix_vector[0][index] = an_equation.FixedDirection()  ;
      matrix_vector[1][index] = an_equation.MovingDirection() ;
      if (myEquationOrientation->Value(ii) == Reverse) {
	matrix_vector[1][index].Reverse() ;
      }
      origin[0][index] = an_equation.FixedPoint()  ;
      origin[1][index] = an_equation.MovingPoint() ;
      distance[index] = an_equation.SignedDistance() ;
      index += 1 ;
      
    }
  }
//
// if we did not skrew up index should be 1 here
// Search for a PLANE_PLACEMENT constraint now
//
  for (ii = 1,
       anIterator.Initialize(myConstraints); 
       index < 3 && ii <= num_equations ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
     anIterator.Value() ;
    const ShapePlacement_Equation & an_equation =
     a_constraint.Equation() ;
    if (myEquationStatus->Value(ii) == Compatible && 

//
//  the condition can lead to colinear vectors : this does not matter
//  the face alignement will translate the cylinder appropriatly
//


	(an_equation.Type() == ShapePlacement_CONE_PLACEMENT ||
	 an_equation.Type() == ShapePlacement_PLANE_PLACEMENT) )   {
      matrix_vector[0][index] = an_equation.FixedNormal()  ;
      matrix_vector[1][index] = an_equation.MovingNormal() ;
      matrix_vector[0][index].Normalize();
      matrix_vector[1][index].Normalize();
      origin[0][index] = an_equation.FixedPoint()  ;
      origin[1][index] = an_equation.MovingPoint() ;
      distance[index] = an_equation.SignedDistance() ;
      index += 1 ;
      if (an_equation.Type() == ShapePlacement_CONE_PLACEMENT) 
	cone = Standard_True;
    }
  }

  //
  // build 2 Ax3 to define the Trsf 
  // the vector defining the Ax3 are 
  //   for the fixed Ax3             vector[0][0], 
  //                                 vector[0][1], 
  //                                 vector[0][2] 
  //
  //   for the moving Ax3            vector[1][0], 
  //                                 vector[1][1], 
  //                                 vector[1][2] 
  //
  if (index == 1) {
    //
    //
    // there is only one AXIS_AXIS_PLACEMENT constraint : try to minimize the rotation
    //

    dot_product = matrix_vector[0][0].Dot(matrix_vector[1][0]) ;
    if (dot_product <= 0.0e0) {
      matrix_vector[1][0].Reverse() ;
    }
  }
  definition_does_not_exists = 1 ;
  if (index >= 2) {
    a_sine_squared = 
      matrix_vector[1][0].CrossSquareMagnitude(matrix_vector[1][1]) ;
    if (a_sine_squared > tolerance_squared) {
    //
    //  the second vector is linerarly independant from the first
    //  vector 
    //
      for (ii = 0 ; ii < 2 ; ii++) {
	vector[ii][0] = matrix_vector[ii][0] ;
	vector[ii][2] = matrix_vector[ii][0].Crossed(matrix_vector[ii][1]) ;
	vector[ii][1] = vector[ii][2].Crossed(matrix_vector[ii][0]) ;
      }
      definition_does_not_exists = 0 ;
    } 
  }

  gp_Pnt a_point = IsoBarycentre(myShapeToPosition);


  if (definition_does_not_exists)  {
//    gp_Pnt temporary_point(0.0e0,
//			   0.0e0,
//			   0.0e0) ;
    gp_Dir temporary_dir0(matrix_vector[0][0]) ;
    gp_Dir temporary_dir1(matrix_vector[1][0]) ;
    gp_Ax3 temporary_axis0(a_point,
			   temporary_dir0) ;
    gp_Ax3 temporary_axis1(a_point,
			   temporary_dir1) ;
//
// still the Z axis should be the first slot of the
// frame vector[0] and vector[1] 
//
    for (ii = 1 ; ii <= 3 ; ii++) {
      vector[0][0].SetCoord(ii, temporary_axis0.Direction().Coord(ii)) ;
      vector[0][1].SetCoord(ii, temporary_axis0.XDirection().Coord(ii)) ;
      vector[0][2].SetCoord(ii, temporary_axis0.YDirection().Coord(ii)) ;
      vector[1][0].SetCoord(ii, temporary_axis1.Direction().Coord(ii)) ;
      vector[1][1].SetCoord(ii, temporary_axis1.XDirection().Coord(ii)) ;
      vector[1][2].SetCoord(ii, temporary_axis1.YDirection().Coord(ii)) ;
    }
  }

  gp_Ax3   fixed_frame(a_point,
		       vector[0][2],
		       vector[0][0]) ;
  gp_Ax3   moving_frame(a_point,
		       vector[1][2],
		       vector[1][0]) ;
  



  gp_Trsf  vectorial_part ;
  vectorial_part.SetDisplacement(moving_frame,
				 fixed_frame) ;



  //
  // we are going to define the image X of the point origin[1][0] which is 
  // the moving point by : 
  // 
  //   X = origin[0][0] + 
  //   {(origin[0][0] - origin[1][0]).matrix_vector[0][0]} matrix_vector[0][0]
  //  
  //  were v is the vectorial_part transformation
  //
  //
  
  
  for (ii = 1 ; ii <= 3 ; ii++) {
    a_vector.SetCoord(ii, origin[0][0].Coord(ii) 
		      -
		      origin[1][0].Transformed(vectorial_part).Coord(ii)) ;
  }
  solution = - a_vector.Dot(matrix_vector[0][0]) ;
  for (ii = 1 ; ii <= 3 ; ii++) {
    a_vector.SetCoord(ii, solution * matrix_vector[0][0].Coord(ii) + 
		      origin[0][0].Coord(ii)) ;
  } 



  //
  // remember X : is the image of origin[1][0] 
  // and the translation part the Trsf is X - v(origin[0][1])
  // We must respect the offset, too !!

  a_point = origin[1][0].Transformed(vectorial_part) ;

  for (ii = 1 ; ii <= 3 ; ii++) {
    a_vector.SetCoord(ii, 
		      a_vector.Coord(ii) - a_point.Coord(ii)) ;
  }
  
  static Standard_Real sign = 1.0e0;

  if ( Abs(a_vector.Magnitude()) > Precision::Confusion()) {
    a_vector = a_vector - sign * distance[0] * a_vector.Normalized();
  }
  else {
    gp_Ax3 axis(origin[0][0], matrix_vector[0][0]);
    a_vector = axis.XDirection();
    a_vector = distance[0] * a_vector.Normalized();
  }
  
  if (distance[0] < 0.0e0)  
    sign = -1.0e0;
  else
    sign = 1.0e0;
  // we keep the translation part of displacement from fixed_frame to
  // moving_frame
  a_vector.Add(vectorial_part.TranslationPart());

  
  // axis-placement Trsf!
  vectorial_part.SetTranslationPart(a_vector) ;


  //
  // Now, we calculate the translation due to PLANE_PLACEMENT.
  // The translation's direction must be the axis' direction 
  //

  if (index >= 2) {
    if (Abs(matrix_vector[0][0].Dot(matrix_vector[0][1])) >= Precision::Angular()) {
      if (cone) 
	distance[1] = 0.0e0;
      origin[1][1].Transform(vectorial_part);
      for (ii = 1; ii <= 3; ii++)
	a_vector1.SetCoord(ii, origin[0][1].Coord(ii) - origin[1][1].Coord(ii));
      Standard_Real k1 = matrix_vector[0][1].Dot(a_vector1)
	                 / matrix_vector[0][0].Dot(matrix_vector[0][1]) 
			 + distance[1] / matrix_vector[0][0].Dot(matrix_vector[0][1]);
      a_vector1 = k1 * matrix_vector[0][0];
    }
  }
 





  // we add the translation due to PLANE_PLACEMENT
  a_vector.Add(a_vector1);


  // final Trsf!!
  vectorial_part.SetTranslationPart(a_vector) ;
  
  myHasSolution = Standard_True ;
  myTrsf = vectorial_part ;
}
  
//=======================================================================
//function : Solve
//purpose  : There are at least two AXIS_AXIS_PLACEMENT equation
// those are either linearly independant or parallele to each other
//=======================================================================

void ShapePlacement_ConstraintAlgo::SolveCase3() 
{
  Standard_Integer 
  ii,
  jj,
  num_equations,
  index ;


  gp_Vec   matrix_vector[2][3],
  vector[2][3],
  a_vector(0.0e0,0.0e0,0.0e0);

  gp_Pnt   origin[2][3] ;
  gp_Pnt   new_origin[2] ;
  Standard_Real distance[3],
  a_sine_squared,
  tolerance_squared,
  a_11,
  a_21,
  b_21,
  first_equation_not_found,
  solution,
  value  ;
  Standard_Boolean NotFound = Standard_True,
                   parallele = Standard_False,
                   fix_p_a_c = Standard_False;
  
  for (ii = 0 ; ii < 2 ; ii++) {
    for (jj = 0 ; jj < 3 ; jj++) {
      matrix_vector[ii][jj] = a_vector ;
    }
  }
//
//    seul cas traiter pour l instant
//    search for linearly independant conditions
//
  tolerance_squared = myAngularTolerance * myAngularTolerance ;
  first_equation_not_found = 1 ;
  num_equations =
    myEquationStatus->Length() ;
  index = 0 ;
  ShapePlacement_ListIteratorOfListOfConstraint   anIterator ;
  for (ii = 1,
       anIterator.Initialize(myConstraints); 
       index < 3 && ii <= num_equations ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
     anIterator.Value() ;
    const ShapePlacement_Equation & an_equation =
     a_constraint.Equation() ;
    if (myEquationStatus->Value(ii) == Compatible &&
	 an_equation.Type() == ShapePlacement_AXIS_AXIS_PLACEMENT) {
      matrix_vector[0][index] = an_equation.FixedDirection()  ;
      matrix_vector[1][index] = an_equation.MovingDirection() ;
      if (myEquationOrientation->Value(ii) == Reverse) {
	matrix_vector[1][index].Reverse() ;
      }
      origin[0][index] = an_equation.FixedPoint()  ;
      origin[1][index] = an_equation.MovingPoint() ;
      distance[index] = an_equation.SignedDistance() ;
      index += 1 ;
    }
      
   }

//
// Particular case : there are 2 AXIS-AXIS-PLACEMENT && 1 compatible PLANE_AXIS_PLACEMENT.
// the axis and the plane must be parallele (eventually after a flip of the axis of AXIS_AXIS_PLACEMENT
// The offset can be fit only if axis of AXIS_AXIS_PLACEMENT are parallele 
// Otherwise we just satisfy the parallelism by needed flips.
//

  for (ii = 1,
       anIterator.Initialize(myConstraints); 
       index < 3 && ii <= num_equations && NotFound; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
      anIterator.Value() ;
    const ShapePlacement_Equation & an_equation =
      a_constraint.Equation() ;
    if (an_equation.Type() == ShapePlacement_PLANE_AXIS_PLACEMENT) {
      matrix_vector[0][index] = an_equation.FixedNormal()  ;
      matrix_vector[1][index] = an_equation.MovingDirection() ;
      if (myEquationOrientation->Value(ii) == Reverse) {
	matrix_vector[1][index].Reverse() ;
      }
      origin[0][index] = an_equation.FixedPoint()  ;
      origin[1][index] = an_equation.MovingPoint() ;
      distance[index] = an_equation.SignedDistance() ;
      matrix_vector[0][index].Normalize();
      matrix_vector[1][index].Normalize();
      index += 1 ;
      NotFound = Standard_False;
    }
    
  }      
  
 //
 // if we did not skrew up index should be 2 here
 //


  // build 2 Ax3 to define the Trsf 
  // the vector defining the Ax3 are 
  //   for the fixed Ax3             vector[0][0], 
  //                                 vector[0][1], 
  //                                 vector[0][2] 
  //
  //   for the moving Ax3            vector[1][0], 
  //                                 vector[1][1], 
  //                                 vector[1][2] 
  //




  if (index >= 2) { 

    gp_Trsf  vectorial_part ;
    
    Standard_Integer alterne = 0,compteur = 0;
    gp_Pnt a_point = IsoBarycentre(myShapeToPosition);

    
    a_sine_squared = 
      matrix_vector[1][0].CrossSquareMagnitude(matrix_vector[1][1]) ;

    do {
      
      if (a_sine_squared > tolerance_squared) {
      
	//
	//  the second vector is linerarly independant from the first
	//  vector 
	//
	for (ii = 0 ; ii < 2 ; ii++) {
	  vector[ii][0] = matrix_vector[ii][0] ;
	  vector[ii][2] = matrix_vector[ii][0].Crossed(matrix_vector[ii][1]) ;
	  vector[ii][1] = vector[ii][2].Crossed(matrix_vector[ii][0]) ;
	}
	//
	// we are going define the image X of the point 
	//  origin[1][0] + a * matrix_vector[1][0] which is
	//  the point that minimises the distance between the two lines :
	// 
	//
	//    origin[1][0] +  lambda * matrix_vector[1][0] 
	//
	//    origin[1][1] + lambda1 * matrix_vector[1][1] 
	//    
	// by  :
	// 
	//   X = origin[0][0] +  a * matrix_vector[0][0]  where  
	//   a is defined by the fact that X is the point that minimises
	//   the distance between the following lines :
	//
	//    origin[0][0] +  lambda * matrix_vector[0][0] 
	//
	//    origin[0][1] + lambda1 * matrix_vector[0][1] 
	//
	//
	// lets compute a : a is given by the following formula
	//
	//
	//      | a11  b12 |     | b11   b12 |
	//  a = |          |  /  |           |
	//      | a21  b22 |     | b21   b22 |
	//
	//   a11 = (origin[0][1] - origin[0][0]).matrix_vector[0][0] 
	//   a21 = (origin[0][1] - origin[0][0]).matrix_vector[0][1] 
	//   b11 = b22 = 1
	//   b12 = matrix_vector[0][0].matrix_vector[0][1] 
	//   
	
	for (jj = 0 ; jj < 2 ; jj++) {
	  for (ii = 1 ; ii <= 3 ; ii++) {
	    a_vector.SetCoord(ii, origin[jj][1].Coord(ii) 
			      - origin[jj][0].Coord(ii)) ;
	  }
	  
	  a_11 = a_vector.Dot(matrix_vector[jj][0]) ;
	  a_21 = a_vector.Dot(matrix_vector[jj][1]) ;
	  b_21 = matrix_vector[jj][1].Dot(matrix_vector[jj][0]) ;
	  solution = 1.0e0 - (b_21 * b_21) ;
	  if (solution > tolerance_squared) {
	    solution = 1.0e0 / solution ;
	    solution *= a_11 - (a_21 * b_21) ;
	    
	    
	    for (ii = 1 ; ii <= 3 ; ii++) {
	      new_origin[jj].SetCoord(ii, 
				      solution * matrix_vector[jj][0].Coord(ii) + 
				      origin[jj][0].Coord(ii)) ;
	    }
	  }
	}
	for (ii = 1 ; ii <= 3 ; ii++) {
	  a_vector.SetCoord(ii,   new_origin[0].Coord(ii)) ;
	}
	
      }
      else {
	//
	// the two axis are colinear but the origins are not on the
	// the same axis : otherwise the caller really skrewed up
	//
	for (jj = 0 ; jj < 2 ; jj++) {
	  vector[jj][0] = matrix_vector[jj][0] ;

	  //
	  // look for the vector which is the projection of 
	  // origin[0][1] - origin[0][0] on the plane normal  
	  // to matrix_vector[0][0] 
	  // 
	
  
	  for (ii = 1 ; ii <= 3 ; ii++) {
	    vector[jj][1].SetCoord(ii, 
				   origin[jj][1].Coord(ii) - origin[jj][0].Coord(ii)) ;
	  }
	  value = vector[jj][1].Dot(matrix_vector[0][0]) ;
	  for (ii = 1 ; ii <= 3 ; ii++) {
	    vector[jj][1].SetCoord(ii, vector[jj][1].Coord(ii) - 
				   value * matrix_vector[jj][0].Coord(ii)) ;
	  }
	  if (vector[jj][1].Magnitude() > Precision::Confusion()) {
	    vector[jj][1].Normalize() ;
	  }
	  vector[jj][2] = vector[jj][0].Crossed(vector[jj][1]) ;
	}
	//
	// any translation that maps the first axis to the second axis will do
	//

	for (ii = 1 ; ii <= 3 ; ii++) {
	  a_vector.SetCoord(ii, origin[0][0].Coord(ii)) ;
	}
	new_origin[0]  = origin[0][0] ;
	new_origin[1]  = origin[1][0] ;
	parallele = Standard_True;
	
      }
      
      gp_Ax3   fixed_frame(a_point,
			   vector[0][2],
			   vector[0][0]) ;
      gp_Ax3   moving_frame(a_point,
			    vector[1][2],
			    vector[1][0]) ;
      
      if (compteur == 1 && parallele) 
	  moving_frame.ZReverse();
       
      
      vectorial_part.SetDisplacement(moving_frame,
				     fixed_frame) ;
      
      if (! parallele) {    
	matrix_vector[1][alterne].Reverse();
	alterne= 1 - alterne;
      }
      compteur++;
	
      if (index >= 3) {
	fix_p_a_c = (Abs(matrix_vector[1][2].Transformed(vectorial_part).
			 Dot(matrix_vector[0][2])) <= Precision::Angular());
      } else {
	fix_p_a_c = Standard_True ; 
      }
      
    }  while (! NotFound 
	      && 
	      ! fix_p_a_c
	      &&
	      compteur <=4);


    //
    // remember : X is the image of new_origin[1] 
    // and the translation part the Trsf is X - v(origin[0][1])
    // 

    a_point = new_origin[1].Transformed(vectorial_part) ;
    if (parallele) {
      gp_Vec temp(a_point, origin[0][0]);
      a_vector = a_vector - temp.Dot(matrix_vector[0][0]) * matrix_vector[0][0];
    }
    for (ii = 1 ; ii <= 3 ; ii++) {
      a_vector.SetCoord(ii, 
			a_vector.Coord(ii) - a_point.Coord(ii)) ;
    }
    
    
    static Standard_Real sign = 1.0e0;
    
    if ( Abs(a_vector.Magnitude()) > Precision::Confusion()) {
      a_vector = a_vector - sign * distance[0] * a_vector.Normalized();
    }
    else {
      gp_Ax3 axis(origin[0][0], matrix_vector[0][0]);
      a_vector = axis.XDirection();
      a_vector = distance[0] * a_vector.Normalized();
    }
    
    if (distance[0] < 0.0e0)  
      sign = -1.0e0;
    else
      sign = 1.0e0;
    
    a_vector.Add(vectorial_part.TranslationPart());
    
    vectorial_part.SetTranslationPart(a_vector) ;
//
// Translation due to PLANE_AXIS_PLACEMENT with 2 AXIS_AXIS_PLACEMENT with parallele axis 
// The direction can only be the direction of these axis.
//

    if (! NotFound && fix_p_a_c && parallele) {
      origin[1][2].Transform(vectorial_part);
      if (Abs(matrix_vector[0][0].Dot(matrix_vector[0][2])) >= myAngularTolerance) {
	gp_Vec vector2(origin[1][2], origin[0][2]);
	Standard_Real coeff = (matrix_vector[0][2].Dot(vector2) + distance[2])
	  /matrix_vector[0][0].Dot(matrix_vector[0][2]);
	a_vector.Add(coeff * matrix_vector[0][0]);
      }
    }
    
    vectorial_part.SetTranslationPart(a_vector) ;


    myHasSolution = Standard_True ;
    myTrsf = vectorial_part ;
  }
  else {
    Standard_ConstructionError::Raise() ;
    }

}
   				       
//=======================================================================
//function : Solve
//purpose  : 
//  Solves the case of 2 AXIS_AXIS_PLACEMENT with one PLANE_PLACEMENT
//=======================================================================

void ShapePlacement_ConstraintAlgo::SolveCase4() 
{
  Standard_Integer 
  ii,
  jj,
  num_equations,
  index ;


  gp_Vec   matrix_vector[2][3],
  vector[2][3],
  a_vector ;

  gp_Pnt   origin[2][3] ;
  gp_Pnt   new_origin[2] ;
  Standard_Real distance[3],
  a_sine_squared,
  tolerance_squared,
  a_11,
  a_21,
  b_21,
  first_equation_not_found,
  solution,
  value1,
  value  ;
//
//    seul cas traiter pour l instant
//    search for linearly independant conditions
//
  tolerance_squared = myAngularTolerance * myAngularTolerance ;
  first_equation_not_found = 1 ;
  num_equations =
    myEquationStatus->Length() ;
  index = 0 ;
  ShapePlacement_ListIteratorOfListOfConstraint   anIterator ;
  for (ii = 1,
       anIterator.Initialize(myConstraints); 
       index < 3 && ii <= num_equations ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
     anIterator.Value() ;
    const ShapePlacement_Equation & an_equation =
     a_constraint.Equation() ;
    if ( myEquationStatus->Value(ii) == Compatible &&
	 an_equation.Type() == ShapePlacement_AXIS_AXIS_PLACEMENT) {
      matrix_vector[0][index] = an_equation.FixedDirection()  ;
      matrix_vector[1][index] = an_equation.MovingDirection() ;

      if (myEquationOrientation->Value(ii) == Reverse) {
	matrix_vector[1][index].Reverse() ;
      }
      origin[0][index] = an_equation.FixedPoint()  ;
      origin[1][index] = an_equation.MovingPoint() ;
      distance[index] = an_equation.SignedDistance() ;
      index += 1 ;
    }
  }      
//
// look for the plane_placement constraint
//  
  for (ii = 1,
       anIterator.Initialize(myConstraints); 
       index < 3 && ii <= num_equations ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
      anIterator.Value() ;
    const ShapePlacement_Equation & an_equation =
      a_constraint.Equation() ;
    if ((an_equation.Type() == ShapePlacement_PLANE_PLACEMENT || 
	 an_equation.Type() == ShapePlacement_CONE_PLACEMENT) &&
	myEquationStatus->Value(ii) ==    Compatible) {
	matrix_vector[0][index] = an_equation.FixedNormal()  ;
	matrix_vector[1][index] = an_equation.MovingNormal() ;
	origin[0][index] = an_equation.FixedPoint()  ;
	origin[1][index] = an_equation.MovingPoint() ;
	distance[index] = an_equation.SignedDistance() ;
	index += 1 ;
      }
  }
 //
 // if we did not skrew up index should be 2 here
 //


  // build 2 Ax3 to define the Trsf 
  // the vector defining the Ax3 are 
  //   for the fixed Ax3             vector[0][0], 
  //                                 vector[0][1], 
  //                                 vector[0][2] 
  //
  //   for the moving Ax3            vector[1][0], 
  //                                 vector[1][1], 
  //                                 vector[1][2] 
  //
  if (index >= 2) { 
     a_sine_squared = 
      matrix_vector[1][0].CrossSquareMagnitude(matrix_vector[1][1]) ;
     if (a_sine_squared > tolerance_squared) {
      //
      //  the second vector is linerarly independant from the first
      //  vector 
      //
       for (ii = 0 ; ii < 2 ; ii++) {
	 vector[ii][0] = matrix_vector[ii][0] ;
	 vector[ii][2] = matrix_vector[ii][0].Crossed(matrix_vector[ii][1]) ;
	 vector[ii][1] = vector[ii][2].Crossed(matrix_vector[ii][0]) ;
       }
    //
    // we are going define the image X of the point 
    //  origin[1][0] + a * matrix_vector[1][0] which is
    //  is the point that minimises the distance between the two lines :
    // 
    //
    //    origin[1][0] +  lambda * matrix_vector[1][0] 
    //
    //    origin[1][1] + lambda1 * matrix_vector[1][1] 
    //    
    // by  :
    // 
    //   X = origin[0][0] +  a * matrix_vector[0][0]  where  
    //   a is defined by the fact that X is the point that minimises
    //   the distance between the following lines :
    //
    //    origin[0][0] +  lambda * matrix_vector[0][0] 
    //
    //    origin[0][1] + lambda1 * matrix_vector[0][1] 
    //
    //
    // lets compute a : a is given by the following formula
    //
    //
    //      | a11  b12 |     | b11   b12 |
    //  a = |          |  /  |           |
    //      | a21  b22 |     | b21   b22 |
    //
    //   a11 = (origin[0][1] - origin[0][0]).matrix_vector[0][0] 
    //   a21 = (origin[0][1] - origin[0][0]).matrix_vector[0][1] 
    //   b11 = b22 = 1
    //   b12 = matrix_vector[0][0].matrix_vector[0][1] 
    //   
    
       for (jj = 0 ; jj < 2 ; jj++) {
	 for (ii = 1 ; ii <= 3 ; ii++) {
	   a_vector.SetCoord(ii, origin[jj][1].Coord(ii) 
			     - origin[jj][0].Coord(ii)) ;
	 }
	 
	 a_11 = a_vector.Dot(matrix_vector[jj][0]) ;
	 a_21 = a_vector.Dot(matrix_vector[jj][1]) ;
	 b_21 = matrix_vector[jj][1].Dot(matrix_vector[jj][0]) ;
	 solution = 1.0e0 - (b_21 * b_21) ;
	 tolerance_squared = myAngularTolerance * myAngularTolerance ;
	 if (solution > tolerance_squared) {
	   solution = 1.0e0 / solution ;
	   solution *= a_11 - (a_21 * b_21) ;
	   
	   
	   for (ii = 1 ; ii <= 3 ; ii++) {
	     new_origin[jj].SetCoord(ii, 
				     solution * matrix_vector[jj][0].Coord(ii) + 
				     origin[jj][0].Coord(ii)) ;
	   }
	 }
       }
       for (ii = 1 ; ii <= 3 ; ii++) {
	 a_vector.SetCoord(ii,   new_origin[0].Coord(ii)) ;
       }
     }
     else {
      //
      // the two axis are colinear but the origins are not on the
      // the same axis : otherwise the caller really skrewed up
      //
       for (jj = 0 ; jj < 2 ; jj++) {
	 vector[jj][0] = matrix_vector[jj][0] ;
	//
	// look for the vector which is the projection of 
	// origin[0][1] - origin[0][0] on the plane normal  
	// to matrix_vector[0][0] 
	// 
	
	 for (ii = 1 ; ii <= 3 ; ii++) {
	   vector[jj][1].SetCoord(ii, 
				  origin[jj][1].Coord(ii) - 
				  origin[jj][0].Coord(ii)) ;
	 }
	 value = vector[jj][1].Dot(matrix_vector[0][0]) ;
	 for (ii = 1 ; ii <= 3 ; ii++) {
	   vector[jj][1].SetCoord(ii, vector[jj][1].Coord(ii) - 
				  value * matrix_vector[jj][0].Coord(ii)) ;
	 }
	 if (vector[jj][1].Magnitude() > Precision::Confusion()) {
	   vector[jj][1].Normalize() ;
	 }
	 vector[jj][2] = vector[jj][0].Crossed(vector[jj][1]) ;
       }
       if (index == 3) {
	 //
	 // compute the intersection point of first axis with the first face
	 // and the intersection point of the second axis with the second 
	 // face store those in the new_origin points 
	 //
	 for (jj = 0 ; jj < 2 ; jj++) {
	   for (ii = 1 ; ii <= 3 ; ii++) {
	     a_vector.SetCoord(ii, origin[jj][2].Coord(ii) -
			       origin[jj][0].Coord(ii)) ;
	   }
	   value = a_vector.Dot(matrix_vector[jj][2]) ;
	   value1 = matrix_vector[jj][0].Dot(matrix_vector[jj][2]) ;
	   if (Abs(value1) > Precision::Confusion()) {
	     value /= value1 ;
	   }
	   
	   for (ii = 1 ; ii <= 3 ; ii++) {
	     new_origin[jj].SetCoord(ii, origin[jj][0].Coord(ii)
				     + value * matrix_vector[jj][0].Coord(ii)) ;
	   }
	 }
	 for (ii = 1 ; ii <= 3 ; ii++) {
	   a_vector.SetCoord(ii, new_origin[0].Coord(ii)) ;
	 }
	 
       }
     }
     gp_Pnt a_point(0.0e0,
		   0.0e0,
		   0.0e0) ;
    
    gp_Ax3   fixed_frame(a_point,
			 vector[0][2],
			 vector[0][0]) ;
    gp_Ax3   moving_frame(a_point,
			  vector[1][2],
			  vector[1][0]) ;
    
    
    
    
    gp_Trsf  vectorial_part ;
    vectorial_part.SetDisplacement(moving_frame,
				   fixed_frame) ;
       
    //
    // remember X : is the image of new_origin[1] that must coincide
    // with new_origin[0]
    // and the translation part the Trsf is X - v(origin[0][1])
    // 
    a_point = new_origin[1].Transformed(vectorial_part) ;
    for (ii = 1 ; ii <= 3 ; ii++) {
      a_vector.SetCoord(ii, 
			a_vector.Coord(ii) - a_point.Coord(ii)) ;
    }
     if (index >= 3) {
       if (Abs(matrix_vector[0][2].Dot(matrix_vector[0][0])) >= Precision::Confusion()) {
	 a_vector += (distance[2]/matrix_vector[0][0].Dot(matrix_vector[0][2])) * matrix_vector[0][0] ;
       }
     }
    vectorial_part.SetTranslationPart(a_vector) ;
    myHasSolution = Standard_True ;
    myTrsf = vectorial_part ;
  }
  else {
    Standard_ConstructionError::Raise() ;
    }

}
   	           
//=======================================================================
//function : SolveCase4
//purpose  : 
//=======================================================================

void ShapePlacement_ConstraintAlgo::SolveCase5() 
{
 Standard_ConstructionError::Raise() ;  
}
 
//=======================================================================
//function : SolveCase5
//purpose  : 
//=======================================================================

void ShapePlacement_ConstraintAlgo::SolveCase6() 
{
 Standard_ConstructionError::Raise() ;  
}

//=======================================================================
//function : SolveCase7
//purpose  : 1, 2, or 3 AXIS_PLANE_PLACEMENT + PLANE_PLACEMENT 
//=======================================================================

void ShapePlacement_ConstraintAlgo::SolveCase7()
{
  Standard_Integer 
  ii,
  jj,
  num_equations,
  index ;
  
  
  gp_Vec   matrix_vector[3][3],
  a_vector(0.0e0, 0.0e0, 0.0e0),
  a_vector1,
  a_vector2,
  a_vector3,
  vector1,
  vector2;
  gp_Pnt   origin[3][3] ;
  Standard_Real distance[3],
  tolerance_squared;
  // contraintes PLANE_AXIS_PLACEMENT


  tolerance_squared = myAngularTolerance * myAngularTolerance ;
  num_equations =
    myEquationStatus->Length() ;
  index = 0 ;
  ShapePlacement_ListIteratorOfListOfConstraint   anIterator ;

  for (ii = 1,
       anIterator.Initialize(myConstraints); 
       index < 3 && ii <= num_equations ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint  = anIterator.Value();
  const ShapePlacement_Equation & an_equation  = a_constraint.Equation();
    if ( an_equation.Type() == ShapePlacement_PLANE_AXIS_PLACEMENT) {
      matrix_vector[0][index] = an_equation.FixedNormal();
      
      matrix_vector[1][index] = an_equation.MovingDirection();
      matrix_vector[0][index].Normalize();
      matrix_vector[1][index].Normalize();

      origin[0][index] = an_equation.FixedPoint()  ;
      origin[1][index] = an_equation.MovingPoint() ;
      distance[index] = an_equation.SignedDistance() ;
      index += 1 ;
    }
  }

  // particulart case : look for PLANE_PLACEMENT

  Standard_Real distance_plane_placement=0.;
  Standard_Boolean NotFound = Standard_True;
  gp_Pnt point1,point2;
  for (ii = 1,anIterator.Initialize(myConstraints);
       index < 3 && ii <= num_equations && NotFound ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint  = anIterator.Value();
    const ShapePlacement_Equation & an_equation  = a_constraint.Equation();
    if (myEquationStatus->Value(ii) == Compatible 
	&& 
	an_equation.Type() == ShapePlacement_PLANE_PLACEMENT) {
      vector2 = an_equation.MovingNormal();
      vector1 = an_equation.FixedNormal();
      NotFound = Standard_False;
      point1 = an_equation.FixedPoint();
      point2 = an_equation.MovingPoint();
      distance_plane_placement = an_equation.SignedDistance();
    }
  }

  if (index == 3) {
    point1 = origin[0][2];
    point2 = origin[1][2];
    vector1 = matrix_vector[0][2];
    distance_plane_placement = distance[2];
  }

 
  // vectorial part of Trsf
  gp_Pnt a_point = IsoBarycentre(myShapeToPosition);
  gp_Trsf  vectorial_part ;

  if (index == 1) {
    if (matrix_vector[0][0].CrossSquareMagnitude(matrix_vector[1][0]) >= tolerance_squared) 
      {
	matrix_vector[0][1] = matrix_vector[0][0].Crossed(matrix_vector[1][0]);
      }
    else {
      if (Abs(matrix_vector[0][0].Coord(2)) + Abs(matrix_vector[0][0].Coord(3))
	  >= 
	  Precision::Confusion()) {  
	matrix_vector[0][1].SetCoord(1,0.0e0);
	matrix_vector[0][1].SetCoord(2,- matrix_vector[0][0].Coord(3));
	matrix_vector[0][1].SetCoord(3,matrix_vector[0][0].Coord(2));
      }
      else {
	matrix_vector[0][1].SetCoord(1,0.0e0);
	matrix_vector[0][1].SetCoord(2,1.0e0);
	matrix_vector[0][1].SetCoord(3,0.0e0);
      } 
    }
    gp_Ax3   fixed_frame(a_point,
		       matrix_vector[0][0].Crossed(matrix_vector[0][1])) ;
    gp_Ax3   moving_frame(a_point,
			  matrix_vector[1][0]);


    // to avoid useless rotation, we check that axis and plane are not 
    // already aligned.
     
    if (Abs(matrix_vector[0][0].Dot(matrix_vector[1][0])) > Precision::Confusion())
      vectorial_part.SetDisplacement(moving_frame,
				     fixed_frame) ;
    a_vector3 = matrix_vector[0][0].Crossed(matrix_vector[0][1]);


  }
  else if (index == 2) {

    Standard_Integer alterne = 0,compteur = 0;
      
    do {
      a_vector = matrix_vector[0][1];
      a_vector3 = matrix_vector[0][0].Crossed(matrix_vector[0][1]);
      a_vector3.Normalize();
      
      
      gp_Vec a_vector4;
      if (matrix_vector[1][0].CrossSquareMagnitude(matrix_vector[1][1])
	  <=
	  tolerance_squared) {
      gp_Ax3 axis(a_point, matrix_vector[1][0]);
      a_vector4 = axis.XDirection();
      
    }
      else {
	a_vector4 = matrix_vector[1][0].Crossed(matrix_vector[1][1]);
      }
      
      gp_Vec vector[2];
      for (ii = 0; ii < index; ii++) {
	vector[ii] = a_vector3.Crossed(matrix_vector[0][ii]);
      }
      Standard_Real dot1 = matrix_vector[1][0].Dot(matrix_vector[1][1]),
      dot0 = vector[0].Dot(vector[1]);
      Standard_Integer e0,e1;
      if ((Abs(dot1) < Abs(dot0)) || dot1 > 0.e0) {
	e0 = 1;
	if (dot0 > 0.e0) 
	  e1 = 1;
	else
	  e1 = -1;
      }
      else {
	e0 = -1;
	if (dot0 > 0.e0) 
	  e1 = -1;
	else 
	  e1 = 1;
      }
      Standard_Real a0Square = e0*(dot1 - e1*dot0)/(1 - e0*e1*dot0);
      if (Abs(1 - a0Square) <= Precision::Confusion()) {
	a_vector1 = Sqrt(a0Square) * a_vector3;
	a_vector2 = Sqrt(a0Square) * e0 * a_vector3;
      }
      else if (Abs(a0Square) <= Precision::Confusion()) {
	a_vector1 = Sqrt(1 -a0Square) * vector[0];
	a_vector2 = e1*Sqrt(1- a0Square) * vector[1];
      } 
      else {
	a_vector1 = Sqrt(a0Square) * a_vector3 + Sqrt(1 -a0Square) * vector[0];
	a_vector2 = e0*Sqrt(a0Square) * a_vector3 + e1*Sqrt(1- a0Square) * vector[1];
      }
      gp_Vec a_vector5;
      if (a_vector1.CrossSquareMagnitude(a_vector2) >= tolerance_squared) {
	a_vector5 = a_vector1.Crossed(a_vector2);
      }
      else {
	gp_Ax3 axis(a_point, a_vector1);
	a_vector5 = axis.XDirection();
      }
      
      gp_Ax3   moving_frame(a_point,
			    a_vector4,
			    matrix_vector[1][0]);
      
      
      
      gp_Ax3   fixed_frame(a_point,
			   a_vector5,
			   a_vector1);
      
      vectorial_part.SetDisplacement(moving_frame,
				     fixed_frame) ;
      
      matrix_vector[1][alterne].Reverse();
      alterne= 1 - alterne;
      compteur++;
    
    } while (! NotFound 
	     && 
	     (! vector2.Transformed(vectorial_part).
	                IsEqual(vector1, Precision::Confusion(), Precision::Angular()))
	     &&
	     compteur <=4);
  }



  // translation part of Trsf
  // We have to solve (u(x) - y1).s1 = d1 & (u(x) - y2).s2 = d2
  // where y1 (resp y2) fixed point of plan 1 (resp plan 2)
  // and s1 (resp s2) fixed normal of plan 1 (resp plan 2)
  // There is only one solution if p1 # p2 but u(x) can be everywhere
  // on the axis solution
  // We assume that (u(x) - v(x)) = k1 * s1 + k2 *s2 to fix u(x)



  Standard_Real k1 = a_vector.Dot(matrix_vector[0][0]), 
                k2 = 0.0e0;


  origin[1][0].Transform(vectorial_part);
  for (ii = 1; ii <=3; ii++) 
    a_vector1.SetCoord(ii, origin[0][0].Coord(ii) -
			  origin[1][0].Coord(ii)); 
  if (index >= 2) {
    origin[1][1].Transform(vectorial_part);
    for (ii = 1; ii <=3; ii++) 
      a_vector2.SetCoord(ii, origin[0][1].Coord(ii) -
		       origin[1][1].Coord(ii));
    if (Abs(k1 - 1) >= Precision::Confusion())
      k2 = (distance[1] - distance[0]*k1 -
	    k1 * a_vector1.Dot(matrix_vector[0][0])
	    + a_vector2.Dot(matrix_vector[0][1]))/(1 - k1*k1);
    else  {
      k1 = 0.0e0;
      k2 = distance[1] + a_vector2.Dot(matrix_vector[0][1]);
    }
  }
  k1 = distance[0] - k2*k1 + a_vector1.Dot(matrix_vector[0][0]);

  // translation's vector


  a_vector = k1 * matrix_vector[0][0] + k2 * matrix_vector[0][1];
  a_vector.Add(vectorial_part.TranslationPart());
  
  vectorial_part.SetTranslationPart(a_vector);
  

  //
  // Third PLANE_AXIS_PLACEMENT constraint. The axis must be already parallel to face.
  // So, there is just a translation to compute, what is done below
  //           OR
  // Particular case : 1 PLANE_PLACEMENT constraint with 2 PLANE_AXIS_PLACEMENT
  // constraints. The 2 faces must be already parallel and not parallel to
  // placed axis. 
  // So, there is just a translation to compute, what is done below
  //


  if (index == 3 || ! NotFound) {
    point2.Transform(vectorial_part);
    if (Abs(a_vector3.Dot(vector1)) >= myAngularTolerance) {
      for (jj = 1; jj <= 3; jj++)
	vector2.SetCoord(jj, point1.Coord(jj) - point2.Coord(jj));
      k1 = (vector1.Dot(vector2) + distance_plane_placement)
	/a_vector3.Dot(vector1);
      a_vector.Add(k1 * a_vector3);
    }
  }
  
  //
  // update the Trsf and end the solve
  //

  vectorial_part.SetTranslationPart(a_vector);
  myTrsf = vectorial_part;
  myHasSolution = Standard_True;
}


//=======================================================================
//function : SolveCase8
//purpose  : 1 PLANE_PLACEMENT & 1 PLANE_AXIS_PLACEMENT or 1 AXIS_PLACEMENT & 1 PLANE_AXIS_PLACEMENT
//=======================================================================

void ShapePlacement_ConstraintAlgo::SolveCase8() 
{
  
  Standard_Integer 
    ii,
    jj,
    num_equations,
    index ;
  gp_Vec   matrix_vector[2][3],
  vector[2][3];
  gp_Pnt   origin[2][3] ;
  Standard_Real distance[3],
  tolerance_squared,
  a_sine_squared;
  Standard_Boolean is_axis=Standard_False,
                   distance_ok=Standard_False;
  // contraintes PLANE_AXIS_PLACEMENT
  tolerance_squared = myAngularTolerance * myAngularTolerance ;
  num_equations =
    myEquationStatus->Length() ;
  index = 0 ;
  ShapePlacement_ListIteratorOfListOfConstraint   anIterator ;
  
  
  for (ii = 1,
       anIterator.Initialize(myConstraints); 
       index < 3 && ii <= num_equations ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
     anIterator.Value() ;
    const ShapePlacement_Equation & an_equation =
     a_constraint.Equation() ;
    if (myEquationStatus->Value(ii) == Compatible && 
	(an_equation.Type() == ShapePlacement_CONE_PLACEMENT ||
	 an_equation.Type() == ShapePlacement_PLANE_PLACEMENT) )   {
      matrix_vector[0][index] = an_equation.FixedNormal()  ;
      matrix_vector[1][index] = an_equation.MovingNormal() ;
      origin[0][index] = an_equation.FixedPoint()  ;
      origin[1][index] = an_equation.MovingPoint() ;
      distance[index] = an_equation.SignedDistance() ;
      index += 1 ;
      is_axis = Standard_False;
    }
    else if (myEquationStatus->Value(ii) == Compatible && an_equation.Type() == ShapePlacement_AXIS_AXIS_PLACEMENT) {
      matrix_vector[0][index] = an_equation.FixedDirection()  ;
      matrix_vector[1][index] = an_equation.MovingDirection() ;
      origin[0][index] = an_equation.FixedPoint()  ;
      origin[1][index] = an_equation.MovingPoint() ;
      distance[index] = an_equation.SignedDistance() ;
      index += 1 ;
      is_axis = Standard_True;
    }


  }

  
  for (ii = 1,
       anIterator.Initialize(myConstraints); 
       index < 3 && ii <= num_equations ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint  = anIterator.Value();
  const ShapePlacement_Equation & an_equation  = a_constraint.Equation();
    if ( an_equation.Type() == ShapePlacement_PLANE_AXIS_PLACEMENT) {
      matrix_vector[0][index] = an_equation.FixedNormal();
      
      matrix_vector[1][index] = an_equation.MovingDirection();
      matrix_vector[0][index].Normalize();
      matrix_vector[1][index].Normalize();

     
      if (myEquationOrientation->Value(ii) == Reverse) {
	matrix_vector[1][index].Reverse() ;
      }
      origin[0][index] = an_equation.FixedPoint()  ;
      origin[1][index] = an_equation.MovingPoint() ;
      distance[index] = an_equation.SignedDistance() ;
      index += 1 ;
    }
  }
  gp_Pnt a_point = IsoBarycentre(myShapeToPosition);

  gp_Trsf  vectorial_part;

  //
  // index must be equal to 2 at this point 
  //

  a_sine_squared = 
    matrix_vector[1][0].CrossSquareMagnitude(matrix_vector[1][1]) ;
  
  if (a_sine_squared > tolerance_squared) {
    if (Abs(matrix_vector[1][0].Dot(matrix_vector[1][1])) >= Precision::Angular()) {
      gp_Vec another_vector = matrix_vector[0][0].Crossed(matrix_vector[0][1]).Normalized(),
             another_vector2 = matrix_vector[0][1].Crossed(another_vector);

      Standard_Real coeff = matrix_vector[1][0].Dot(matrix_vector[1][1])/matrix_vector[0][0].Dot(another_vector2);
      if (Abs(1 - coeff*coeff) >= Precision::Confusion())
	another_vector = coeff * another_vector2 + Sqrt(1 - coeff*coeff) * another_vector;
      else 
	another_vector = coeff * another_vector2;
      gp_Ax3 fixed_frame(a_point,
			 matrix_vector[0][0].Crossed(another_vector),
			 matrix_vector[0][0]);

      gp_Ax3 moving_frame(a_point,
			  matrix_vector[1][0].Crossed(matrix_vector[1][1]),
			  matrix_vector[1][0]);

      
      vectorial_part.SetDisplacement(moving_frame,
				     fixed_frame) ;
    }
    else {
      // cas perpendiculaire
      
      for (ii = 0 ; ii < 2 ; ii++) {
	vector[ii][0] = matrix_vector[ii][0] ;
	vector[ii][2] = matrix_vector[ii][0].Crossed(matrix_vector[ii][1]) ;
	vector[ii][1] = vector[ii][2].Crossed(matrix_vector[ii][0]) ;
      }
      vector[1][2] = matrix_vector[1][1];
      
      gp_Ax3   fixed_frame(a_point,
			   vector[0][2],
			   vector[0][0]) ;
      gp_Ax3   moving_frame(a_point,
			    vector[1][2],
			    vector[1][0]) ;
  
      vectorial_part.SetDisplacement(moving_frame,
				     fixed_frame) ;
      
    }
  }
  else {
    //
    // moving vectors are parallele which means that face of PLAN_PLACEMENT is perpendicular to 
    // axis of PLANE_AXIS_PLACEMENT or that axis of AXIS_PLACEMENT is parallel to axis of P_A_P.   
    // In the first case, solution is trivial
    // In the second case, there is exact solution only if distance between the 2 axis is greater 
    // than distance between fixed elements of the constraint (plan & axis). If this condition is not
    // satisfied, we can at least make the the elements parallel
    //
    

    if (! is_axis) {
      gp_Ax3 fixed_frame(a_point,matrix_vector[0][0]);
      gp_Ax3 moving_frame(a_point,matrix_vector[1][0]);
      
      vectorial_part.SetDisplacement(moving_frame,
				     fixed_frame) ;
    }
    else if (origin[1][0].IsEqual(origin[1][1], Precision::Confusion())) {
      //
      // the two moving axis are the same : it is equivalent that the AXIS_PLACEMENT alone
      //

      gp_Ax3 fixed_frame(a_point,matrix_vector[0][0]);
      gp_Ax3 moving_frame(a_point,matrix_vector[1][0]);
      
      vectorial_part.SetDisplacement(moving_frame,
				     fixed_frame) ;
      if (distance[0]*distance[0] >= distance[1]*distance[1]) {
	distance_ok = Standard_True;
	if (distance[0]*distance[0] - distance[1]*distance[1] > Precision::Confusion()) {
	  matrix_vector[0][1] = distance[1] * matrix_vector[0][1] 
	  + 
	  Sqrt(distance[0]*distance[0]-distance[1]*distance[1])*matrix_vector[0][1].Crossed(matrix_vector[0][0]);
	}
	else {
	  distance[0] = - distance[0];
	}
	matrix_vector[0][1].Normalize();
      }
      else 
	distance_ok = Standard_False;
      
      
    }
    else {
      for (jj =1 ; jj <=3 ; jj++) {
	vector[1][1].SetCoord(jj, origin[1][0].Coord(jj) - origin[1][1].Coord(jj));
      	vector[0][1].SetCoord(jj, origin[0][0].Coord(jj) - origin[0][1].Coord(jj));
      }   
      vector[1][1] = - vector[ii][1].Dot(matrix_vector[ii][0]) * matrix_vector[ii][0];
      vector[0][1] = (vector[0][1].Dot(matrix_vector[0][1]) + distance[1] + distance[0]) * matrix_vector[0][1];
      
      gp_Pnt projete;

      for (jj = 1; jj <= 3; jj++) {
	vector[1][1].SetCoord(jj, origin[1][0].Coord(jj) + vector[1][1].Coord(jj) - origin[1][1].Coord(jj));
	projete.SetCoord(jj, origin[0][0].Coord(jj) + vector[0][1].Coord(jj));
      }

      

      Standard_Real dis1 = vector[1][1].Magnitude(),
                    dis0 = projete.Distance(origin[0][0]);
 
      if (dis0 < dis1) {
	distance_ok = Standard_True;
	for (jj = 1; jj <= 3; jj++)
	projete.SetCoord(jj,projete.Coord(jj) 
			 +
			 Sqrt(dis1*dis1 - dis0*dis0)*matrix_vector[0][0].Crossed(matrix_vector[0][1]).Coord(jj)); 
	gp_Vec vec_solution(origin[0][0], projete);
	gp_Ax3 fixed_frame(a_point,
			   matrix_vector[0][0].Crossed(vec_solution),
			   matrix_vector[0][0]);
	
	gp_Ax3 moving_frame(a_point,
			    matrix_vector[1][0].Crossed(vector[1][1]),
			    matrix_vector[1][0]);
	vectorial_part.SetDisplacement(moving_frame,
				       fixed_frame) ;
      }
      else
	distance_ok = Standard_False;
    }
  }


  //
  // translation part of the Trsf 
  // we have (u(x0) - y0).s0 = d0 & (u(x1) - y1).s1 = d1
  // & (u(x) - v(x)) = k0*s0 + k1*s1
  //

  matrix_vector[0][1].Normalize();
  matrix_vector[0][0].Normalize();
  gp_Vec a_vector(origin[1][0].Transformed(vectorial_part), origin[0][0]),
         a_vector1(origin[1][1].Transformed(vectorial_part), origin[0][1]) ;
  gp_Vec a_vector3 = vectorial_part.TranslationPart();
  Standard_Real ro = matrix_vector[0][0].Dot(matrix_vector[0][1]);

  if (is_axis && Abs(ro >= Precision::Confusion())) {
    
    //
    // translation adapted to AXIS_PLACEMENT
    // NB : if ro = 0.0e0, which means the fixed axis and the fixed plane are parallele, no translation
    // can be found, the constraint are fully respected only if distances between moving axis and fixed 
    // elements (axis & plane) are the same.

    matrix_vector[1][1].Transform(vectorial_part);
    for (ii = 1 ; ii <= 3; ii++) 
      a_vector1.SetCoord(ii, origin[0][1].Coord(ii) - origin[1][1].Transformed(vectorial_part).Coord(ii) - a_vector.Coord(ii));
    a_vector3.Add(((distance[1] + a_vector1.Dot(matrix_vector[0][1])) / ro) * matrix_vector[0][0] + a_vector);
    if (Abs(distance[0]) >= Precision::Confusion()) {
      matrix_vector[1][1].Transform(vectorial_part);
      gp_Vec temp = matrix_vector[0][0].Crossed(matrix_vector[1][1]);
      temp.Cross(matrix_vector[0][0]);
      temp.Normalize();
      a_vector3.Add((distance[0]/(temp.Dot(matrix_vector[1][1]))) * matrix_vector[1][1]);
    }
  }
  else if (is_axis && distance_ok) {
    gp_Vec temp; 
    temp =  - matrix_vector[0][0].Dot(a_vector) * matrix_vector[0][0];
    a_vector3.Add(temp + a_vector + distance[0] * matrix_vector[0][1]);
  }
  
  else if (! is_axis) {
    
    // 
    // translation adapted to PLANE_PLACEMENT
    //

    Standard_Real k1 = distance[1] + a_vector1.Dot(matrix_vector[0][1]);
    Standard_Real k0;
    k0 = (distance[0] - distance[1]*ro + a_vector.Dot(matrix_vector[0][0])
	  - a_vector1.Dot(matrix_vector[0][1])*ro) / (1 - ro*ro);
    k1 = k1 - k0 * ro; 
    a_vector3.Add(k1*matrix_vector[0][1] + k0*matrix_vector[0][0]);
    
  }
    
  // update the Trsf and end SolveCase8

  vectorial_part.SetTranslationPart(a_vector3);
  myTrsf = vectorial_part;
  myHasSolution = Standard_True;

}

//=======================================================================
//function : SolveCase9()
//purpose : temporaire : only ANGULAR_PLACEMENT
//=======================================================================
void ShapePlacement_ConstraintAlgo::SolveCase9() {
  Standard_Integer 
    ii,
    jj,
    num_equations,
    index ;
  gp_Vec   matrix_vector[2][3],
  vector[2][3];
  gp_Pnt   origin[2][3] ;
  Standard_Real angle[3],
  tolerance_squared;
  tolerance_squared = myAngularTolerance * myAngularTolerance ;
  num_equations =
    myEquationStatus->Length() ;
  index = 0 ;
  Standard_Boolean fliped = Standard_False,
                   particular_case = Standard_False;
  ShapePlacement_ListIteratorOfListOfConstraint   anIterator ;
  
  
  for (ii = 1,
       anIterator.Initialize(myConstraints); 
       index < 3 && ii <= num_equations ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
     anIterator.Value() ;
    const ShapePlacement_Equation & an_equation =
     a_constraint.Equation() ;
    if (an_equation.Type() == ShapePlacement_A_ANGULAR_PLACEMENT 
	|| 
	an_equation.Type() == ShapePlacement_P_ANGULAR_PLACEMENT)   {
      matrix_vector[0][index] = an_equation.FixedNormal()  ;
      matrix_vector[1][index] = an_equation.MovingNormal() ;
      if (myEquationOrientation->Value(ii) == Reverse) {
	matrix_vector[1][index].Reverse() ;
      }
      matrix_vector[0][index].Normalize();
      matrix_vector[1][index].Normalize();
      origin[0][index] = an_equation.FixedPoint()  ;
      origin[1][index] = an_equation.MovingPoint() ;

      //
      // angles have to be positive and lower than PI/2
      // Furthermore, if they are greater than PI/2 we reverse moving element 
      // and we use PI - angle.
      //

      angle[index] = an_equation.Angle();
      if (angle[index] < 0) {
	angle[index] = Abs(angle[index]);
      }
      if (M_PI/2.0 - angle[index] < 0) {
	matrix_vector[1][index].Reverse();
	angle[index] = M_PI - angle[index];
      }

      //
      // as we use angle[1] to define a cone, it must not be equal to PI/2
      // If it is, but not angle[0], we permute the order of the constraints
      // If both angle[0] and angle[1] are equal to PI/2, it is a particular case.
      //

      if ((index == 1) && 
	  (Abs(M_PI/2.0 - angle[0]) > Precision::Angular())
	  &&
	  (Abs(M_PI/2.0 - angle[1]) <= Precision::Angular())) {
	
	gp_Vec temp;
	Standard_Real tempo;
	for (jj = 0; jj < 2; jj++) {
	  
	  temp = matrix_vector[jj][0];
	  matrix_vector[jj][0] = matrix_vector[jj][1];
	  matrix_vector[jj][1] = temp;
	}
	tempo = angle[0];
	angle[0] = angle[1];
	angle[1] = tempo;
      }
      else if ((index == 1)
	       && 
	       (Abs(M_PI/2.0 - angle[0]) <= Precision::Angular())
	       &&
	       (Abs(M_PI/2.0 - angle[1]) <= Precision::Angular())) {
	particular_case = Standard_True;
	
      }
      
      index += 1 ;
    }
  }

  //
  // placements ANGULAR_PLACEMENT
  // The used algorithm is describe in SPEC (G956)
  // It is based on cones and planes intersections.
  //
  gp_Pnt a_point = IsoBarycentre(myShapeToPosition);
  gp_Trsf vectorial_part;
  
  if (index == 1) {
    gp_Vec orientation;
    if (matrix_vector[0][0].CrossSquareMagnitude(matrix_vector[1][0]) 
	>= 
	tolerance_squared) 
      orientation = matrix_vector[0][0].Crossed(matrix_vector[1][0]);
    else {
      gp_Ax3 temp(a_point,matrix_vector[0][0]);
      orientation = temp.XDirection();
    }
    gp_Ax1 axe_rot(a_point,orientation);
    gp_Vec vector = matrix_vector[0][0].Rotated(axe_rot, angle[0]);

    gp_Ax3 fixed_frame(a_point, 
		       vector);
    gp_Ax3 moving_frame(a_point, 
			matrix_vector[1][0]);
    vectorial_part.SetDisplacement(moving_frame,fixed_frame);
  }
  if (index >= 2) {
    if (! particular_case) {
      //
      // step 1 : compatibility's check.
      //
      Standard_Real beta_min,beta_max,fixed_angle,moving_angle;
      fixed_angle = matrix_vector[0][1].Angle(matrix_vector[0][0]);
      moving_angle = matrix_vector[1][1].Angle(matrix_vector[1][0]);

      Standard_Real mov_semi_angle = moving_angle;
      if (M_PI/2.0 - moving_angle <= Precision::Angular()) { 
	mov_semi_angle = M_PI - moving_angle;
	if (M_PI/2.0 -fixed_angle <= Precision::Angular()) 
	  fliped = Standard_True;;
      }
      gp_Vec orientation;
      
      //
      // The following booleans are used to deal with particular cases, respectively :
      // - the 2 constraints involve the same fixed face or axis
      // - the 2 moving vectors are perpendicular
      // - the 2 constraints involve the same moving face or axis
      //
      Standard_Boolean same_fixed = matrix_vector[0][0].
	                            IsParallel(matrix_vector[0][1], Precision::Angular()),
                       moving_ortho = (Abs(moving_angle - M_PI/2.0e0) 
				       <= Precision::Angular()),
                       same_moving = matrix_vector[1][0].
	                            IsParallel(matrix_vector[1][1], Precision::Angular());
      if (!same_fixed) {
      
	 orientation = matrix_vector[0][0].Crossed(matrix_vector[0][1]);
	
	beta_min = fixed_angle - angle[0] - angle[1];
	if (beta_min < 0) 
	  beta_min = 0.0e0;
      }
      else {
	beta_min = Abs(angle[0] - angle[1]);
	gp_Ax3 axe(a_point,matrix_vector[0][0]);
	orientation = axe.XDirection();
      }
      beta_max = fixed_angle + angle[0] + angle[1];
      gp_Ax1 axe_rot(a_point,orientation);

      if ((mov_semi_angle >= beta_min && moving_angle <= beta_max) || same_moving) {
	
	//
	// step 2 : compute of v(s0);
	//
	Standard_Real init_angle;
	
	if (!same_fixed)
	  if (! same_moving)
	    init_angle = fixed_angle - mov_semi_angle - angle[1] 
	                 +     
		         (mov_semi_angle - beta_min)/2.0;
	  else
	    init_angle = fixed_angle - angle[1] 
	                 +
			 (angle[0] - fixed_angle + angle[1])/2.0;
	else
	  init_angle = (angle[0] + angle[1] - mov_semi_angle)/2.0e0;

	Standard_Integer sign = 1;
	if (fliped)
	  sign = -1;
	vector[0][0] = matrix_vector[0][0].Rotated(axe_rot,sign * init_angle);
	if (! same_moving) {
	  Standard_Real dist1 = vector[0][0].Magnitude()*Abs(Sin(init_angle)),
	  coeff = matrix_vector[0][0].Dot(vector[0][0]),
	  dist2 = coeff*matrix_vector[0][0].Magnitude()*Abs(Tan(angle[0])),
	  dist3 = Sqrt(dist2*dist2 - dist1*dist1);
	  for (ii = 1; ii <= 3; ii++) 
	    vector[0][0].SetCoord(ii,vector[0][0].Coord(ii) + 
				  dist3 * axe_rot.Direction().Coord(ii));
	}
	vector[0][0].Normalize();

	//
	// end of step 2 : v(s0) = vector[0][0]
	//
	
	//
	// step 3 : compute of v(s1)
	// In some particular cases, we can not compute needed cones 
	// then we use planes 
	//

	IntAna_QuadQuadGeo Intersection3d_1,Intersection3d_2;
	gp_Ax3 fixed_frame(a_point,matrix_vector[0][1]),
	       moving_frame(a_point,vector[0][0]);
	gp_Cone fixed_cone(fixed_frame, angle[1], 0.0e0);
	
	if (!same_moving) {
	  if (!moving_ortho) {
	    gp_Pnt a_point2;
	    for (ii = 1; ii <= 3; ii ++) 
	      a_point2.SetCoord(ii, a_point.Coord(ii) + matrix_vector[0][1].Coord(ii));
	    gp_Ax3 fixed_frame2(a_point2, matrix_vector[0][1]);
	    gp_Cone moving_cone(moving_frame, mov_semi_angle, 0.0e0);
	    gp_Pln fixed_plan(fixed_frame2);
	    Intersection3d_1.Perform(fixed_plan, fixed_cone, 
				     Precision::Angular(), Precision::Confusion());
	    Intersection3d_2.Perform(fixed_plan, moving_cone,
				     Precision::Angular(), Precision::Confusion());
	  }
	  else {
	    gp_Pln moving_plan(moving_frame);
	    Intersection3d_2.Perform(moving_plan, fixed_cone, 
				     Precision::Angular(), Precision::Confusion());
	  }
	}
	else {
	  gp_Pnt a_point2;
	  for (ii = 1; ii <= 3; ii ++) 
	    a_point2.SetCoord(ii, a_point.Coord(ii) + vector[0][0].Coord(ii));
	  gp_Ax3 fixed_frame2(a_point, matrix_vector[0][0]);

	  moving_frame.SetLocation(a_point2);
	  moving_frame.SetDirection(matrix_vector[0][1]);
	  gp_Cone fixed_cone2(fixed_frame2, angle[0], 0.0e0);
	  gp_Pln moving_plan(moving_frame);
	  Intersection3d_1.Perform(moving_plan, fixed_cone, 
				   Precision::Angular(), Precision::Confusion());
	  Intersection3d_2.Perform(moving_plan, fixed_cone2,
				   Precision::Angular(), Precision::Confusion());
	}
	if (!moving_ortho 
	    && 
	    (!Intersection3d_1.IsDone() || !Intersection3d_2.IsDone())) {
	  Standard_ConstructionError::Raise() ;  
	}
	else if (moving_ortho && !Intersection3d_2.IsDone()) {
	  Standard_ConstructionError::Raise() ;
	}
	gp_Circ cercle3d;
	
	if (!moving_ortho && Intersection3d_1.TypeInter() == IntAna_Circle) { 
	  cercle3d = Intersection3d_1.Circle(1);
	}
	else if (moving_ortho) {
	  gp_Ax2 ax_cerc(a_point, moving_frame.Direction(), moving_frame.XDirection());
	  cercle3d.SetPosition(ax_cerc);
	  cercle3d.SetRadius(1.0e0);
	}
	else
	  Standard_ConstructionError::Raise() ;  
	
	gp_Pnt zero (0.0e0,0.0e0,0.0e0);
	gp_Vec x(1.0e0,0.0e0,0.0e0),z(0.0e0,0.0e0,1.0e0);
	gp_Ax3 ref(zero,z,x),
	ax_circle(cercle3d.Position());
	gp_Trsf circle_to_ref;
	circle_to_ref.SetDisplacement(ax_circle,ref);
	gp_Pnt2d centre_cercle2d(0.0e0,0.0e0);
	gp_Vec2d vec2d1(1.0e0,0.0e0),vec2d2(0.0e0,1.0e0);
	gp_Ax22d axe2d(centre_cercle2d,vec2d1,vec2d2);
	gp_Circ2d cercle2d(axe2d, cercle3d.Radius());
	IntAna2d_AnaIntersection intersection2d;
	if (Intersection3d_2.TypeInter() == IntAna_Ellipse) {
	  gp_Elips ellipse3d = Intersection3d_2.Ellipse(1);
	  gp_Ax2 el_ax = ellipse3d.Position();
	  el_ax.Transform(circle_to_ref);
	  gp_Pnt2d el_pnt2d;
	  for (ii = 1 ; ii <= 2 ; ii++) {
	    vec2d1.SetCoord(ii, el_ax.XDirection().Coord(ii));
	    vec2d2.SetCoord(ii, el_ax.YDirection().Coord(ii));
	    el_pnt2d.SetCoord(ii, el_ax.Location().Coord(ii));
	  }
	  gp_Ax22d axe2d2(el_pnt2d,vec2d1,vec2d2);
	  gp_Elips2d elipse2d(axe2d2,ellipse3d.MajorRadius(),ellipse3d.MinorRadius());
	  IntAna2d_Conic elip_conic(elipse2d);
	  intersection2d.Perform(cercle2d,elip_conic);
	}
	
	else if (Intersection3d_2.TypeInter() == IntAna_Circle) {
	  gp_Circ cercle3d = Intersection3d_2.Circle(1);
	  gp_Ax2 ci_ax = cercle3d.Position();
	  ci_ax.Transform(circle_to_ref);
	  gp_Pnt2d ci_pnt2d;
	  for (ii = 1 ; ii <= 2 ; ii++) {
	    vec2d1.SetCoord(ii, ci_ax.XDirection().Coord(ii));
	    vec2d2.SetCoord(ii, ci_ax.YDirection().Coord(ii));
	    ci_pnt2d.SetCoord(ii, ci_ax.Location().Coord(ii));
	  }
	  gp_Ax22d axe2d2(ci_pnt2d,vec2d1,vec2d2);
	  gp_Circ2d cercle2d(axe2d2,cercle3d.Radius());
	  IntAna2d_Conic cercle_conic(cercle2d);
	  intersection2d.Perform(cercle2d,cercle_conic);
	}
	
	else if (Intersection3d_2.TypeInter() == IntAna_Parabola) {
	  gp_Parab parab3d = Intersection3d_2.Parabola(1);
	  gp_Ax2 pa_ax = parab3d.Position().Transformed(circle_to_ref);
	  gp_Pnt2d pa_pnt2d;
	  for (ii = 1; ii <= 2; ii++) {
	    pa_pnt2d.SetCoord(ii, pa_ax.Location().Coord(ii));
	    vec2d1.SetCoord(ii, pa_ax.XDirection().Coord(ii));
	    vec2d2.SetCoord(ii, pa_ax.YDirection().Coord(ii));
	  }
	  gp_Ax22d axe2d2bis(pa_pnt2d,vec2d1,vec2d2);
	  gp_Parab2d para2d(axe2d2bis,parab3d.Focal());
	  IntAna2d_Conic parab_conic(para2d);
	  intersection2d.Perform(cercle2d, parab_conic);
	}

	else if (Intersection3d_2.TypeInter() == IntAna_Hyperbola) {
	  gp_Hypr hyp3d = Intersection3d_2.Hyperbola(1);
	  gp_Ax2 hy_ax = hyp3d.Position().Transformed(circle_to_ref);
	  gp_Pnt2d hy_pnt2d;
	  for (ii = 1; ii <= 2; ii++) {
	    hy_pnt2d.SetCoord(ii, hy_ax.Location().Coord(ii));
	    vec2d1.SetCoord(ii, hy_ax.XDirection().Coord(ii));
	    vec2d2.SetCoord(ii, hy_ax.YDirection().Coord(ii));
	  }
	  gp_Ax22d axe2d2bis(hy_pnt2d,vec2d1,vec2d2);
	  gp_Hypr2d hyp2d(axe2d2bis,hyp3d.MajorRadius(),hyp3d.MinorRadius());
	  IntAna2d_Conic hyp_conic(hyp2d);
	  intersection2d.Perform(cercle2d, hyp_conic);
	}
	else if (Intersection3d_2.TypeInter() == IntAna_Line) {
	  gp_Lin ligne3d = Intersection3d_2.Line(1);
	  gp_Ax1 li_ax = ligne3d.Position();
	  li_ax.Transform(circle_to_ref);
	  gp_Pnt2d li_pnt2d;
	  for (ii = 1 ; ii <= 2 ; ii++) {
	    vec2d1.SetCoord(ii, li_ax.Direction().Coord(ii));
	    li_pnt2d.SetCoord(ii, li_ax.Location().Coord(ii));
	  }
	  gp_Ax2d axe1d2(li_pnt2d,vec2d1);
	  gp_Lin2d ligne2d(axe1d2);
	  IntAna2d_Conic ligne_conic(ligne2d);
	  intersection2d.Perform(cercle2d,ligne_conic);
	}
	else {
	  Standard_ConstructionError::Raise();
	}
	circle_to_ref.Invert();
	Standard_Boolean NotFound = Standard_True;
	gp_Pnt solution;
	if (intersection2d.IsEmpty()) {
	  Standard_ConstructionError::Raise();
	}
	ii = 1;
	
	//
	// In some cases, when we have an hyperbola, some false solutions are
	// added which we need to avoid
	//
	
	do {
	  for (jj = 1; jj <= 2 ; jj++)
	    solution.SetCoord(jj, intersection2d.Point(ii).Value().Coord(jj));
	  solution.SetCoord(3, 0.0e0);
	  solution.Transform(circle_to_ref);
	  for (jj =1 ; jj <= 3; jj++)
	    vector[0][1].SetCoord(jj, solution.Coord(jj) - a_point.Coord(jj));
	  vector[0][1].Normalize();
	  NotFound = (((!same_moving) 
		      && 
		      (Abs(vector[0][1].Angle(vector[0][0]) - moving_angle)
		      >= 
		      Precision::Angular()))
		      ||
		      (same_moving 
		       &&
		       (Abs(vector[0][1].Angle(matrix_vector[0][0]) - angle[0])
			>= 
			Precision::Angular())));
	  ii++;
	} while (ii <= intersection2d.NbPoints() && NotFound);
	
	if (!same_moving) {
	  vector[0][2] = vector[0][0].Crossed(vector[0][1]);
	  vector[0][2].Normalize();
	  vector[1][2] = matrix_vector[1][0].Crossed(matrix_vector[1][1]);
	  vector[1][2].Normalize();
	  gp_Ax3 fixed_framebis(a_point, 
				vector[0][2],
				vector[0][0]);
	  gp_Ax3 moving_framebis(a_point, 
				 vector[1][2],
				 matrix_vector[1][0]);
	  
	  vectorial_part.SetDisplacement(moving_framebis,fixed_framebis);
	}
	else {
	  gp_Ax3 fixed_framebis(a_point, 
				vector[0][1]);
	  gp_Ax3 moving_framebis(a_point, 
				 matrix_vector[1][0]);
	  
	  vectorial_part.SetDisplacement(moving_framebis,fixed_framebis);
	} 
      
      }
    }
    else {
      //
      // particular case : PI/2 angles : the algorithm used is similar to
      // the one used in PLANE-AXIS-PLACEMENT
      //
      gp_Vec a_vector1, a_vector2;
      
      vector[0][1] = matrix_vector[0][1];
      vector[0][2] = matrix_vector[0][0].Crossed(matrix_vector[0][1]);
      vector[0][2].Normalize();
      if (matrix_vector[1][0].CrossSquareMagnitude(matrix_vector[1][1])
	  <=
	  tolerance_squared) {
	gp_Ax3 axis(a_point, matrix_vector[1][0]);
	vector[1][2] = axis.XDirection();
	
      }
      else {
	vector[1][2] = matrix_vector[1][0].Crossed(matrix_vector[1][1]);
      }
      
      for (ii = 0; ii < index; ii++) {
	vector[1][ii] = vector[0][2].Crossed(matrix_vector[0][ii]);
      }
      Standard_Real dot1 = matrix_vector[1][0].Dot(matrix_vector[1][1]),
      dot0 = vector[1][0].Dot(vector[1][1]);
      Standard_Integer e0,e1;
      if ((Abs(dot1) < Abs(dot0)) || dot1 > 0.e0) {
	e0 = 1;
	if (dot0 > 0.e0) 
	  e1 = 1;
	else
	  e1 = -1;
      }
      else {
	e0 = -1;
	if (dot0 > 0.e0) 
	  e1 = -1;
	else 
	  e1 = 1;
      }
      Standard_Real a0Square = e0*(dot1 - e1*dot0)/(1 - e0*e1*dot0);
      if (Abs(1 - a0Square) <= Precision::Confusion()) {
	a_vector1 = Sqrt(a0Square) * vector[0][2];
	a_vector2 = Sqrt(a0Square) * e0 * vector[0][2];
      }
      else if (Abs(a0Square) <= Precision::Confusion()) {
	a_vector1 = Sqrt(1 -a0Square) * vector[1][0];
	a_vector2 = e1*Sqrt(1- a0Square) * vector[1][1];
      } 
      else {
	a_vector1 = Sqrt(a0Square) * vector[0][2] + Sqrt(1 -a0Square) * vector[1][0];
	a_vector2 = e0*Sqrt(a0Square) * vector[0][2] + e1*Sqrt(1- a0Square) * vector[1][1];
      }
      if (a_vector1.CrossSquareMagnitude(a_vector2) >= tolerance_squared) {
	vector[0][0] = a_vector1.Crossed(a_vector2);
      }
      else {
	gp_Ax3 axis(a_point, a_vector1);
	vector[0][0] = axis.XDirection();
      }
      
      gp_Ax3   moving_frame(a_point,
			    vector[1][2],
			    matrix_vector[1][0]);
      
      
      
      gp_Ax3   fixed_frame(a_point,
			   vector[0][0],
			   a_vector1);
      
      vectorial_part.SetDisplacement(moving_frame,
				     fixed_frame) ;
    }
  }

  // 
  // end of solve_case9
  //
  myTrsf = vectorial_part;
  myHasSolution = Standard_True;

}



//=======================================================================
//function : SolveCase10()
//purpose : temporaire : 1 ANGULAR_PLACEMENT and 1 AXIS_AXIS_PLACEMENT
//                       or 1 PLAN_PLACEMENT
//=======================================================================

void ShapePlacement_ConstraintAlgo::SolveCase10() {
  Standard_Integer 
    ii,
    jj,
    num_equations,
    index ;
  gp_Vec   matrix_vector[2][3],
           vector[2][3], 
           a_vector;
  gp_Pnt   origin[2][3] ;
  Standard_Real angle=0.,distance=0.,coeff=0.,
  tolerance_squared;
  tolerance_squared = myAngularTolerance * myAngularTolerance ;
  num_equations =
    myEquationStatus->Length() ;
  index = 0 ;
  Standard_Boolean fliped = Standard_False,
                   particular_case = Standard_False,
                   moving_ortho= Standard_False,
                   isaxis = Standard_False;
  ShapePlacement_ListIteratorOfListOfConstraint   anIterator ;
  
  //
  // angular constraint (there should be only one)
  //
  
  for (ii = 1,
       anIterator.Initialize(myConstraints); 
       index < 3 && ii <= num_equations ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
     anIterator.Value() ;
    const ShapePlacement_Equation & an_equation =
     a_constraint.Equation() ;
    if (an_equation.Type() == ShapePlacement_A_ANGULAR_PLACEMENT 
	|| 
	an_equation.Type() == ShapePlacement_P_ANGULAR_PLACEMENT)   {
      matrix_vector[0][index] = an_equation.FixedNormal()  ;
      matrix_vector[1][index] = an_equation.MovingNormal() ;
      if (myEquationOrientation->Value(ii) == Reverse) {
	matrix_vector[1][index].Reverse() ;
      }

      matrix_vector[0][index].Normalize();
      matrix_vector[1][index].Normalize();
      origin[0][index] = an_equation.FixedPoint()  ;
      origin[1][index] = an_equation.MovingPoint() ;
      angle = an_equation.Angle();
      
      if (angle < 0) {
	angle = Abs(angle);;
      }
      if (M_PI/2.0 - angle < 0) {
	matrix_vector[1][index].Reverse();
	angle = M_PI - angle;
      }
      index += 1 ;
    }
  }
  //
  // plane or axis constraint (there should be only one)
  //
  for (ii = 1,
       anIterator.Initialize(myConstraints); 
       index < 3 && ii <= num_equations ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
     anIterator.Value() ;
    const ShapePlacement_Equation & an_equation =
     a_constraint.Equation() ;
    if (an_equation.Type() == ShapePlacement_PLANE_PLACEMENT)   {
      matrix_vector[0][index] = an_equation.FixedNormal()  ;
      matrix_vector[1][index] = an_equation.MovingNormal() ;
      if (myEquationOrientation->Value(ii) == Reverse) {
	matrix_vector[1][index].Reverse() ;
      }
      matrix_vector[0][index].Normalize();
      matrix_vector[1][index].Normalize();
      origin[0][index] = an_equation.FixedPoint();
      origin[1][index] = an_equation.MovingPoint();
      distance = an_equation.SignedDistance();
      index += 1;
    }
    if (an_equation.Type() == ShapePlacement_AXIS_AXIS_PLACEMENT)   {
      matrix_vector[0][index] = an_equation.FixedDirection()  ;
      matrix_vector[1][index] = an_equation.MovingDirection() ;
      if (myEquationOrientation->Value(ii) == Reverse) {
	matrix_vector[1][index].Reverse() ;
      }
      matrix_vector[0][index].Normalize();
      matrix_vector[1][index].Normalize();
      origin[0][index] = an_equation.FixedPoint();
      origin[1][index] = an_equation.MovingPoint();
      distance = an_equation.SignedDistance();
      index += 1;
      isaxis = Standard_True;
    }
  }
  
//
// The algorithm and particular cases are quite the same than in SolveCase9()
//

  gp_Pnt a_point = IsoBarycentre(myShapeToPosition);
  gp_Trsf vectorial_part;
  particular_case = (Abs(angle - M_PI/2.0) <= Precision::Angular());
  if (index == 2) {
    if (! particular_case) {
      Standard_Real beta_min,beta_max,fixed_angle,moving_angle;
      fixed_angle = matrix_vector[0][1].Angle(matrix_vector[0][0]);
      moving_angle = matrix_vector[1][1].Angle(matrix_vector[1][0]);
      Standard_Real mov_semi_angle = moving_angle,
      fix_semi_angle = fixed_angle;
      if (M_PI/2.0 - moving_angle <= Precision::Angular()) { 
	mov_semi_angle = M_PI - moving_angle;
	if (M_PI/2.0 -fixed_angle < Precision::Angular()) 
	  fliped = Standard_True;
      }
      
      if (M_PI/2.0 -fixed_angle <= Precision::Angular()) 
	fix_semi_angle = M_PI- fixed_angle;
      
      
      moving_ortho = (Abs(moving_angle - M_PI/2.0) <= Precision::Angular());
      gp_Vec orientation = matrix_vector[0][0].Crossed(matrix_vector[0][1]);
      
      beta_min = fix_semi_angle - angle;
      if (beta_min < 0) 
	beta_min = 0.0e0;
      
      beta_max = fixed_angle + angle;
      gp_Ax1 axe_rot(a_point,orientation);
      if (mov_semi_angle >= beta_min && moving_angle <= beta_max) {
	Standard_Real init_angle;
	
	init_angle = fixed_angle + (angle - fixed_angle)/2.0;
	Standard_Integer sign = 1;
	if (fliped)
	  sign = -1;
	    

	vector[0][0] = matrix_vector[0][0].Rotated(axe_rot,sign * init_angle);
	vector[0][0].Normalize();
	
	//
	// step 3 : compute of v(s1)
	//
	IntAna_QuadQuadGeo Intersection3d_1,Intersection3d_2;
	gp_Ax3 fixed_frame2(a_point, matrix_vector[0][0]),
	fixed_frame(a_point, matrix_vector[0][1]);
      gp_Cone fixed_cone2(fixed_frame2, angle, 0.0e0);
	if (! moving_ortho) {
	  gp_Pnt a_point2;
	  for (ii = 1; ii <= 3; ii ++) 
	    a_point2.SetCoord(ii, a_point.Coord(ii) + vector[0][0].Coord(ii));
	  gp_Ax3 moving_frame(a_point2,matrix_vector[0][1]);
	  gp_Pln moving_plan(moving_frame);
	  gp_Cone fixed_cone(fixed_frame, mov_semi_angle, 0.0e0);
	  Intersection3d_1.Perform(moving_plan, fixed_cone, 
				   Precision::Angular(), Precision::Confusion());
	  Intersection3d_2.Perform(moving_plan, fixed_cone2,
				   Precision::Angular(), Precision::Confusion());
	}
      else {
	gp_Pln fixed_plan(fixed_frame);
	Intersection3d_2.Perform(fixed_plan, fixed_cone2,
				 Precision::Angular(), Precision::Confusion());
      }

	if (!moving_ortho 
	    &&
	    (!Intersection3d_1.IsDone() || !Intersection3d_2.IsDone())) {
	  Standard_ConstructionError::Raise() ;  
	}
	else if (moving_ortho && !Intersection3d_2.IsDone()) {
	Standard_ConstructionError::Raise() ;
      }
	gp_Circ cercle3d;
	
	if (!moving_ortho && Intersection3d_1.TypeInter() == IntAna_Circle) { 
	  cercle3d = Intersection3d_1.Circle(1);
	}
	else if (moving_ortho) {
	  gp_Ax2 ax_cerc(a_point, fixed_frame.Direction(), fixed_frame.XDirection());
	  cercle3d.SetPosition(ax_cerc);
	  cercle3d.SetRadius(1.0e0);
	}
	else
	  Standard_ConstructionError::Raise();  
	
	gp_Pnt zero (0.0e0,0.0e0,0.0e0);
	gp_Vec x(1.0e0,0.0e0,0.0e0),z(0.0e0,0.0e0,1.0e0);
	gp_Ax3 ref(zero,z,x),
	ax_circle(cercle3d.Position());
	gp_Trsf circle_to_ref;
	circle_to_ref.SetDisplacement(ax_circle,ref);
	gp_Pnt2d centre_cercle2d(0.0e0,0.0e0);
	gp_Vec2d vec2d1(1.0e0,0.0e0),vec2d2(0.0e0,1.0e0);
	gp_Ax22d axe2d(centre_cercle2d,vec2d1,vec2d2);
	gp_Circ2d cercle2d(axe2d, cercle3d.Radius());
	IntAna2d_AnaIntersection intersection2d;
	if (Intersection3d_2.TypeInter() == IntAna_Ellipse) {
	  gp_Elips ellipse3d = Intersection3d_2.Ellipse(1);
	  gp_Ax2 el_ax = ellipse3d.Position();
	  el_ax.Transform(circle_to_ref);
	  gp_Pnt2d el_pnt2d;
	  for (ii = 1 ; ii <= 2 ; ii++) {
	    vec2d1.SetCoord(ii, el_ax.XDirection().Coord(ii));
	    vec2d2.SetCoord(ii, el_ax.YDirection().Coord(ii));
	    el_pnt2d.SetCoord(ii, el_ax.Location().Coord(ii));
	  }
	  gp_Ax22d axe2d2(el_pnt2d,vec2d1,vec2d2);
	  gp_Elips2d elipse2d(axe2d2,ellipse3d.MajorRadius(),ellipse3d.MinorRadius());
	  IntAna2d_Conic elip_conic(elipse2d);
	  intersection2d.Perform(cercle2d,elip_conic);
	}
	
	else if (Intersection3d_2.TypeInter() == IntAna_Circle) {
	  gp_Circ cercle3d = Intersection3d_2.Circle(1);
	  gp_Ax2 ci_ax = cercle3d.Position();
	  ci_ax.Transform(circle_to_ref);
	  gp_Pnt2d ci_pnt2d;
	  for (ii = 1 ; ii <= 2 ; ii++) {
	    vec2d1.SetCoord(ii, ci_ax.XDirection().Coord(ii));
	    vec2d2.SetCoord(ii, ci_ax.YDirection().Coord(ii));
	    ci_pnt2d.SetCoord(ii, ci_ax.Location().Coord(ii));
	  }
	  gp_Ax22d axe2d2(ci_pnt2d,vec2d1,vec2d2);
	  gp_Circ2d cercle2d(axe2d2,cercle3d.Radius());
	  IntAna2d_Conic cercle_conic(cercle2d);
	  intersection2d.Perform(cercle2d,cercle_conic);
	}
	
	else if (Intersection3d_2.TypeInter() == IntAna_Parabola) {
	  gp_Parab parab3d = Intersection3d_2.Parabola(1);
	  gp_Ax2 pa_ax = parab3d.Position().Transformed(circle_to_ref);
	  gp_Pnt2d pa_pnt2d;
	  for (ii = 1; ii <= 2; ii++) {
	    pa_pnt2d.SetCoord(ii, pa_ax.Location().Coord(ii));
	    vec2d1.SetCoord(ii, pa_ax.XDirection().Coord(ii));
	    vec2d2.SetCoord(ii, pa_ax.YDirection().Coord(ii));
	  }
	  gp_Ax22d axe2d2bis(pa_pnt2d,vec2d1,vec2d2);
	  gp_Parab2d para2d(axe2d2bis,parab3d.Focal());
	  IntAna2d_Conic parab_conic(para2d);
	  intersection2d.Perform(cercle2d, parab_conic);
	}
	
	else if (Intersection3d_2.TypeInter() == IntAna_Hyperbola) {
	  gp_Hypr hyp3d = Intersection3d_2.Hyperbola(1);
	  gp_Ax2 hy_ax = hyp3d.Position().Transformed(circle_to_ref);
	  gp_Pnt2d hy_pnt2d;
	  for (ii = 1; ii <= 2; ii++) {
	    hy_pnt2d.SetCoord(ii, hy_ax.Location().Coord(ii));
	    vec2d1.SetCoord(ii, hy_ax.XDirection().Coord(ii));
	    vec2d2.SetCoord(ii, hy_ax.YDirection().Coord(ii));
	  }
	  gp_Ax22d axe2d2bis(hy_pnt2d,vec2d1,vec2d2);
	  gp_Hypr2d hyp2d(axe2d2bis,hyp3d.MajorRadius(),hyp3d.MinorRadius());
	  IntAna2d_Conic hyp_conic(hyp2d);
	  intersection2d.Perform(cercle2d, hyp_conic);
	}
	else if (Intersection3d_2.TypeInter() == IntAna_Line) {
	  gp_Lin ligne3d = Intersection3d_2.Line(1);
	  gp_Ax1 li_ax = ligne3d.Position();
	  li_ax.Transform(circle_to_ref);
	  gp_Pnt2d li_pnt2d;
	  for (ii = 1 ; ii <= 2 ; ii++) {
	    vec2d1.SetCoord(ii, li_ax.Direction().Coord(ii));
	    li_pnt2d.SetCoord(ii, li_ax.Location().Coord(ii));
	  }
	  gp_Ax2d axe1d2(li_pnt2d,vec2d1);
	  gp_Lin2d ligne2d(axe1d2);
	  IntAna2d_Conic ligne_conic(ligne2d);
	  intersection2d.Perform(cercle2d,ligne_conic);
	}
	else {
	  Standard_ConstructionError::Raise();
	}
	circle_to_ref.Invert();
	Standard_Boolean NotFound = Standard_True;
	gp_Pnt solution;
	if (intersection2d.IsEmpty()) {
	  Standard_ConstructionError::Raise();
	}
	ii = 1;
	do {
	  for (jj = 1; jj <= 2 ; jj++)
	    solution.SetCoord(jj, intersection2d.Point(ii).Value().Coord(jj));
	  solution.SetCoord(3, 0.0e0);
	  solution.Transform(circle_to_ref);
	  for (jj =1 ; jj <= 3; jj++)
	    vector[0][1].SetCoord(jj, solution.Coord(jj) - a_point.Coord(jj));
	  vector[0][1].Normalize();
	  
	  NotFound = (Abs(vector[0][1].Angle(matrix_vector[0][0]) - moving_angle)
		      >= 
		      Precision::Angular());
	  ii++;
	} while (ii <= intersection2d.NbPoints());
	
	  vector[0][2] = vector[0][1].Crossed(matrix_vector[0][1]);
	  vector[0][2].Normalize();
	  vector[1][2] = matrix_vector[1][0].Crossed(matrix_vector[1][1]);
	  vector[1][2].Normalize();
	  gp_Ax3 fixed_framebis(a_point, 
				vector[0][2],
				vector[0][1]);
	  gp_Ax3 moving_framebis(a_point, 
				 vector[1][2],
				 matrix_vector[1][0]);
	  
	  vectorial_part.SetDisplacement(moving_framebis,fixed_framebis);
	 
      }
    }
    else {
      if (Abs(matrix_vector[1][0].Dot(matrix_vector[1][1])) >= Precision::Angular()) {
	gp_Vec another_vector = matrix_vector[0][1].
	                        Crossed(matrix_vector[0][0]).Normalized(),
	       another_vector2 = matrix_vector[0][0].Crossed(another_vector);
	
	Standard_Real coeff = matrix_vector[1][1].Dot(matrix_vector[1][0])
	                      /
			      matrix_vector[0][1].Dot(another_vector2);
	if (Abs(1 - coeff*coeff) >= Precision::Confusion())
	  another_vector = coeff * another_vector2 
	                   +
			   Sqrt(1 - coeff*coeff) * another_vector;
	else 
	  another_vector = coeff * another_vector2;

	gp_Ax3 fixed_frame(a_point,
			   matrix_vector[0][1].Crossed(another_vector),
			   matrix_vector[0][1]);
	
	gp_Ax3 moving_frame(a_point,
			    matrix_vector[1][1].Crossed(matrix_vector[1][0]),
			    matrix_vector[1][1]);
	
	
	vectorial_part.SetDisplacement(moving_frame,
				       fixed_frame) ;
      }
      else {
	for (ii = 0 ; ii < 2 ; ii++) {
	  vector[ii][0] = matrix_vector[ii][1] ;
	  vector[ii][2] = matrix_vector[ii][1].Crossed(matrix_vector[ii][0]) ;
	  vector[ii][1] = vector[ii][2].Crossed(matrix_vector[ii][1]) ;
	}
	vector[1][2] = matrix_vector[1][0];
	
	gp_Ax3   fixed_frame(a_point,
			     vector[0][2],
			     vector[0][1]) ;
	gp_Ax3   moving_frame(a_point,
			      vector[1][2],
			      vector[1][1]) ;
	
	vectorial_part.SetDisplacement(moving_frame,
				       fixed_frame) ;
	
      }
    }
  }
  //
  // Translation part of displacement due to PLANE or AXIS constraints
  //

  a_point = origin[1][1].Transformed(vectorial_part);
   
  if (! isaxis) {
    for (ii = 1 ; ii <= 3 ; ii++) {
      a_vector.SetCoord(ii, origin[0][1].Coord(ii) - a_point.Coord(ii)) ;
    }
    coeff = a_vector.Dot(matrix_vector[0][1]) + distance;
    
    a_vector = vectorial_part.TranslationPart();
    a_vector.Add(coeff * matrix_vector[0][1]);
    vectorial_part.SetTranslationPart(a_vector);
  }
  else {
    for (ii = 1 ; ii <= 3 ; ii++) {
      a_vector.SetCoord(ii, origin[0][1].Coord(ii) 
			-
			origin[1][1].Transformed(vectorial_part).Coord(ii)) ;
    }
    coeff = - a_vector.Dot(matrix_vector[0][1]) ;
    for (ii = 1 ; ii <= 3 ; ii++) {
      a_vector.SetCoord(ii, coeff * matrix_vector[0][1].Coord(ii) + 
			origin[0][1].Coord(ii)) ;
    } 

    for (ii = 1 ; ii <= 3 ; ii++) {
      a_vector.SetCoord(ii, 
			a_vector.Coord(ii) - a_point.Coord(ii)) ;
    }
  
    static Standard_Real signbis = 1.0e0;

    if ( Abs(a_vector.Magnitude()) > Precision::Confusion()) {
      a_vector = a_vector - signbis * distance * a_vector.Normalized();
    }
    else {
      gp_Ax3 axis(origin[0][1], matrix_vector[0][1]);
      a_vector = axis.XDirection();
      a_vector = distance * a_vector.Normalized();
    }
    if (distance < 0.0e0)  
      signbis = -1.0e0;
    else
      signbis = 1.0e0;
    a_vector.Add(vectorial_part.TranslationPart());
    vectorial_part.SetTranslationPart(a_vector);
  }
  myTrsf = vectorial_part;
  myHasSolution = Standard_True;

}



//=======================================================================
//function : SolveCase11()
//purpose : 1 ANGULAR_PLACEMENT and 1 PLANE_AXIS_PLACEMENT
//                      
//=======================================================================
void ShapePlacement_ConstraintAlgo::SolveCase11() {
  Standard_Integer 
    ii,
    jj,
    num_equations,
    index ;
  gp_Vec   matrix_vector[2][3],
  vector[2][3],
  a_vector;
  gp_Pnt   origin[2][3] ;
  Standard_Real angle[2],
  tolerance_squared,
  distance=0.;
  tolerance_squared = myAngularTolerance * myAngularTolerance ;
  num_equations =
    myEquationStatus->Length() ;
  index = 0 ;
  Standard_Boolean fliped = Standard_False,
                   particular_case = Standard_False;
  ShapePlacement_ListIteratorOfListOfConstraint   anIterator ;
  for (ii = 1,
       anIterator.Initialize(myConstraints); 
       index < 3 && ii <= num_equations ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
      anIterator.Value() ;
    const ShapePlacement_Equation & an_equation =
      a_constraint.Equation() ;
    
    if (an_equation.Type() == ShapePlacement_PLANE_AXIS_PLACEMENT)   {
      matrix_vector[0][index] = an_equation.FixedNormal()  ;
      matrix_vector[1][index] = an_equation.MovingDirection() ;
      if (myEquationOrientation->Value(ii) == Reverse) {
	matrix_vector[1][index].Reverse() ;
      }
      matrix_vector[0][index].Normalize();
      matrix_vector[1][index].Normalize();
      origin[0][index] = an_equation.FixedPoint()  ;
      origin[1][index] = an_equation.MovingPoint() ;
      angle[index] = 1.5707963267949;
      distance = an_equation.SignedDistance();
      index += 1 ;
    }
  }
  
  
  for (ii = 1,
       anIterator.Initialize(myConstraints); 
       index < 3 && ii <= num_equations ; ii++,
       anIterator.Next()) {
    ShapePlacement_Constraint & a_constraint = 
     anIterator.Value() ;
    const ShapePlacement_Equation & an_equation =
     a_constraint.Equation() ;
    if (an_equation.Type() == ShapePlacement_A_ANGULAR_PLACEMENT 
	|| 
	an_equation.Type() == ShapePlacement_P_ANGULAR_PLACEMENT)   {
      matrix_vector[0][index] = an_equation.FixedNormal()  ;
      matrix_vector[1][index] = an_equation.MovingNormal() ;
      if (myEquationOrientation->Value(ii) == Reverse) {
	matrix_vector[1][index].Reverse() ;
      }
      matrix_vector[0][index].Normalize();
      matrix_vector[1][index].Normalize();
      origin[0][index] = an_equation.FixedPoint()  ;
      origin[1][index] = an_equation.MovingPoint() ;

      //
      // angles have to be positive and lower than PI/2
      // Furthermore, if they are greater than PI/2 we reverse moving element 
      // and we use PI - angle.
      //

      angle[index] = an_equation.Angle();
      if (angle[index] < 0) {
	angle[index] = Abs(angle[index]);
      }
      if (M_PI/2.0 - angle[index] < 0) {
	matrix_vector[1][index].Reverse();
	angle[index] = M_PI - angle[index];
      }

      //
      // as we use angle[1] to define a cone, it must not be equal to PI/2
      // If it is, we have a particular case.
      //
      if ((index == 1)
	  && 
	  (Abs(M_PI/2.0 - angle[1]) <= Precision::Angular())) {
	particular_case = Standard_True;
	
      }
      
      index += 1 ;
    }
  }

  gp_Pnt a_point = IsoBarycentre(myShapeToPosition);
  gp_Trsf vectorial_part;
  

  if (index == 2) {
    if (! particular_case) {
      //
      // step 1 : compatibility's check.
      //
      Standard_Real beta_min,beta_max,fixed_angle,moving_angle;
      fixed_angle = matrix_vector[0][1].Angle(matrix_vector[0][0]);
      moving_angle = matrix_vector[1][1].Angle(matrix_vector[1][0]);

      Standard_Real mov_semi_angle = moving_angle;
      if (M_PI/2.0 - moving_angle <= Precision::Angular()) { 
	mov_semi_angle = M_PI - moving_angle;
	if (M_PI/2.0 -fixed_angle <= Precision::Angular()) 
	  fliped = Standard_True;;
      }
      gp_Vec orientation;
      
      //
      // The following booleans are used to deal with particular cases, respectively :
      // - the 2 constraints involve the same fixed face or axis
      // - the 2 moving vectors are perpendicular
      // - the 2 constraints involve the same moving face or axis
      //
      Standard_Boolean same_fixed = matrix_vector[0][0].
	                            IsParallel(matrix_vector[0][1], Precision::Angular()),
                       moving_ortho = (Abs(moving_angle - M_PI/2.0e0) 
				       <= Precision::Angular()),
                       same_moving = matrix_vector[1][0].
	                            IsParallel(matrix_vector[1][1], Precision::Angular());
      if (!same_fixed) {
      
	 orientation = matrix_vector[0][0].Crossed(matrix_vector[0][1]);
	
	beta_min = fixed_angle - angle[0] - angle[1];
	if (beta_min < 0) 
	  beta_min = 0.0e0;
      }
      else {
	beta_min = Abs(angle[0] - angle[1]);
	gp_Ax3 axe(a_point,matrix_vector[0][0]);
	orientation = axe.XDirection();
      }
      beta_max = fixed_angle + angle[0] + angle[1];
      gp_Ax1 axe_rot(a_point,orientation);

      if ((mov_semi_angle >= beta_min && moving_angle <= beta_max) || same_moving) {
	
	//
	// step 2 : compute of v(s0);
	//
	Standard_Real init_angle;
	
	if (!same_fixed)
	  if (! same_moving)
	    init_angle = fixed_angle - mov_semi_angle - angle[1] 
	                 +     
		         (mov_semi_angle - beta_min)/2.0;
	  else
	    init_angle = fixed_angle - angle[1] 
	                 +
			 (angle[0] - fixed_angle + angle[1])/2.0;
	else
	  init_angle = (angle[0] + angle[1] - mov_semi_angle)/2.0e0;

	Standard_Integer sign = 1;
	if (fliped)
	  sign = -1;
	vector[0][0] = matrix_vector[0][0].Rotated(axe_rot,sign * init_angle);
	if (! same_moving) {
	  Standard_Real dist1 = vector[0][0].Magnitude()*Abs(Sin(init_angle)),
	  coeff = matrix_vector[0][0].Dot(vector[0][0]),
	  dist2 = coeff*matrix_vector[0][0].Magnitude()*Abs(Tan(angle[0])),
	  dist3 = Sqrt(dist2*dist2 - dist1*dist1);
	  for (ii = 1; ii <= 3; ii++) 
	    vector[0][0].SetCoord(ii,vector[0][0].Coord(ii) + 
				  dist3 * axe_rot.Direction().Coord(ii));
	}
	vector[0][0].Normalize();

	//
	// end of step 2 : v(s0) = vector[0][0]
	//
	
	//
	// step 3 : compute of v(s1)
	// In some particular cases, we can not compute needed cones 
	// then we use planes 
	//

	IntAna_QuadQuadGeo Intersection3d_1,Intersection3d_2;
	gp_Ax3 fixed_frame(a_point,matrix_vector[0][1]),
	       moving_frame(a_point,vector[0][0]);
	gp_Cone fixed_cone(fixed_frame, angle[1], 0.0e0);
	
	if (!same_moving) {
	  if (!moving_ortho) {
	    gp_Pnt a_point2;
	    for (ii = 1; ii <= 3; ii ++) 
	      a_point2.SetCoord(ii, a_point.Coord(ii) + matrix_vector[0][1].Coord(ii));
	    gp_Ax3 fixed_frame2(a_point2, matrix_vector[0][1]);
	    gp_Cone moving_cone(moving_frame, mov_semi_angle, 0.0e0);
	    gp_Pln fixed_plan(fixed_frame2);
	    Intersection3d_1.Perform(fixed_plan, fixed_cone, 
				     Precision::Angular(), Precision::Confusion());
	    Intersection3d_2.Perform(fixed_plan, moving_cone,
				     Precision::Angular(), Precision::Confusion());
	  }
	  else {
	    gp_Pln moving_plan(moving_frame);
	    Intersection3d_2.Perform(moving_plan, fixed_cone, 
				     Precision::Angular(), Precision::Confusion());
	  }
	}
	else {
	  gp_Pnt a_point2;
	  for (ii = 1; ii <= 3; ii ++) 
	    a_point2.SetCoord(ii, a_point.Coord(ii) + vector[0][0].Coord(ii));
	  gp_Ax3 fixed_frame2(a_point, matrix_vector[0][0]);

	  moving_frame.SetLocation(a_point2);
	  moving_frame.SetDirection(matrix_vector[0][1]);
	  gp_Cone fixed_cone2(fixed_frame2, angle[0], 0.0e0);
	  gp_Pln moving_plan(moving_frame);
	  Intersection3d_1.Perform(moving_plan, fixed_cone, 
				   Precision::Angular(), Precision::Confusion());
	  Intersection3d_2.Perform(moving_plan, fixed_cone2,
				   Precision::Angular(), Precision::Confusion());
	}
	if (!moving_ortho 
	    && 
	    (!Intersection3d_1.IsDone() || !Intersection3d_2.IsDone())) {
	  Standard_ConstructionError::Raise() ;  
	}
	else if (moving_ortho && !Intersection3d_2.IsDone()) {
	  Standard_ConstructionError::Raise() ;
	}
	gp_Circ cercle3d;
	
	if (!moving_ortho && Intersection3d_1.TypeInter() == IntAna_Circle) { 
	  cercle3d = Intersection3d_1.Circle(1);
	}
	else if (moving_ortho) {
	  gp_Ax2 ax_cerc(a_point, moving_frame.Direction(), moving_frame.XDirection());
	  cercle3d.SetPosition(ax_cerc);
	  cercle3d.SetRadius(1.0e0);
	}
	else
	  Standard_ConstructionError::Raise() ;  
	
	gp_Pnt zero (0.0e0,0.0e0,0.0e0);
	gp_Vec x(1.0e0,0.0e0,0.0e0),z(0.0e0,0.0e0,1.0e0);
	gp_Ax3 ref(zero,z,x),
	ax_circle(cercle3d.Position());
	gp_Trsf circle_to_ref;
	circle_to_ref.SetDisplacement(ax_circle,ref);
	gp_Pnt2d centre_cercle2d(0.0e0,0.0e0);
	gp_Vec2d vec2d1(1.0e0,0.0e0),vec2d2(0.0e0,1.0e0);
	gp_Ax22d axe2d(centre_cercle2d,vec2d1,vec2d2);
	gp_Circ2d cercle2d(axe2d, cercle3d.Radius());
	IntAna2d_AnaIntersection intersection2d;
	if (Intersection3d_2.TypeInter() == IntAna_Ellipse) {
	  gp_Elips ellipse3d = Intersection3d_2.Ellipse(1);
	  gp_Ax2 el_ax = ellipse3d.Position();
	  el_ax.Transform(circle_to_ref);
	  gp_Pnt2d el_pnt2d;
	  for (ii = 1 ; ii <= 2 ; ii++) {
	    vec2d1.SetCoord(ii, el_ax.XDirection().Coord(ii));
	    vec2d2.SetCoord(ii, el_ax.YDirection().Coord(ii));
	    el_pnt2d.SetCoord(ii, el_ax.Location().Coord(ii));
	  }
	  gp_Ax22d axe2d2(el_pnt2d,vec2d1,vec2d2);
	  gp_Elips2d elipse2d(axe2d2,ellipse3d.MajorRadius(),ellipse3d.MinorRadius());
	  IntAna2d_Conic elip_conic(elipse2d);
	  intersection2d.Perform(cercle2d,elip_conic);
	}
	
	else if (Intersection3d_2.TypeInter() == IntAna_Circle) {
	  gp_Circ cercle3d = Intersection3d_2.Circle(1);
	  gp_Ax2 ci_ax = cercle3d.Position();
	  ci_ax.Transform(circle_to_ref);
	  gp_Pnt2d ci_pnt2d;
	  for (ii = 1 ; ii <= 2 ; ii++) {
	    vec2d1.SetCoord(ii, ci_ax.XDirection().Coord(ii));
	    vec2d2.SetCoord(ii, ci_ax.YDirection().Coord(ii));
	    ci_pnt2d.SetCoord(ii, ci_ax.Location().Coord(ii));
	  }
	  gp_Ax22d axe2d2(ci_pnt2d,vec2d1,vec2d2);
	  gp_Circ2d cercle2d(axe2d2,cercle3d.Radius());
	  IntAna2d_Conic cercle_conic(cercle2d);
	  intersection2d.Perform(cercle2d,cercle_conic);
	}
	
	else if (Intersection3d_2.TypeInter() == IntAna_Parabola) {
	  gp_Parab parab3d = Intersection3d_2.Parabola(1);
	  gp_Ax2 pa_ax = parab3d.Position().Transformed(circle_to_ref);
	  gp_Pnt2d pa_pnt2d;
	  for (ii = 1; ii <= 2; ii++) {
	    pa_pnt2d.SetCoord(ii, pa_ax.Location().Coord(ii));
	    vec2d1.SetCoord(ii, pa_ax.XDirection().Coord(ii));
	    vec2d2.SetCoord(ii, pa_ax.YDirection().Coord(ii));
	  }
	  gp_Ax22d axe2d2bis(pa_pnt2d,vec2d1,vec2d2);
	  gp_Parab2d para2d(axe2d2bis,parab3d.Focal());
	  IntAna2d_Conic parab_conic(para2d);
	  intersection2d.Perform(cercle2d, parab_conic);
	}

	else if (Intersection3d_2.TypeInter() == IntAna_Hyperbola) {
	  gp_Hypr hyp3d = Intersection3d_2.Hyperbola(1);
	  gp_Ax2 hy_ax = hyp3d.Position().Transformed(circle_to_ref);
	  gp_Pnt2d hy_pnt2d;
	  for (ii = 1; ii <= 2; ii++) {
	    hy_pnt2d.SetCoord(ii, hy_ax.Location().Coord(ii));
	    vec2d1.SetCoord(ii, hy_ax.XDirection().Coord(ii));
	    vec2d2.SetCoord(ii, hy_ax.YDirection().Coord(ii));
	  }
	  gp_Ax22d axe2d2bis(hy_pnt2d,vec2d1,vec2d2);
	  gp_Hypr2d hyp2d(axe2d2bis,hyp3d.MajorRadius(),hyp3d.MinorRadius());
	  IntAna2d_Conic hyp_conic(hyp2d);
	  intersection2d.Perform(cercle2d, hyp_conic);
	}
	else if (Intersection3d_2.TypeInter() == IntAna_Line) {
	  gp_Lin ligne3d = Intersection3d_2.Line(1);
	  gp_Ax1 li_ax = ligne3d.Position();
	  li_ax.Transform(circle_to_ref);
	  gp_Pnt2d li_pnt2d;
	  for (ii = 1 ; ii <= 2 ; ii++) {
	    vec2d1.SetCoord(ii, li_ax.Direction().Coord(ii));
	    li_pnt2d.SetCoord(ii, li_ax.Location().Coord(ii));
	  }
	  gp_Ax2d axe1d2(li_pnt2d,vec2d1);
	  gp_Lin2d ligne2d(axe1d2);
	  IntAna2d_Conic ligne_conic(ligne2d);
	  intersection2d.Perform(cercle2d,ligne_conic);
	}
	else {
	  Standard_ConstructionError::Raise();
	}
	circle_to_ref.Invert();
	Standard_Boolean NotFound = Standard_True;
	gp_Pnt solution;
	if (intersection2d.IsEmpty()) {
	  Standard_ConstructionError::Raise();
	}
	ii = 1;
	
	//
	// In some cases, when we have an hyperbola, some false solutions are
	// added which we need to avoid
	//
	
	do {
	  for (jj = 1; jj <= 2 ; jj++)
	    solution.SetCoord(jj, intersection2d.Point(ii).Value().Coord(jj));
	  solution.SetCoord(3, 0.0e0);
	  solution.Transform(circle_to_ref);
	  for (jj =1 ; jj <= 3; jj++)
	    vector[0][1].SetCoord(jj, solution.Coord(jj) - a_point.Coord(jj));
	  vector[0][1].Normalize();
	  NotFound = (((!same_moving) 
		      && 
		      (Abs(vector[0][1].Angle(vector[0][0]) - moving_angle)
		      >= 
		      Precision::Angular()))
		      ||
		      (same_moving 
		       &&
		       (Abs(vector[0][1].Angle(matrix_vector[0][0]) - angle[0])
			>= 
			Precision::Angular())));
	  ii++;
	} while (ii <= intersection2d.NbPoints() && NotFound);
	
	if (!same_moving) {
	  vector[0][2] = vector[0][0].Crossed(vector[0][1]);
	  vector[0][2].Normalize();
	  vector[1][2] = matrix_vector[1][0].Crossed(matrix_vector[1][1]);
	  vector[1][2].Normalize();
	  gp_Ax3 fixed_framebis(a_point, 
				vector[0][2],
				vector[0][0]);
	  gp_Ax3 moving_framebis(a_point, 
				 vector[1][2],
				 matrix_vector[1][0]);
	  
	  vectorial_part.SetDisplacement(moving_framebis,fixed_framebis);
	}
	else {
	  gp_Ax3 fixed_framebis(a_point, 
				vector[0][1]);
	  gp_Ax3 moving_framebis(a_point, 
				 matrix_vector[1][0]);
	  
	  vectorial_part.SetDisplacement(moving_framebis,fixed_framebis);
	} 
      
      }
    }
    else {
      //
      // particular case : PI/2 angles : the algorithm used is similar to
      // the one used in PLANE-AXIS-PLACEMENT
      //
      gp_Vec a_vector1, a_vector2;
      
      vector[0][1] = matrix_vector[0][1];
      vector[0][2] = matrix_vector[0][0].Crossed(matrix_vector[0][1]);
      vector[0][2].Normalize();
      if (matrix_vector[1][0].CrossSquareMagnitude(matrix_vector[1][1])
	  <=
	  tolerance_squared) {
	gp_Ax3 axis(a_point, matrix_vector[1][0]);
	vector[1][2] = axis.XDirection();
	
      }
      else {
	vector[1][2] = matrix_vector[1][0].Crossed(matrix_vector[1][1]);
      }
      
      for (ii = 0; ii < index; ii++) {
	vector[1][ii] = vector[0][2].Crossed(matrix_vector[0][ii]);
      }
      Standard_Real dot1 = matrix_vector[1][0].Dot(matrix_vector[1][1]),
      dot0 = vector[1][0].Dot(vector[1][1]);
      Standard_Integer e0,e1;
      if ((Abs(dot1) < Abs(dot0)) || dot1 > 0.e0) {
	e0 = 1;
	if (dot0 > 0.e0) 
	  e1 = 1;
	else
	  e1 = -1;
      }
      else {
	e0 = -1;
	if (dot0 > 0.e0) 
	  e1 = -1;
	else 
	  e1 = 1;
      }
      Standard_Real a0Square = e0*(dot1 - e1*dot0)/(1 - e0*e1*dot0);
      if (Abs(1 - a0Square) <= Precision::Confusion()) {
	a_vector1 = Sqrt(a0Square) * vector[0][2];
	a_vector2 = Sqrt(a0Square) * e0 * vector[0][2];
      }
      else if (Abs(a0Square) <= Precision::Confusion()) {
	a_vector1 = Sqrt(1 -a0Square) * vector[1][0];
	a_vector2 = e1*Sqrt(1- a0Square) * vector[1][1];
      } 
      else {
	a_vector1 = Sqrt(a0Square) * vector[0][2] + Sqrt(1 -a0Square) * vector[1][0];
	a_vector2 = e0*Sqrt(a0Square) * vector[0][2] + e1*Sqrt(1- a0Square) * vector[1][1];
      }
      if (a_vector1.CrossSquareMagnitude(a_vector2) >= tolerance_squared) {
	vector[0][0] = a_vector1.Crossed(a_vector2);
      }
      else {
	gp_Ax3 axis(a_point, a_vector1);
	vector[0][0] = axis.XDirection();
      }
      
      gp_Ax3   moving_frame(a_point,
			    vector[1][2],
			    matrix_vector[1][0]);
      
      
      
      gp_Ax3   fixed_frame(a_point,
			   vector[0][0],
			   a_vector1);
      
      vectorial_part.SetDisplacement(moving_frame,
				     fixed_frame) ;

     
    }
    
    //
    // translation's vector due to PLANE_AXIS_PLACEMENT constraint
    //

    Standard_Real k1 = 0.0e0; 
    origin[1][0].Transform(vectorial_part);
    for (ii = 1; ii <=3; ii++) 
      a_vector.SetCoord(ii, origin[0][0].Coord(ii) -
			origin[1][0].Coord(ii)); 
    
    k1 = distance + a_vector.Dot(matrix_vector[0][0]);
    a_vector = k1 * matrix_vector[0][0];
    a_vector.Add(vectorial_part.TranslationPart());
    vectorial_part.SetTranslationPart(a_vector);
    
    
    myTrsf = vectorial_part;
    myHasSolution = Standard_True;
    
  }

// 
// end of solve_case11
//

}



//=======================================================================
//function : AddConstraint
//purpose  : 
//=======================================================================

Standard_Integer ShapePlacement_ConstraintAlgo::AddConstraint(
			  const ShapePlacement_TypeOfConstraint aKeyWord, 
                          const TopoDS_Shape& aSubShape, 
                          const ShapePlacement_TypeOfAxisConstraint aSubAxis, 
                          const TopoDS_Shape& aFixedShape, 
                          const ShapePlacement_TypeOfAxisConstraint aFixAxis) 
{


  ShapePlacement_Constraint aConstraint(myAngularTolerance,
					aKeyWord, 
                                        aSubShape, 
                                        aSubAxis, 
                                        aFixedShape, 
                                        aFixAxis, 
                                        0, 
                                        Standard_False);
  
  myConstraints.Append(aConstraint);
  return 0;
}

//=======================================================================
//function : AddConstraint
//purpose  : 
//=======================================================================

Standard_Integer ShapePlacement_ConstraintAlgo::AddConstraint(
                const ShapePlacement_TypeOfConstraint aKeyWord, 
                const TopoDS_Shape& aSubShape, 
                const ShapePlacement_TypeOfAxisConstraint aSubAxis,  
                const TopoDS_Shape& aFixedShape, 
                const ShapePlacement_TypeOfAxisConstraint aFixAxis, 
                const Standard_Real aCotation) 
{

  ShapePlacement_Constraint aConstraint(myAngularTolerance,
					aKeyWord, aSubShape, aSubAxis, aFixedShape, aFixAxis, aCotation, Standard_True);

  myConstraints.Append(aConstraint);
  return 0;
}

//=======================================================================
//function : PrintConstraint
//purpose  : 
//=======================================================================

void ShapePlacement_ConstraintAlgo::PrintConstraint(Standard_OStream& S) const
{

}

//=======================================================================
//function : PositionShape
//purpose  : 
//=======================================================================

Standard_Integer ShapePlacement_ConstraintAlgo::PositionShape(
              const TopoDS_Shape& ShapeToPosition) 
{
  mySubShapes.Clear();
  myShapeToPosition = ShapeToPosition;
  TopExp::MapShapes(myShapeToPosition,mySubShapes);
  return 0;
}

//=======================================================================
//function : GetShapeToPosition
//purpose  : 
//=======================================================================

TopoDS_Shape ShapePlacement_ConstraintAlgo::GetShapeToPosition() const 
{
  return myShapeToPosition ;
}


//=======================================================================
//function : IsOverConstrained
//purpose  : 
//=======================================================================

Standard_Boolean ShapePlacement_ConstraintAlgo::IsOverConstrained() const 
{
  return myIsOverConstrained ;
}

