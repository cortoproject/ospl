/*
 * ospl_metaserializer.c
 *
 *  Created on: Dec 14, 2012
 *      Author: sander
 */

#include "ospl/ospl.h"
#include "ospl/MetaXml.h"


/* #define SER_DEBUG */

/* Quick hack to be able to debug the serializer */
corto_struct ospl_Time_t_o = NULL;

/* --------------------- Serialization driving functions -------------------- */

typedef struct ospl_context_s* ospl_context;
struct ospl_context_s {
    corto_ll items; /* corto_ll<ospl_item> - Dependency ordered list of contextItems. */
    corto_ll declarations; /* corto_ll<ospl_item> - Declarations - for cycle detection. */

    /* Variables used for printing */
    corto_object module; /* Used for keeping track of current scope while printing. */
    corto_ll inlineProcessed; /* corto_ll<corto_type> - Keeps a list of the inline types defined in the current top-level type. This list is needed
                         because types can be used more than once within a given type-scope and the algorithm must be able
                         to determine whether a type should be defined or whether it already is. */
    corto_ll xmlbuff;  /* corto_ll<corto_char*> This contains a list of string-buffers, which together form the metadescriptor. The
                        final step is to aggegrate these buffers into one string. */

    /* Module optimization */
    corto_ll modules; /* corto_ll<ospl_moduleItem> - This list will be populated with used modules when types are parsed with modules. Per module
                       the number of types is counted. At print-time, arrays are allocated of size typeCount * sizeof(ospl_item).
                       These arrays will at any time hold the unprinted root-type items with dependency count 0. */
};

/* This structure is only used in the final module optimization step. */
typedef struct ospl_item_s* ospl_item;
typedef struct ospl_moduleItem_s* ospl_moduleItem;
struct ospl_moduleItem_s {
    corto_object self;
    corto_uint32 typeCount; /* Worst case amount of types that can be stored at once in this module. */
    ospl_item* array; /* Array which at any time contains the unprinted root-type items with dependency count 0 */
    ospl_item* start; /* Pointer to the next item that should be printed. */
    ospl_item* end; /* Pointer which indicates the next free element in the list. */
};

struct ospl_item_s {
    corto_type self;
    corto_int32 refcount; /* This is the number of unresolved(unprinted) dependencies */
    corto_ll dependees; /* corto_ll<ospl_item> */
    ospl_moduleItem module; /* Pointer to module-item. This overcomes the need for doing
                             module-lookups at print-time, when a type must be added
                             to the type-array of a module when it reaches dependency-count 0. */
};


/* --------------------- Serialization utilities --------------------- */

/* Find the root-type (NULL if type is not an inlined type) */
corto_type
ospl_utilRootType(
    corto_type type)
{
    corto_object result, prev;

    prev = NULL;

    if(corto_checkAttr(type, CORTO_ATTR_SCOPED)) {
        result = corto_parentof(type);
        while(result && (corto_class_instanceof(corto_type_o, result))) {
            prev = result;

            /* If result isn't a module, there must be higher levels in the hierarchy. */
            result = corto_parentof(result);
            corto_assert(result != NULL, "missing parent.");
        }
    }

    return corto_type(prev);
}

/* Check whether an object is inlined.
 * Collections and primitives are conceptually always inlined,
 * though the implementation might place them in a scope.
 */
static corto_bool
ospl_utilIsInlined(
    corto_type type)
{
    corto_bool inlined;

    inlined = FALSE;

    if(corto_type(type) == type) {
        inlined = (type->kind == CORTO_COLLECTION) || ((type->kind == CORTO_PRIMITIVE) && (corto_primitive(type)->kind != CORTO_ENUM));
    }

    if(!inlined) {
        inlined = corto_class_instanceof(corto_type_o, corto_parentof(type));
    }

    return inlined;
}

static ospl_item
ospl_itemNew(
    corto_type self)
{
    ospl_item result;

    result = corto_alloc(sizeof(struct ospl_item_s));
    result->self = self;
    result->refcount = 0;
    result->dependees = NULL;
    result->module = NULL;

    return result;
}

static void
ospl_itemFree(
    ospl_item item)
{
    if(item->dependees) {
        corto_llFree(item->dependees);
    }
    corto_dealloc(item);
}

static void
ospl_moduleItemFree(
    ospl_moduleItem item)
{
    if(item->array) {
        corto_dealloc(item->array);
    }
    corto_dealloc(item);
}

static ospl_moduleItem
ospl_moduleItemNew(
    corto_object module)
{
    ospl_moduleItem result;

    result = corto_alloc(sizeof(struct ospl_moduleItem_s));
    result->array = NULL;
    result->start = NULL;
    result->end = NULL;
    result->typeCount = 0;
    result->self = module;

    return result;
}

static ospl_context
ospl_contextNew(void)
{
    ospl_context result;

    result = corto_alloc(sizeof(struct ospl_context_s));
    result->items = corto_llNew();
    result->declarations = corto_llNew();

    result->module = NULL;
    result->xmlbuff = corto_llNew();
    result->inlineProcessed = corto_llNew();
    result->modules = corto_llNew();

    return result;
}

static int
ospl_freeItems(
    void* o,
    void* udata)
{
    ospl_item item;
    CORTO_UNUSED(udata);

    item = o;
    ospl_itemFree(item);

    return 1;
}

static int
ospl_freeModuleItems(
    void* o,
    void* udata)
{
    ospl_moduleItem item;
    CORTO_UNUSED(udata);

    item = o;
    ospl_moduleItemFree(item);

    return 1;
}

static void
ospl_contextFree(
    ospl_context context)
{
    /* Free items */
    corto_llWalk(context->items, ospl_freeItems, NULL);
    corto_llFree(context->items);

    /* Free declarations */
    corto_llFree(context->declarations);

    /* Free modules */
    corto_llWalk(context->modules, ospl_freeModuleItems, NULL);
    corto_llFree(context->modules);

    corto_assert(context->xmlbuff == NULL, "xmlbuffer is not NULL");

    corto_dealloc(context);
}

/* Add a dependee to the dependee list of an item */
static void
ospl_itemAddDependee(
    ospl_item item,
    ospl_item dependee)
{
    if(!item->dependees) {
        item->dependees = corto_llNew();
    }
    corto_llInsert(item->dependees, dependee);

    /* Increase the refcount of the dependee. */
    dependee->refcount++;
    corto_assert(item->dependees != NULL, "dependees is empty.");
}

/* Add item to module array */
static void
ospl_itemAddToModule(
    ospl_item item) {
    ospl_moduleItem module = item->module;

    if(!module->array) {
        module->array = corto_alloc(module->typeCount * sizeof(ospl_item));
        module->start = module->array;
        module->end = module->array;
    }

    *(module->end) = item;
    module->end++;

    corto_assert(module->end <= (module->array + module->typeCount), "type must not exceed bounds for module.");
}

struct ospl_itemLookup_t {
    corto_type type;
    ospl_item result;
};

static int
ospl_itemDerefDependee(
    void* o, void* udata)
{
    CORTO_UNUSED(udata);

    if(!--((ospl_item)o)->refcount) {
        ospl_itemAddToModule(o);
    }
    return 1;
}

/* Deref dependees. Dependees are deref'd when a dependency (the item passed to this function) is
 * resolved. When a dependee reaches refcount zero, it can be processed itself. */
static void
ospl_itemDerefDependees(
    ospl_item item)
{
    if(item->dependees) {
        corto_llWalk(item->dependees, ospl_itemDerefDependee, NULL);
    }
}

int
ospl_contextLookupAction(
    void* o,
    void* userData)
{
    struct ospl_itemLookup_t* data;

    data = userData;
    if(((ospl_item)o)->self == data->type) {
        data->result = o;
    }

    return data->result == NULL;
}

/* Check whether an item is already processed */
static ospl_item
ospl_contextIsProcessed(
    ospl_context context,
    corto_type type)
{
    struct ospl_itemLookup_t walkData;

    walkData.type = type;
    walkData.result = NULL;
    corto_llWalk(context->items, ospl_contextLookupAction, &walkData);

    return walkData.result;
}

/* Check for cycles */
static ospl_item
ospl_contextCheckCycles(
    ospl_context context,
    corto_type type)
{
    struct ospl_itemLookup_t walkData;

    walkData.type = type;
    walkData.result = NULL;
    corto_llWalk(context->declarations, ospl_contextLookupAction, &walkData);

    return walkData.result;
}

struct ospl_contextFindModule_t {
    corto_object find;
    ospl_moduleItem result;
};

static int
ospl_contextFindModule(
    void* o,
    void* userData)
{
    struct ospl_contextFindModule_t* data;

    data = userData;
    if(((ospl_moduleItem)o)->self == data->find) {
        data->result = o;
    }

    return data->result == NULL;
}

/* Mark an item as processed, add to the context->items list */
static void
ospl_contextProcessed(
    ospl_context context,
    ospl_item item)
{
    struct ospl_contextFindModule_t walkData;
    corto_assert(!ospl_contextIsProcessed(context, item->self), "function called on already processed object.");
    corto_llAppend(context->items, item);

    /* Find corresponding module */
    walkData.find = corto_parentof(item->self);
    walkData.result = NULL;
    corto_llWalk(context->modules, ospl_contextFindModule, &walkData);

    /* If module is not found, create new module object. */
    if(!walkData.result) {
        walkData.result = ospl_moduleItemNew(walkData.find);
        corto_llInsert(context->modules, walkData.result);
    }

    /* Administrate extra type for module. */
    walkData.result->typeCount++;

    /* Store pointer in item to module, so that during printing no lookups are required. */
    item->module = walkData.result;
}

/* Mark an item as declared, add to the context->declarations list */
static void
ospl_contextDeclare(
    ospl_context context,
    ospl_item item)
{
    corto_assert(!ospl_contextIsProcessed(context, item->self), "item passed to ospl_contextDeclare that is already processed.");
    corto_llAppend(context->declarations, item);
}

/* Serialize a type */
static int
ospl_serializeType(
    ospl_context context,
    ospl_item rootType,
    corto_object type,
    corto_bool allowCycles,
    ospl_item* out);

/* Serialize dependencies of a typedef */
static int
ospl_serializeTypedefDependencies(
    ospl_context context,
    ospl_item rootType,
    corto_bool allowCycles,
    ospl_Typedef type)
{
    ospl_item alias;

    alias = NULL;

    /* Resolve dependency */
    if(ospl_serializeType(context, rootType, type->type, allowCycles, &alias)) {
        goto error;
    }

    /* Add typedef to dependee list of dependency */
    if(alias) {
        ospl_itemAddDependee(alias, rootType);
    }

    return 0;
error:
    return -1;
}

/* Serialize dependencies of a collection. */
static int
ospl_serializeCollectionDependencies(
    ospl_context context,
    ospl_item rootType,
    corto_collection type)
{
    ospl_item subType;
    corto_bool allowCycles;

    subType = NULL;
    allowCycles = FALSE;

    /* Check if type is a sequence or array to find out if cycles are allowed. */
    if(type->kind == CORTO_SEQUENCE) {
        allowCycles = TRUE;
    }

    /* Resolve dependency */
    if(ospl_serializeType(context, rootType, type->elementType, allowCycles, &subType)) {
        goto error;
    }

    /* Add collection to dependee list of subType */
    if(subType) {
        ospl_itemAddDependee(subType, rootType);
    }

    return 0;
error:
    return -1;
}

corto_object ospl_actualMemberType(corto_member m) {
    corto_object result = NULL;
    if (corto_instanceof(ospl_Member_o, m)) {
        /* Type was likely parsed from IDL */
        result = ospl_Member(m)->actualType;
    }
    if (!result) {
        result = m->type;
    }
    return result;
}

typedef struct ospl_ser_member_t {
    ospl_context context;
    ospl_item rootType;
} ospl_ser_member_t;

corto_int16 ospl_ser_member(corto_serializer s, corto_value *info, void *userData) {
    ospl_ser_member_t *data = userData;
    corto_member member = info->is.member.t;
    ospl_item memberType = NULL;

    if (member) {
        if(!member->type->reference) {
            corto_object actualType = ospl_actualMemberType(member);
            if(ospl_serializeType(data->context, data->rootType, actualType, FALSE, &memberType)) {
                goto error;
            }

            if(memberType) {
                ospl_itemAddDependee(memberType, data->rootType);
            }
        }
    }

    return 0;
error:
    return -1;
}

/* Serialize dependencies for a struct */
static int
ospl_serializeStructureDependencies(
    ospl_context context,
    ospl_item rootType,
    corto_interface type)
{
    /* Serialize members */
    struct corto_serializer_s s;
    corto_serializerInit(&s);
    s.access = CORTO_LOCAL|CORTO_PRIVATE|CORTO_HIDDEN;
    s.accessKind = CORTO_NOT;
    s.optionalAction = CORTO_SERIALIZER_OPTIONAL_ALWAYS;
    s.metaprogram[CORTO_MEMBER] = ospl_ser_member;
    ospl_ser_member_t walkData = {context, rootType};

    if (corto_metaWalk(&s, corto_type(type), &walkData)) {
        goto error;
    }

    return 0;
error:
    return -1;
}

/* Serialize dependencies of a type.
 *
 * Parameter 'rootType' is typically the same as the 'type' parameter.
 * However, for inline types these two differ. The distinction
 * enables that dependencies are always added to the
 * top-level type. */
static int
ospl_serializeTypeDependencies(
    ospl_context context,
    ospl_item rootType,
    corto_bool allowCycles,
    corto_object t)
{
    int result;
    corto_type type = ospl_actualType(t);

    result = 0;

    /* Forward to the correct dependency-resolve function, depending on metaKind */
    if(type != t) {
        /* Allowance of cycles is transparently forwarded to typedefs. */
        result = ospl_serializeTypedefDependencies(context, rootType, allowCycles, ospl_Typedef(t));
    }else {
        switch(type->kind) {
        case CORTO_COLLECTION:
            result = ospl_serializeCollectionDependencies(context, rootType, corto_collection(type));
            break;
        case CORTO_PRIMITIVE:
            switch(corto_primitive(type)->kind) {
            case CORTO_ENUM:
                /* Enumerations have no dependencies */
                break;
            default:
                corto_assert(0, "Invalid type-kind for dependency-routine.");
                break;
            }
            break;
        case CORTO_COMPOSITE:
            result = ospl_serializeStructureDependencies(context, rootType, corto_interface(type));
            break;
        default:
            corto_assert(0, "Invalid type-kind.");
            break;
        }
    }

    return result;
}

/* Serialize a type */
static int
ospl_serializeType(
    ospl_context context,
    ospl_item rootType,
    corto_object t,
    corto_bool allowCycles,
    ospl_item* out)
{
    ospl_item item;
    corto_bool isTypedef;
    corto_type type = ospl_actualType(t);

#ifdef SER_DEBUG
    static corto_uint32 indent = 0;
    printf("%*s>> type %s\n", indent*3, " ", corto_fullpath(NULL, type));
    indent++;
#endif

    item = NULL;

    if(type != t) {
        isTypedef = TRUE;
    }else {
        isTypedef = FALSE;
    }

    switch(type->kind) {
    case CORTO_PRIMITIVE:
        /* Primitives and collections are always treated as inline types, and are generated when a type is printed. */
        if(!isTypedef && (corto_primitive(type)->kind != CORTO_ENUM)) {
            break;
        }
        /* no break */
    case CORTO_COMPOSITE:
        /* Check if type is the c_time type in the rootscope which is not handled as a seperate type, but will result in a <Time/>. */
    {
        corto_id id;
        corto_path(id, NULL, type, "::");
        if(!strcmp("::DDS::Time_t", id)) {
            break; /* Don't handle c_time */
        }
    }
    /* no break */
    /* Fallthrough on purpose. */
    case CORTO_COLLECTION: /* Collections are always treated as inline types, but do have dependencies. */
        /* Check if item is already processed */
        if(!(item = ospl_contextIsProcessed(context, type))) {
            /* Check for invalid cycles. Don't process type if a cycle is detected. */
            if(!isTypedef && (type->kind == CORTO_COMPOSITE) && ospl_contextCheckCycles(context, type)) {
                if(!allowCycles) {
                    corto_seterr("ospl: ospl_serializerXMLTypeInfoSerialize: unsupported cycle detected!");
                    goto error;
                }
            }else {
                /* Don't process inlined types in graph. These are resolved when the parent-type is printed, which is always in correct
                 * dependency order, which is the order of members(structs) or cases(unions). Collections are also considered
                 * inlined, because the actual location of a collection type is implementation specific. */
                if(!ospl_utilIsInlined(type)) {
                    /* Here, the usage of 'rootType' is prohibited since this is not an inlined type. */

                    /* Create contextItem */
                    item = ospl_itemNew(type);

                    /* Add forward-declaration marker for structs and unions, to allow references to self, so that
                     * the cycles can be detected. IDL does not allow other types to be forward-declared, thus
                     * cannot introduce cyclic references. */
                    if(!isTypedef && (type->kind == CORTO_COMPOSITE)) {
                        ospl_contextDeclare(context, item);
                    }

                    /* Resolve dependencies of type */
                    if(ospl_serializeTypeDependencies(context, item, allowCycles, type)) {
                        /* An error occurred, most likely an unsupported cyclic dependency */
                        ospl_itemFree(item);
                        goto error;
                    }

                    /* If success, add item to processed list */
                    if(item) {
                        ospl_contextProcessed(context, item);
                    }
                    break;
                }else {
                    corto_type typeRoot;

                    corto_assert(rootType != NULL, "rootType is zero.");

                    /* Still need to resolve dependencies for inlined types */
                    if(ospl_serializeTypeDependencies(context, rootType, allowCycles, type)) {
                        goto error;
                    }

                    /* If rootType is referencing an inlined type that is defined in the scope of another type,
                     * rootType is implicitly dependent on the rootType of the inlined type. */
                    if(rootType->self != (typeRoot = ospl_utilRootType(type))) {
                        ospl_item typeRootItem = NULL;

                        /* An empty typeRoot means that the type is not stored as inlined object, but conceptually it is.
                         * This will typically occur for intern collectiontypes like c_string, who are defined
                         * in the root. */
                        if(typeRoot) {
                            /* Serialize rootType of type */
                            if(ospl_serializeType(context, NULL, typeRoot, allowCycles, &typeRootItem)) {
                                goto error;
                            }

                            /* Because the 'typeRoot' is the root type of 'type', this can never be an inlined
                             * type, neither can it be a primitive type (which is not serialized). Thus, typeRootItem
                             * must always be set.
                             */
                            corto_assert(typeRootItem != NULL, "see comment.");

                            /* Add the root-type of the type to 'rootType' */
                            ospl_itemAddDependee(typeRootItem, rootType);
                        }
                    }
                }
            }
        }
        break;
    default:
        /* The serializer shouldn't call serializeType with metaKinds other than the ones described above. */
        corto_assert(0, "invalid typekind.");
        break;
    }

    /* Return item if out is set */
    if(out) {
        *out = item;
    }

#ifdef SER_DEBUG
    indent--;
    printf("%*s<< finish\n", indent*3, " ");
#endif

    return 0;
error:
    return -1;
}

#define ospl_XML_BUFFER (512)

/* Append to buffer-list. Parameter 'str' is never larger than ospl_XML_BUFFER bytes. */
static void
ospl_printXmlAppend(
    ospl_context context,
    char* str /* char[ospl_XML_BUFFER] */)
{
    char* lastBuffer;
    corto_uint32 strLen, buffLen, spaceLeft;

    /* Get first buffer from list, create if it didn't exist */
    lastBuffer = corto_llGet(context->xmlbuff, 0);
    if(!lastBuffer) {
        lastBuffer = corto_alloc(ospl_XML_BUFFER + 1);
        *lastBuffer = '\0';
        corto_llInsert(context->xmlbuff, lastBuffer);
    }

    strLen = strlen(str);
    buffLen = strlen(lastBuffer);
    spaceLeft = ospl_XML_BUFFER - buffLen;

    /* If length of string is larger than the space left in the buffer, allocate new buffer. */
    if(strLen >= spaceLeft) {
        char* nextBuffer;
        /* Fill remaining bytes of current buffer. */
        memcpy(lastBuffer + buffLen, str, spaceLeft);
        lastBuffer[ospl_XML_BUFFER] = '\0';

        /* Do the remaining string in the next buffer. */
        nextBuffer = corto_alloc(ospl_XML_BUFFER + 1);
        memcpy(nextBuffer, str + spaceLeft, strLen - spaceLeft + 1 /* 0-terminator */);

        /* Insert new buffer in iterator */
        corto_llInsert(context->xmlbuff, nextBuffer);

    /* ... otherwise, just append the string to the current buffer. */
    }else {
        strcat(lastBuffer, str);
    }
}

static void
ospl_printXmlAggegrate(
    ospl_context context,
    corto_char** result)
{
    corto_iter iter;
    corto_uint32 size;
    corto_char* chunk;

    size = corto_llSize(context->xmlbuff);

    /* Allocate memory for string */
    *result = corto_alloc(size * (ospl_XML_BUFFER) + 1);

    /* Copy buffers in final structure. */
    iter = corto_llIter(context->xmlbuff);
    while(corto_iterHasNext(&iter)) {
        chunk = corto_iterNext(&iter);
#ifdef SER_DEBUG
        printf("   ### %d: %s\n", strlen(chunk), chunk);
#endif
        memcpy((corto_char*)(*result) + ospl_XML_BUFFER * (size-1), chunk, ospl_XML_BUFFER);
        corto_dealloc(chunk);
        size--;
    }

}

/* Print xml */
static void
ospl_printXml(
    ospl_context context,
    char* fmt,
    ...)
{
    va_list args;
    corto_char buffer[ospl_XML_BUFFER]; /* Large buffer on stack, safe since function is never called recursively. */
    int length;

    va_start(args, fmt);
    length = vsnprintf(buffer, ospl_XML_BUFFER, fmt, args); /* C99 function */
    if(length >= ospl_XML_BUFFER) {
        corto_critical("ospl_serializerXMLTypeInfoSerialize: buffer too small!");
    }

    ospl_printXmlAppend(context, buffer);

    va_end(args);
}

/* Print type. */
static void
ospl_printXmlType(
    ospl_context context,
    corto_type current,
    corto_type type);

/* Print typedef */
static void
ospl_printXmlTypedef(
    ospl_context context,
    corto_type type)
{
    ospl_printXml(context, "<TypeDef name=\"%s\">", corto_idof(type));
    ospl_printXmlType(context, corto_type(type), type);
    ospl_printXml(context, "</TypeDef>");
}

/* Print primitive */
static void
ospl_printXmlPrimitive(
    ospl_context context,
    corto_primitive type)
{
    switch(type->kind) {
    case CORTO_BOOLEAN:
        ospl_printXml(context, "<Boolean/>");
        break;
    case CORTO_CHARACTER:
        switch(type->width) {
        case CORTO_WIDTH_8:
            ospl_printXml(context, "<Char/>");
            break;
        case CORTO_WIDTH_16:
            ospl_printXml(context, "<WChar/>");
            break;
        default:
            corto_seterr("ospl: Character width not supported when converting to DDS type.");
            break;
        }
        break;
    case CORTO_BINARY:
        ospl_printXml(context, "<Octet/>");
        break;
    case CORTO_INTEGER:
        switch(type->width) {
        case CORTO_WIDTH_8:
            ospl_printXml(context, "<Char/>");
            break;
        case CORTO_WIDTH_16:
            ospl_printXml(context, "<Short/>");
            break;
        case CORTO_WIDTH_32:
            ospl_printXml(context, "<Long/>");
            break;
        case CORTO_WIDTH_64:
            ospl_printXml(context, "<LongLong/>");
            break;
        default:
            corto_seterr("ospl: Integer width unsupported when converting to DDS type.");
            break;
        }
        break;
    case CORTO_UINTEGER:
        switch(type->width) {
        case CORTO_WIDTH_8:
            ospl_printXml(context, "<Char/>");
            break;
        case CORTO_WIDTH_16:
            ospl_printXml(context, "<UShort/>");
            break;
        case CORTO_WIDTH_32:
            ospl_printXml(context, "<ULong/>");
            break;
        case CORTO_WIDTH_64:
            ospl_printXml(context, "<ULongLong/>");
            break;
        default:
            corto_seterr("ospl: Unsigned integer width unsupported when converting to DDS type.");
            break;
        }
        break;
    case CORTO_FLOAT:
        switch(type->width) {
        case CORTO_WIDTH_32:
            ospl_printXml(context, "<Float/>");
            break;
        case CORTO_WIDTH_64:
            ospl_printXml(context, "<Double/>");
            break;
        default:
            corto_seterr("ospl: Floating-point width unsupported when converting to DDS type.");
            break;
        }
        break;
    case CORTO_BITMASK:
        ospl_printXml(context, "<Long/>");
        break;
    default:
        /* Use this serializer only to serialize userdata */
        {
            corto_critical("Invalid primitive type '%s'.", corto_fullpath(NULL, type));
        }
        break;
    }
}

/* Print enumeration */
static void
ospl_printXmlEnumeration(
    ospl_context context,
    corto_enum type)
{
    corto_uint32 i;
    corto_constant* constant;
    corto_uint32 value;

    ospl_printXml(context, "<Enum name=\"%s\">", corto_idof(type));

    /* Walk constants */
    for(i=0; i<type->constants.length; i++) {
        constant = type->constants.buffer[i];
        value = *constant;
        ospl_printXml(context, "<Element name=\"%s\" value=\"%d\"/>", corto_idof(constant), value);
    }

    ospl_printXml(context, "</Enum>");
}


typedef struct ospl_ser_printMember_t {
    ospl_context context;
    corto_interface type;
} ospl_ser_printMember_t;

corto_int16 ospl_ser_printMember(corto_serializer s, corto_value *info, void *userData) {
    ospl_ser_printMember_t *data = userData;
    corto_member member = info->is.member.t;

    if (member) {
        /* Serialize member and member type */
        ospl_printXml(data->context, "<Member name=\"%s\">", corto_idof(member));
        if (member->type->reference) {
            ospl_printXml(data->context,"<Long/>");
        } else {
            corto_object actualType = ospl_actualMemberType(member);
            ospl_printXmlType(data->context, corto_type(data->type), actualType);
        }
        ospl_printXml(data->context, "</Member>");
    }

    return 0;
}

/* Print structure */
static void
ospl_printXmlStructure(
    ospl_context context,
    corto_interface type,
    corto_bool serializeBase)
{
    /* Serialize members */
    struct corto_serializer_s s;
    corto_serializerInit(&s);
    s.access = CORTO_LOCAL|CORTO_PRIVATE|CORTO_HIDDEN;
    s.accessKind = CORTO_NOT;
    s.optionalAction = CORTO_SERIALIZER_OPTIONAL_ALWAYS;
    s.metaprogram[CORTO_MEMBER] = ospl_ser_printMember;
    ospl_ser_printMember_t walkData = {context, type};

    if(!serializeBase) {
        ospl_printXml(context, "<Struct name=\"%s\">", corto_idof(type));
    }

    corto_metaWalk(&s, corto_type(type), &walkData);

    if(!serializeBase) {
        ospl_printXml(context, "</Struct>");
    }
}

/* Print collection */
static void
ospl_printXmlCollection(
    ospl_context context,
    corto_type current,
    corto_collection type) /* Strings are not handled by this function! */
{
    corto_char* elementName;
    corto_uint32 size = 0;

    elementName = NULL;

    switch(type->kind) {
    case CORTO_LIST:
    case CORTO_MAP:
        ospl_printXml(context, "<Long/>"); /* Placeholder for lists and maps */
        return;
    default:
        break;
    }

    /* Print collection header */
    switch(type->kind) {
    case CORTO_SEQUENCE:
        elementName = "Sequence";
        break;
    case CORTO_ARRAY:
        elementName = "Array";
        break;
    default:
        corto_seterr("ospl: ospl_printXmlCollection: unsupported collectionkind for DDS-serializer.");
        break;
    }

    ospl_printXml(context, "<%s", elementName);

    /* Print collection size, subType and footer. Pass current rootType to
     * ospl_printXmlType so inlined types that are used as subtype are defined within
     * the inline collection. */
    switch(type->kind) {
    case CORTO_SEQUENCE:
    case CORTO_ARRAY:
        size = corto_collection(type)->max;
        break;
    default:
        corto_seterr("ospl: ospl_printXmlCollection: unsupported collectionkind for DDS-serializer(2).");
        break;
    }

    if(size) {
        ospl_printXml(context, " size=\"%d\">", size);
    }else {
        ospl_printXml(context, ">");
    }
    ospl_printXmlType(context, current, type->elementType);
    ospl_printXml(context, "</%s>", elementName);
}

/* Print text */
static void
ospl_printXmlText(
    ospl_context context,
    corto_text type) /* Strings are not handled by this function! */
{
    corto_char* elementName;
    corto_uint32 size;

    elementName = "String";

    ospl_printXml(context, "<%s", elementName);
    size = type->length;

    if(size) {
        ospl_printXml(context, " length=\"%d\"/>", size);
    }else {
        ospl_printXml(context, "/>");
    }
}

#define ospl_MAX_SCOPE_DEPTH (64) /* Should be on the safe side */

/* Function builds a scope-stack from root to module */
static void
ospl_utilModuleStack(
    corto_object module,
    corto_object* stack /* corto_object[ospl_MAX_SCOPE_DEPTH] */)
{
    corto_uint32 count;
    corto_object ptr;

    corto_assert(module != NULL, "NULL passed for module to ospl_utilModuleStack");

    /* Count scope depth */
    ptr = module;
    count = 1; /* For self */
    while((ptr = corto_parentof(ptr))) {
        count++;
    }

    if(count > ospl_MAX_SCOPE_DEPTH) {
        corto_seterr("ospl: ospl_printXmlCollection: unsupported scope-depth (depth=%d, max=%d).", count, ospl_MAX_SCOPE_DEPTH);
    }
    corto_assert(count <= ospl_MAX_SCOPE_DEPTH, "MAX_SCOPE_DEPTH overflow.");

    /* Fill module stack */
    ptr = module;
    while(count) {
        stack[count-1] = ptr;
        ptr = corto_parentof(ptr);
        count--;
    }

    /* ptr should be NULL */
    corto_assert(!ptr, "ptr is NULL.");
}

/* Find first common module in two module-stacks */
static corto_object
ospl_utilFirstCommonModule(
    corto_object from,
    corto_object to,
    corto_object* fromStack,
    corto_object* toStack,
    corto_uint32* i_out)
{
    corto_object fromPtr, toPtr;
    corto_uint32 i;

    /* fromPtr and toPtr will initially point to base */
    i = 0;
    do {
        fromPtr = fromStack[i];
        toPtr = toStack[i];
        i++;
    }while((fromPtr != from) && (toPtr != to) && (fromStack[i] == toStack[i]));

    /* Common module is now stored in fromPtr and toPtr. */

    if(i_out) {
        *i_out = i;
    }

    return fromPtr;
}

/* Print typeref */
static void
ospl_printXmlTyperef(
    ospl_context context,
    corto_type type)
{
    /* Print typeref. Use relative names (if possible) to obtain the shortest possible reference to another type. */
    if(corto_parentof(type) != context->module) {
        corto_object fromStack[ospl_MAX_SCOPE_DEPTH], toStack[ospl_MAX_SCOPE_DEPTH];
        corto_object from, to, common;
        corto_uint32 i;

        /* Get first common module between current module and the referenced type. */
        from = context->module;
        to = corto_parentof(type);
        ospl_utilModuleStack(from, fromStack);
        ospl_utilModuleStack(to, toStack);
        ospl_utilFirstCommonModule(from, to, fromStack, toStack, &i);

        ospl_printXml(context, "<Type name=\"");

        /* Print modules from the common module until the current */
        i--;
        do {
            common = toStack[i];
            i++;
            if(corto_idof(common)) {
                ospl_printXml(context, "%s", corto_idof(common));
            }
            ospl_printXml(context, "::");
        }while(common != to);

        /* Print the typename */
        ospl_printXml(context, "%s\"/>", corto_idof(type));
    }else {
        /* If module of type is equal to the current, just print the typename. */
        ospl_printXml(context, "<Type name=\"%s\"/>", corto_idof(type));
    }
}

/* Open module. This function finds the shortest path from the current module to the next,
 * and opens and closes modules where necessary. */
static void
ospl_printXmlModuleOpen(
    ospl_context context,
    corto_object to)
{
    corto_object from;

    /* If context->module is NULL, start from root */
    from = context->module;
    if(!from) {
        from = root_o;
    }

    /* If from and to are not equal, find shortest path between modules. */
    if(from != to) {
        corto_object fromStack[ospl_MAX_SCOPE_DEPTH], toStack[ospl_MAX_SCOPE_DEPTH];
        corto_object fromPtr, toPtr;
        corto_uint32 i;

        /* Find common module. First build up a scope-stack for the two modules which
         * are ordered base -> <module>. Then walk through these stacks to find the
         * last common module. */
        ospl_utilModuleStack(from, fromStack);
        ospl_utilModuleStack(to, toStack);
        fromPtr = toPtr = ospl_utilFirstCommonModule(from, to, fromStack, toStack, &i);

        /* Walk down from module 'from' to 'toPtr' */
        fromPtr = from;
        while(fromPtr != toPtr) {
            ospl_printXml(context, "</Module>");
            fromPtr = corto_parentof(fromPtr);
        }

        /* Walk from toPtr to 'to' */
        while(toPtr != to) {
            toPtr = toStack[i];
            ospl_printXml(context, "<Module name=\"%s\">", corto_idof(toPtr));
            i++;
        }

        /* Update context->module */
        context->module = to;
    }
}

/* Close module */
static void ospl_printXmlModuleClose(
    ospl_context context)
{
    corto_object ptr;

    if(context->module) {
        ptr = context->module;
        while((ptr = corto_parentof(ptr))) {
            ospl_printXml(context, "</Module>");
        }

        context->module = NULL;
    }
}

/* Print type. */
static void
ospl_printXmlType(
    ospl_context context,
    corto_type current,
    corto_type type)
{
    if(type == corto_type(ospl_Time_t_o)) {
        ospl_printXml(context, "<Time/>");
    }else {
        corto_bool parseTypeRef = TRUE;

        /* If object is defined outside the current scope and is not a collection or primitive, serialize a typeref.
         * Also, if the type is already defined serialize a typeref. This prevents inline types to be defined multiple
         * times if they are used multiple times within a module-scoped type. */
        if(corto_type(type) == type) {
            switch(type->kind) {
            case CORTO_PRIMITIVE:
            case CORTO_COLLECTION:
                parseTypeRef = FALSE;
                break;
            default:
                if(corto_parentof(type) == current) {
                    if(!context->inlineProcessed || !corto_llHasObject(context->inlineProcessed, type)) {
                        parseTypeRef = FALSE;
                    }
                }
                break;
            }
        }else {
            if(corto_parentof(type) == current) {
                if(!context->inlineProcessed || !corto_llHasObject(context->inlineProcessed, type)) {
                    parseTypeRef = FALSE;
                }
            }
        }

        if(parseTypeRef) {
            ospl_printXmlTyperef(context, type);
        }else {
            /* Serialize type-definition. */
            if(corto_type(type) != type) {
                ospl_printXmlTypedef(context, type);
            }else {
                switch(type->kind) {
                case CORTO_PRIMITIVE:
                    switch(corto_primitive(type)->kind) {
                    case CORTO_ENUM:
                        ospl_printXmlEnumeration(context, corto_enum(type));
                        break;
                    case CORTO_TEXT:
                        ospl_printXmlText(context, corto_text(type));
                        break;
                    default:
                        ospl_printXmlPrimitive(context, corto_primitive(type));
                        break;
                    }
                    break;
                case CORTO_COLLECTION:
                    ospl_printXmlCollection(context, current, corto_collection(type));
                    break;
                case CORTO_COMPOSITE:
                    /* Make an exception for c_time. */
                    ospl_printXmlStructure(context, corto_interface(type), FALSE);
                    break;
                default:
                    /* This may not happen. Types other than the ones listed
                     * above cannot be printed. */
                    corto_critical("Invalid typekind for ospl_printXmlType.");
                    break;
                }
            }

            /* Mark type as processed. */
            if(!context->inlineProcessed) {
                context->inlineProcessed = corto_llNew();
            }
            corto_llInsert(context->inlineProcessed, type);
        }
    }
}

/* Top-level printroutine. */
static void
ospl_printXmlItem(
    ospl_item item,
    ospl_context context)
{
    if(context->inlineProcessed) {
        corto_llFree(context->inlineProcessed);
        context->inlineProcessed = NULL;
    }

#ifdef SER_DEBUG
    {
        printf("%s\n", corto_fullpath(NULL, item->self));
    }
#endif

    /*if(item->self != corto_type(item->self)) {

    } else */{
        switch(item->self->kind) {
        case CORTO_COMPOSITE:
        case CORTO_PRIMITIVE:
            ospl_printXmlModuleOpen(context, corto_parentof(item->self));
            ospl_printXmlType(context, corto_parentof(item->self), item->self);
            break;
        default:
            /* Typedefs are not yet supported */

            /*if(item->self->type != corto_type(item->self->real)) {
                ospl_printXmlModuleOpen(context, corto_parentof(item->self));
                ospl_printXmlType(context, corto_type(corto_parentof(item->self)), item->self);
            }else {
                 * This may not happen. Types other than the ones listed
                 * above cannot be directly printed.
                corto_critical("ospl_printXmlItem: invalid typeKind for serializer.");
            }*/
            break;
        }
    }

    /* Dereference dependees of item. This will populate module-objects with types
     * that reach refcount 0. */
    ospl_itemDerefDependees(item);
}

static int
ospl_addInitialItems(
    void* o,
    void* udata)
{
    ospl_item item;
    CORTO_UNUSED(udata);

    item = o;

    /* If refcount of item is zero, it means that it has no unresolved dependencies, thus that it can
     * be processed. */
    if(!item->refcount){
        ospl_itemAddToModule(item);
    }

    return 1;
}

struct ospl_findLargestModule_t {
    ospl_moduleItem largest;
};

static int
ospl_findLargestModule(
    void* o,
    void* userData)
{
    ospl_moduleItem module, largest;
    struct ospl_findLargestModule_t* data;

    module = o;
    data = userData;
    largest = data->largest;

    /* Find largest module. */
    if(!largest) {
        /* Only set largest if module has types. */
        if(module->end - module->start) {
            data->largest = module;
        }
    }else {
        if((largest->end - largest->start) < (module->end - module->start)) {
            data->largest = module;
        }
    }

    return 1;
}

/* Output types in optimized module order. The algorithm attempts to reduce the number
 * of module transitions by selecting the module with the most types in each iteration.
 * The algorithm stops when all modules are empty.
 */
static void
ospl_printModules(
    ospl_context context)
{
    struct ospl_findLargestModule_t walkData;
    ospl_moduleItem module;

    /* There must be at least one type, and as such at least one module. */
    corto_assert(context->modules != NULL, "context->modules is NULL.");

    /* Find initial largest module */
    walkData.largest = NULL;
    corto_llWalk(context->modules, ospl_findLargestModule, &walkData);

    /* There must be at least one module with types. */
    corto_assert(walkData.largest != NULL, "No modules with types?");

    do {
        module = walkData.largest;

        /* Walk types */
        while(module->start != module->end) {
            /* If the printing of an item causes other types to be 'unlocked' (refcount becomes 0) in the current module,
             * the type will be added to the 'module' object, causing the module->end pointer to shift. This
             * automatically causes these types to be processed within the current iteration.
             */
            ospl_printXmlItem(*(module->start), context);
            module->start++;
        }

        /* Lookup next largest module */
        walkData.largest = NULL;
        corto_llWalk(context->modules, ospl_findLargestModule, &walkData);
    }while(walkData.largest);
}

#ifdef SER_DEBUG
static int
ospl_printTypes(
    void* o,
    void* udata)
{
    CORTO_UNUSED(o);
    CORTO_UNUSED(udata);

    printf("%s\n", corto_fullpath(NULL, ((ospl_item)o)->self));

    return 1;
}
#endif

static void
ospl_printXmlDescriptor(
    ospl_context context,
    corto_char** result)
{
#ifdef SER_DEBUG
    printf("=== Dependency ordered: (%d types).\n", corto_llSize(context->items));
    corto_llWalk(context->items, ospl_printTypes, NULL);
#endif

    /* Set initial module to base */
    context->module = root_o;

    /* Insert initial types with refcount 0 in module objects */
    corto_llWalk(context->items, ospl_addInitialItems, NULL);

    /* Print xml */
    ospl_printXml(context, "<MetaData version=\"1.0.0\">");

    /* Print types, ordered by modules. */
#ifdef SER_DEBUG
    printf("=== Module ordered output:\n");
#endif
    ospl_printModules(context);

    /* Clean inlineProcessed. */
    if(context->inlineProcessed) {
        corto_llFree(context->inlineProcessed);
        context->inlineProcessed = NULL;
    }

    /* Close last used module */
    ospl_printXmlModuleClose(context);
    ospl_printXml(context, "</MetaData>");

    /* Aggegrate xml-buffers */
#ifdef SER_DEBUG
    printf("\n=== XML buffers:\n");
#endif
    ospl_printXmlAggegrate(context, result);

    /* Free xmlbuff list */
    corto_llFree(context->xmlbuff);
    context->xmlbuff = NULL;

#ifdef SER_DEBUG
    printf("\n=== XML MetaDescriptor:\n");
    printf("'%s'\n", *result);
#endif
}

corto_string
ospl_metaXmlGet(
    corto_type type)
{
    ospl_context context;
    corto_char* result;
#ifdef SER_DEBUG
    corto_time t, start, stop;

    corto_timeGet(&start);
    printf("=== Start serializing..\n");
#endif

    context = ospl_contextNew();
    result = NULL;

    /* Serialize type */
    if(ospl_serializeType(context, NULL, type, FALSE, NULL)) {
        goto error;
    }

    /* Print XML */
    ospl_printXmlDescriptor(context, &result);

    /* Free context */
    ospl_contextFree(context);

#ifdef SER_DEBUG
    corto_timeGet(&stop);
    t = corto_timeSub(stop, start);
    printf("=== Serializing finished in %d.%09d seconds.\n", t.tv_sec, t.tv_nsec);
#endif

    return result;
error:
    return NULL;
}
