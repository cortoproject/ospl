/* idl__api.h
 *
 * API convenience functions for C-language.
 * This file contains generated code. Do not modify!
 */

#ifndef corto_idl__API_H
#define corto_idl__API_H

#include "corto.h"
#include "idl__interface.h"
#ifdef __cplusplus
extern "C" {
#endif
/* ::corto::idl::Declarator */
CORTO_IDL_EXPORT idl_Declarator _idl_DeclaratorCreate(corto_string identifier, corto_uint32List arraySizes);
#define idl_DeclaratorCreate(identifier, arraySizes) _idl_DeclaratorCreate(identifier, arraySizes)
CORTO_IDL_EXPORT idl_Declarator _idl_DeclaratorCreateChild(corto_object _parent, corto_string _name, corto_string identifier, corto_uint32List arraySizes);
#define idl_DeclaratorCreateChild(_parent, _name, identifier, arraySizes) _idl_DeclaratorCreateChild(_parent, _name, identifier, arraySizes)
CORTO_IDL_EXPORT corto_int16 _idl_DeclaratorUpdate(idl_Declarator _this, corto_string identifier, corto_uint32List arraySizes);
#define idl_DeclaratorUpdate(_this, identifier, arraySizes) _idl_DeclaratorUpdate(idl_Declarator(_this), identifier, arraySizes)

CORTO_IDL_EXPORT idl_Declarator _idl_DeclaratorDeclare(void);
#define idl_DeclaratorDeclare() _idl_DeclaratorDeclare()
CORTO_IDL_EXPORT idl_Declarator _idl_DeclaratorDeclareChild(corto_object _parent, corto_string _name);
#define idl_DeclaratorDeclareChild(_parent, _name) _idl_DeclaratorDeclareChild(_parent, _name)
CORTO_IDL_EXPORT corto_int16 _idl_DeclaratorDefine(idl_Declarator _this, corto_string identifier, corto_uint32List arraySizes);
#define idl_DeclaratorDefine(_this, identifier, arraySizes) _idl_DeclaratorDefine(idl_Declarator(_this), identifier, arraySizes)
CORTO_IDL_EXPORT void _idl_DeclaratorSet(idl_Declarator _this, corto_string identifier, corto_uint32List arraySizes);
#define idl_DeclaratorSet(_this, identifier, arraySizes) _idl_DeclaratorSet(idl_Declarator(_this), identifier, arraySizes)
CORTO_IDL_EXPORT corto_string _idl_DeclaratorStr(idl_Declarator value);
#define idl_DeclaratorStr(value) _idl_DeclaratorStr(idl_Declarator(value))
CORTO_IDL_EXPORT idl_Declarator idl_DeclaratorFromStr(idl_Declarator value, corto_string str);
CORTO_IDL_EXPORT corto_int16 _idl_DeclaratorCopy(idl_Declarator *dst, idl_Declarator src);
#define idl_DeclaratorCopy(dst, src) _idl_DeclaratorCopy(dst, idl_Declarator(src))
CORTO_IDL_EXPORT corto_int16 _idl_DeclaratorCompare(idl_Declarator dst, idl_Declarator src);
#define idl_DeclaratorCompare(dst, src) _idl_DeclaratorCompare(idl_Declarator(dst), idl_Declarator(src))

/* ::corto::idl::InheritanceSpec */
CORTO_IDL_EXPORT idl_InheritanceSpec* _idl_InheritanceSpecCreate(corto_interface base, corto_typeList supports);
#define idl_InheritanceSpecCreate(base, supports) _idl_InheritanceSpecCreate(corto_interface(base), supports)
CORTO_IDL_EXPORT idl_InheritanceSpec* _idl_InheritanceSpecCreateChild(corto_object _parent, corto_string _name, corto_interface base, corto_typeList supports);
#define idl_InheritanceSpecCreateChild(_parent, _name, base, supports) _idl_InheritanceSpecCreateChild(_parent, _name, corto_interface(base), supports)
CORTO_IDL_EXPORT corto_int16 _idl_InheritanceSpecUpdate(idl_InheritanceSpec* _this, corto_interface base, corto_typeList supports);
#define idl_InheritanceSpecUpdate(_this, base, supports) _idl_InheritanceSpecUpdate(_this, corto_interface(base), supports)

CORTO_IDL_EXPORT idl_InheritanceSpec* _idl_InheritanceSpecDeclare(void);
#define idl_InheritanceSpecDeclare() _idl_InheritanceSpecDeclare()
CORTO_IDL_EXPORT idl_InheritanceSpec* _idl_InheritanceSpecDeclareChild(corto_object _parent, corto_string _name);
#define idl_InheritanceSpecDeclareChild(_parent, _name) _idl_InheritanceSpecDeclareChild(_parent, _name)
CORTO_IDL_EXPORT corto_int16 _idl_InheritanceSpecDefine(idl_InheritanceSpec* _this, corto_interface base, corto_typeList supports);
#define idl_InheritanceSpecDefine(_this, base, supports) _idl_InheritanceSpecDefine(_this, corto_interface(base), supports)
CORTO_IDL_EXPORT void _idl_InheritanceSpecSet(idl_InheritanceSpec* _this, corto_interface base, corto_typeList supports);
#define idl_InheritanceSpecSet(_this, base, supports) _idl_InheritanceSpecSet(_this, corto_interface(base), supports)
CORTO_IDL_EXPORT corto_string _idl_InheritanceSpecStr(idl_InheritanceSpec* value);
#define idl_InheritanceSpecStr(value) _idl_InheritanceSpecStr(value)
CORTO_IDL_EXPORT idl_InheritanceSpec* idl_InheritanceSpecFromStr(idl_InheritanceSpec* value, corto_string str);
CORTO_IDL_EXPORT corto_int16 _idl_InheritanceSpecCopy(idl_InheritanceSpec* *dst, idl_InheritanceSpec* src);
#define idl_InheritanceSpecCopy(dst, src) _idl_InheritanceSpecCopy(dst, src)
CORTO_IDL_EXPORT corto_int16 _idl_InheritanceSpecCompare(idl_InheritanceSpec* dst, idl_InheritanceSpec* src);
#define idl_InheritanceSpecCompare(dst, src) _idl_InheritanceSpecCompare(dst, src)

CORTO_IDL_EXPORT corto_int16 _idl_InheritanceSpecInit(idl_InheritanceSpec* value);
#define idl_InheritanceSpecInit(value) _idl_InheritanceSpecInit(value)
CORTO_IDL_EXPORT corto_int16 _idl_InheritanceSpecDeinit(idl_InheritanceSpec* value);
#define idl_InheritanceSpecDeinit(value) _idl_InheritanceSpecDeinit(value)

/* ::corto::idl::Parser */
CORTO_IDL_EXPORT idl_Parser _idl_ParserCreate(corto_string filename, corto_string source);
#define idl_ParserCreate(filename, source) _idl_ParserCreate(filename, source)
CORTO_IDL_EXPORT idl_Parser _idl_ParserCreateChild(corto_object _parent, corto_string _name, corto_string filename, corto_string source);
#define idl_ParserCreateChild(_parent, _name, filename, source) _idl_ParserCreateChild(_parent, _name, filename, source)
CORTO_IDL_EXPORT corto_int16 _idl_ParserUpdate(idl_Parser _this, corto_string filename, corto_string source);
#define idl_ParserUpdate(_this, filename, source) _idl_ParserUpdate(idl_Parser(_this), filename, source)

CORTO_IDL_EXPORT idl_Parser _idl_ParserDeclare(void);
#define idl_ParserDeclare() _idl_ParserDeclare()
CORTO_IDL_EXPORT idl_Parser _idl_ParserDeclareChild(corto_object _parent, corto_string _name);
#define idl_ParserDeclareChild(_parent, _name) _idl_ParserDeclareChild(_parent, _name)
CORTO_IDL_EXPORT corto_int16 _idl_ParserDefine(idl_Parser _this, corto_string filename, corto_string source);
#define idl_ParserDefine(_this, filename, source) _idl_ParserDefine(idl_Parser(_this), filename, source)
CORTO_IDL_EXPORT void _idl_ParserSet(idl_Parser _this, corto_string filename, corto_string source);
#define idl_ParserSet(_this, filename, source) _idl_ParserSet(idl_Parser(_this), filename, source)
CORTO_IDL_EXPORT corto_string _idl_ParserStr(idl_Parser value);
#define idl_ParserStr(value) _idl_ParserStr(idl_Parser(value))
CORTO_IDL_EXPORT idl_Parser idl_ParserFromStr(idl_Parser value, corto_string str);
CORTO_IDL_EXPORT corto_int16 _idl_ParserCopy(idl_Parser *dst, idl_Parser src);
#define idl_ParserCopy(dst, src) _idl_ParserCopy(dst, idl_Parser(src))
CORTO_IDL_EXPORT corto_int16 _idl_ParserCompare(idl_Parser dst, idl_Parser src);
#define idl_ParserCompare(dst, src) _idl_ParserCompare(idl_Parser(dst), idl_Parser(src))


/* <0x9840854> */
#define corto_uint32ListForeach(list, elem) \
    corto_iter elem##_iter = corto_llIter(list);\
    corto_uint32 elem;\
    while(corto_iterHasNext(&elem##_iter) ? elem = (corto_uint32)(corto_word)corto_iterNext(&elem##_iter), TRUE : FALSE)

CORTO_IDL_EXPORT void corto_uint32ListInsert(corto_uint32List list, corto_uint32 element);
CORTO_IDL_EXPORT void corto_uint32ListAppend(corto_uint32List list, corto_uint32 element);
CORTO_IDL_EXPORT corto_uint32 corto_uint32ListTakeFirst(corto_uint32List list);
CORTO_IDL_EXPORT corto_uint32 corto_uint32ListLast(corto_uint32List list);
CORTO_IDL_EXPORT void corto_uint32ListClear(corto_uint32List list);
CORTO_IDL_EXPORT corto_uint32 corto_uint32ListGet(corto_uint32List list, corto_uint32 index);
CORTO_IDL_EXPORT corto_uint32 corto_uint32ListSize(corto_uint32List list);

/* <0x9841ccc> */
#define corto_typeListForeach(list, elem) \
    corto_iter elem##_iter = corto_llIter(list);\
    corto_type elem;\
    while(corto_iterHasNext(&elem##_iter) ? elem = corto_iterNext(&elem##_iter), TRUE : FALSE)

CORTO_IDL_EXPORT void corto_typeListInsert(corto_typeList list, corto_type element);
CORTO_IDL_EXPORT void corto_typeListAppend(corto_typeList list, corto_type element);
CORTO_IDL_EXPORT corto_type corto_typeListTakeFirst(corto_typeList list);
CORTO_IDL_EXPORT corto_type corto_typeListLast(corto_typeList list);
CORTO_IDL_EXPORT void corto_typeListClear(corto_typeList list);
CORTO_IDL_EXPORT corto_type corto_typeListGet(corto_typeList list, corto_uint32 index);
CORTO_IDL_EXPORT corto_uint32 corto_typeListSize(corto_typeList list);

/* <0x98483b4> */
#define corto_stringListForeach(list, elem) \
    corto_iter elem##_iter = corto_llIter(list);\
    corto_string elem;\
    while(corto_iterHasNext(&elem##_iter) ? elem = (corto_string)(corto_word)corto_iterNext(&elem##_iter), TRUE : FALSE)

CORTO_IDL_EXPORT void corto_stringListInsert(corto_stringList list, corto_string element);
CORTO_IDL_EXPORT void corto_stringListAppend(corto_stringList list, corto_string element);
CORTO_IDL_EXPORT corto_string corto_stringListTakeFirst(corto_stringList list);
CORTO_IDL_EXPORT corto_string corto_stringListLast(corto_stringList list);
CORTO_IDL_EXPORT void corto_stringListClear(corto_stringList list);
CORTO_IDL_EXPORT corto_string corto_stringListGet(corto_stringList list, corto_uint32 index);
CORTO_IDL_EXPORT corto_uint32 corto_stringListSize(corto_stringList list);

/* <0x9847374> */
#define idl_DeclaratorListForeach(list, elem) \
    corto_iter elem##_iter = corto_llIter(list);\
    idl_Declarator elem;\
    while(corto_iterHasNext(&elem##_iter) ? elem = corto_iterNext(&elem##_iter), TRUE : FALSE)

CORTO_IDL_EXPORT void idl_DeclaratorListInsert(idl_DeclaratorList list, idl_Declarator element);
CORTO_IDL_EXPORT void idl_DeclaratorListAppend(idl_DeclaratorList list, idl_Declarator element);
CORTO_IDL_EXPORT idl_Declarator idl_DeclaratorListTakeFirst(idl_DeclaratorList list);
CORTO_IDL_EXPORT idl_Declarator idl_DeclaratorListLast(idl_DeclaratorList list);
CORTO_IDL_EXPORT void idl_DeclaratorListClear(idl_DeclaratorList list);
CORTO_IDL_EXPORT idl_Declarator idl_DeclaratorListGet(idl_DeclaratorList list, corto_uint32 index);
CORTO_IDL_EXPORT corto_uint32 idl_DeclaratorListSize(idl_DeclaratorList list);

/* <0x9847e4c> */
#define corto_parameterListForeach(list, elem) \
    corto_iter elem##_iter = corto_llIter(list);\
    corto_parameter *elem;\
    while(corto_iterHasNext(&elem##_iter) ? elem = corto_iterNext(&elem##_iter), TRUE : FALSE)

CORTO_IDL_EXPORT corto_parameter* corto_parameterListInsertAlloc(corto_parameterList list);
CORTO_IDL_EXPORT corto_parameter* corto_parameterListInsert(corto_parameterList list, corto_parameter* element);
CORTO_IDL_EXPORT corto_parameter* corto_parameterListAppendAlloc(corto_parameterList list);
CORTO_IDL_EXPORT corto_parameter* corto_parameterListAppend(corto_parameterList list, corto_parameter* element);
CORTO_IDL_EXPORT corto_parameter* corto_parameterListTakeFirst(corto_parameterList list);
CORTO_IDL_EXPORT corto_parameter* corto_parameterListLast(corto_parameterList list);
CORTO_IDL_EXPORT void corto_parameterListClear(corto_parameterList list);
CORTO_IDL_EXPORT corto_parameter* corto_parameterListGet(corto_parameterList list, corto_uint32 index);
CORTO_IDL_EXPORT corto_uint32 corto_parameterListSize(corto_parameterList list);

#ifdef __cplusplus
}
#endif
#endif

