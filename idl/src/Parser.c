/* $CORTO_GENERATED
 *
 * Parser.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/idl/idl.h>

corto_int16 _idl_Parser_construct(
    idl_Parser this)
{
/* $begin(ospl/idl/Parser/construct) */

    if (!this->source) {
        if (this->filename) {
            this->source = corto_fileLoad(this->filename);
            if (!this->source) {
                corto_seterr("idl: failed to load '%s'", this->filename);
                goto error;
            }
        }
    }

    return 0;
error:
    return -1;
/* $end */
}

corto_struct _idl_Parser_declareStruct(
    idl_Parser this,
    corto_string name)
{
/* $begin(ospl/idl/Parser/declareStruct) */

    corto_struct result = corto_structDeclareChild(this->scope, name);
    corto_setref(&this->scope, result);

    return result;
/* $end */
}

corto_class _idl_Parser_declareValueType(
    idl_Parser this,
    corto_string name,
    idl_InheritanceSpec *inherits)
{
/* $begin(ospl/idl/Parser/declareValueType) */
    corto_interfaceseq seq = {0, NULL};

    if (inherits->supports && corto_llSize(inherits->supports)) {
        corto_uint32 i = 0;

        seq.length = corto_llSize(inherits->supports);
        seq.buffer = corto_alloc(sizeof(corto_interface) * seq.length);

        corto_typeListForeach(inherits->supports, interface) {
            corto_setref(&seq.buffer[i++], interface);
        }
    }

    corto_class result = corto_classDeclareChild(this->scope, name);
    corto_setref(&corto_interface(result)->base, inherits->base);
    result->implements = seq;
    corto_setref(&this->scope, result);

    return result;
/* $end */
}

corto_int16 _idl_Parser_defineStruct(
    idl_Parser this,
    corto_struct s)
{
/* $begin(ospl/idl/Parser/defineStruct) */

    corto_setref(&this->scope, corto_parentof(this->scope));

    if (corto_define(s)) {
        corto_delete(s);
        goto error;
    }

    return 0;
error:
    return -1;
/* $end */
}

corto_int16 _idl_Parser_defineValueType(
    idl_Parser this,
    corto_class v)
{
/* $begin(ospl/idl/Parser/defineValueType) */

    corto_setref(&this->scope, corto_parentof(this->scope));

    if (corto_define(v)) {
        corto_delete(v);
        goto error;
    }

    return 0;
error:
    return -1;
/* $end */
}

corto_int16 _idl_Parser_parse(
    idl_Parser this)
{
/* $begin(ospl/idl/Parser/parse) */

    this->line = 1;
    corto_int16 idl_yparse(idl_Parser p);
    idl_yparse(this);

    return this->errors;
/* $end */
}

corto_enum _idl_Parser_parseEnum(
    idl_Parser this,
    corto_string name,
    corto_stringList enumerators)
{
/* $begin(ospl/idl/Parser/parseEnum) */

    corto_enum result = corto_enumDeclareChild(this->scope, name);

    corto_stringListForeach(enumerators, e) {
        corto_constant *c = corto_constantDeclareChild(result, e);
        if (corto_define(c)) {
            corto_delete(result);
            goto error;
        }
    }

    if (corto_define(result)) {
        corto_delete(result);
        goto error;
    }

    return result;
error:
    return NULL;
/* $end */
}

corto_int16 _idl_Parser_parseMember(
    idl_Parser this,
    corto_type type,
    idl_DeclaratorList name,
    corto_bool readonly)
{
/* $begin(ospl/idl/Parser/parseMember) */
     corto_modifier modifiers = CORTO_GLOBAL;

     if (readonly) {
        modifiers |= CORTO_READONLY;
     }

    idl_DeclaratorListForeach(name, d) {
        corto_type t = idl_Declarator_getType(d, type);
        corto_member m = corto_memberDeclareChild(this->scope, d->identifier);
        corto_setref(&m->type, t);
        m->modifiers = modifiers;
        m->state = CORTO_DEFINED | CORTO_DECLARED;
        m->weak = FALSE;
        if (corto_define(m)) {
            goto error;
        }
        corto_release(t);
    }

    return 0;
error:
    return -1;
/* $end */
}

corto_int16 _idl_Parser_parseMethod(
    idl_Parser this,
    corto_type returnType,
    corto_string name,
    corto_parameterList parameters)
{
/* $begin(ospl/idl/Parser/parseMethod) */
    corto_id sig;
    corto_method m = NULL;
    corto_uint32 i = 0;

    strcpy(sig, name);
    strcat(sig, "(");

    if (parameters && corto_llSize(parameters)) {
        corto_parameterListForeach(parameters, p) {
            if (i) {
                strcat(sig, ",");
            }
            if (corto_parentof(p->type) == corto_lang_o) {
                strcat(sig, corto_idof(p->type));
            } else {
                corto_id paramType;
                corto_fullpath(paramType, p->type);
                strcat(sig, paramType);
            }
            if (p->passByReference && !p->type->reference) {
                strcat(sig, "&");
            }
            strcat(sig, " ");
            strcat(sig, p->name);
            i++;
        }
    }
    strcat(sig, ")");

    if (!(m = corto_methodDeclareChild(this->scope, sig))) {
        goto error;
    }

    corto_function(m)->kind = CORTO_PROCEDURE_STUB;
    corto_setref(&corto_function(m)->returnType, returnType);

    return 0;
error:
    return -1;
/* $end */
}

corto_type _idl_Parser_parseTypedef(
    idl_Parser this,
    corto_type t,
    idl_DeclaratorList declarators)
{
/* $begin(ospl/idl/Parser/parseTypedef) */
    corto_type result = NULL;

    /* Only allow typedefs to primitives or non-scoped types */
    idl_DeclaratorListForeach(declarators, d) {
        corto_type declaratorType = idl_Declarator_getType(d, t);
        if (!declaratorType) {
            goto error;
        }
        if ((declaratorType->kind == CORTO_PRIMITIVE) || !corto_checkAttr(declaratorType, CORTO_ATTR_SCOPED)) {
            result = corto_declareChild(this->scope, d->identifier, corto_typeof(declaratorType));
            if (!result) {
                goto error;
            }
            corto_copy((corto_object*)&result, declaratorType);
            if (corto_define(result)) {
                goto error;
            }
            corto_release(declaratorType);
        } else {
            result = ospl_TypedefCreateChild(this->scope, d->identifier, corto_typeof(declaratorType));
            if (!result) {
                goto error;
            }
        }
    }

    return result;
error:
    return NULL;
/* $end */
}

corto_void _idl_Parser_popModule(
    idl_Parser this)
{
/* $begin(ospl/idl/Parser/popModule) */

    corto_setref(&this->scope, corto_parentof(this->scope));

/* $end */
}

corto_package _idl_Parser_pushModule(
    idl_Parser this,
    corto_string name)
{
/* $begin(ospl/idl/Parser/pushModule) */
    corto_package p = corto_packageCreateChild(this->scope, name, NULL);
    if (!p) {
        goto error;
    }

    corto_setref(&this->scope, p);

    return p;
error:
    return NULL;
/* $end */
}
