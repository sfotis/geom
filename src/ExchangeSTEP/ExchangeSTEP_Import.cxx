// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
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
// File:	STEPImport.cxx
// Created:	Wed May 19 14:41:10 2004
// Author:	Pavel TELKOV
//		<ptv@mutex.nnov.opencascade.com>

#include "utilities.h"

#include <GEOM_OCCTVersion.hxx>

#include <TDF_ChildIDIterator.hxx>
#include <TDF_Label.hxx>
#include <TDataStd_Name.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_NamedShape.hxx>

#include <IFSelect_ReturnStatus.hxx>
#include <Interface_InterfaceModel.hxx>
#include <Interface_Static.hxx>

#include <STEPControl_Reader.hxx>
#include <StepBasic_Product.hxx>
#include <StepBasic_ProductDefinition.hxx>
#include <StepBasic_ProductDefinitionFormation.hxx>
#include <StepGeom_GeometricRepresentationItem.hxx>
#include <StepShape_TopologicalRepresentationItem.hxx>
#include <TransferBRep.hxx>
#include <Transfer_Binder.hxx>
#include <Transfer_TransientProcess.hxx>
#include <XSControl_TransferReader.hxx>
#include <XSControl_WorkSession.hxx>

#include <BRep_Builder.hxx>

#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>

#include <TCollection_AsciiString.hxx>
#include <TColStd_SequenceOfAsciiString.hxx>

#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx> // CAREFUL ! position of this file is critic : see Lucien PIGNOLONI / OCC

#ifdef WNT
#define SALOME_WNT_EXPORT __declspec(dllexport)
#else
#define SALOME_WNT_EXPORT
#endif

//=============================================================================
/*!
 *  Import()
 */
//=============================================================================

extern "C"
{
  SALOME_WNT_EXPORT
  Handle(TCollection_HAsciiString) GetValue (const TCollection_AsciiString& theFileName,
                                             const TCollection_AsciiString& theParameterName,
                                             TCollection_AsciiString&       theError)
  {
    Handle(TCollection_HAsciiString) aValue;

    if (theParameterName != "LEN_UNITS") {
      theError = theParameterName + " parameter reading is not supported by STEP plugin";
      return aValue;
    }

    STEPControl_Reader aReader;

    Interface_Static::SetCVal("xstep.cascade.unit","M");
    Interface_Static::SetIVal("read.step.ideas", 1);
    Interface_Static::SetIVal("read.step.nonmanifold", 1);

    try {
#if OCC_VERSION_LARGE > 0x06010000
      OCC_CATCH_SIGNALS;
#endif
      IFSelect_ReturnStatus status = aReader.ReadFile(theFileName.ToCString());
      if (status == IFSelect_RetDone) {
        TColStd_SequenceOfAsciiString anUnitLengthNames;
        TColStd_SequenceOfAsciiString anUnitAngleNames;
        TColStd_SequenceOfAsciiString anUnitSolidAngleNames;
        aReader.FileUnits(anUnitLengthNames, anUnitAngleNames, anUnitSolidAngleNames);
        if (anUnitLengthNames.Length() > 0) {
          aValue = new TCollection_HAsciiString( anUnitLengthNames.First() );
          /*
          TCollection_AsciiString aLenUnits = anUnitLengthNames.First();
          if (aLenUnits == "millimetre")
            aValue = new TCollection_HAsciiString ("MM");
          else if (aLenUnits == "centimetre")
            aValue = new TCollection_HAsciiString ("CM");
          else if (aLenUnits == "metre")
            aValue = new TCollection_HAsciiString ("M");
          else if (aLenUnits == "INCH")
            aValue = new TCollection_HAsciiString ("INCH");
          // TODO (for other units than mm, cm, m or inch)
          //else if (aLenUnits == "")
          //  aValue = new TCollection_HAsciiString ("");

          // tmp begin
          //std::cout << "$$$ --- " << anUnitLengthNames.First();
          //for (int ii = 2; ii <= anUnitLengthNames.Length(); ii++)
          //  std::cout << ", " << anUnitLengthNames.Value(ii);
          //std::cout << std::endl;
          // tmp end
          */
        }
      }
      else {
        theError = theFileName + " reading failed";
      }
    }
    catch (Standard_Failure) {
      Handle(Standard_Failure) aFail = Standard_Failure::Caught();
      theError = aFail->GetMessageString();
    }

    return aValue;
  }

  SALOME_WNT_EXPORT
  TopoDS_Shape Import (const TCollection_AsciiString& theFileName,
                       const TCollection_AsciiString& theFormatName,
                       TCollection_AsciiString&       theError,
                       const TDF_Label&               theShapeLabel)
  {
    TopoDS_Shape aResShape;

    STEPControl_Reader aReader;
    //VSR: 16/09/09: Convert to METERS
    Interface_Static::SetCVal("xstep.cascade.unit","M");
    Interface_Static::SetIVal("read.step.ideas", 1);
    Interface_Static::SetIVal("read.step.nonmanifold", 1);

    BRep_Builder B;
    TopoDS_Compound compound;
    B.MakeCompound( compound );
    try {
      OCC_CATCH_SIGNALS;

      IFSelect_ReturnStatus status = aReader.ReadFile(theFileName.ToCString());

      if (status == IFSelect_RetDone) {

        // Regard or not the model units
        if (theFormatName == "STEP_SCALE") {
          // set UnitFlag to units from file
          TColStd_SequenceOfAsciiString anUnitLengthNames;
          TColStd_SequenceOfAsciiString anUnitAngleNames;
          TColStd_SequenceOfAsciiString anUnitSolidAngleNames;
          aReader.FileUnits(anUnitLengthNames, anUnitAngleNames, anUnitSolidAngleNames);
          if (anUnitLengthNames.Length() > 0) {
            TCollection_AsciiString aLenUnits = anUnitLengthNames.First();
            if (aLenUnits == "millimetre")
              Interface_Static::SetCVal("xstep.cascade.unit", "MM");
            else if (aLenUnits == "centimetre")
              Interface_Static::SetCVal("xstep.cascade.unit", "CM");
            else if (aLenUnits == "metre" || aLenUnits.IsEmpty())
              Interface_Static::SetCVal("xstep.cascade.unit", "M");
            else if (aLenUnits == "INCH")
              Interface_Static::SetCVal("xstep.cascade.unit", "INCH");
            else {
              theError = "The file contains not supported units.";
              return aResShape;
            }
            // TODO (for other units than mm, cm, m or inch)
            //else if (aLenUnits == "")
            //  Interface_Static::SetCVal("xstep.cascade.unit", "???");
          }
        }
        else {
          //cout<<"need re-scale a model"<<endl;
          // set UnitFlag to 'meter'
          Interface_Static::SetCVal("xstep.cascade.unit","M");
        }

	Standard_Boolean failsonly = Standard_False ;
	aReader.PrintCheckLoad (failsonly, IFSelect_ItemsByEntity);
	/* Root transfers */
	Standard_Integer nbr = aReader.NbRootsForTransfer();
	aReader.PrintCheckTransfer (failsonly, IFSelect_ItemsByEntity);

	for ( Standard_Integer n=1; n <= nbr; n++) {
	  Standard_Boolean ok = aReader.TransferRoot(n);
	  /* Collecting resulting entities */
	  Standard_Integer nbs = aReader.NbShapes();
	  if (!ok || nbs == 0)
	  {
            // THROW_SALOME_CORBA_EXCEPTION("Exception catched in GEOM_Gen_i::ImportStep", SALOME::BAD_PARAM);
	    continue; // skip empty root
	  }
	  /* For a single entity */
	  else if (nbr == 1 && nbs == 1) {
	    aResShape = aReader.Shape(1);
        // ATTENTION: this is a workaround for mantis issue 0020442 remark 0010776
        // It should be removed after patching OCCT for bug OCC22436
        // (fix for OCCT is expected in service pack next to OCCT6.3sp12)
        if (aResShape.ShapeType() == TopAbs_COMPOUND) {
          int nbSub1 = 0;
          TopoDS_Shape currShape;
          TopoDS_Iterator It (aResShape, Standard_True, Standard_True);
          for (; It.More(); It.Next()) {
            nbSub1++;
            currShape = It.Value();
          }
          if (nbSub1 == 1)
            aResShape = currShape;
        }
        // END workaround
	    break;
	  }

	  for ( Standard_Integer i=1; i<=nbs; i++ ) {
		TopoDS_Shape aShape = aReader.Shape(i);
		if ( aShape.IsNull() ) {
              // THROW_SALOME_CORBA_EXCEPTION("Null shape in GEOM_Gen_i::ImportStep", SALOME::BAD_PARAM) ;
              //return aResShape;
	      continue;
	    }
	    else {
	      B.Add( compound, aShape ) ;
	    }
	  }
	}
	if ( aResShape.IsNull() )
	  aResShape = compound;

        // Check if any BRep entity has been read, there must be at least a vertex
        if ( !TopExp_Explorer( aResShape, TopAbs_VERTEX ).More() )
        {
          theError = "No geometrical data in the imported file.";
          return TopoDS_Shape();
        }

        // BEGIN: Store names of sub-shapes from file
        TopTools_IndexedMapOfShape anIndices;
        TopExp::MapShapes(aResShape, anIndices);

        Handle(Interface_InterfaceModel) Model = aReader.WS()->Model();
        Handle(XSControl_TransferReader) TR = aReader.WS()->TransferReader();
        if (!TR.IsNull()) {
          Handle(Transfer_TransientProcess) TP = TR->TransientProcess();
          Handle(Standard_Type) tPD  = STANDARD_TYPE(StepBasic_ProductDefinition);
          Handle(Standard_Type) tShape  = STANDARD_TYPE(StepShape_TopologicalRepresentationItem);
          Handle(Standard_Type) tGeom  = STANDARD_TYPE(StepGeom_GeometricRepresentationItem);

          Standard_Integer nb = Model->NbEntities();
          for (Standard_Integer ie = 1; ie <= nb; ie++) {
            Handle(Standard_Transient) enti = Model->Value(ie);
            Handle(TCollection_HAsciiString) aName;
            if ( enti->IsKind( tShape ) || enti->IsKind(tGeom))
            {
              aName = Handle(StepRepr_RepresentationItem)::DownCast(enti)->Name();
            }
            else if (enti->DynamicType() == tPD)
            {
            Handle(StepBasic_ProductDefinition) PD =
              Handle(StepBasic_ProductDefinition)::DownCast(enti);
            if (PD.IsNull()) continue;

            Handle(StepBasic_Product) Prod = PD->Formation()->OfProduct();
              aName = Prod->Name();
            }
            else
            {
              continue;
            }
            if ( aName->UsefullLength() < 1 )
              continue;
            // skip 'N0NE' name
            if ( aName->UsefullLength() == 4 &&
                 toupper (aName->Value(1)) == 'N' &&
                 toupper (aName->Value(2)) == 'O' &&
                 toupper (aName->Value(3)) == 'N' &&
                 toupper (aName->Value(4)) == 'E')
              continue;

            // special check to pass names like "Open CASCADE STEP translator 6.3 1"
            TCollection_AsciiString aSkipName ("Open CASCADE STEP translator");
            if (aName->Length() >= aSkipName.Length()) {
              if (aName->String().SubString(1, aSkipName.Length()).IsEqual(aSkipName))
                continue;
            }
            TCollection_ExtendedString aNameExt (aName->ToCString());

            // find target shape
            Handle(Transfer_Binder) binder = TP->Find(enti);
            if (binder.IsNull()) continue;
            TopoDS_Shape S = TransferBRep::ShapeResult(binder);
            if (S.IsNull()) continue;

            // as PRODUCT can be included in the main shape
            // several times, we look here for all iclusions.
            Standard_Integer isub, nbSubs = anIndices.Extent();
            for (isub = 1; isub <= nbSubs; isub++)
            {
              TopoDS_Shape aSub = anIndices.FindKey(isub);
              if (aSub.IsPartner(S)) {
                TDF_Label L;
                if (enti->IsKind(tGeom)) {
                  // check all named shapes using iterator
                  TDF_ChildIDIterator anIt (theShapeLabel, TDataStd_Name::GetID(), Standard_True);
                  for (; anIt.More(); anIt.Next()) {
                    Handle(TDataStd_Name) nameAttr =
                      Handle(TDataStd_Name)::DownCast(anIt.Value());
                    if (nameAttr.IsNull()) continue;
                    TDF_Label Lab = nameAttr->Label();
                    Handle(TNaming_NamedShape) shAttr; 
                    if (Lab.FindAttribute(TNaming_NamedShape::GetID(), shAttr) && shAttr->Get().IsEqual(aSub))
                      L = Lab;
                  }
                }
                // create label and set shape
                if (L.IsNull())
                {
                TDF_TagSource aTag;
                L = aTag.NewChild(theShapeLabel);
                TNaming_Builder tnBuild (L);
                //tnBuild.Generated(S);
                tnBuild.Generated(aSub);
                }
                // set a name
                TDataStd_Name::Set(L, aNameExt);
              }
            }
          }
        }
        // END: Store names
      }
      else {
//        switch (status) {
//        case IFSelect_RetVoid:
//          theError = "Nothing created or No data to process";
//          break;
//        case IFSelect_RetError:
//          theError = "Error in command or input data";
//          break;
//        case IFSelect_RetFail:
//          theError = "Execution was run, but has failed";
//          break;
//        case IFSelect_RetStop:
//          theError = "Execution has been stopped. Quite possible, an exception was raised";
//          break;
//        default:
//          break;
//        }
        theError = "Wrong format of the imported file. Can't import file.";
	aResShape.Nullify();
      }
    }
    catch (Standard_Failure) {
      Handle(Standard_Failure) aFail = Standard_Failure::Caught();
      theError = aFail->GetMessageString();
      aResShape.Nullify();
    }
    // Return previous locale
    return aResShape;
  }
}
