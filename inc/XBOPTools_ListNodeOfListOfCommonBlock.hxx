// This file is generated by WOK (CPPExt).
// Please do not edit this file; modify original file instead.
// The copyright and license terms as defined for the original file apply to 
// this header file considered to be the "object code" form of the original source.

#ifndef _XBOPTools_ListNodeOfListOfCommonBlock_HeaderFile
#define _XBOPTools_ListNodeOfListOfCommonBlock_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_DefineHandle_HeaderFile
#include <Standard_DefineHandle.hxx>
#endif
#ifndef _Handle_XBOPTools_ListNodeOfListOfCommonBlock_HeaderFile
#include <Handle_XBOPTools_ListNodeOfListOfCommonBlock.hxx>
#endif

#ifndef _XBOPTools_CommonBlock_HeaderFile
#include <XBOPTools_CommonBlock.hxx>
#endif
#ifndef _TCollection_MapNode_HeaderFile
#include <TCollection_MapNode.hxx>
#endif
#ifndef _TCollection_MapNodePtr_HeaderFile
#include <TCollection_MapNodePtr.hxx>
#endif
class XBOPTools_CommonBlock;
class XBOPTools_ListOfCommonBlock;
class XBOPTools_ListIteratorOfListOfCommonBlock;



class XBOPTools_ListNodeOfListOfCommonBlock : public TCollection_MapNode {

public:

  
      XBOPTools_ListNodeOfListOfCommonBlock(const XBOPTools_CommonBlock& I,const TCollection_MapNodePtr& n);
  
        XBOPTools_CommonBlock& Value() const;




  DEFINE_STANDARD_RTTI(XBOPTools_ListNodeOfListOfCommonBlock)

protected:




private: 


XBOPTools_CommonBlock myValue;


};

#define Item XBOPTools_CommonBlock
#define Item_hxx <XBOPTools_CommonBlock.hxx>
#define TCollection_ListNode XBOPTools_ListNodeOfListOfCommonBlock
#define TCollection_ListNode_hxx <XBOPTools_ListNodeOfListOfCommonBlock.hxx>
#define TCollection_ListIterator XBOPTools_ListIteratorOfListOfCommonBlock
#define TCollection_ListIterator_hxx <XBOPTools_ListIteratorOfListOfCommonBlock.hxx>
#define Handle_TCollection_ListNode Handle_XBOPTools_ListNodeOfListOfCommonBlock
#define TCollection_ListNode_Type_() XBOPTools_ListNodeOfListOfCommonBlock_Type_()
#define TCollection_List XBOPTools_ListOfCommonBlock
#define TCollection_List_hxx <XBOPTools_ListOfCommonBlock.hxx>

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
