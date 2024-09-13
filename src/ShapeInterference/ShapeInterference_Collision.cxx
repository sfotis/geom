// File:	ShapeInterference_Collision.cxx
// Created:	Tue Jan 28 10:39:21 1997
// Author:	Prestataire Xuan PHAM PHU
//		<xpu>


#include <ShapeInterference_Collision.ixx>

 ShapeInterference_Collision::ShapeInterference_Collision()
{
}

void ShapeInterference_Collision::SetCommon(const TopoDS_Shape& aCommonResult)
{
  myCommon = aCommonResult;
}

ShapeInterference_TypeOfInterf ShapeInterference_Collision::GetInterfType() const 
{
  return ShapeInterference_TOI_COLLISION;
}

TopoDS_Shape ShapeInterference_Collision::GetCommon() const 
{
  return myCommon;
}

