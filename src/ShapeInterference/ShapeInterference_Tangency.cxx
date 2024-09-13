// File:	ShapeInterference_Tangency.cxx
// Created:	Tue Jan 28 10:38:41 1997
// Author:	Prestataire Xuan PHAM PHU
//		<xpu>


#include <ShapeInterference_Tangency.ixx>

 ShapeInterference_Tangency::ShapeInterference_Tangency()
: myTypeOfTgcy( ShapeInterference_TOT_TGCYONFACE ) , areOnSameMatterSide( Standard_False )
{
}

void ShapeInterference_Tangency::SetTgcyType(const ShapeInterference_TypeOfTgcy aType)
{
  myTypeOfTgcy = aType;
}

void ShapeInterference_Tangency::SetGeneratingShapes
(const TopoDS_Shape& aShapeOf1, const TopoDS_Shape& aShapeOf2)
{
  myShapeOn1 = aShapeOf1;
  myShapeOn2 = aShapeOf2;
}

void ShapeInterference_Tangency::SetSameMatterSide(const Standard_Boolean aBoolean)
{
  areOnSameMatterSide = aBoolean;
}

ShapeInterference_TypeOfInterf ShapeInterference_Tangency::GetInterfType() const 
{
  return  ShapeInterference_TOI_TGCY;
}

ShapeInterference_TypeOfTgcy ShapeInterference_Tangency::GetTgcyType() const 
{
  return myTypeOfTgcy;
}

void ShapeInterference_Tangency::GetGeneratingShapes(TopoDS_Shape& aShapeOf1, TopoDS_Shape& aShapeOf2) const
{
  aShapeOf1 = myShapeOn1;
  aShapeOf2 = myShapeOn2;
}

Standard_Boolean ShapeInterference_Tangency::AreOnSameMatterSide() const 
{
  return areOnSameMatterSide;
}

