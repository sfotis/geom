// This file is generated by WOK (CPPExt).
// Please do not edit this file; modify original file instead.
// The copyright and license terms as defined for the original file apply to 
// this header file considered to be the "object code" form of the original source.

#ifndef _XBOP_ShellSolidHistoryCollector_HeaderFile
#define _XBOP_ShellSolidHistoryCollector_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_DefineHandle_HeaderFile
#include <Standard_DefineHandle.hxx>
#endif
#ifndef _Handle_XBOP_ShellSolidHistoryCollector_HeaderFile
#include <Handle_XBOP_ShellSolidHistoryCollector.hxx>
#endif

#ifndef _XBOP_HistoryCollector_HeaderFile
#include <XBOP_HistoryCollector.hxx>
#endif
#ifndef _XBOP_Operation_HeaderFile
#include <XBOP_Operation.hxx>
#endif
#ifndef _XBOPTools_PDSFiller_HeaderFile
#include <XBOPTools_PDSFiller.hxx>
#endif
class TopoDS_Shape;



class XBOP_ShellSolidHistoryCollector : public XBOP_HistoryCollector {

public:

  
  Standard_EXPORT   XBOP_ShellSolidHistoryCollector(const TopoDS_Shape& theShape1,const TopoDS_Shape& theShape2,const XBOP_Operation theOperation);
  
  Standard_EXPORT     void AddNewFace(const TopoDS_Shape& theOldShape,const TopoDS_Shape& theNewShape,const XBOPTools_PDSFiller& theDSFiller) ;
  
  Standard_EXPORT   virtual  void SetResult(const TopoDS_Shape& theResult,const XBOPTools_PDSFiller& theDSFiller) ;




  DEFINE_STANDARD_RTTI(XBOP_ShellSolidHistoryCollector)

protected:




private: 

  
  Standard_EXPORT     void FillSection(const XBOPTools_PDSFiller& theDSFiller) ;
  
  Standard_EXPORT     void FillEdgeHistory(const XBOPTools_PDSFiller& theDSFiller) ;



};





// other Inline functions and methods (like "C++: function call" methods)


#endif
