/* _type.h
 *
 * Type definitions for C-language.
 * This file contains generated code. Do not modify!
 */

#ifndef SHAPETYPE__TYPE_H
#define SHAPETYPE__TYPE_H

#include <corto/corto.h>
#include <corto/core/c/c.h>
#include <corto/lang/c/c.h>
#include <corto/ext/idl/idl.h>
#include <ospl/ospl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Casting macro's */
#define ShapeType(o) ((ShapeType*)corto_assertType((corto_type)ShapeType_o, o))

/* Type definitions */
/*  /ShapeType */
typedef struct ShapeType ShapeType;

struct ShapeType {
    corto_string color;
    corto_int32 x;
    corto_int32 y;
    corto_int32 shapesize;
};

#ifdef __cplusplus
}
#endif
#endif

