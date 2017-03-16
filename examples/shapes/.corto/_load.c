/* _load.c
 *
 * Loads objects in object store.
 * This file contains generated code. Do not modify!
 */

#include <include/ShapeType.h>

/* Forward declarations */

/* Variable definitions */
corto_struct ShapeType_o;
idl_Member ShapeType_color_o;
idl_Member ShapeType_shapesize_o;
idl_Member ShapeType_x_o;
idl_Member ShapeType_y_o;

/* Load objects in object store. */
int shapes_load(void) {
    corto_object _a_; /* Used for resolving anonymous objects */
    corto_object _e_; /* Used for resolving extern objects */
    (void)_e_;
    _a_ = NULL;

    corto_attr prevAttr = corto_setAttr(CORTO_ATTR_PERSISTENT);

    ShapeType_o = corto_struct(corto_declareChild(root_o, "ShapeType", corto_struct_o));
    if (!ShapeType_o) {
        corto_error("shapes_load: failed to declare 'ShapeType_o' (%s)", corto_lasterr());
        goto error;
    }

    ShapeType_color_o = idl_Member(corto_declareChild(ShapeType_o, "color", idl_Member_o));
    if (!ShapeType_color_o) {
        corto_error("shapes_load: failed to declare 'ShapeType_color_o' (%s)", corto_lasterr());
        goto error;
    }

    if (!corto_checkState(ShapeType_color_o, CORTO_DEFINED)) {
        ((corto_member)ShapeType_color_o)->type = corto_type(corto_resolve(NULL, "string"));
        ((corto_member)ShapeType_color_o)->modifiers = 0x84;
        ((corto_member)ShapeType_color_o)->unit = NULL;
        ((corto_member)ShapeType_color_o)->state = 0x6;
        ((corto_member)ShapeType_color_o)->stateCondExpr = NULL;
        ((corto_member)ShapeType_color_o)->weak = FALSE;
        ((corto_member)ShapeType_color_o)->id = 0;
        ShapeType_color_o->actualType = NULL;
        if (corto_define(ShapeType_color_o)) {
            corto_error("shapes_load: failed to define 'ShapeType_color_o' (%s)", corto_lasterr());
            goto error;
        }
    }

    ShapeType_shapesize_o = idl_Member(corto_declareChild(ShapeType_o, "shapesize", idl_Member_o));
    if (!ShapeType_shapesize_o) {
        corto_error("shapes_load: failed to declare 'ShapeType_shapesize_o' (%s)", corto_lasterr());
        goto error;
    }

    if (!corto_checkState(ShapeType_shapesize_o, CORTO_DEFINED)) {
        ((corto_member)ShapeType_shapesize_o)->type = corto_type(corto_resolve(NULL, "int32"));
        ((corto_member)ShapeType_shapesize_o)->modifiers = 0x0;
        ((corto_member)ShapeType_shapesize_o)->unit = NULL;
        ((corto_member)ShapeType_shapesize_o)->state = 0x6;
        ((corto_member)ShapeType_shapesize_o)->stateCondExpr = NULL;
        ((corto_member)ShapeType_shapesize_o)->weak = FALSE;
        ((corto_member)ShapeType_shapesize_o)->id = 3;
        ShapeType_shapesize_o->actualType = NULL;
        if (corto_define(ShapeType_shapesize_o)) {
            corto_error("shapes_load: failed to define 'ShapeType_shapesize_o' (%s)", corto_lasterr());
            goto error;
        }
    }

    ShapeType_x_o = idl_Member(corto_declareChild(ShapeType_o, "x", idl_Member_o));
    if (!ShapeType_x_o) {
        corto_error("shapes_load: failed to declare 'ShapeType_x_o' (%s)", corto_lasterr());
        goto error;
    }

    if (!corto_checkState(ShapeType_x_o, CORTO_DEFINED)) {
        ((corto_member)ShapeType_x_o)->type = corto_type(corto_resolve(NULL, "int32"));
        ((corto_member)ShapeType_x_o)->modifiers = 0x0;
        ((corto_member)ShapeType_x_o)->unit = NULL;
        ((corto_member)ShapeType_x_o)->state = 0x6;
        ((corto_member)ShapeType_x_o)->stateCondExpr = NULL;
        ((corto_member)ShapeType_x_o)->weak = FALSE;
        ((corto_member)ShapeType_x_o)->id = 1;
        ShapeType_x_o->actualType = NULL;
        if (corto_define(ShapeType_x_o)) {
            corto_error("shapes_load: failed to define 'ShapeType_x_o' (%s)", corto_lasterr());
            goto error;
        }
    }

    ShapeType_y_o = idl_Member(corto_declareChild(ShapeType_o, "y", idl_Member_o));
    if (!ShapeType_y_o) {
        corto_error("shapes_load: failed to declare 'ShapeType_y_o' (%s)", corto_lasterr());
        goto error;
    }

    if (!corto_checkState(ShapeType_y_o, CORTO_DEFINED)) {
        ((corto_member)ShapeType_y_o)->type = corto_type(corto_resolve(NULL, "int32"));
        ((corto_member)ShapeType_y_o)->modifiers = 0x0;
        ((corto_member)ShapeType_y_o)->unit = NULL;
        ((corto_member)ShapeType_y_o)->state = 0x6;
        ((corto_member)ShapeType_y_o)->stateCondExpr = NULL;
        ((corto_member)ShapeType_y_o)->weak = FALSE;
        ((corto_member)ShapeType_y_o)->id = 2;
        ShapeType_y_o->actualType = NULL;
        if (corto_define(ShapeType_y_o)) {
            corto_error("shapes_load: failed to define 'ShapeType_y_o' (%s)", corto_lasterr());
            goto error;
        }
    }

    if (!corto_checkState(ShapeType_o, CORTO_DEFINED)) {
        ((corto_type)ShapeType_o)->kind = CORTO_COMPOSITE;
        ((corto_type)ShapeType_o)->reference = FALSE;
        ((corto_type)ShapeType_o)->attr = 0x10;
        ((corto_type)ShapeType_o)->options.parentType = NULL;
        ((corto_type)ShapeType_o)->options.parentState = 0x6;
        ((corto_type)ShapeType_o)->options.defaultType = NULL;
        ((corto_type)ShapeType_o)->options.defaultProcedureType = NULL;
        ((corto_interface)ShapeType_o)->base = NULL;
        ShapeType_o->baseAccess = 0x2;
        ShapeType_o->keys.length = 0;
        ShapeType_o->keys.buffer = NULL;
        if (corto_define(ShapeType_o)) {
            corto_error("shapes_load: failed to define 'ShapeType_o' (%s)", corto_lasterr());
            goto error;
        }
    }

    if (corto_type(ShapeType_o)->size != sizeof(ShapeType)) {
        corto_error("shapes_load: calculated size '%d' of type 'ShapeType_o' doesn't match C-type size '%d'", corto_type(ShapeType_o)->size, sizeof(ShapeType));
    }

    corto_setAttr(prevAttr);

    if (_a_) {
        corto_release(_a_);
    }

    return 0;
error:
    if (_a_) {
        corto_release(_a_);
    }

    return -1;
}
