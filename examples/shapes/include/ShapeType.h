/* ShapeType.h
 *
 * API convenience functions for C-language.
 * This file contains generated code. Do not modify!
 */

#ifndef SHAPETYPE__API_H
#define SHAPETYPE__API_H

#include <corto/corto.h>
#include <include/_project.h>
#include <include/_type.h>
#ifdef __cplusplus
extern "C" {
#endif
/* /ShapeType */
SHAPETYPE_EXPORT ShapeType* _ShapeTypeCreate(corto_int32 x, corto_int32 y, corto_int32 shapesize);
#define ShapeTypeCreate(x, y, shapesize) _ShapeTypeCreate(x, y, shapesize)
#define ShapeTypeCreate_auto(_id, x, y, shapesize) ShapeType* _id = ShapeTypeCreate(x, y, shapesize); (void)_id
SHAPETYPE_EXPORT ShapeType* _ShapeTypeCreateChild(corto_object _parent, corto_string _id, corto_int32 x, corto_int32 y, corto_int32 shapesize);
#define ShapeTypeCreateChild(_parent, _id, x, y, shapesize) _ShapeTypeCreateChild(_parent, _id, x, y, shapesize)
#define ShapeTypeCreateChild_auto(_parent, _id, x, y, shapesize) ShapeType* _id = ShapeTypeCreateChild(_parent, #_id, x, y, shapesize); (void)_id
SHAPETYPE_EXPORT corto_int16 _ShapeTypeUpdate(ShapeType* _this, corto_int32 x, corto_int32 y, corto_int32 shapesize);
#define ShapeTypeUpdate(_this, x, y, shapesize) _ShapeTypeUpdate(ShapeType(_this), x, y, shapesize)

SHAPETYPE_EXPORT ShapeType* _ShapeTypeDeclare(void);
#define ShapeTypeDeclare() _ShapeTypeDeclare()
#define ShapeTypeDeclare_auto(_id) ShapeType* _id = ShapeTypeDeclare(); (void)_id
SHAPETYPE_EXPORT ShapeType* _ShapeTypeDeclareChild(corto_object _parent, corto_string _id);
#define ShapeTypeDeclareChild(_parent, _id) _ShapeTypeDeclareChild(_parent, _id)
#define ShapeTypeDeclareChild_auto(_parent, _id) ShapeType* _id = ShapeTypeDeclareChild(_parent, #_id); (void)_id
SHAPETYPE_EXPORT corto_int16 _ShapeTypeDefine(ShapeType* _this, corto_int32 x, corto_int32 y, corto_int32 shapesize);
#define ShapeTypeDefine(_this, x, y, shapesize) _ShapeTypeDefine(ShapeType(_this), x, y, shapesize)
SHAPETYPE_EXPORT ShapeType* _ShapeTypeAssign(ShapeType* _this, corto_int32 x, corto_int32 y, corto_int32 shapesize);
#define ShapeType__optional_NotSet NULL
#define ShapeType__optional_Set(x, y, shapesize) ShapeTypeAssign((ShapeType*)corto_calloc(sizeof(ShapeType)), x, y, shapesize)
#define ShapeType__optional_SetCond(cond, x, y, shapesize) cond ? ShapeTypeAssign((ShapeType*)corto_calloc(sizeof(ShapeType)), x, y, shapesize) : NULL
#define ShapeTypeUnset(_this) _this ? corto_ptr_deinit(_this, ShapeType_o) : 0; corto_dealloc(_this); _this = NULL;
#define ShapeTypeAssign(_this, x, y, shapesize) _ShapeTypeAssign(_this, x, y, shapesize)
#define ShapeTypeSet(_this, x, y, shapesize) _this = _this ? _this : (ShapeType*)corto_calloc(sizeof(ShapeType)); _ShapeTypeAssign(_this, x, y, shapesize)
SHAPETYPE_EXPORT corto_string _ShapeTypeStr(ShapeType* value);
#define ShapeTypeStr(value) _ShapeTypeStr(value)
SHAPETYPE_EXPORT ShapeType* ShapeTypeFromStr(ShapeType* value, corto_string str);
SHAPETYPE_EXPORT corto_equalityKind ShapeTypeCompare(ShapeType* dst, ShapeType* src);

SHAPETYPE_EXPORT corto_int16 _ShapeTypeInit(ShapeType* value);
#define ShapeTypeInit(value) _ShapeTypeInit(value)
SHAPETYPE_EXPORT corto_int16 _ShapeTypeDeinit(ShapeType* value);
#define ShapeTypeDeinit(value) _ShapeTypeDeinit(value)


#ifdef __cplusplus
}
#endif
#endif

