// File:	ShapeInterference_InterfObj.cxx
// Created:	Tue Jan 28 10:37:54 1997
// Author:	Prestataire Xuan PHAM PHU
//		<xpu>


#include <ShapeInterference_InterfObj.ixx>

 ShapeInterference_InterfObj::ShapeInterference_InterfObj()
{
}

void ShapeInterference_InterfObj::SetSection(const TopoDS_Shape& aSectionResult)
{
  mySection = aSectionResult;
}

ShapeInterference_TypeOfInterf ShapeInterference_InterfObj::GetInterfType() const 
{
  return ShapeInterference_TOI_TGCY;
}

TopoDS_Shape ShapeInterference_InterfObj::GetSection() const 
{
  return mySection;
}

