// File:	ShapeInterference.cxx
// Created:	Tue Jan 28 10:35:23 1997
// Author:	Prestataire Xuan PHAM PHU
//		<xpu@salgox.paris1.matra-dtv.fr>


#include <ShapeInterference.ixx>
#include <ShapeInterference_methods.hxx>
#include <ShapeInterference_Interference.hxx>
#include <ShapeInterference_SequenceOfInterfObj.hxx>
#include <ShapeInterference_Collision.hxx>
#include <ShapeInterference_Tangency.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>

#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeShape.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_DataMapIteratorOfDataMapOfShapeShape.hxx>
#include <TopTools_SequenceOfShape.hxx>

#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>

#include <TColStd_Array1OfBoolean.hxx>

#include <BRepAlgo_Section.hxx>
#include <BRepAlgo_Common.hxx>

#include <Bnd_Array1OfBox.hxx>
#include <BRepBndLib.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>

#include <Precision.hxx>
//#include <Standard_ErrorHandler.hxx>


Standard_Boolean ShapeInterference::ComputeFirstCollision(const TopoDS_Shape& aShape1,
							  const TopoDS_Shape& aShape2,
							  ShapeInterference_Interference& theInterf)
{
  Standard_Boolean ok = Standard_True;
  return ok;
}

Standard_Boolean ShapeInterference::ComputeCommon(const TopoDS_Shape& aShape1,
						  const TopoDS_Shape& aShape2,
						  ShapeInterference_Interference& theInterf)
{
  Standard_Boolean ok = Standard_True;
  TopoDS_Shape common = BRepAlgo_Common(aShape1, aShape2);

  if(common.IsNull()){return !ok;}
  else
  {
    Handle(ShapeInterference_Collision) acollision = new ShapeInterference_Collision();
    acollision->SetCommon(common);
    theInterf.AddInterfObj(acollision);
  }

  return ok;
}

//=======================================================================
//function : ComputeInterference

//           NYI!!!!!!!!! UPDATE THE COMMENTS


//           This method calls the following methods :
//           - IsInBndBox ( b1, b2, 1In2 ) returning <True> if one of the boxes
//           is contained in the other. <1In2> is true if <b1> is in <b2>.

//           - TestOnVertex( s1, s2, allvof1ON2) returning <True> if <s1> is contained in <s2>.
//           It is called after IsInBndBox if IsInBndBox returns True.
 
//           - TgcyOnEdToTgcyOnFa( HDS, sh, shOnS, anc ) returning in <anc>
//           the ancestors generating the section edge of ancestors <shOnS> 
//           while looking for a case of tangency on face. 
//           Uses : the data structure <HDS>,
//                  the shapes of the section <sh>.

//           - AddToMap( map, shOn1, shOn2, se ) adding the section edge <se>
//           to the couple of ancestor shapes (<shOn1>,<shOn2>) in the map <map>.

//           - MapOfTgciesToInterf( map, interf ) adding the tangency cases found
//           in <map> to <interf>.

//           - GetAncestorsFacesFromEdge( ed, HB, isaC, Sh, Anc, 2cornercase )
//           returns in <Anc> the ancestor shapes generating the section edge <ed>.
//           Sets <2cornercase> to true if edge <ed> belongs to two corners of <sh>.
//           Uses :<HB>, the builder attached to the topological operation Section
//                <isaC>, setted to true if <ed> is a curveedge
//                <Sh>, the shapes involved in the section.

//           - ShapeInterference_CollisionOnSectionEdge(HDS, se, sh, anc, 2cornercase)
//           returns true if there is a collision about the section edge <se>
//           common to its ancestors shapes <anc>
//           Uses : <HDS>, the HData Structure
//                  <anc>, the ancestors computed by the previous method
//                  2cornercase, setted to true if the two ancestors are edges.

//purpose  : This method uses the topological operator Section to detect
//           collisions or  tangencies between the two shapes <theShape1> 
//           and  <theShape2>.
//           Section returns a compound of edges. These edges belong to the
//           two shapes' shells.
//           There are two kind of edges returned by Section:
//           - the new_edges computed with a common between two faces,
//           - the old_edges computed with an edge of one of the two shapes. 
//           If a section edge is part of two edges of the old topology,
//           Section returns it only if we demand it, so do we.
 
//           If <withApprox> is False, the new_edges are computed with 
//           approximation BSPLINE1.

//           If <stopatfirstcoll> is true : the analysis will stop at first
//           collision found and tangency cases will not be precisely
//           described.
//           Else : even if a collision is found, the tangency cases will be 
//           precisely described.

//           !!: The DS Interference returned is partially filled,
//               improvement is expected if the user want to specify the
//               number of collisions occured in the section.
//=======================================================================

Standard_Boolean ShapeInterference::ComputeInterference(const TopoDS_Shape& theShape1,
							const TopoDS_Shape& theShape2,
							ShapeInterference_Interference& theInterf,
							const Standard_Boolean withApprox,
							const Standard_Boolean stopatfirstcoll)

{
  theInterf.Empty();
  // NYI : modifier cdl : SetInclusion (aboolean);
  if (theInterf.HasInclusion()) theInterf.SetInclusion();

  Standard_Boolean isaCollision, isContained=Standard_False;
  TopoDS_Shape aResultSection;
  TopoDS_Compound aCollisionResult;
  
  if( theShape1.IsNull() || theShape2.IsNull() )
    Standard_Failure::Raise("ShapeInterference: Null Shape");
  
  // Computes the array of shapes theShape(1,2)
  // Test on the bounding boxes aBndBox(1), aBndBox(2) of the shapes:
  // If they are independant, there is no interference :
  // returns False
  
  Standard_Integer i,j;
  TopTools_Array1OfShape theShape(1,2), ancestorOfEd(1,2);
  theShape(1) = theShape1;
  theShape(2) = theShape2;
  
  Bnd_Array1OfBox aBndBox(1, 2);
  for(i = 1; i <= 2; i++) {
    BRepBndLib::Add( theShape(i), aBndBox(i) ); 
    aBndBox(i).SetGap(0.);
  }
  if( aBndBox(1).IsOut( aBndBox(2) )){return Standard_False;}

  // + 11-07-97
  // See if one bounding box is contained in the other
  // if so, we can have a collision as well as a case of tangency, or even 
  // no interference.
  Standard_Boolean OneIsInTwo = Standard_False, isinotherbox, allverticesON = Standard_False;
  isinotherbox = ShapeInterference_IsInBndBox ( aBndBox(1), aBndBox(2), OneIsInTwo);

  if (isinotherbox) {
    i = OneIsInTwo?1:2;
    j = OneIsInTwo?2:1;
    isContained = ShapeInterference_TestOnVertex(theShape(i), theShape(j),allverticesON);
  }
  // + 11-07-97

  // ( The boxes have common part )
  // Computes the section between the solids : 
  // sets aResultSection =  the section result,
  // sets isaCollision = False.
  
  // !!!!!!!!!!!!See when implemented
//  BRepAlgo_Section section(theShape(1), theShape(2), withApprox);
  BRep_Builder BB;
  BRepAlgo_Section section(theShape(1), theShape(2));
  aResultSection =  section.Shape();  
  isaCollision = Standard_False;
  BB.MakeCompound(aCollisionResult);
    
  //(a) ( section returns empty compound ) :
  // If Boxi is contained in Boxj :
    // isContained = TestOnVertex (theShape(i), theShape(j)),
    // if isContained is true :
       // returns an isContained interference,
    // else returns False.
  TopExp_Explorer expl(aResultSection, TopAbs_EDGE);
  if (!expl.More()) {
    /*Standard_Boolean OneIsInTwo, isinotherbox;
    isinotherbox = ShapeInterference_IsInBndBox ( aBndBox(1), aBndBox(2), OneIsInTwo);
    if (isinotherbox) {
      i = OneIsInTwo?1:2;
      j = OneIsInTwo?2:1;
      isContained = ShapeInterference_TestOnVertex(theShape(i), theShape(j));
      if (!isContained) { return Standard_False;}
      theInterf.SetInclusion();      
      Handle(ShapeInterference_Collision) acollision = new ShapeInterference_Collision();
      acollision->SetCommon(theShape(i));
      theInterf.AddInterfObj(acollision);
      return Standard_True;
    }*/
    // + 11-07-97
    if (isinotherbox) {
      if (!isContained) { return Standard_False;}
      theInterf.SetInclusion();      
      Handle(ShapeInterference_Collision) acollision = new ShapeInterference_Collision();
      Standard_Integer icontained = OneIsInTwo?1:2;
      acollision->SetCommon(theShape(icontained));
      theInterf.AddInterfObj(acollision);
      return Standard_True;
    }
    // + 11-07-97
    return Standard_False;
  }  

  // Data Structure to store tangent shapes generating section edges:
  // mapoftgcies is an indexed map, with :
  // as Key : one shape on <theShape1>.
  // as Item : the list of shapes on <theShape2> tangent to the Key,
  //           and the list of compounds of section edges generated 
  //           These objects are stored in the following order :
  //           (a shape on s2, a compound of section edges), (,)...

//  TopTools_DataMapOfShapeListOfShape mapoftgcies; // xpu : - 16-07-97
  // xpu : +16-07-97
  TopTools_IndexedDataMapOfShapeShape mapf1tgTof2;
  TopTools_DataMapOfIntegerListOfShape mapTgcies;
  // xpu : +16-07-97

  // Getting the HBuilder and the DataStructure attached to Section :

  const Handle( TopOpeBRepBuild_HBuilder )& HB = section.Builder();
  const Handle( TopOpeBRepDS_HDataStructure )& HDS = HB->DataStructure();
  const TopOpeBRepDS_DataStructure& DS = HDS->DS();
  
  //(b) If there is at least one edge returned by Section of kind new_edge,
    // While !isaCollision explode the compound of new_edges, and for 
    // each edge see  :
      // if !( the two faces generating the edge are tangent ) :
        // sets isaCollision = True.
        // if stopatfirstcoll : returns the collision interference with all section edges. 
        // else : adds the edge to the compound aCollisionResult and removes it from the
        //        compound aResultSection
      // else : 
        // if !stopatfirstcoll : adds it to the map mapoftgcies.

  // As result in the end :
    // if stopatfirstcoll : if there is an new_edge generated by a collision, the algorithm
    //                      will return all section edges in a collision interference.
    // else : compound aResultSection is unchanged. Interference is returned in step (c) of the algo.

  Standard_Integer ic = 1, nbcurves = DS.NbCurves();
  Standard_Boolean stop = Standard_False;

  if (nbcurves > 0)
  { 
    // Getting the compound of new_edges 
    // !!! Call TopOpeBRepBuild_Builder::SectionCurves if possible
    for(; ic <= nbcurves; ic++) 
      {
	TopTools_ListIteratorOfListOfShape itloe(HB->NewEdges(ic));	  
	for(; itloe.More();itloe.Next())
	  {
	    // See if the faces connexed to the new_edge are tangent.
	    // If not, set isaCollision = True
	    TopoDS_Shape ancestor1, ancestor2;
	    Standard_Integer iic;
	    HB->EdgeCurveAncestors(itloe.Value(),ancestor1,ancestor2,iic); 
//	    isaCollision = !ShapeInterference_SameDomainandDiffOri(ancestor1,ancestor2,HDS); // xpu : - 16-07-97
	    // xpu : + 16-07-97
	    Standard_Boolean sdm = ShapeInterference_SameDomain(ancestor1,ancestor2,HDS);
	    Standard_Boolean dori = ShapeInterference_DiffOri(ancestor1,ancestor2,HDS);
	    isaCollision = !(sdm && dori);
	    // xpu : + 16-07-97
	    stop = stopatfirstcoll && isaCollision;
	    
	    // xpu : + 16-07-97
	    if (!isaCollision) {
	      Standard_Integer If1f2 = mapf1tgTof2.Add(ancestor1,ancestor2);
	      ShapeInterference_addToItem(mapTgcies,If1f2,itloe.Value());
	    }
	    // xpu : + 16-07-97

	    if(!stopatfirstcoll) {
	      if(!isaCollision) {
//		ShapeInterference_AddToMap(mapoftgcies,ancestor1,ancestor2,itloe.Value()); // xpu : - 16-07-97
		BB.Remove(aResultSection,itloe.Value());
	      }
	      else {
		BB.Add(aCollisionResult, itloe.Value());
		BB.Remove(aResultSection,itloe.Value());
	      }
	    } // !stopatfirstcoll
	    
	    if(stop){ break;}
	  } // itloe
	
	if(stop){ break;}
      } // ic
    
    if(stop) {
      Handle(ShapeInterference_Collision) acollision = new ShapeInterference_Collision();
      acollision->SetSection(aResultSection);
      theInterf.AddInterfObj(acollision);
      return Standard_True;
    }      
    
  } //nbcurves>0 


  //(c) If there is at least one edge returned by Section of kind old_edge
  // ( there is none of kind new_edge in aResultSection),   
   // While !isaCollision explode the compound of section edges, and for 
    // each edge as it can be in one of the two cases :
    // (generated by an edge E and a face ) || (generated by two edges E and E') :
      // search the two faces connexed to E
      // while !isaCollision, explode the array of two faces :
        // if one of the two faces is "locally" contained by the opposite solid  :
        // set isaCollision = True.
        // returns the collision interference
   //( isaCollision is false ), it's a case of tangency
   // returns the tangency interference

  expl.Init(aResultSection, TopAbs_EDGE);
  for(; expl.More(); expl.Next())
  {
    const TopoDS_Shape& se = expl.Current();
    // Getting the connexed topology to the old_edge :
    // If 2cornerCase : ancestorOfEd contains the two edges from the
    // two shapes which common gives the old_edge.
    // Else : ancestorOfEd contains the edge and the face of the two 
    // distinct shapes which common gives the old_edge.

    // xpu : - 15-07-97
    /*TopTools_ListOfShape LF1,LF2,LE1,LE2;
      Standard_Boolean ises = HB->EdgeSectionAncestors(se,LF1,LF2,LE1,LE2);

      if (!ises) {
	cout << "ShapeInterference::ComputeInterference : Bad Data on one section edge"<<endl;
	// bad data : removing the section edge
	if(stopatfirstcoll) BB.Remove(aResultSection, se);
	continue;
      }

      // !!!!!!!!!!!!!!!     MODIFS TO BRING   !!!!!!!!!!!!!!!!!!!!!!!
      // ShapeInterference_CollisionOnSectionEdge :
      // !!!!!!!!To update when the interferences transitions IN/OUT for
      // solids will be implemented in Section :
      // You will only have to analyse the transitions given by the DS!!

      Standard_Boolean isonEof1 = !LE1.IsEmpty();
      Standard_Boolean isonEof2 = !LE2.IsEmpty();
      Standard_Boolean twocornercase = isonEof1 && isonEof2;
      if (isonEof1) ancestorOfEd.SetValue(1,LE1.First());
      if (isonEof2) ancestorOfEd.SetValue(2,LE2.First());
      if (!isonEof1) ancestorOfEd.SetValue(1,LF1.First());
      if (!isonEof2) ancestorOfEd.SetValue(2,LF2.First());
      TopTools_Array1OfListOfShape tgtAnc(1,2);
      isaCollision = ShapeInterference_CollisionOnSectionEdge(HDS,se,theShape,ancestorOfEd, 
							      twocornercase,tgtAnc);
      if (!ises){
	cout << "ShapeInterference::ComputeInterference : Bad Data on one section edge"<<endl;
	// bad data : removing the section edge
	if(stopatfirstcoll) BB.Remove(aResultSection, se);
	continue;
      }*/ 
      // xpu : - 15-07-97
     
    TopTools_Array1OfShape AncToEd(1,2); // the section edge ancestors. If has tangent faces, only edges.
    TopTools_DataMapOfShapeShape TgtAnc; // tangent faces same and diff oriented
    Standard_Boolean sedone = ShapeInterference_CollisionOnSectionEdge(HB,HDS,se,theShape,
								       AncToEd,TgtAnc,isaCollision);
    if (!sedone){
      cout << "ShapeInterference::ComputeInterference : bad data on section edge"<<endl;
      // bad data : removing the section edge
      if(stopatfirstcoll) BB.Remove(aResultSection, se);
      continue;
    }

    // Analysis :
    TColStd_Array1OfBoolean isEof(1,2);
    for (Standard_Integer i = 1; i <= 2; i++)
      isEof(i) = !AncToEd(i).IsNull() && (AncToEd(i).ShapeType() == TopAbs_EDGE);

    Standard_Boolean twocornercase = isEof(1) && isEof(2);
    Standard_Boolean hastgtF = (TgtAnc.Extent() > 0);

    if (twocornercase && !hastgtF && !isaCollision) {
      Handle(ShapeInterference_Tangency) atgcy = new ShapeInterference_Tangency();
      atgcy->SetSection(aResultSection);
      atgcy->SetGeneratingShapes(AncToEd(1), AncToEd(2));
      atgcy->SetTgcyType(ShapeInterference_TOT_TGCYONEDGE);
      theInterf.AddInterfObj(atgcy);      
    }
    
    // Filling the maps :
    
    if (hastgtF) {
      TopTools_DataMapIteratorOfDataMapOfShapeShape it(TgtAnc);
      for (; it.More(); it.Next()) {
	const TopoDS_Shape& f1 = it.Key();
	const TopoDS_Shape& f2 = it.Value();
	mapf1tgTof2.Add(f1,f2);
	Standard_Integer If1f2 = mapf1tgTof2.Add(f1,f2);
	ShapeInterference_addToItem(mapTgcies,If1f2,se);
      }
    }

    if(stopatfirstcoll && isaCollision) {
      Handle(ShapeInterference_Collision) acollision = new ShapeInterference_Collision();
      acollision->SetSection(aResultSection);
      theInterf.AddInterfObj(acollision);
      return Standard_True;
    }
    if(!stopatfirstcoll) {

      if(isaCollision) BB.Add(aCollisionResult,se);  
      else {	
//	  ShapeInterference_AddToMap(mapoftgcies,ancestorOfEd(1),ancestorOfEd(2),se); // xpu : - 15-07-97
      }      
    }
  
  }  // expl(aResultSection, TopAbs_EDGE)
  

  // (d) Filling <theInterf> :
  
  ShapeInterference_TgciesToInterf(mapf1tgTof2,mapTgcies,theInterf);
  
  /*if(stopatfirstcoll) // only tangent cases
  {
   Handle(ShapeInterference_Tangency) atgcy = new ShapeInterference_Tangency();
    atgcy->SetSection(aResultSection);
    theInterf.AddInterfObj(atgcy);
  }
  else
  {
    expl.Init(aCollisionResult, TopAbs_EDGE);
    if(expl.More())
    {
      Handle(ShapeInterference_Collision) acollision = new ShapeInterference_Collision();
      acollision->SetSection(aCollisionResult);
      theInterf.AddInterfObj(acollision);
    }
    ShapeInterference_MapOfTgciesToInterf(mapoftgcies, theInterf);
  }*/ // xpu : - 16-07-97    

  expl.Init(aCollisionResult, TopAbs_EDGE);
  if (expl.More()) {

    if (allverticesON) {
      theInterf.SetInclusion();     
      Handle(ShapeInterference_Collision) acollision = new ShapeInterference_Collision();
      Standard_Integer icontained = OneIsInTwo?1:2;
      acollision->SetSection(aCollisionResult);
      acollision->SetCommon(theShape(icontained));
      theInterf.AddInterfObj(acollision); 
    }
    else {
      Handle(ShapeInterference_Collision) acollision = new ShapeInterference_Collision();
      acollision->SetSection(aCollisionResult);
      theInterf.AddInterfObj(acollision);
    }
  }
  
  // Even if <stopatfirstcoll>, if its an inclusion case, <I> must bind
  // all tangencies.
  /*if (theInterf.GetCollisionNbr() == 0) {
    if (allverticesON) {
      // If there is a couple of tangent faces for which the topological
      // normals are same oriented, we have an inclusion.
      // Else, we only have tangency cases.
      Standard_Boolean isaninclusion = ShapeInterference_IsAnInclusion(theInterf,OneIsInTwo);
      if (!isaninclusion) return Standard_True; 
      else isContained = Standard_False;
    }
    if (isContained) {
      theInterf.SetInclusion();     
      Handle(ShapeInterference_Collision) acollision = new ShapeInterference_Collision();
      Standard_Integer icontained = OneIsInTwo?1:2;
      acollision->SetCommon(theShape(icontained));
      theInterf.AddInterfObj(acollision); 
    }     
  }*/

  return Standard_True; 
}

