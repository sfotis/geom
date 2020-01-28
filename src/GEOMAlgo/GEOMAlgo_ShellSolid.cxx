// Copyright (C) 2007-2013  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// File:        GEOMAlgo_ShellSolid.cxx
// Created:     Wed Jan 12 12:49:45 2005
// Author:      Peter KURNEV
//              <pkv@irinox>
//
#include <GEOMAlgo_ShellSolid.hxx>

#include <Standard_Failure.hxx>

#include <gp_Pnt2d.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>

#include <BRep_Tool.hxx>
#include <BRepTools.hxx>

#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopExp_Explorer.hxx>

#include <BRepClass3d_SolidClassifier.hxx>

#include <XIntTools_Context.hxx>
#include <XBOPTColStd_Dump.hxx>
#include <XBooleanOperations_ShapesDataStructure.hxx>

#include <XBOPTools_PaveFiller.hxx>
#include <XBOPTools_SolidStateFiller.hxx>
#include <XBOPTools_PCurveMaker.hxx>
#include <XBOPTools_DEProcessor.hxx>
#include <XBOPTools_InterferencePool.hxx>
#include <XBOPTools_CArray1OfSSInterference.hxx>
#include <XBOPTools_ListOfPaveBlock.hxx>
#include <XBOPTools_ListIteratorOfListOfPaveBlock.hxx>
#include <XBOPTools_PaveBlock.hxx>
#include <XBOPTools_SSInterference.hxx>
#include <XBOPTools_SequenceOfCurves.hxx>
#include <XBOPTools_Curve.hxx>
#include <XBOPTools_PaveFiller.hxx>
#include <XBOPTools_SplitShapesPool.hxx>
#include <XBOPTools_Tools3D.hxx>
#include <XBOPTools_DSFiller.hxx>

#include <XBOP_WireEdgeSet.hxx>
#include <XBOP_SDFWESFiller.hxx>
#include <XBOP_FaceBuilder.hxx>

//=======================================================================
//function : GEOMAlgo_ShellSolid
//purpose  :
//=======================================================================
GEOMAlgo_ShellSolid::GEOMAlgo_ShellSolid()
:
  GEOMAlgo_ShapeSolid()
{
}
//=======================================================================
//function : ~
//purpose  :
//=======================================================================
GEOMAlgo_ShellSolid::~GEOMAlgo_ShellSolid()
{
}
//=======================================================================
// function:
// purpose:
//=======================================================================
void GEOMAlgo_ShellSolid::Perform()
{
  myErrorStatus=0;
  //
  try {
    if (myDSFiller==NULL) {
      myErrorStatus=10;
      return;
    }
    if(!myDSFiller->IsDone()) {
      myErrorStatus=11;
      return;
    }
    //
    Standard_Boolean bIsNewFiller;
    //
    bIsNewFiller=myDSFiller->IsNewFiller();
    if (bIsNewFiller) {
      Prepare();
      myDSFiller->SetNewFiller(!bIsNewFiller);
    }
    //
    myRank=(myDSFiller->DS().Object().ShapeType()==TopAbs_SHELL) ? 1 : 2;
    BuildResult();
  }
  catch (Standard_Failure) {
    myErrorStatus=12;
  }
}
//=======================================================================
// function: Prepare
// purpose:
//=======================================================================
void GEOMAlgo_ShellSolid::Prepare()
{
  const XBOPTools_PaveFiller& aPaveFiller=myDSFiller->PaveFiller();
  //
  // 1 States
  XBOPTools_SolidStateFiller aStateFiller(aPaveFiller);
  aStateFiller.Do();
  //
  // 2 Project section edges on corresp. faces -> P-Curves on edges.
  XBOPTools_PCurveMaker aPCurveMaker(aPaveFiller);
  aPCurveMaker.Do();
  //
  // 3. Degenerated Edges Processing
  XBOPTools_DEProcessor aDEProcessor(aPaveFiller);
  aDEProcessor.Do();
  //
  // 4. Detect Same Domain Faces
  DetectSDFaces();
}
//=================================================================================
// function: BuildResult
// purpose:
//=================================================================================
void GEOMAlgo_ShellSolid::BuildResult()
{
  Standard_Boolean bIsTouchCase;
  Standard_Integer i, j, nF1, nF2, aNbFFs, aNbS, aNbCurves, nSp, iRank1;
  Standard_Integer nE, nF, aNbPB, iBeg, iEnd;
  XBooleanOperations_StateOfShape aState;
  TopExp_Explorer anExp;
  TopAbs_ShapeEnum aType;
  gp_Pnt2d aP2D;
  gp_Pnt aP3D;
  //
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  const XBOPTools_InterferencePool& anInterfPool=myDSFiller->InterfPool();
  XBOPTools_InterferencePool* pInterfPool=(XBOPTools_InterferencePool*) &anInterfPool;
  XBOPTools_CArray1OfSSInterference& aFFs=pInterfPool->SSInterferences();
  const XBOPTools_PaveFiller& aPaveFiller=myDSFiller->PaveFiller();
  const XBOPTools_SplitShapesPool& aSplitShapesPool=aPaveFiller.SplitShapesPool();
  //
  // 1. process pf non-interferring faces
  iBeg=1;
  iEnd=aDS.NumberOfShapesOfTheObject();
  if (myRank==2) {
    iBeg=iEnd+1;
    iEnd=aDS.NumberOfSourceShapes();
  }
  //
  for (i=iBeg; i<=iEnd; ++i) {
    aType=aDS.GetShapeType(i);
    if (aType!=TopAbs_FACE) {
      continue;
    }
    //
    const TopoDS_Face& aF1=TopoDS::Face(aDS.Shape(i));
    aState=aDS.GetState(i);
    if (aState==XBooleanOperations_IN) {
      myLSIN.Append(aF1);
    }
    else if (aState==XBooleanOperations_OUT) {
      myLSOUT.Append(aF1);
    }
  }
  //
  // 2. process pf interferred faces
  aNbFFs=aFFs.Extent();
  for (i=1; i<=aNbFFs; ++i) {
    XBOPTools_SSInterference& aFFi=aFFs(i);
    //
    nF1=aFFi.Index1();
    nF2=aFFi.Index2();
    iRank1=aDS.Rank(nF1);
    nF=(iRank1==myRank) ? nF1 : nF2;
    const TopoDS_Face& aF1=TopoDS::Face(aDS.Shape(nF));
    //
    bIsTouchCase=aFFi.IsTangentFaces();
    //
    if (bIsTouchCase) {
      myLSON.Append(aF1);
      continue;
    }
    //
    // Has section edges ?
    aNbS=0;
    XBOPTools_SequenceOfCurves& aBCurves=aFFi.Curves();
    aNbCurves=aBCurves.Length();
    for (j=1; j<=aNbCurves; j++) {
      XBOPTools_Curve& aBC=aBCurves(j);
      const XBOPTools_ListOfPaveBlock& aSectEdges=aBC.NewPaveBlocks();
      aNbS=aSectEdges.Extent();
      if (aNbS) {
        break;
      }
    }
    //
    if (aNbS) { // it has
      continue;
    }
    //
    anExp.Init(aF1, TopAbs_EDGE);
    for (; anExp.More(); anExp.Next()) {
      const TopoDS_Edge& aE=TopoDS::Edge(anExp.Current());
      if (BRep_Tool::Degenerated(aE)) {
        continue;
      }
      //
      nE=aDS.ShapeIndex(aE, myRank);
      const XBOPTools_ListOfPaveBlock& aLPB=aSplitShapesPool(aDS.RefEdge(nE));
      aNbPB=aLPB.Extent();
      //
      if (aNbPB<2) {
        nSp=nE;
        if (aNbPB) {
          const XBOPTools_PaveBlock& aPB=aLPB.First();
          nSp=aPB.Edge();
        }
        /*const TopoDS_Shape& aSp=*/aDS.Shape(nSp);
        //
        aState=aDS.GetState(nSp);
        if (aState==XBooleanOperations_IN) {
          myLSIN.Append(aF1);
        }
        else if (aState==XBooleanOperations_OUT) {
          myLSOUT.Append(aF1);
        }
        else if (aState==XBooleanOperations_ON) {
          Standard_Real aTol;
          TopAbs_State aSt;
          //
          //const TopoDS_Face& aF2=TopoDS::Face(aDS.Shape((iRank1==myRank)? nF2 : nF1));
          //aTol=BRep_Tool::Tolerance(aF2);
          aTol=1.e-7;
          //
          XBOPTools_Tools3D::PointNearEdge(aE, aF1, aP2D, aP3D);
          const TopoDS_Solid& aRefSolid=(myRank==1) ?
            TopoDS::Solid(aDS.Tool()) : TopoDS::Solid(aDS.Object());
          //
          XBOPTools_PaveFiller* pPF=(XBOPTools_PaveFiller*)& aPaveFiller;
          const Handle(XIntTools_Context)& aCtx=pPF->Context();
          //
          BRepClass3d_SolidClassifier& aSC=aCtx->SolidClassifier(aRefSolid);
          aSC.Perform(aP3D, aTol);
          aSt=aSC.State();
          if (aSt==TopAbs_IN) {
            myLSIN.Append(aF1);
          }
          else if (aSt==TopAbs_OUT) {
            myLSOUT.Append(aF1);
          }
        }
        break;
      } // if (aNbPB<2) {
    } //for (; anExp.More(); anExp.Next())
  }
}
//=======================================================================
// function: DetectSDFaces
// purpose:
//=======================================================================
void GEOMAlgo_ShellSolid::DetectSDFaces()
{
  const XBooleanOperations_ShapesDataStructure& aDS=myDSFiller->DS();
  XBOPTools_InterferencePool* pIntrPool=(XBOPTools_InterferencePool*)&myDSFiller->InterfPool();
  XBOPTools_CArray1OfSSInterference& aFFs=pIntrPool->SSInterferences();
  //
  Standard_Boolean bFlag;
  Standard_Integer i, aNb, nF1, nF2,  iZone, aNbSps, iSenseFlag;
  gp_Dir aDNF1, aDNF2;

  aNb=aFFs.Extent();
  for (i=1; i<=aNb; i++) {
    bFlag=Standard_False;

    XBOPTools_SSInterference& aFF=aFFs(i);

    nF1=aFF.Index1();
    nF2=aFF.Index2();
    const TopoDS_Face& aF1=TopoDS::Face(aDS.Shape(nF1));
    const TopoDS_Face& aF2=TopoDS::Face(aDS.Shape(nF2));
    //
    // iSenseFlag;
    const XBOPTools_ListOfPaveBlock& aLPB=aFF.PaveBlocks();
    aNbSps=aLPB.Extent();

    if (!aNbSps) {
      continue;
    }

    const XBOPTools_PaveBlock& aPB=aLPB.First();
    const TopoDS_Edge& aSpE=TopoDS::Edge(aDS.Shape(aPB.Edge()));

    XBOPTools_Tools3D::GetNormalToFaceOnEdge (aSpE, aF1, aDNF1);
    XBOPTools_Tools3D::GetNormalToFaceOnEdge (aSpE, aF2, aDNF2);
    iSenseFlag=XBOPTools_Tools3D::SenseFlag (aDNF1, aDNF2);
    //
    if (iSenseFlag==1 || iSenseFlag==-1) {
    //
    //
      TopoDS_Face aF1FWD=aF1;
      aF1FWD.Orientation (TopAbs_FORWARD);

      XBOP_WireEdgeSet aWES (aF1FWD);
      XBOP_SDFWESFiller aWESFiller(nF1, nF2, *myDSFiller);
      aWESFiller.SetSenseFlag(iSenseFlag);
      aWESFiller.SetOperation(XBOP_COMMON);
      aWESFiller.Do(aWES);

      XBOP_FaceBuilder aFB;
      aFB.Do(aWES);
      const TopTools_ListOfShape& aLF=aFB.NewFaces();

      iZone=0;
      TopTools_ListIteratorOfListOfShape anIt(aLF);
      for (; anIt.More(); anIt.Next()) {
        const TopoDS_Shape& aFR=anIt.Value();

        if (aFR.ShapeType()==TopAbs_FACE) {
          const TopoDS_Face& aFaceResult=TopoDS::Face(aFR);
          //
          Standard_Boolean bIsValidIn2D, bNegativeFlag;
          bIsValidIn2D=XBOPTools_Tools3D::IsValidArea (aFaceResult, bNegativeFlag);
          if (bIsValidIn2D) {
            //if(CheckSameDomainFaceInside(aFaceResult, aF2)) {
            iZone=1;
            break;
            //}
          }
          //
        }
      }

      if (iZone) {
        bFlag=Standard_True;
        aFF.SetStatesMap(aWESFiller.StatesMap());
      }

    }// if (iSenseFlag)

  aFF.SetTangentFacesFlag(bFlag);
  aFF.SetSenseFlag (iSenseFlag);
  }// end of for (i=1; i<=aNb; i++)
}
