/* idl__type.h
 *
 * Type definitions for C-language.
 * This file contains generated code. Do not modify!
 */

#ifndef corto_idl__type_H
#define corto_idl__type_H

#include "corto.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Casting macro's for classes */
#define idl_Declarator(o) ((idl_Declarator)corto_assertType((corto_type)idl_Declarator_o, o))
#define idl_InheritanceSpec(o) ((idl_InheritanceSpec *)corto_assertType((corto_type)idl_InheritanceSpec_o, o))
#define idl_Parser(o) ((idl_Parser)corto_assertType((corto_type)idl_Parser_o, o))

/* Type definitions */
CORTO_LIST(corto_uint32List);

/*  ::corto::idl::Declarator */
CORTO_CLASS(idl_Declarator);

CORTO_CLASS_DEF(idl_Declarator) {
    corto_string identifier;
    corto_uint32List arraySizes;
};

CORTO_LIST(corto_typeList);

/*  ::corto::idl::InheritanceSpec */
typedef struct idl_InheritanceSpec idl_InheritanceSpec;

struct idl_InheritanceSpec {
    corto_interface base;
    corto_typeList supports;
};

/*  ::corto::idl::Parser */
CORTO_CLASS(idl_Parser);

CORTO_CLASS_DEF(idl_Parser) {
    corto_string filename;
    corto_string source;
    corto_int32 line;
    corto_int32 column;
    corto_int32 errors;
    corto_object scope;
};

CORTO_LIST(corto_stringList);

CORTO_LIST(idl_DeclaratorList);

CORTO_LIST(corto_parameterList);

#ifdef __cplusplus
}
#endif
#endif

