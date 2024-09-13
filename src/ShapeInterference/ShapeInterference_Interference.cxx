// File:	ShapeInterference_Interference.cxx
// Created:	Tue Jan 28 10:36:53 1997
// Author:	Prestataire Xuan PHAM PHU
//		<xpu>


#include <ShapeInterference_Interference.ixx>
#include <ShapeInterference_SequenceOfInterfObj.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>

 ShapeInterference_Interference::ShapeInterference_Interference()
: hasInclusion(Standard_False)
{
}

void ShapeInterference_Interference::Empty()
{
  mySeqOfInterfObj.Clear();
}

void ShapeInterference_Interference::AddInterfObj(const Handle(ShapeInterference_InterfObj)& anInterfObject)
{
  mySeqOfInterfObj.Append(anInterfObject);
}

void ShapeInterference_Interference::SetInclusion()
{
  hasInclusion = !hasInclusion;
}

Standard_Integer ShapeInterference_Interference::GetInterfNbr() const 
{
  return mySeqOfInterfObj.Length();
}

Handle(ShapeInterference_InterfObj) ShapeInterference_Interference::GetInterfObj(const Standard_Integer anIndex) const 
{
  return mySeqOfInterfObj.Value(anIndex);
}

Standard_Boolean ShapeInterference_Interference::HasTangency() const 
{
  Standard_Boolean ok = Standard_False;
  Standard_Integer ii = 1;
  while( (ii<=mySeqOfInterfObj.Length()) && !ok )
  {
    ok = (mySeqOfInterfObj.Value(ii)->GetInterfType()
	  == ShapeInterference_TOI_TGCY );
    ii++;
  }
  return ok;  
}

Standard_Boolean ShapeInterference_Interference::HasCollision() const 
{
  Standard_Boolean ok = Standard_False;
  Standard_Integer ii = 1;
  while( (ii<=mySeqOfInterfObj.Length()) && !ok )
  {
    ok = (mySeqOfInterfObj.Value(ii)->GetInterfType()
	  == ShapeInterference_TOI_COLLISION );
    ii++;
  }
  return ok; 
}

Standard_Boolean ShapeInterference_Interference::HasInclusion() const 
{
  return hasInclusion;
}

Standard_Integer ShapeInterference_Interference::GetTangencyNbr() const 
{
  Standard_Integer nbr = 0;
  for(Standard_Integer ii = 1; ii <= mySeqOfInterfObj.Length(); ii++)
    if(mySeqOfInterfObj.Value(ii)->GetInterfType()
       == ShapeInterference_TOI_TGCY ){ nbr++; }
  return nbr;
}

Standard_Integer ShapeInterference_Interference::GetCollisionNbr() const 
{
  Standard_Integer nbr = 0;
  for(Standard_Integer ii = 1; ii <= mySeqOfInterfObj.Length(); ii++)
    if(mySeqOfInterfObj.Value(ii)->GetInterfType()
       == ShapeInterference_TOI_COLLISION ){ nbr++; }
  return nbr;
}

void ShapeInterference_Interference::GetSeqOfTangencies(ShapeInterference_SequenceOfInterfObj& aSequence) const 
{
  for(Standard_Integer ii = 1; ii <= mySeqOfInterfObj.Length(); ii++)
    if(mySeqOfInterfObj.Value(ii)->GetInterfType()  == ShapeInterference_TOI_TGCY )
      aSequence.Append(mySeqOfInterfObj.Value(ii)); 
}

void ShapeInterference_Interference::GetSeqOfCollisions(ShapeInterference_SequenceOfInterfObj& aSequence) const 
{
  for(Standard_Integer ii = 1; ii <= mySeqOfInterfObj.Length(); ii++)
    if(mySeqOfInterfObj.Value(ii)->GetInterfType() == ShapeInterference_TOI_COLLISION )
      aSequence.Append(mySeqOfInterfObj.Value(ii)); 
}

TopoDS_Shape ShapeInterference_Interference::GetAllSections() const 
{
  TopoDS_Compound aComp;
  aComp.Nullify();

  if(mySeqOfInterfObj.IsEmpty ())   
    return aComp;
                                                                                        
  BRep_Builder BB;
  BB.MakeCompound(aComp);
  for(Standard_Integer ii = 1; ii <= mySeqOfInterfObj.Length(); ii++)
    BB.Add(aComp,mySeqOfInterfObj.Value(ii)->GetSection());
  return aComp;
}

