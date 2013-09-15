// This file is generated by WOK (CPPExt).
// Please do not edit this file; modify original file instead.
// The copyright and license terms as defined for the original file apply to 
// this header file considered to be the "object code" form of the original source.

#ifndef _XBOPTools_ListNodeOfListOfCoupleOfInteger_HeaderFile
#define _XBOPTools_ListNodeOfListOfCoupleOfInteger_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_DefineHandle_HeaderFile
#include <Standard_DefineHandle.hxx>
#endif
#ifndef _Handle_XBOPTools_ListNodeOfListOfCoupleOfInteger_HeaderFile
#include <Handle_XBOPTools_ListNodeOfListOfCoupleOfInteger.hxx>
#endif

#ifndef _XBOPTools_CoupleOfInteger_HeaderFile
#include <XBOPTools_CoupleOfInteger.hxx>
#endif
#ifndef _TCollection_MapNode_HeaderFile
#include <TCollection_MapNode.hxx>
#endif
#ifndef _TCollection_MapNodePtr_HeaderFile
#include <TCollection_MapNodePtr.hxx>
#endif
class XBOPTools_CoupleOfInteger;
class XBOPTools_ListOfCoupleOfInteger;
class XBOPTools_ListIteratorOfListOfCoupleOfInteger;



class XBOPTools_ListNodeOfListOfCoupleOfInteger : public TCollection_MapNode {

public:

  
      XBOPTools_ListNodeOfListOfCoupleOfInteger(const XBOPTools_CoupleOfInteger& I,const TCollection_MapNodePtr& n);
  
        XBOPTools_CoupleOfInteger& Value() const;




  DEFINE_STANDARD_RTTI(XBOPTools_ListNodeOfListOfCoupleOfInteger)

protected:




private: 


XBOPTools_CoupleOfInteger myValue;


};

#define Item XBOPTools_CoupleOfInteger
#define Item_hxx <XBOPTools_CoupleOfInteger.hxx>
#define TCollection_ListNode XBOPTools_ListNodeOfListOfCoupleOfInteger
#define TCollection_ListNode_hxx <XBOPTools_ListNodeOfListOfCoupleOfInteger.hxx>
#define TCollection_ListIterator XBOPTools_ListIteratorOfListOfCoupleOfInteger
#define TCollection_ListIterator_hxx <XBOPTools_ListIteratorOfListOfCoupleOfInteger.hxx>
#define Handle_TCollection_ListNode Handle_XBOPTools_ListNodeOfListOfCoupleOfInteger
#define TCollection_ListNode_Type_() XBOPTools_ListNodeOfListOfCoupleOfInteger_Type_()
#define TCollection_List XBOPTools_ListOfCoupleOfInteger
#define TCollection_List_hxx <XBOPTools_ListOfCoupleOfInteger.hxx>

#include <TCollection_ListNode.lxx>

#undef Item
#undef Item_hxx
#undef TCollection_ListNode
#undef TCollection_ListNode_hxx
#undef TCollection_ListIterator
#undef TCollection_ListIterator_hxx
#undef Handle_TCollection_ListNode
#undef TCollection_ListNode_Type_
#undef TCollection_List
#undef TCollection_List_hxx


// other Inline functions and methods (like "C++: function call" methods)


#endif