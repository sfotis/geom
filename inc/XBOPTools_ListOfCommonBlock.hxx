// This file is generated by WOK (CPPExt).
// Please do not edit this file; modify original file instead.
// The copyright and license terms as defined for the original file apply to 
// this header file considered to be the "object code" form of the original source.

#ifndef _XBOPTools_ListOfCommonBlock_HeaderFile
#define _XBOPTools_ListOfCommonBlock_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

#ifndef _Standard_Address_HeaderFile
#include <Standard_Address.hxx>
#endif
#ifndef _Handle_XBOPTools_ListNodeOfListOfCommonBlock_HeaderFile
#include <Handle_XBOPTools_ListNodeOfListOfCommonBlock.hxx>
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
class Standard_NoSuchObject;
class XBOPTools_ListIteratorOfListOfCommonBlock;
class XBOPTools_CommonBlock;
class XBOPTools_ListNodeOfListOfCommonBlock;



class XBOPTools_ListOfCommonBlock  {
public:

  void* operator new(size_t,void* anAddress) 
  {
    return anAddress;
  }
  void* operator new(size_t size) 
  {
    return Standard::Allocate(size); 
  }
  void  operator delete(void *anAddress) 
  {
    if (anAddress) Standard::Free((Standard_Address&)anAddress); 
  }

  
  Standard_EXPORT   XBOPTools_ListOfCommonBlock();
  
  Standard_EXPORT     void Assign(const XBOPTools_ListOfCommonBlock& Other) ;
    void operator=(const XBOPTools_ListOfCommonBlock& Other) 
{
  Assign(Other);
}
  
  Standard_EXPORT     Standard_Integer Extent() const;
  
  Standard_EXPORT     void Clear() ;
~XBOPTools_ListOfCommonBlock()
{
  Clear();
}
  
        Standard_Boolean IsEmpty() const;
  
  Standard_EXPORT     void Prepend(const XBOPTools_CommonBlock& I) ;
  
  Standard_EXPORT     void Prepend(const XBOPTools_CommonBlock& I,XBOPTools_ListIteratorOfListOfCommonBlock& theIt) ;
  
  Standard_EXPORT     void Prepend(XBOPTools_ListOfCommonBlock& Other) ;
  
  Standard_EXPORT     void Append(const XBOPTools_CommonBlock& I) ;
  
  Standard_EXPORT     void Append(const XBOPTools_CommonBlock& I,XBOPTools_ListIteratorOfListOfCommonBlock& theIt) ;
  
  Standard_EXPORT     void Append(XBOPTools_ListOfCommonBlock& Other) ;
  
  Standard_EXPORT     XBOPTools_CommonBlock& First() const;
  
  Standard_EXPORT     XBOPTools_CommonBlock& Last() const;
  
  Standard_EXPORT     void RemoveFirst() ;
  
  Standard_EXPORT     void Remove(XBOPTools_ListIteratorOfListOfCommonBlock& It) ;
  
  Standard_EXPORT     void InsertBefore(const XBOPTools_CommonBlock& I,XBOPTools_ListIteratorOfListOfCommonBlock& It) ;
  
  Standard_EXPORT     void InsertBefore(XBOPTools_ListOfCommonBlock& Other,XBOPTools_ListIteratorOfListOfCommonBlock& It) ;
  
  Standard_EXPORT     void InsertAfter(const XBOPTools_CommonBlock& I,XBOPTools_ListIteratorOfListOfCommonBlock& It) ;
  
  Standard_EXPORT     void InsertAfter(XBOPTools_ListOfCommonBlock& Other,XBOPTools_ListIteratorOfListOfCommonBlock& It) ;


friend class XBOPTools_ListIteratorOfListOfCommonBlock;



protected:





private:

  
  Standard_EXPORT   XBOPTools_ListOfCommonBlock(const XBOPTools_ListOfCommonBlock& Other);


Standard_Address myFirst;
Standard_Address myLast;


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

#include <TCollection_List.lxx>

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
