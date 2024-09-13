// File:	ShapeInterference_methods.cxx
// Created:	Wed Jan 29 14:37:54 1997
// Author:	Prestataire Xuan PHAM PHU
//		<xpu@salgox.paris1.matra-dtv.fr>

#include <ShapeInterference_Interference.hxx>
#include <ShapeInterference_Tangency.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Compound.hxx>

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_DataMapOfIntegerListOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeShape.hxx>
#include <TopTools_DataMapIteratorOfDataMapOfShapeShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_DataMapIteratorOfDataMapOfShapeListOfShape.hxx>
#include <TopTools_DataMapIteratorOfDataMapOfIntegerListOfShape.hxx>
#include <TopTools_SequenceOfShape.hxx>
#include <TopTools_Array1OfShape.hxx>
#include <TopTools_Array1OfListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>

#include <TopOpeBRepDS_DataStructure.hxx>
#include <TopOpeBRepDS_HDataStructure.hxx>
#include <TopOpeBRepDS_Interference.hxx>
#include <TopOpeBRepDS_ListIteratorOfListOfInterference.hxx>
#include <TopOpeBRepBuild_HBuilder.hxx>
#include <TopAbs.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>

#include <IntPatch_Intersection.hxx>

#include <Precision.hxx>

#include <Bnd_Box.hxx>
#include <Bnd_Array1OfBox.hxx>
#include <TColStd_Array1OfReal.hxx>

#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepClass3d_SolidClassifier.hxx>

#include <Extrema_LocateExtPC.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>

#include <gp_Vec.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>

#ifdef DRAW
#include <DBRep.hxx>
#include <TCollection_AsciiString.hxx>
void FUN_shaint_draw (const TopoDS_Shape& s)
{
  char* nnn = TCollection_AsciiString("name").ToCString();
  if (s.IsNull()) cout<<"nullshape"<<endl;
  DBRep::Set(nnn,s);
}
#endif

//=======================================================================
//function : TestOnVertex
//purpose  : 
//=======================================================================
Standard_Boolean ShapeInterference_TestOnVertex(const TopoDS_Shape& aShape1,
						const TopoDS_Shape& aShape2,
						Standard_Boolean& allvof1ON2)
{
  allvof1ON2 = Standard_False;
  //Get Vtx, one of aShape1's vertex
  // Call BRepClass3d_SolidClassifier :
  // if Vtx is contained in aShape2 or is on aShape2's frontier : returns True.
  // else : returns False.

  TopExp_Explorer anExpl(aShape1, TopAbs_VERTEX);
  if( !anExpl.More() )
  {
    cout <<"ShapeInterference: No vertex on given object"<<endl; 
    return Standard_False;
  }
  
  for (; anExpl.More(); anExpl.Next()) {
    const TopoDS_Vertex& v = TopoDS::Vertex(anExpl.Current());
    
    BRepClass3d_SolidClassifier 
    classif(aShape2, BRep_Tool::Pnt(v), 
	    Precision::Confusion());

    TopAbs_State sta = classif.State();
    if(sta == TopAbs_IN) {
      allvof1ON2 = Standard_True;
      return Standard_True;
    }
    else if (sta == TopAbs_OUT)
      return Standard_False;
  }
  // All vertices are ON a face of the other shape
  // we do not know whereas the shapes are tangent on the same
  // side of the matter (collision) or not.
  allvof1ON2 = Standard_True;
//  Standard_Failure::Raise("UNKNOWN");
  // POP pour NT
  return Standard_False;
}

//=======================================================================
//function : IsInBndBox
//purpose  : Returns <True> if aBndBoxi is contained in aBndBoxj.
//           IsInBndBox does not deal with infinite boxes.
//           HEAVY, but no other method provided
//=======================================================================
Standard_Boolean ShapeInterference_IsInBndBox(const Bnd_Box& aBndBox1,
					      const Bnd_Box& aBndBox2,
					      Standard_Boolean& OneIsInTwo)
{
  // IsInBndBox returns <True> if aBndBoxi is contained in aBndBoxj.
  // IsInBndBox does not deal with infinite boxes.

  // To deal with an array :
  Bnd_Array1OfBox Box(1, 2);
  Box(1).Add( aBndBox1 );
  Box(2).Add( aBndBox2 );

  // To avoid infinite cases
  Standard_Integer i;
  for(i = 1; i <= 2; i++)
    if( Box(i).IsOpenXmin() || Box(i).IsOpenXmax() ||
        Box(i).IsOpenYmin() || Box(i).IsOpenYmax() ||
        Box(i).IsOpenZmin() || Box(i).IsOpenZmax() )
    {
      cout <<"ShapeInterference: RangeError on bounding boxes" <<endl;
      return Standard_False;
    }

  // aBox contains Box(1) & Box(2)
  Bnd_Box aBox;
  for(i = 1; i <= 2; i++)
    aBox.Add(Box(i));

  // Getting aBox xyz boundaries
  TColStd_Array1OfReal xyz(1, 6);
  aBox.Get(xyz(1), xyz(2), xyz(3), xyz(4), xyz(5), xyz(6)); 

  // If aBox is same as Box(i), Box(i) contains Box(j) (j =/= i). Returns <True>
  // Else, returns <False>.
  Standard_Boolean isin = Standard_False;

  for (i = 1; i<=2; i++) {
    Standard_Boolean issame = Standard_True; 
    TColStd_Array1OfReal xyzi(1, 6);
    Box(i).Get(xyzi(1), xyzi(2), xyzi(3), xyzi(4), xyzi(5), xyzi(6)); 

    for(Standard_Integer j = 1; j <= 6; j++) {
      issame = issame && (Abs(xyz(j) - xyzi(j)) <= 1.e-7 );
      if (!issame) break;
    }        
    if (issame) {
      isin = Standard_True;      
      break;
    }
  } 
  
  if (!isin) return Standard_False;

  OneIsInTwo = (i == 2)? Standard_True: Standard_False;
  return Standard_True;
}


// xpu : -16-07-97
/*

//=======================================================================
//function : AddToMap 
//purpose  : Adds the tangent shapes <shapeOn1>,<shapeOn2>, and the section
//          edge <anEdge> generated in the map <mapoftgcies>
//=======================================================================
void ShapeInterference_AddToMap(TopTools_DataMapOfShapeListOfShape& mapoftgcies,
				const TopoDS_Shape& shapeOn1,
				const TopoDS_Shape& shapeOn2,
				const TopoDS_Shape& anEdge)
{
  // If <shapeOn1> is bound as Key in the map :
  //   if <shapeOn2> is bound too, adds anEdge to the corresponding 
  //       compound of edges
  //   else appends a couple (<shapeOn2>,<anEdge>) to the Item of Key
  // Else : Adds (theKey = <shapeOn1> ,theItem = (<shapeOn2>,<anEdge>)) to the map

  if( mapoftgcies.IsBound(shapeOn1) )
  {
    TopTools_ListOfShape& lOfSh = mapoftgcies.ChangeFind(shapeOn1);// Item with key shapeOn1
    TopTools_ListOfShape copy; copy.Assign(lOfSh);
    TopTools_ListIteratorOfListOfShape lite(lOfSh);// a copy of the list
    Standard_Boolean isbound = Standard_False;
    BRep_Builder BB;    

    while(lite.More()) {
      if(lite.Value().IsSame(shapeOn2))
      {	
	// Adding the section edge to the compound of 
	// edges common to the two shapes <shapeOn1> and
	// <shapeOn2>
	isbound = Standard_True;
	copy.Append(lite.Value()); lite.Next();
	TopoDS_Compound comp; BB.MakeCompound(comp);
	BB.Add(comp, lite.Value()); BB.Add(comp, anEdge);
	copy.Append(comp); lite.Next();
      }
      else {
	copy.Append(lite.Value()); lite.Next();
	copy.Append(lite.Value()); lite.Next();
      } 
    }
    // Adding the shapes (shapeOn2,anEdge) at the end of the list
    // of shapes.
    if(isbound) lOfSh.Assign(copy);
    else {lOfSh.Append(shapeOn2); lOfSh.Append(anEdge);}
  }
  else
  {
    TopTools_ListOfShape lofsh;
    lofsh.Append(shapeOn2);
    lofsh.Append(anEdge);
    mapoftgcies.Bind(shapeOn1,lofsh);
  } 

}

//=======================================================================
//function :  MapOfTgciesToInterf
//purpose  :  Adds tangency cases found in the map <mapoftgcies> to the
//            interference <theInterf>.
//=======================================================================
void ShapeInterference_MapOfTgciesToInterf
(const TopTools_DataMapOfShapeListOfShape& mapoftgcies,
 ShapeInterference_Interference& theInterf)
{
  TopTools_DataMapIteratorOfDataMapOfShapeListOfShape mapite(mapoftgcies);
  TopAbs_ShapeEnum typ;
  ShapeInterference_TypeOfTgcy typOftgcy;
  for(; mapite.More(); mapite.Next()) 
  {
    const TopoDS_Shape& shOn1 = mapite.Key();
    TopAbs_ShapeEnum typ = shOn1.ShapeType();
    TopTools_ListIteratorOfListOfShape lite(mapite.Value());
    for(; lite.More(); lite.Next()) 
    {
      const TopoDS_Shape& shOn2  = lite.Value();
      lite.Next();
      typOftgcy = ((typ == TopAbs_FACE)&&(shOn2.ShapeType() == TopAbs_FACE))?
                  ShapeInterference_TOT_TGCYONFACE : ShapeInterference_TOT_TGCYONEDGE;
      Handle(ShapeInterference_Tangency) tgcy = new ShapeInterference_Tangency;
      tgcy->SetTgcyType(typOftgcy);
      tgcy->SetGeneratingShapes(shOn1, shOn2);
      tgcy->SetSection(lite.Value());
      theInterf.AddInterfObj(tgcy);
    }
  }
}

//=======================================================================
//function : GetAncestorsFromEdge

//purpose  : Returns the data map of faces of the old topology connexed
//           to a given edge. 
//           If <isaCurveEdge> : <AncestorToEdge> contains faces from <Shape1>
//           and <Shape2>  generating the section edge <theEdge>,
//           Else : they contain the 2 edges in case 2Corner interference,
//                               the edge and the face otherwise.
//=======================================================================
Standard_Boolean  ShapeInterference_GetAncestorsFromEdge
(const TopoDS_Shape& theEdge,
 const Handle(TopOpeBRepBuild_HBuilder)& HB,
 const Standard_Boolean& isaCurveEdge,
 const TopTools_Array1OfShape& Shape,
 TopTools_Array1OfShape& AncestorToEdge,
 Standard_Boolean& twocornercase) 
{
  Standard_Boolean ok = Standard_True;
  twocornercase = Standard_False;
  Standard_Integer ii = 1;
  for(; ii<=2; ii++)
    AncestorToEdge(ii).Nullify();

  TopTools_IndexedMapOfShape mapOf1;
  TopExp::MapShapes( Shape(1), mapOf1 );
  TopTools_SequenceOfShape ancestor;
  
  const Handle( TopOpeBRepDS_HDataStructure )& HDS = HB->DataStructure();  
  const TopOpeBRepDS_DataStructure& DS = HDS->DS();

  if( !isaCurveEdge )
    // !!!!!!!!! To Change when a method GetAncestorsFromSectionEgde
    // is implemented!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  {
    // Two possible cases : 
    // - <theEdge> belongs to 2 edges from the 2 shapes ( twocornercase )
    // - it belongs to 1 edge and one face.
    
    // When an section edge is built on a corner edge it's a KPart,
    // the corresponding corner edge is filled in the DS of the Section as
    // a Section Edge
    // !! If the section edge belongs to 2 corners, ONLY one edge ancestor 
    // is binded in the DS

    Standard_Integer ise = 1; 
    Standard_Boolean found = Standard_False;
    while((ise<= DS.NbSectionEdges()) && !found)
    {      
      const TopoDS_Edge& sectionedge = DS.SectionEdge(ise);
      if(HB->IsSplit(sectionedge, TopAbs_ON))
      {
	TopTools_ListIteratorOfListOfShape itloe(HB->Splits(sectionedge, TopAbs_ON));
	for(;itloe.More(); itloe.Next())
	  if( itloe.Value().IsSame(theEdge) ) 
	  {
	    ancestor.Append(sectionedge);
	    found = Standard_True;
	    break;
	  }
      }
      ise++;
    }
    
    // Now, ancestor(1) is the section edge ancestor of <theEdge>
    const TopoDS_Shape& ed = ancestor.First();   
    Standard_Boolean onefoundfirst = mapOf1.Contains(ed);
    Standard_Integer indexOfse = onefoundfirst?1:2;
    Standard_Integer indexOfOthersh = onefoundfirst?2:1;
 
    if(HDS->HasSameDomain(ed))
    {
      TopTools_IndexedMapOfShape Othermap;
      TopExp::MapShapes(Shape(indexOfOthersh),TopAbs_EDGE,Othermap);
      // If ancestor(1) shares same geometric domain with an edge of Othermap 
      // it's a TwoCornerCase.
      // To get the corresponding edge, we classify middle point pmil of <theEdge>
      // on each edge of the list of shapes same domain with ancestor(1). 
      TopTools_ListIteratorOfListOfShape liOflOfsh = HDS->SameDomain(ed);
      BRepAdaptor_Curve BC(TopoDS::Edge(theEdge));
      gp_Pnt pmil = BC.Value((BC.FirstParameter()+BC.LastParameter())*.5); 

      for(;liOflOfsh.More(); liOflOfsh.Next())
      {
	if(Othermap.Contains(liOflOfsh.Value()))
	{
	  if(!liOflOfsh.More()){ ancestor(2) = liOflOfsh.Value(); break;}
	  // The corresponding edge is the one containing the point pmil.
	  BC.Initialize(TopoDS::Edge(liOflOfsh.Value()));
	  Extrema_LocateExtPC extr(pmil,BC,BC.FirstParameter(),
				   BC.FirstParameter(),BC.LastParameter(),
				   Precision::Confusion());
	  if(extr.IsDone())
	    if(extr.Value()<=Precision::Confusion()*10.)
	      {ancestor(2) = liOflOfsh.Value();break;}
	}
      }
    }
    if(ancestor.Length() == 2)
    {
      AncestorToEdge(1) = ancestor(indexOfse);
      AncestorToEdge(2) = ancestor(indexOfOthersh);
      return ok;
    }
    
    // !TwoCornerCase :
    // Getting the face on the other shape containing <theEdge> :
    // * We get a face explorer on the other shape
    TopExp_Explorer expl;  
    expl.Init(Shape(indexOfOthersh), TopAbs_FACE);
    // * As the ancestor edge found has generated a section edge, he is 
    // contained in the DS, and there are faces in the DS which 
    // interference's description contains this ancestor edge.
    // We add these contained in the other shape in the sequence ancestor
    TopOpeBRepDS_ListIteratorOfListOfInterference it;
    found = Standard_False;
    for(; expl.More(); expl.Next()) {
      if(DS.HasShape(expl.Current())) {

	it.Initialize(DS.ShapeInterferences(expl.Current())); 	
	found = Standard_False;
	while(it.More() && !found)
	{
	  if(it.Value()->GeometryType() == TopOpeBRepDS_EDGE)
	    found = DS.Shape(it.Value()->Geometry()).IsSame(ed);
	  it.Next();
	}
	if(found){ancestor.Append(expl.Current());}
      }   
    } 

    Standard_Integer nb = ancestor.Length();
    if(nb < 2){return !ok;}
    if(nb > 2) {
      // !!!!!!!!!!!!!!!!! If you want more tangent faces => Changes 
      // to keep these other faces !!!!!!!!!!!!!!!!!!!!!!!
      if(found){ancestor.Remove(3,nb);} // Removing from index 3 to nb
    }
    
    AncestorToEdge(1) = ancestor(indexOfse);
    AncestorToEdge(2) = ancestor(indexOfOthersh);
  } // !isaCurveEdge

  else
// WAITING FOR NEW GTI ->DEB    
    // Getting the faces attached to theEdge :
    // Classifying the faces: 
//    ok = HB->EdgeCurveAncestors(theEdge, AncestorToEdge.ChangeValue(1), 
//				AncestorToEdge.ChangeValue(2));
// WAITING FOR NEW GTI ->FIN    
    ok  = Standard_False;

  return ok;
}

//=======================================================================
//function : SameDomainandDiffOri 
//purpose  : Returns True if <aShape1> and <aShape2> are of same domain
//           and of different  orientation relative to the reference.
//=======================================================================
Standard_Boolean ShapeInterference_SameDomainandDiffOri
(const TopoDS_Shape& aShape1,
 const TopoDS_Shape& aShape2,
 const Handle(TopOpeBRepDS_HDataStructure)& HDS)
{
  // If aShape1 && aShape2 have SDM topology
  // Else return False
  if(!(HDS->HasSameDomain(aShape1) && HDS->HasSameDomain(aShape2))) 
    return Standard_False;

  // If <aShape1> has SameDomain with <aShape1> : 
    // If  SameDomainOri<aShape1> != SameDomainOri <aShape2> : return True
    // (Else : one of the two shapes is partially contained in the other ) 
  // Else return False
  if(HDS->SameDomainReference(aShape1) == HDS->SameDomainReference(aShape2))
    if(HDS->SameDomainOrientation(aShape1) != HDS->SameDomainOrientation(aShape2))
      return Standard_True;

  return Standard_False;  
}

//=======================================================================
//function : HasTgtFaces
//           Uses : SameDomain and DiffOri
//purpose  : Look for faces in <theAncestors>, of same domain and diff ori
//           Fills <TgtFaces> with them, removes them from <theAncestors>.
//           Returns False if no tangent faces are found

//           <TgtFaces> is an array of list of shapes because you can have 
//           many couple of tangent faces connexed to the same edge.
//=======================================================================
Standard_Boolean ShapeInterference_HasTgtFaces
(const Handle( TopOpeBRepDS_HDataStructure )& HDS,
 TopTools_Array1OfListOfShape& theConnexedFaces,
 TopTools_Array1OfListOfShape& TgtFaces)
{
  // xpu : -16-07-97
  for( Standard_Integer i = 1; i >=2; i++) TgtFaces(i).Clear();

  Standard_Boolean ok = Standard_True;
  TopTools_ListIteratorOfListOfShape it1(theConnexedFaces(1)), it2;
  TopTools_Array1OfListOfShape NoTgtFaces(1,2);

  for(;it1.More();it1.Next())
  {
    it2.Initialize(theConnexedFaces(2));
    for(;it2.More();it2.Next())
    {
      if(ShapeInterference_SameDomainandDiffOri(it1.Value(),it2.Value(), HDS))
      { 
	TgtFaces(1).Append(it1.Value()); 
	TgtFaces(2).Append(it2.Value());
      }      
      // You cannot work on current list because Remove moves the iterator 
      // to the next item
      else{ NoTgtFaces(1).Append(it1.Value()); NoTgtFaces(2).Append(it2.Value());}
    }
  }
  if(TgtFaces(1).Extent() < 1){ return !ok;} 
  else{ theConnexedFaces.Assign(NoTgtFaces);}
  return ok;
}*/
  // xpu : -16-07-97

//=======================================================================
//function : GetConnexedFaces
//purpose  : Gets the connexed faces to <theAncestors>, fills <ConnexedFaces>
//           with them.
//=======================================================================
Standard_Boolean ShapeInterference_GetConnexedFaces
(const TopTools_Array1OfShape& theShape,
 const TopTools_Array1OfShape& theAncestors,
 TopTools_Array1OfListOfShape& ConnexedFaces)
{
  Standard_Boolean ok = Standard_True;
  Standard_Integer i;

  for(i = 1; i<=2; i++)
  {
    if(theAncestors(i).ShapeType() == TopAbs_EDGE)
    {
      TopTools_IndexedDataMapOfShapeListOfShape map;
      TopExp::MapShapesAndAncestors(theShape(i),TopAbs_EDGE,TopAbs_FACE,map);
      if(!map.Contains(theAncestors(i)))
      {
	cout<<"ShapeInterference_TgcyOnEdToTgcyOnFa : Data Error"<<endl;
	return !ok;
      }
      TopTools_ListIteratorOfListOfShape ite(map.FindFromKey(theAncestors(i)));
      for(; ite.More(); ite.Next()){ConnexedFaces(i).Append(ite.Value());}
    }
    else{ConnexedFaces(i).Append(theAncestors(i));}
  }
 return ok;
}

//=======================================================================
//function : SameDomain
//purpose  : Returns True if <aShape1> and <aShape2> are same domain
//=======================================================================
Standard_Boolean ShapeInterference_SameDomain
(const TopoDS_Shape& aShape1,
 const TopoDS_Shape& aShape2,
 const Handle(TopOpeBRepDS_HDataStructure)& HDS)
{
  // If aShape1 && aShape2 have SDM topology
  // Else return False
  if(!(HDS->HasSameDomain(aShape1) && HDS->HasSameDomain(aShape2))) 
    return Standard_False;
  // If <aShape1> is SameDomain with <aShape1> : 
  if(HDS->SameDomainReference(aShape1) == HDS->SameDomainReference(aShape2))
       return Standard_True;
  else return Standard_False; 
}

//=======================================================================
//function : DiffOri
//purpose  : Returns True if <aShape1> and <aShape2> are of different 
//           orientation relative to the reference.
//           Dummy if <aShape1> and <aShape2> are not same domain
//=======================================================================
Standard_Boolean ShapeInterference_DiffOri
(const TopoDS_Shape& aShape1,
 const TopoDS_Shape& aShape2,
 const Handle(TopOpeBRepDS_HDataStructure)& HDS)
{
  if(HDS->SameDomainOrientation(aShape1) != HDS->SameDomainOrientation(aShape2)) 
        return Standard_True;
  else  return Standard_False;    
}

//=======================================================================
//function : HasSdmFaces
//           Uses : SameDomain
//purpose  : Look for faces in <theAncestors>, of same domain 
//=======================================================================
Standard_Boolean ShapeInterference_HasSdmFaces
(const Handle( TopOpeBRepDS_HDataStructure )& HDS,
 TopTools_Array1OfListOfShape& theConnexedFaces,
 TopTools_DataMapOfShapeShape& SdmF)
{
  SdmF.Clear();
  TopTools_ListIteratorOfListOfShape it1(theConnexedFaces(1)), it2;
  for(;it1.More();it1.Next())
  {
    it2.Initialize(theConnexedFaces(2));
    for(;it2.More();it2.Next())
      if (ShapeInterference_SameDomain(it1.Value(),it2.Value(), HDS))
	SdmF.Bind(it1.Value(),it2.Value());
  }
  if (SdmF.IsEmpty()) return Standard_False;
  else return Standard_True;
}

//=======================================================================
//function : PInFclosetoE
//purpose  : Computes a point near <theEdge> and IN <theFace>
//=======================================================================
Standard_Boolean FUN_projonS(const gp_Pnt& P,
			     const Handle(Geom_Surface)& S,
			     Standard_Real& u,
			     Standard_Real& v)
{  
  GeomAPI_ProjectPointOnSurf PonS(P,S);
  if (!PonS.Extrema().IsDone()) return Standard_False;
  if (PonS.NbPoints() == 0) return Standard_False;
  PonS.LowerDistanceParameters(u,v);
  return Standard_True;
}

Standard_Boolean FUN_UVinUVminmax
(const Standard_Real& u,
 const Standard_Real& v,
 const TColStd_Array1OfReal& UV)
{
  Standard_Boolean isin = (u >= UV(1)) && (u <= UV(2));
  isin = isin&& ((v >= UV(3)) && (v <= UV(4)));
  return isin;
}

Standard_Boolean ShapeInterference_PInFclosetoE (const TopoDS_Shape& theEdge,
						 const TopoDS_Shape& theFace,
						 gp_Pnt& thePoint)
{
  Standard_Integer i, j;
  Standard_Real delta;
  
#ifdef DRAW
  Standard_Boolean trc = Standard_False;
  if (trc) FUN_shaint_draw(theEdge);
  if (trc) FUN_shaint_draw(theFace);
#endif

  TColStd_Array1OfReal UV(1,4);
  BRepTools::UVBounds(TopoDS::Face(theFace),UV(1),UV(2),UV(3),UV(4)); 
  Standard_Real Duvmin = UV(2) - UV(1);
  Duvmin = (Duvmin < (UV(4)-UV(3)))? Duvmin: (UV(4)-UV(3)); 
  BRepAdaptor_Surface BS = BRepAdaptor_Surface(TopoDS::Face(theFace));
  Standard_Real pf,pl;
  const Handle(Geom2d_Curve) C = 
    BRep_Tool::CurveOnSurface(TopoDS::Edge(theEdge),TopoDS::Face(theFace),pf,pl);
 
  if (!C.IsNull()) {
    TColStd_Array1OfReal uv(1,4), uvpnt(1,2);   
    BRepTools::UVBounds(TopoDS::Face(theFace),TopoDS::Edge(theEdge),uv(1),uv(2),uv(3),uv(4));        
   j = 1;
    for(i = 1; i <=2; i++)
      {
	delta = (UV(j+1) - UV(j))/10.;
	delta = (uv(j) < UV(j+1))?delta:-delta;
	uvpnt(i) = uv(j) + delta;
	while(uvpnt(i) >= UV(j+1)){delta /= 10.;uvpnt(i) = uv(j) + delta;}
	j += 2;
      }   
    BS.D0(uvpnt(1), uvpnt(2), thePoint);
    return Standard_True;
  }
  else {
    BRepAdaptor_Curve BC(TopoDS::Edge(theEdge));
    pf = BC.FirstParameter();
    pl = BC.LastParameter();
    Standard_Real paronE = (pl -pf)*.5; gp_Pnt ponE; gp_Vec tgE; 
    BC.D1(paronE,ponE,tgE);

    Handle(Geom_Surface) su = BRep_Tool::Surface(TopoDS::Face(theFace));
    Standard_Real u,v;
    Standard_Boolean projok = FUN_projonS (ponE,su,u,v);
    if (!projok) {cout<<"::PInFclosetoE fails"<<endl; return Standard_False;}

    gp_Vec d1u,d1v, norm; BS.D1(u,v,ponE,d1u,d1v);
    norm = d1u.Normalized() ^ d1v.Normalized();
    
    // NYI : improvement is needed for the compute of deltaE
    gp_Vec dponE = norm^tgE;
    Standard_Real deltaE = Duvmin*.005;
    dponE.Multiply(deltaE);
    
    for (Standard_Integer i=1; i<=2; i++) {
      if (i == 2) dponE.Reverse();
      thePoint = ponE.Translated(dponE);
      Standard_Boolean projok = FUN_projonS(thePoint,su,u,v);
      if (!projok) return Standard_False;
      Standard_Boolean ok = FUN_UVinUVminmax(u,v,UV);
      if (ok) return Standard_True;
    }
    
  }
  return Standard_False;
}

//=======================================================================
//function : FaceIsInSh
//           Uses : PInFclosetoE
//purpose  : Returns <True> if <theFace> has on internal point IN the 
//           solid <theShape>.
//           <theFace> is tangent to <theShape> on <theEdge>.
//=======================================================================
Standard_Boolean ShapeInterference_FaceIsInSh(const TopoDS_Shape& theEdge,
					      const TopoDS_Shape& theFace,
					      const TopoDS_Shape& theShape)
{
  Standard_Boolean ok = Standard_True;
  gp_Pnt thePoint;
  if (!ShapeInterference_PInFclosetoE(theEdge,theFace,thePoint)) {
    cout <<"error in ShapeInterference_FaceIsInSh"<<endl;
    return Standard_False;
  }

  // Classifying the point
  BRepClass3d_SolidClassifier classif( theShape, thePoint, Precision::Confusion() ); 
  if(classif.State() != TopAbs_IN){ ok = Standard_False; }

  return ok;
}

//=======================================================================
//function : ShareSameDomain
//purpose  : Faces <f1> and <f2> are same domain. They share the same
//           edge geometry.
//           ShareSameDomain returns <True> if they share common 2d
//           geometric domain ie if there is a 3d point internal to
//           <f1> and internal to <f2>.
//=======================================================================

Standard_Boolean ShapeInterference_ShareSameDomain
(const TopoDS_Shape& ed,
 const TopoDS_Shape& f1,
 const TopoDS_Shape& f2)
{
  // IMPORTANT : When ShareSameDomain called, <ed> have an edge ancestor
  // of rank = rank f1!
  gp_Pnt pnt;
  if (!ShapeInterference_PInFclosetoE(ed,f1,pnt)) {
    /*cout <<"Error in ShapeInterference_ShareSameDomain"<<endl;*/
    return Standard_False;
  }  

  Handle(Geom_Surface) su = BRep_Tool::Surface(TopoDS::Face(f2));
  Standard_Real u,v;
  GeomAPI_ProjectPointOnSurf PonS(pnt,su);
  Standard_Boolean projok=Standard_False;
  if (!PonS.Extrema().IsDone()) projok = Standard_False;
  if (PonS.NbPoints() == 0) projok = Standard_False;
  if (!projok) {/*cout<<"Error in ShapeInterference_ShareSameDomain"<<endl;*/ return Standard_False;}
  PonS.LowerDistanceParameters(u,v);
  
  TColStd_Array1OfReal UV(1,4);
  BRepTools::UVBounds(TopoDS::Face(f2),UV(1),UV(2),UV(3),UV(4));
  
  Standard_Boolean isinbound = FUN_UVinUVminmax(u,v,UV);
  if (!isinbound) return Standard_False;
  if (PonS.LowerDistance() > 10.*Precision::Intersection()) return Standard_False;
  else return Standard_True;
}

//=======================================================================
//function : CollisionOnSectionEdge
//           Uses : HasTgtFaces, FaceIsInSh

//purpose  : Looks if there is a collision near the section edge
//           Binds in <TangentAncestors> couple of tangent faces containing
//           the section edge <theEdge>.
//           If <TangentAncestors> is empty, <TangentAncestors> binds the
//           ancestors of <theEdge>.
//           Else, they contain edges ancestors of <theEdge>.
//=======================================================================
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
 Standard_Boolean& hascollision)
{
  // xpu : - 15-07-97
  /*Standard_Integer i, j;
  for(i = 1; i>=2; i++)
    TangentAncestors(i).Clear();
  TopTools_Array1OfListOfShape ConnexedFaces(1,2);
  if(ShapeInterference_GetConnexedFaces(Shape, AncestorToEdge, ConnexedFaces))
  {
    ShapeInterference_HasTgtFaces(HDS, ConnexedFaces, TangentAncestors);
    if( !(ConnexedFaces(1).IsEmpty() && ConnexedFaces(2).IsEmpty()) )
      // At least one list has an element
    {
      i = (ConnexedFaces(1).Extent() > ConnexedFaces(2).Extent())?1:2;
      j = (i == 1)?1:2;
      TopTools_ListIteratorOfListOfShape ite(ConnexedFaces(i));
      for(;ite.More(); ite.Next())
	nocollision = nocollision && 
	  !ShapeInterference_FaceIsInSh(theEdge, ite.Value(), Shape(j)); 
    }
  }	
  return !nocollision;*/
  // xpu : - 15-07-97

#ifdef DEB
  Standard_Boolean trc = Standard_False;
#ifdef DRAW
  if (trc) FUN_shaint_draw(theEdge);
#endif
#endif
 
  hascollision = Standard_False;
  TangentAncestors.Clear();
  TopTools_ListOfShape LF1,LF2,LE1,LE2;
  Standard_Boolean ises = HB->EdgeSectionAncestors(theEdge,LF1,LF2,LE1,LE2);
  if (!ises) return Standard_False;
  
  Standard_Boolean isEof1 = !LE1.IsEmpty();
  Standard_Boolean isEof2 = !LE2.IsEmpty();
  Standard_Boolean twocornercase = isEof1 && isEof2;
  if (!isEof1 && !isEof2) return Standard_False;

  // !!!!!!!!!!!!!!!! EdgeSectionAncestors not complete !!!!!!!!!!!!!!!!
  TopTools_Array1OfShape AncestortoE(1,2);
  TopTools_Array1OfListOfShape ConnexedF(1,2);
  if (isEof1) AncestortoE(1) = LE1.First();
  else        AncestortoE(1) = LF1.First();
  if (isEof2) AncestortoE(2) = LE2.First();
  else        AncestortoE(2) = LF2.First();
  Standard_Boolean ok = ShapeInterference_GetConnexedFaces(Shape, AncestortoE, ConnexedF);
  if (!ok) return Standard_False;
  LF1.Clear(); LF2.Clear();
  for (Standard_Integer ii = 1; ii<=2; ii++) {
    const TopTools_ListOfShape& lof = ConnexedF(ii);
    for (TopTools_ListIteratorOfListOfShape it(lof); it.More(); it.Next()) {
      if (ii == 1) LF1.Append(it.Value());
      else         LF2.Append(it.Value());

#ifdef DRAW
      if (trc) FUN_shaint_draw(it.Value());
#endif
    }
  }  

  TopTools_ListIteratorOfListOfShape it;
  TopTools_Array1OfListOfShape ConnexedFaces(1,2);
  for (it.Initialize(LF1); it.More();it.Next()) {
    TopTools_ListOfShape& los = ConnexedFaces.ChangeValue(1);
    los.Append(it.Value());
  } 
  for (it.Initialize(LF2); it.More();it.Next()) {
    TopTools_ListOfShape& los = ConnexedFaces.ChangeValue(2);
    los.Append(it.Value());
  } 
  
  Standard_Integer nF1 = ConnexedFaces(1).Extent();
  Standard_Integer nF2 = ConnexedFaces(2).Extent();
  Standard_Boolean nofaces = (nF1 == 0) && (nF2 == 0);
  if (nofaces && !twocornercase) return Standard_False;

  TopTools_DataMapOfShapeShape SdmF, doubleTgtF;
  Standard_Boolean hassdmF = 
    ShapeInterference_HasSdmFaces(HDS, ConnexedFaces, SdmF);

  if (hassdmF) {

    // !! A face can be same domain with another one without
    // sharing common 2d geometric domain
    // Two faces are tangent if they share same domain and 
    // common 2d geometric domain.

    TopTools_DataMapIteratorOfDataMapOfShapeShape it(SdmF);
    TopTools_DataMapOfShapeShape SdmDiffOri, SdmSameOri;
    for (; it.More(); it.Next()) {
      const TopoDS_Shape& f1 = it.Key();
      const TopoDS_Shape& f2 = it.Value();
      if (ShapeInterference_DiffOri(f1,f2,HDS)) SdmDiffOri.Bind(f1,f2);
      else SdmSameOri.Bind(f1,f2);
    }
    
    if (SdmDiffOri.Extent() >0) {
      it.Initialize(SdmDiffOri);
      for (; it.More(); it.Next()) {
	const TopoDS_Shape& f1 = it.Key();
	const TopoDS_Shape& f2 = it.Value();
	Standard_Boolean sharesdm 
	  = isEof1? ShapeInterference_ShareSameDomain(theEdge,f1,f2):
	    ShapeInterference_ShareSameDomain(theEdge,f2,f1);
	    
	if (sharesdm) TangentAncestors.Bind(f1,f2);
      }
    }
    
    // If faces of same domain and diff orientation share common
    // geometric domain, we have a collision
    if (SdmSameOri.Extent() >0) {
      it.Initialize(SdmSameOri);
      for (; it.More(); it.Next()) {
	const TopoDS_Shape& f1 = it.Key();
	const TopoDS_Shape& f2 = it.Value();
	if (ShapeInterference_ShareSameDomain(theEdge,f1,f2)){
	  TangentAncestors.Bind(f1,f2);
	  doubleTgtF.Bind(f1,f2); doubleTgtF.Bind(f2,f1);
	  hascollision = Standard_True;
	  break;  
	}      
      }
    }
    if (hascollision) {
      if (isEof1) AncestorToEdge(1) = LE1.First(); 
      if (isEof2) AncestorToEdge(2) = LE2.First();
      return Standard_True;
    }
  } 

  if (twocornercase) { AncestorToEdge(1) = LE1.First(); AncestorToEdge(2) = LE2.First(); }
  Standard_Integer ntgtF = TangentAncestors.Extent();
  Standard_Boolean hastgtF = (ntgtF > 0);
 
  Standard_Integer i = (nF1 > nF2)? 1: 2;
  Standard_Integer j = (i == 1)? 2: 1;
  Standard_Boolean nocollision = Standard_True;

  if (hastgtF) {

   if (ConnexedFaces(j).Extent() - ntgtF < 1) return Standard_True;

    // We process on faces that are not tangent
    TopTools_ListIteratorOfListOfShape ite(ConnexedFaces(i));
    for(;ite.More(); ite.Next()) {
      if (doubleTgtF.IsBound(ite.Value())) continue;
      nocollision = nocollision && 
	!ShapeInterference_FaceIsInSh(theEdge, ite.Value(), Shape(j)); 
      if (!nocollision) break;
    }    

  }
  else {
    
    if (!twocornercase) {
      if (isEof1) { AncestorToEdge(1) = LE1.First(); AncestorToEdge(2) = LF2.First(); }
      if (isEof2) { AncestorToEdge(2) = LE2.First(); AncestorToEdge(1) = LF1.First(); }
    }

    TopTools_ListIteratorOfListOfShape ite(ConnexedFaces(i));
    for(;ite.More(); ite.Next()) {
      nocollision = nocollision && 
	!ShapeInterference_FaceIsInSh(theEdge, ite.Value(), Shape(j)); 
      if (!nocollision) break;
    }    
    hascollision = !nocollision;
  }
  
  return Standard_True;
}

//=======================================================================
//function : parVonF

//purpose  : Computes UV parameters of <v> in <F>. Returns <False> if the
//           compute fails.
//=======================================================================
Standard_Boolean ShapeInterference_parVonF
(const TopoDS_Vertex& v,
 const TopoDS_Face& F,
 gp_Pnt2d& p2d)
{
  Standard_Boolean isVofF=Standard_False;
  for (TopExp_Explorer ex(F,TopAbs_VERTEX); ex.More(); ex.Next()) {
    const TopoDS_Vertex& vcur = TopoDS::Vertex(ex.Current());
    isVofF = vcur.IsSame(v);
    if (isVofF) {
      p2d = BRep_Tool::Parameters(vcur,F);
      break; 
    }
  }
  if (!isVofF) { 
    gp_Pnt pt = BRep_Tool::Pnt(v);
    // <v> can share same domain with a vertex of <F>
    for (TopExp_Explorer ex(F,TopAbs_VERTEX); ex.More(); ex.Next()) {
      const TopoDS_Vertex& vex = TopoDS::Vertex(ex.Current());
      gp_Pnt ptex = BRep_Tool::Pnt(vex);
      Standard_Real tol = Precision::Confusion();
      if (ptex.IsEqual(pt,tol)) {
	p2d = BRep_Tool::Parameters(vex,F);
	return Standard_True;
      }
    }
    Handle(Geom_Surface) su = BRep_Tool::Surface(F);
    GeomAPI_ProjectPointOnSurf pro(pt,su);
    Standard_Boolean done = pro.Extrema().IsDone() && (pro.NbPoints() >0);
    if (!done) return Standard_False;
    Standard_Real u,v;
    pro.LowerDistanceParameters(u,v);
    p2d = gp_Pnt2d(u,v);
  }
  return Standard_True;
}

//=======================================================================
//function : getnormal
//           Uses : parVonF

//purpose  : Computes topological normal to face <F> at vertex <v>.
//           Returns <False> if the compute of UV parameters fails
//=======================================================================
Standard_Boolean ShapeInterference_getnormal
(const TopoDS_Vertex& v,
 const TopoDS_Face& F,
 gp_Vec normt)
{
  gp_Pnt2d UV;
  Standard_Boolean parok = ShapeInterference_parVonF(v,F,UV);
  if (!parok) return Standard_False;
  
  BRepAdaptor_Surface BS(F);
  gp_Pnt p; gp_Vec d1u,d1v;
  BS.D1(UV.X(), UV.Y(), p, d1u, d1v);
  normt = d1u^d1v; normt.Normalize();
  if (F.Orientation() == TopAbs_REVERSED) normt.Reverse();
  return Standard_True;
}

//=======================================================================
//function : IsAnInclusion
//           Uses : getnormal,parVonF

//purpose  : Returns <True> is <theInterf> describes a case of inclusion
//           with <theInterf> binding only tangent cases.
//           Is called when : 
//           - one bounding box of the shapes is included in the other
//           - all vertices of the shape of smaller box are classified ON
//             faces of the other solid
//=======================================================================
Standard_Boolean ShapeInterference_IsAnInclusion(const ShapeInterference_Interference& theInterf,
						 const Standard_Boolean OneIsInTwo)
{
  // The bounding box of shape<Ismaller> is contained by the other one,
  // All vertices of shape<Ismaller> are classified ON the opposite face.
  Standard_Integer Ismaller = (OneIsInTwo)? 1: 2;
  Standard_Integer Igreater = (OneIsInTwo)? 2: 1;

  ShapeInterference_SequenceOfInterfObj seqofT;
  theInterf.GetSeqOfTangencies(seqofT);
  for(Standard_Integer ii = 1; ii <= seqofT.Length(); ii++){
    Handle(ShapeInterference_Tangency) tgcy = 
      Handle(ShapeInterference_Tangency)::DownCast(seqofT.Value(ii));
    if (tgcy->GetTgcyType() == ShapeInterference_TOT_TGCYONEDGE) continue;
    TopTools_Array1OfShape face(1,2);
    tgcy->GetGeneratingShapes(face(1),face(2));
    
    // getting UV parameters on faces
    TopExp_Explorer exv(face(Ismaller),TopAbs_VERTEX);
    const TopoDS_Vertex& v = TopoDS::Vertex(exv.Current());
    gp_Vec Nsma(gp_Dir(1.,0.,0.)), Ngre(gp_Dir(1.,0.,0.));
    Standard_Boolean smaok = ShapeInterference_getnormal(v,TopoDS::Face(face(Ismaller)),Nsma);
    Standard_Boolean greok = ShapeInterference_getnormal(v,TopoDS::Face(face(Igreater)),Ngre);
    if (!smaok || !greok) continue;
  
    // If there exist at least on couple of faces with topological normals
    // with same direction, we have an inclusion
    // else, as no collision has been found yet, all cases are tangency cases
    if (Nsma.Dot(Ngre) >0.) return Standard_True;   
    else return Standard_False;
  }

  cout <<"Error in ShapeInterference_IsAnInclusion "<<endl; 
  return Standard_False;
}

void ShapeInterference_addToItem
(TopTools_DataMapOfIntegerListOfShape& mapTgcies,
 const Standard_Integer& Ise,
 const TopoDS_Shape& fa)
{
  if (mapTgcies.IsBound(Ise)){
    TopTools_ListOfShape& los = mapTgcies.ChangeFind(Ise);
    los.Append(fa);
  }
  else{ 
    TopTools_ListOfShape los; los.Append(fa);
    mapTgcies.Bind(Ise,los);
  }
}

void ShapeInterference_TgciesToInterf
(const TopTools_IndexedDataMapOfShapeShape& mapf1tgTof2,
 const TopTools_DataMapOfIntegerListOfShape& mapTgcies,
 ShapeInterference_Interference& theInterf)
{
  TopTools_DataMapIteratorOfDataMapOfIntegerListOfShape it(mapTgcies);
  BRep_Builder BB;
  for (; it.More(); it.Next()) {
    Standard_Integer If1f2 = it.Key();
    const TopTools_ListOfShape& loe = it.Value();

    TopoDS_Compound comp; BB.MakeCompound(comp);
    for (TopTools_ListIteratorOfListOfShape itlo(loe); itlo.More(); itlo.Next())
      BB.Add(comp, itlo.Value());
      
    if (If1f2 > mapf1tgTof2.Extent()) 
      { cout<<"ShapeInterference_TgciesToInterf: error on map"<<endl; return;}
    const TopoDS_Shape& f1 = mapf1tgTof2.FindKey(If1f2);
    const TopoDS_Shape& f2 = mapf1tgTof2.FindFromIndex(If1f2);

    Handle(ShapeInterference_Tangency) tgcy = new ShapeInterference_Tangency;
    tgcy->SetTgcyType(ShapeInterference_TOT_TGCYONFACE);
    tgcy->SetGeneratingShapes(f1,f2);
    tgcy->SetSection(comp);
    theInterf.AddInterfObj(tgcy);
  }
}
