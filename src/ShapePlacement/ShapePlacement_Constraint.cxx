// File:	ShapePlacement_Constraint.cxx
// Created:	Mon Mar 11 10:28:06 1996
// Author:	Christian MATHY
//		<cmy@mastox>


#include <ShapePlacement_Constraint.ixx>

ShapePlacement_Constraint::ShapePlacement_Constraint() 
{

}

//=======================================================================
//function : ShapePlacement_Constraint
//purpose  : 
//=======================================================================

ShapePlacement_Constraint::ShapePlacement_Constraint(
		       const ShapePlacement_TypeOfConstraint aKey, 
		       const TopoDS_Shape& aSubShape, 
                       const ShapePlacement_TypeOfAxisConstraint aSubAxis, 
                       const TopoDS_Shape& aFixShape, 
                       const ShapePlacement_TypeOfAxisConstraint aFixedAxis, 
                       const Standard_Real aValue, 
                       const Standard_Boolean IsaValue) 
:myEquation(aKey,
	    aSubShape,
	    aSubAxis,
	    aFixShape,
	    aFixedAxis,
	    aValue,
	    IsaValue)
{
  myKeyWord = aKey;
  myFixedShape = aFixShape;
  mySubShape = aSubShape;
  myValue = aValue;
  mySubAxis = aSubAxis;
  myFixedAxis = aFixedAxis;
  IsSignificatedValue = IsaValue;
}
//=======================================================================
//function : ShapePlacement_Constraint
//purpose  : 
//=======================================================================

ShapePlacement_Constraint::ShapePlacement_Constraint(
		       const Standard_Real     anAngularTolerance,
		       const ShapePlacement_TypeOfConstraint aKey, 
		       const TopoDS_Shape& aSubShape, 
                       const ShapePlacement_TypeOfAxisConstraint aSubAxis, 
                       const TopoDS_Shape& aFixShape, 
                       const ShapePlacement_TypeOfAxisConstraint aFixedAxis, 
                       const Standard_Real aValue, 
                       const Standard_Boolean IsaValue) 
:myEquation(anAngularTolerance,
	    aKey,
	    aSubShape,
	    aSubAxis,
	    aFixShape,
	    aFixedAxis,
	    aValue,
	    IsaValue)
{
  myKeyWord = aKey;
  myFixedShape = aFixShape;
  mySubShape = aSubShape;
  myValue = aValue;
  mySubAxis = aSubAxis;
  myFixedAxis = aFixedAxis;
  IsSignificatedValue = IsaValue;
}

void ShapePlacement_Constraint::Set(const ShapePlacement_TypeOfConstraint aKey, const TopoDS_Shape& aSubShape, const ShapePlacement_TypeOfAxisConstraint aSubAxis, const TopoDS_Shape& aFixShape, const ShapePlacement_TypeOfAxisConstraint aFixedAxis, const Standard_Real aValue, const Standard_Boolean IsaValue)
{
  myKeyWord = aKey;
  myFixedShape = aFixShape;
  mySubShape = aSubShape;
  myValue = aValue;
  mySubAxis = aSubAxis;
  myFixedAxis = aFixedAxis;
  IsSignificatedValue = IsaValue;
}
//=======================================================================
//function : Equation
//purpose  : 
//=======================================================================

const ShapePlacement_Equation& ShapePlacement_Constraint::Equation() const 
{ return myEquation ; }




