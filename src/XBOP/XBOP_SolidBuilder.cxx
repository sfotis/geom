// Copyright (c) 1999-2012 OPEN CASCADE SAS
//
// The content of this file is subject to the Open CASCADE Technology Public
// License Version 6.5 (the "License"). You may not use the content of this file
// except in compliance with the License. Please obtain a copy of the License
// at http://www.opencascade.org and read it completely before using this file.
//
// The Initial Developer of the Original Code is Open CASCADE S.A.S., having its
// main offices at: 1, place des Freres Montgolfier, 78280 Guyancourt, France.
//
// The Original Code and all software distributed under the License is
// distributed on an "AS IS" basis, without warranty of any kind, and the
// Initial Developer hereby disclaims all such warranties, including without
// limitation, any warranties of merchantability, fitness for a particular
// purpose or non-infringement. Please see the License for the specific terms
// and conditions governing the rights and limitations under the License.

#include <XBOP_SolidBuilder.ixx>

#include <TopoDS.hxx>

#include <XBOP_ShellFaceClassifier.hxx>
#include <XBOP_Loop.hxx>
#include <XBOP_SFSCorrector.hxx>



// ==============================================================
// function: XBOP_SolidBuilder
// purpose: 
// ==============================================================
XBOP_SolidBuilder::XBOP_SolidBuilder()
{
}

// ==============================================================
// function: XBOP_SolidBuilder
// purpose: 
// ==============================================================
  XBOP_SolidBuilder::XBOP_SolidBuilder(XBOP_ShellFaceSet&      theSFS,
				     const Standard_Boolean theForceClassFlag)
{
  InitSolidBuilder(theSFS, theForceClassFlag);
}

// ==============================================================
// function: InitSolidBuilder
// purpose: 
// ==============================================================
  void XBOP_SolidBuilder::InitSolidBuilder(XBOP_ShellFaceSet&      theSFS,
					  const Standard_Boolean theForceClassFlag) 
{
  //
  XBOP_SFSCorrector aSFSCor;
  aSFSCor.SetSFS(theSFS);
  aSFSCor.Do();
  XBOP_ShellFaceSet& aNewSFS=aSFSCor.NewSFS();
  //
  //MakeLoops(theSFS);
  
  MakeLoops(aNewSFS);
  XBOP_ShellFaceClassifier SFC(myBlockBuilder);
  //
  mySolidAreaBuilder.InitSolidAreaBuilder(myLoopSet, SFC, theForceClassFlag);
}

// ==============================================================
// function: InitSolid
// purpose: 
// ==============================================================
  Standard_Integer XBOP_SolidBuilder::InitSolid() 
{
  return mySolidAreaBuilder.InitArea();
}

// ==============================================================
// function: MoreSolid
// purpose: 
// ==============================================================
  Standard_Boolean XBOP_SolidBuilder::MoreSolid() const
{
  return mySolidAreaBuilder.MoreArea();
}

// ==============================================================
// function: NextSolid
// purpose: 
// ==============================================================
  void XBOP_SolidBuilder::NextSolid() 
{
  mySolidAreaBuilder.NextArea();
}

// ==============================================================
// function: InitShell
// purpose: 
// ==============================================================
  Standard_Integer XBOP_SolidBuilder::InitShell() 
{
  return mySolidAreaBuilder.InitLoop();
}

// ==============================================================
// function: MoreShell
// purpose: 
// ==============================================================
  Standard_Boolean XBOP_SolidBuilder::MoreShell() const
{
  return mySolidAreaBuilder.MoreLoop();
}

// ==============================================================
// function: NextShell
// purpose: 
// ==============================================================
  void XBOP_SolidBuilder::NextShell() 
{
  mySolidAreaBuilder.NextLoop();
}

// ==============================================================
// function: IsOldShell
// purpose: 
// ==============================================================
  Standard_Boolean XBOP_SolidBuilder::IsOldShell() const
{
  return mySolidAreaBuilder.Loop()->IsShape();  
}

// ==============================================================
// function: OldShell
// purpose: 
// ==============================================================
  TopoDS_Shell XBOP_SolidBuilder::OldShell() const
{
  if(!IsOldShell()) {
    Standard_DomainError::Raise("XBOP_SolidBuilder::OldShell");
  }
  
  return TopoDS::Shell(mySolidAreaBuilder.Loop()->Shape());
}

// ==============================================================
// function: InitFace
// purpose: 
// ==============================================================
  Standard_Integer XBOP_SolidBuilder::InitFace() 
{
  const Handle(XBOP_Loop)& aLoop = mySolidAreaBuilder.Loop();
  
  if(aLoop->IsShape())
    Standard_DomainError::Raise("XBOP_SolidBuilder::InitFace");
  else {
    myBlockIterator = aLoop->BlockIterator();
    myBlockIterator.Initialize();
  }
  return myBlockIterator.Extent();
}

// ==============================================================
// function: MoreFace
// purpose: 
// ==============================================================
  Standard_Boolean XBOP_SolidBuilder::MoreFace() const
{
  return myBlockIterator.More();
}

// ==============================================================
// function: NextFace
// purpose: 
// ==============================================================
  void XBOP_SolidBuilder::NextFace() 
{
  myBlockIterator.Next();
}

// ==============================================================
// function: Face
// purpose: 
// ==============================================================
  const TopoDS_Face& XBOP_SolidBuilder::Face() const
{
  const TopoDS_Shape& aShape = myBlockBuilder.Element(myBlockIterator);
  return TopoDS::Face(aShape);
}

// ==============================================================
// function: MakeLoops
// purpose: 
// ==============================================================
  void XBOP_SolidBuilder::MakeLoops(XBOP_ShapeSet& theSFS) 
{
  myBlockBuilder.MakeBlock(theSFS);

  XBOP_ListOfLoop& aList = myLoopSet.ChangeListOfLoop();
  aList.Clear();

  // Add shapes of theSFS as shape loops
  for(theSFS.InitShapes(); theSFS.MoreShapes(); theSFS.NextShape()) {
    Handle(XBOP_Loop) aShapeLoop = new XBOP_Loop(theSFS.Shape());
    aList.Append(aShapeLoop);
  }

  // Add blocks of myBlockBuilder as block loops
  for(myBlockBuilder.InitBlock(); myBlockBuilder.MoreBlock(); myBlockBuilder.NextBlock()) {
    XBOP_BlockIterator aBlockIterator = myBlockBuilder.BlockIterator();
    Handle(XBOP_Loop) aShapeLoop = new XBOP_Loop(aBlockIterator);
    aList.Append(aShapeLoop);
  }
}

