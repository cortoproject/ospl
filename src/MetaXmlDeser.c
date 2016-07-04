
#include "ospl/ospl.h"
#include "ospl/MetaXml.h"
#include "corto/fmt/xml/xml.h"

/* Example of simple definition
 *
 * <Struct name="Point">
 *   <Member name="x"><Long/></Member>
 *   <Member name="y"><Long/></Member>
 * </Struct>
 */

typedef struct ospl_MetaXmlData_t {
    corto_xmlnode node;
    corto_object scope;
} ospl_MetaXmlData_t;

int ospl_MetaXmlParseNode(corto_xmlnode node, void* userData);
corto_object ospl_MetaXmlParseType(corto_xmlnode node, corto_bool followTypedef, ospl_MetaXmlData_t *userData);

/* Parse type element */
corto_type ospl_MetaXmlParseTypeChild(corto_xmlnode node, ospl_MetaXmlData_t *data) {
    /* Count children of node, ignore potential redundant elements */
    if (corto_xmlnodeChildrenCount(node) < 1) {
        corto_seterr("%d: expected type", corto_xmlnodeLine(node));
        goto error;
    }

    /* Get type of member */
    corto_xmlnode typeNode = corto_xmlnodeGetChild(node, 0);
    if (!typeNode) {
        corto_seterr("%d: could not obtain type for member",
          corto_xmlnodeLine(node));
        goto error;
    }

    /* Parse node to obtain type of member */
    corto_type t = ospl_MetaXmlParseType(typeNode, TRUE, data);
    if (!t) {
        goto error;
    }

    return t;
error:
    return NULL;
}

/* Parse a scope (Module, Struct, Union or Enum) */
corto_int16 ospl_MetaXmlParseScope(corto_xmlnode node, corto_type t, void *userData) {
    corto_string name = (corto_string)corto_xmlnodeAttrStr(node, "name");
    ospl_MetaXmlData_t *data = userData;

    if (!name) {
        corto_seterr(
            "%d: %s is missing name attribute",
            corto_idof(t),
            corto_xmlnodeLine(data->node));
        goto error;
    }

    corto_object o = corto_declareChild(data->scope, name, t);
    if (!o) {
        goto error;
    }

    if (corto_instanceof(corto_struct_o, o)) {
        corto_setref(&corto_interface(o)->base, corto_interface(ospl_BaseType_o));
    }

    corto_object prevScope = data->scope;
    data->scope = o;
    if (!corto_xmlnodeWalkChildren(node, ospl_MetaXmlParseNode, data)) {
        goto error;
    }
    data->scope = prevScope;

    if (corto_define(o)) {
        goto error;
    }

    return 0;
error:
    return -1;
}

/* Parse array */
corto_type ospl_MetaXmlParseArray(corto_xmlnode node, ospl_MetaXmlData_t *data) {
    corto_string sizeStr = (corto_string)corto_xmlnodeAttrStr(node, "size");
    if (!sizeStr) {
        /* If an Array is found without a size, the code is probably
         * deserializing a metadescriptor from the kernel, where arrays are
         * the same as sequences. For translation to DCPS, skip & pray. */
        corto_warning(
            "%d: warning: missing size attribute for Array",
            corto_xmlnodeLine(node));

        return corto_type(corto_binaryCreate(CORTO_WIDTH_WORD));
    } else {
        corto_array o = corto_declare(corto_array_o);
        if (!o) {
            goto error;
        }

        corto_type t = ospl_MetaXmlParseTypeChild(node, data);
        if (!t) {
            goto error_child;
        }

        corto_int32 size = atoi(sizeStr);
        if (!size) {
            corto_seterr(
                "%d: invalid size for Array",
                corto_xmlnodeLine(node));
            goto error_size;
        }

        corto_setref(&corto_collection(o)->elementType, t);
        corto_collection(o)->max = size;

        if (corto_define(o)) {
            goto error_define;
        }

        return corto_type(o);
    error_define:
    error_size:
        corto_release(t);
    error_child:
        corto_delete(o);
    error:
        return NULL;
    }
}

/* Parse sequence */
corto_type ospl_MetaXmlParseSequence(corto_xmlnode node, ospl_MetaXmlData_t *data) {
    corto_string sizeStr = (corto_string)corto_xmlnodeAttrStr(node, "size");
    corto_int32 size = 0;
    if (sizeStr) {
        size = atoi(sizeStr);
    }

    corto_sequence o = corto_declare(corto_sequence_o);
    if (!o) {
        goto error;
    }

    corto_type t = ospl_MetaXmlParseTypeChild(node, data);
    if (!t) {
        goto error_child;
    }

    corto_setref(&corto_collection(o)->elementType, t);
    corto_collection(o)->max = size;

    if (corto_define(o)) {
        goto error_define;
    }

    return corto_type(o);
error_define:
    corto_release(t);
error_child:
    corto_delete(o);
error:
    return NULL;
}

/* Parse string */
corto_type ospl_MetaXmlParseString(corto_xmlnode node, ospl_MetaXmlData_t *data) {
    corto_string sizeStr = (corto_string)corto_xmlnodeAttrStr(node, "size");
    corto_text o = NULL;
    corto_int32 size = 0;
    if (sizeStr) {
        size = atoi(sizeStr);
    }

    if (size) {
        o = corto_declare(corto_text_o);
        corto_text(o)->length = size;
        corto_primitive(o)->width = CORTO_WIDTH_8;
        if (corto_define(o)) {
            goto error;
        }
    } else {
        o = corto_string_o;
    }

    if (!o) {
        goto error;
    }

    return corto_type(o);
error:
    corto_delete(o);
    return NULL;
}

/* Parse reference */
corto_type ospl_MetaXmlParseRef(corto_xmlnode node, ospl_MetaXmlData_t *data) {
    corto_string ref = (corto_string)corto_xmlnodeAttrStr(node, "name");
    if (!ref) {
        corto_seterr(
            "%d: missing name attribute of Type",
            corto_xmlnodeLine(node));
        goto error;
    }

    corto_object type = corto_resolve(data->scope, ref);
    if (!type) {
        corto_seterr("%d: unresolved type '%s'",
            corto_xmlnodeLine(node),
            ref);
        goto error;
    }

    /* Since it's metadata, assume it's not going anywhere */
    corto_release(type);

    return type;
error:
    return NULL;
}

/* Parse reference to a type */
corto_type ospl_MetaXmlParseTypeRef(corto_xmlnode node, ospl_MetaXmlData_t *data) {
    corto_object type = ospl_MetaXmlParseRef(node, data);
    if (!type) {
        goto error;
    }

    /* Follow typedef until an actual type is found */
    while (corto_instanceof(ospl_Typedef_o, type)) {
        type = ospl_Typedef(type)->type;
    }

    /* Check whether object is a type */
    if (!corto_instanceof(corto_type_o, type)) {
        corto_seterr("%d: '%s' is not a type",
            corto_xmlnodeLine(node),
            corto_fullpath(NULL, type));
        goto error;
    }

    return corto_type(type);
error:
    return NULL;
}

/* Parse type */
corto_object ospl_MetaXmlParseType(corto_xmlnode node, corto_bool followTypedef, ospl_MetaXmlData_t *data) {
    corto_string name = (corto_string)corto_xmlnodeName(node);
    corto_object type = NULL;

    if (!strcmp(name, "Boolean")) {
        type = corto_bool_o;
    } else if (!strcmp(name, "Char")) {
        type = corto_char_o;
    } else if (!strcmp(name, "Octet")) {
        type = corto_octet_o;
    } else if (!strcmp(name, "Short")) {
        type = corto_int16_o;
    } else if (!strcmp(name, "Long")) {
        type = corto_int32_o;
    } else if (!strcmp(name, "LongLong")) {
        type = corto_int64_o;
    } else if (!strcmp(name, "UShort")) {
        type = corto_uint16_o;
    } else if (!strcmp(name, "ULong")) {
        type = corto_uint32_o;
    } else if (!strcmp(name, "ULongLong")) {
        type = corto_uint64_o;
    } else if (!strcmp(name, "Float")) {
        type = corto_float32_o;
    } else if (!strcmp(name, "Double")) {
        type = corto_float64_o;
    } else if (!strcmp(name, "Time")) {
        type = corto_time_o;
    } else if (!strcmp(name, "String")) {
        type = ospl_MetaXmlParseString(node, data);
    } else if (!strcmp(name, "Array")) {
        type = ospl_MetaXmlParseArray(node, data);
    } else if (!strcmp(name, "Sequence")) {
        type = ospl_MetaXmlParseSequence(node, data);
    } else if (!strcmp(name, "Type")) {
        if (followTypedef) {
            type = ospl_MetaXmlParseTypeRef(node, data);
        } else {
            type = ospl_MetaXmlParseRef(node, data);
        }
    }

    if (!type) {
        corto_seterr("invalid type '%s'", name);
        goto error;
    }

    return type;
error:
    return NULL;
}

/* Parse a member */
corto_int16 ospl_MetaXmlParseMember(corto_xmlnode node, void *userData) {
    corto_string name = (corto_string)corto_xmlnodeAttrStr(node, "name");
    ospl_MetaXmlData_t *data = userData;

    if (!name) {
        corto_seterr(
            "%d: member is missing name attribute",
            corto_xmlnodeLine(data->node));
        goto error;
    }

    corto_type t = ospl_MetaXmlParseTypeChild(node, data);
    if (!t) {
        goto error;
    }

    /* Lookup member first, because it is possibly created with a different
     * type (ospl/Member)*/
    corto_object o = corto_lookup(data->scope, name);
    if (!o) {
        o = corto_declareChild(data->scope, name, corto_member_o);

        if (!o) {
            goto error;
        }

        corto_setref(&corto_member(o)->type, t);

        if (corto_define(o)) {
            goto error;
        }
    }

    return 0;
error:
    return -1;
}

/* Parse union discriminator type */
corto_int16 ospl_MetaXmlParseSwitchType(corto_xmlnode node, void *userData) {
    ospl_MetaXmlData_t *data = userData;

    if (!corto_instanceof(corto_union_o, data->scope)) {
        corto_seterr(
            "%d: switchtype invalid in non-union scope",
            corto_xmlnodeLine(data->node));
        goto error;
    }

    corto_type t = ospl_MetaXmlParseTypeChild(node, data);
    if (!t) {
        goto error;
    }

    corto_setref(&corto_union(data->scope)->discriminator, t);

    return 0;
error:
    return -1;
}

/* Parse a case */
corto_int16 ospl_MetaXmlParseCase(corto_xmlnode node, void *userData) {
    corto_string name = (corto_string)corto_xmlnodeAttrStr(node, "name");
    ospl_MetaXmlData_t *data = userData;

    if (!name) {
        corto_seterr(
            "%d: member is missing name attribute",
            corto_xmlnodeLine(data->node));
        goto error;
    }

    corto_type t = ospl_MetaXmlParseTypeChild(node, data);
    if (!t) {
        goto error;
    }

    corto_case o = corto_declareChild(data->scope, name, corto_case_o);
    if (!o) {
        goto error;
    }

    corto_setref(&corto_member(o)->type, t);

    /* Walk labels */
    corto_object prevScope = data->scope;
    data->scope = o;
    if (!corto_xmlnodeWalkChildren(node, ospl_MetaXmlParseNode, data)) {
        goto error;
    }
    data->scope = prevScope;

    if (corto_define(o)) {
        goto error;
    }

    return 0;
error:
    return -1;
}

/* Parse a label */
corto_int16 ospl_MetaXmlParseLabel(corto_xmlnode node, void *userData) {
    corto_string value = (corto_string)corto_xmlnodeAttrStr(node, "value");
    ospl_MetaXmlData_t *data = userData;

    if (!value) {
        corto_seterr(
            "%d: label is missing value attribute",
            corto_xmlnodeLine(data->node));
        goto error;
    }

    if (!corto_instanceof(corto_case_o, data->scope)) {
        corto_seterr(
            "%d: label invalid in non-case scope",
            corto_xmlnodeLine(data->node));
        goto error;
    }

    corto_union u = corto_parentof(data->scope);
    if (!u->discriminator) {
        corto_seterr(
            "%d: switchtype of union must be set before case",
            corto_xmlnodeLine(data->node));
        goto error;
    }

    /* Translate label value into discriminator label */
    void *ptr = alloca(corto_type_sizeof(u->discriminator));
    corto_convert(corto_string_o, &value, u->discriminator, ptr);
    corto_case c = data->scope;
    corto_int32seqAppend(&c->discriminator, *(corto_int32*)ptr);

    return 0;
error:
    return -1;
}

/* Parse an enumeration constant */
corto_int16 ospl_MetaXmlParseElement(corto_xmlnode node, void *userData) {
    corto_string name = (corto_string)corto_xmlnodeAttrStr(node, "name");
    ospl_MetaXmlData_t *data = userData;

    if (!name) {
        corto_seterr(
            "%d: element is missing name attribute",
            corto_xmlnodeLine(data->node));
        goto error;
    }

    corto_object o = corto_declareChild(data->scope, name, corto_constant_o);
    if (!o) {
        goto error;
    }

    corto_string value = corto_xmlnodeAttrStr(node, "value");
    if (!value) {
        corto_seterr("%d: missing value for element",
            corto_xmlnodeLine(node));
        goto error;
    }

    *(corto_constant*)o = atoi(value);

    if (corto_define(o)) {
        goto error;
    }

    return 0;
error:
    return -1;
}

/* Parse a typedef */
corto_int16 ospl_MetaXmlParseTypedef(corto_xmlnode node, ospl_MetaXmlData_t *data) {
    corto_string name = (corto_string)corto_xmlnodeAttrStr(node, "name");
    if (!name) {
        corto_seterr("%d: missing name attribute for Typedef");
        goto error;
    }

    corto_type t = ospl_MetaXmlParseTypeChild(node, data);
    if (!t) {
        corto_seterr("%d: can't parse type of typedef: %s",
            corto_xmlnodeLine(data->node),
            corto_lasterr());
        goto error;
    }

    /* Create new typedef object */
    if (!ospl_TypedefCreateChild(data->scope, name, t)) {
        corto_seterr("%d: failed to create Typedef: %s",
            corto_xmlnodeLine(data->node),
            corto_lasterr());
        goto error;
    }

    return 0;
error:
    return -1;
}

int ospl_MetaXmlParseNode(corto_xmlnode node, void* userData) {
    corto_string name = (corto_string)corto_xmlnodeName(node);
    ospl_MetaXmlData_t *data = userData;

    if (!strcmp(name, "Module")) {
        return !ospl_MetaXmlParseScope(node, corto_type(corto_package_o), data);
    } else if (!strcmp(name, "Struct")) {
        return !ospl_MetaXmlParseScope(node, corto_type(corto_struct_o), data);
    } else if (!strcmp(name, "Union")) {
        return !ospl_MetaXmlParseScope(node, corto_type(corto_union_o), data);
    } else if (!strcmp(name, "Enum")) {
        return !ospl_MetaXmlParseScope(node, corto_type(corto_enum_o), data);
    } else if (!strcmp(name, "TypeDef")) {
        return !ospl_MetaXmlParseTypedef(node, data);
    } else if (!strcmp(name, "Member")) {
        return !ospl_MetaXmlParseMember(node, data);
    } else if (!strcmp(name, "Case")) {
        return !ospl_MetaXmlParseCase(node, data);
    } else if (!strcmp(name, "Label")) {
        return !ospl_MetaXmlParseLabel(node, data);
    } else if (!strcmp(name, "SwitchType")) {
        return !ospl_MetaXmlParseSwitchType(node, data);
    } else if (!strcmp(name, "Element")) {
        return !ospl_MetaXmlParseElement(node, data);
    }

    return 1; /* Continue parsing */
}

corto_int16 ospl_metaXmlParse(corto_string xml) {
    corto_xmlreader reader = corto_xmlMemoryReaderNew(xml, "MetaData");

    if (reader) {
        ospl_MetaXmlData_t walkData;

        walkData.node = corto_xmlreaderRoot(reader);
        walkData.scope = root_o;

        /* Walk children */
        if (!corto_xmlnodeWalkChildren(
            walkData.node,
            ospl_MetaXmlParseNode,
            &walkData))
        {
            goto error;
        }

        /* Cleanup */
        corto_xmlreaderFree(reader);
    } else {
        corto_error("failed to parse xml: %s", corto_lasterr());
        goto error;
    }

    return 0;
error:
    return -1;
}
