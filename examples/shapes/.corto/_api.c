/* _api.c
 *
 * API convenience functions for C-language.
 * This file contains generated code. Do not modify!
 */

#include <include/ShapeType.h>
ShapeType* _ShapeTypeCreate(corto_int32 x, corto_int32 y, corto_int32 shapesize) {
    ShapeType* _this;
    _this = ShapeType(corto_declare(ShapeType_o));
    if (!_this) {
        return NULL;
    }
    if (!corto_checkState(_this, CORTO_DEFINED)) {
        ((ShapeType*)_this)->x = x;
        ((ShapeType*)_this)->y = y;
        ((ShapeType*)_this)->shapesize = shapesize;
        if (corto_define(_this)) {
            corto_release(_this);
            _this = NULL;
        }
    }
    return _this;
}

ShapeType* _ShapeTypeCreateChild(corto_object _parent, corto_string _id, corto_int32 x, corto_int32 y, corto_int32 shapesize) {
    ShapeType* _this;
    _this = ShapeType(corto_declareChild(_parent, _id, ShapeType_o));
    if (!_this) {
        return NULL;
    }
    if (!corto_checkState(_this, CORTO_DEFINED)) {
        ((ShapeType*)_this)->x = x;
        ((ShapeType*)_this)->y = y;
        ((ShapeType*)_this)->shapesize = shapesize;
        if (corto_define(_this)) {
            corto_release(_this);
            _this = NULL;
        }
    }
    return _this;
}

corto_int16 _ShapeTypeUpdate(ShapeType* _this, corto_int32 x, corto_int32 y, corto_int32 shapesize) {
    CORTO_UNUSED(_this);
    if (!corto_updateBegin(_this)) {
        if ((corto_typeof(corto_typeof(_this)) == (corto_type)corto_target_o) && !corto_owned(_this)) {
            ((ShapeType*)((ShapeType*)CORTO_OFFSET(_this, ((corto_type)ShapeType_o)->size)))->x = x;
            ((ShapeType*)((ShapeType*)CORTO_OFFSET(_this, ((corto_type)ShapeType_o)->size)))->y = y;
            ((ShapeType*)((ShapeType*)CORTO_OFFSET(_this, ((corto_type)ShapeType_o)->size)))->shapesize = shapesize;
        } else {
            ((ShapeType*)_this)->x = x;
            ((ShapeType*)_this)->y = y;
            ((ShapeType*)_this)->shapesize = shapesize;
        }
        corto_updateEnd(_this);
    } else {
        return -1;
    }
    return 0;
}

ShapeType* _ShapeTypeDeclare(void) {
    ShapeType* _this;
    _this = ShapeType(corto_declare(ShapeType_o));
    if (!_this) {
        return NULL;
    }
    return _this;
}

ShapeType* _ShapeTypeDeclareChild(corto_object _parent, corto_string _id) {
    ShapeType* _this;
    _this = ShapeType(corto_declareChild(_parent, _id, ShapeType_o));
    if (!_this) {
        return NULL;
    }
    return _this;
}

corto_int16 _ShapeTypeDefine(ShapeType* _this, corto_int32 x, corto_int32 y, corto_int32 shapesize) {
    CORTO_UNUSED(_this);
    ((ShapeType*)_this)->x = x;
    ((ShapeType*)_this)->y = y;
    ((ShapeType*)_this)->shapesize = shapesize;
    return corto_define(_this);
}

ShapeType* _ShapeTypeAssign(ShapeType* _this, corto_int32 x, corto_int32 y, corto_int32 shapesize) {
    CORTO_UNUSED(_this);
    ((ShapeType*)_this)->x = x;
    ((ShapeType*)_this)->y = y;
    ((ShapeType*)_this)->shapesize = shapesize;
    return _this;
}

corto_string _ShapeTypeStr(ShapeType* value) {
    corto_string result;
    corto_value v;
    v = corto_value_value(corto_type(ShapeType_o), value);
    result = corto_strv(&v, 0);
    return result;
}

ShapeType* ShapeTypeFromStr(ShapeType* value, corto_string str) {
    corto_fromStrp(&value, corto_type(ShapeType_o), str);
    return value;
}

corto_equalityKind ShapeTypeCompare(ShapeType* dst, ShapeType* src) {
    return corto_comparep(dst, ShapeType_o, src);
}

corto_int16 _ShapeTypeInit(ShapeType* value) {
    corto_int16 result;
    memset(value, 0, corto_type(ShapeType_o)->size);
    corto_value v;
    v = corto_value_value(corto_type(ShapeType_o), value);
    result = corto_initv(&v);
    return result;
}

corto_int16 _ShapeTypeDeinit(ShapeType* value) {
    corto_int16 result;
    corto_value v;
    v = corto_value_value(corto_type(ShapeType_o), value);
    result = corto_deinitv(&v);
    return result;
}

