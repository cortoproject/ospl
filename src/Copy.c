/*
 * DDS__dataserializer.c
 *
 *  Created on: Feb 22, 2013
 *      Author: sander
 */

#include "ospl/ospl.h"

/* Copy program structures */
typedef struct ospl_copyElement {
    void* program;
    corto_type srcType;
    corto_type dstType;
    corto_uint32 ddsSize;
} ospl_copyElement;

struct ospl_copyProgram {
    ospl_copyElement base;
    corto_uint32 key_offsets[OSPL_MAX_KEYS];
    corto_type key_types[OSPL_MAX_KEYS];
    corto_uint32 nKeys;
};

typedef enum ospl_opKind {
    OSPL_OP_COPY,
    OSPL_OP_STRING,
    OSPL_OP_REFERENCE, /* References are mapped to strings */
    OSPL_OP_OPTIONAL, /* Optional is mapped on a sequence */
    OSPL_OP_SEQUENCE,
    OSPL_OP_LIST,
    OSPL_OP_UNION,
    OSPL_OP_STOP
} ospl_opKind;

typedef struct ospl_op {
    ospl_opKind kind;
    corto_uint32 from;
    corto_uint32 to;
} ospl_op;

typedef struct ospl_opCopy {
    ospl_op op;
    corto_uint32 size;
} ospl_opCopy;

typedef struct ospl_opSequence {
    ospl_op op;
    ospl_copyElement program;
} ospl_opSequence;

typedef struct ospl_opOptional {
    ospl_op op;
    ospl_copyElement program;
} ospl_opOptional;

typedef struct ospl_copyUnionCase {
    corto_int32seq discriminator;
    ospl_copyElement program;
} ospl_copyUnionCase;
typedef struct ospl_opUnion {
    ospl_op op;
    ospl_copyUnionCase *cases;
    corto_uint32 caseCount;
    corto_uint8 offset; /* Value starts here */
} ospl_opUnion;

typedef struct ospl_opList {
    ospl_op op;
    ospl_copyElement program;
} ospl_opList;

typedef struct ospl_opString {
    ospl_op op;
} ospl_opString;

typedef struct ospl_opReference {
    ospl_op op;
} ospl_opReference;

typedef struct ospl_serdata ospl_serdata;
struct ospl_serdata {
    corto_ll program;
    corto_type srcType;
    corto_type dstType;
    corto_uint32 src_base_offset;
    corto_uint32 dst_base_offset;
    corto_uint32 src_offset;
    corto_uint32 dst_offset;
    corto_ll compiled; /* Types for which a program is already created -
                       either because it's a root-type or because
                       the type is used in a sequence or list. */

    /* Copy from list to array */
    corto_uint32 size; /* Size of final instruction sequence */
    void* result; /* Actual resulting instruction sequence */
};

ospl_copyElement ospl_copyProgramCompile(corto_type srcType, corto_type dstType);

/* Find compiled type (only root-type and types used in sequences appear in this list */
ospl_copyElement *ospl_findType(ospl_serdata *data, corto_type srcType, corto_type dstType) {
    corto_iter iter;
    ospl_copyElement *found = NULL;

    iter = corto_llIter(data->compiled);
    while(corto_iterHasNext(&iter)) {
        found = corto_iterNext(&iter);
        if ((found->dstType == dstType) && (found->srcType == srcType)) {
            break;
        } else {
            found = NULL;
        }
    }

    return NULL;
}

ospl_copyElement* ospl_getProgram(ospl_serdata *data, corto_type srcType, corto_type dstType) {
    ospl_copyElement *found = NULL;

    if (!(found = ospl_findType(data, srcType, dstType))) {
        ospl_copyElement e = ospl_copyProgramCompile(srcType, dstType);
        if (e.program) {
            found = corto_alloc(sizeof(ospl_copyElement));
            *found = e;
        } else {
            corto_seterr("failed to create a program for (dst='%s', src='%s')",
              corto_fullpath(NULL, dstType),
              corto_fullpath(NULL, srcType));
            goto error;
        }
    }

    return found;
error:
    return NULL;
}

ospl_serdata* ospl_serdataNew(corto_type srcType, corto_type dstType, corto_ll compiled) {
    ospl_serdata *data;

    data = corto_alloc(sizeof(ospl_serdata));
    data->srcType = srcType;
    data->dstType = dstType;
    data->src_base_offset = 0;
    data->dst_base_offset = 0;
    data->src_offset = 0;
    data->dst_offset = 0;
    data->program = corto_llNew();
    data->result = NULL;
    data->size = 0;
    if (compiled) {
        data->compiled = compiled;
    }else {
        data->compiled = corto_llNew();
    }
    corto_llInsert(data->compiled, data);
    return data;
}

void ospl_serdataFree(ospl_serdata *data) {
    if (data->program) {
        corto_iter iter = corto_llIter(data->program);
        while (corto_iterHasNext(&iter)) {
            corto_dealloc(corto_iterNext(&iter));
        }
        corto_llFree(data->program);
    }
    corto_dealloc(data);
}

/* Insert copy-instruction */
void ospl_copyInsert(corto_ll program, corto_int32 from, corto_int32 to, corto_int32 size) {
    ospl_opCopy *instr;

    corto_assert(from >= 0, "");
    corto_assert(to >= 0, "");
    corto_assert(size > 0, "");

    instr = corto_alloc(sizeof(ospl_opCopy));
    instr->op.kind = OSPL_OP_COPY;
    instr->op.from = from;
    instr->op.to = to;
    instr->size = size;
    corto_llAppend(program, instr);
}

/* Insert copy-operation for strings */
void ospl_stringInsert(corto_ll program, corto_uint32 from, corto_uint32 to) {
    ospl_opString *instr;
    instr = corto_alloc(sizeof(ospl_opString));
    instr->op.kind = OSPL_OP_STRING;
    instr->op.from = from;
    instr->op.to = to;
    corto_llAppend(program, instr);
}

/* Insert copy-operation for references */
void ospl_referenceInsert(corto_ll program, corto_uint32 from, corto_uint32 to) {
    ospl_opReference *instr;
    instr = corto_alloc(sizeof(ospl_opReference));
    instr->op.kind = OSPL_OP_REFERENCE;
    instr->op.from = from;
    instr->op.to = to;
    corto_llAppend(program, instr);
}

/* Insert optional-operation for sequences */
void ospl_optionalInsert(corto_ll program, corto_uint32 from, corto_uint32 to, ospl_copyElement *elem) {
    ospl_opOptional *instr;
    instr = corto_alloc(sizeof(ospl_opOptional));
    instr->op.kind = OSPL_OP_OPTIONAL;
    instr->op.from = from;
    instr->op.to = to;
    instr->program = *elem;
    corto_llAppend(program, instr);
}

/* Insert sequence-operation for sequences */
void ospl_sequenceInsert(corto_ll program, corto_uint32 from, corto_uint32 to, ospl_copyElement *elem) {
    ospl_opSequence *instr;
    instr = corto_alloc(sizeof(ospl_opSequence));
    instr->op.kind = OSPL_OP_SEQUENCE;
    instr->op.from = from;
    instr->op.to = to;
    instr->program = *elem;
    corto_llAppend(program, instr);
}

/* Insert list-operation for sequences */
void ospl_listInsert(corto_ll program, corto_uint32 from, corto_uint32 to, ospl_copyElement *elem) {
    ospl_opList *instr;
    instr = corto_alloc(sizeof(ospl_opList));
    instr->op.kind = OSPL_OP_LIST;
    instr->op.from = from;
    instr->op.to = to;
    instr->program = *elem;
    corto_llAppend(program, instr);
}

/* Insert union-operation */
void ospl_unionInsert(
    corto_ll program,
    corto_uint32 from,
    corto_uint32 to,
    ospl_copyUnionCase *cases,
    corto_uint32 caseCount,
    corto_uint8 offset)
{
    ospl_opUnion *instr;
    instr = corto_alloc(sizeof(ospl_opUnion));
    instr->op.kind = OSPL_OP_UNION;
    instr->op.from = from;
    instr->op.to = to;
    instr->cases = cases;
    instr->caseCount = caseCount;
    instr->offset = offset;
    corto_llAppend(program, instr);
}

/* Insert stop-operation */
void ospl_stopInsert(corto_ll program) {
    ospl_op *instr;
    instr = corto_alloc(sizeof(ospl_op));
    instr->kind = OSPL_OP_STOP;
    corto_llAppend(program, instr);
}

/* Clone instruction */
void ospl_instrClone(corto_ll program, ospl_op *op) {
    switch(op->kind) {
    case OSPL_OP_COPY:
        ospl_copyInsert(program, op->from, op->to, ((ospl_opCopy*)op)->size);
        break;
    case OSPL_OP_REFERENCE:
        ospl_referenceInsert(program, op->from, op->to);
        break;
    case OSPL_OP_STRING:
        ospl_stringInsert(program, op->from, op->to);
        break;
    case OSPL_OP_OPTIONAL:
        ospl_optionalInsert(program, op->from, op->to, &((ospl_opOptional*)op)->program);
        break;
    case OSPL_OP_SEQUENCE:
        ospl_sequenceInsert(program, op->from, op->to, &((ospl_opSequence*)op)->program);
        break;
    case OSPL_OP_LIST:
        ospl_listInsert(program, op->from, op->to, &((ospl_opList*)op)->program);
        break;
    case OSPL_OP_UNION:
        ospl_unionInsert(
          program,
          op->from,
          op->to,
          ((ospl_opUnion*)op)->cases,
          ((ospl_opUnion*)op)->caseCount,
          ((ospl_opUnion*)op)->offset
        );
        break;
    case OSPL_OP_STOP:
        ospl_stopInsert(program);
        break;
    }
}

corto_uint32 ospl_getDdsOffset(corto_type type, corto_string memberName, corto_member *m);
corto_uint8 ospl_getDdsAlignment(corto_type type);

corto_int16 ospl_ddsAlignment_primitive(
    corto_serializer s,
    corto_value *info,
    void *userData)
{
    corto_uint8 *alignment = userData;
    *alignment = corto_value_getType(info)->alignment;
    return 0;
}

corto_int16 ospl_ddsAlignment_collection(
    corto_serializer s,
    corto_value *info,
    void *userData)
{
    corto_uint8 *alignment = userData;
    corto_collection t = corto_collection(corto_value_getType(info));
    if (t->kind != CORTO_ARRAY) {
        *alignment = CORTO_ALIGNMENT(DDS_SampleInfoSeq);
    } else {
        *alignment = ospl_getDdsAlignment(t->elementType);
    }
    return 0;
}

corto_int16 ospl_ddsAlignment_item(
    corto_serializer s,
    corto_value *info,
    void *data)
{
    corto_uint8 *alignment = data;
    corto_type t = corto_value_getType(info);
    corto_member m = info->is.member.t;

    /* Alignment is the same as corto types, except for sequences */
    if ((m->modifiers & CORTO_OPTIONAL) || ((t->kind == CORTO_COLLECTION) && (corto_collection(t)->kind != CORTO_ARRAY))) {
        if (CORTO_ALIGNMENT(DDS_SampleInfoSeq) > *alignment) {
            *alignment = CORTO_ALIGNMENT(DDS_SampleInfoSeq);
        }
    } else {
        if (t->alignment > *alignment) {
            *alignment = t->alignment;
        }
    }

    return 0;
}

struct corto_serializer_s ospl_ddsAlignmentSerializer(void) {
    struct corto_serializer_s s;
    corto_serializerInit(&s);
    s.access = CORTO_LOCAL;
    s.accessKind = CORTO_NOT;
    s.traceKind = CORTO_SERIALIZER_TRACE_ON_FAIL;
    s.optionalAction = CORTO_SERIALIZER_OPTIONAL_ALWAYS;
    s.program[CORTO_PRIMITIVE] = ospl_ddsAlignment_primitive;
    s.program[CORTO_COLLECTION] = ospl_ddsAlignment_collection;
    s.metaprogram[CORTO_MEMBER] = ospl_ddsAlignment_item;
    return s;
}

typedef struct ddsOffset_t {
    corto_string memberName;

    /* Alignment of DDS type */
    corto_uint8 alignment;

    /* Offset of member when found */
    corto_int32 result;
    corto_member m;

    /* Counting offset */
    corto_int32 offset;

    /* Keep track of maximum member size (for unions) */
    corto_int32 maxSize;

    /* Is the type a union */
    corto_bool isUnion;
} ddsOffset_t;

corto_int16 ospl_ddsOffset_primitive(
    corto_serializer s,
    corto_value *info,
    void *userData)
{
    ddsOffset_t *data = userData;
    data->offset += corto_value_getType(info)->size;
    return 0;
}

corto_int16 ospl_ddsOffset_collection(
    corto_serializer s,
    corto_value *info,
    void *userData)
{
    ddsOffset_t *data = userData;
    corto_collection t = corto_collection(corto_value_getType(info));
    if (t->kind != CORTO_ARRAY) {
        data->offset = sizeof(DDS_SampleInfoSeq);
    } else {
        corto_uint32 size = ospl_getDdsOffset(t->elementType, NULL, NULL);
        data->offset = size * t->max;
    }
    return 0;
}

corto_int16 ospl_ddsOffset_item(
    corto_serializer s,
    corto_value *info,
    void *userData)
{
    ddsOffset_t *data = userData;
    corto_type t = corto_value_getType(info);
    corto_member m = info->is.member.t;
    corto_uint32 size;
    corto_uint8 alignment;

    /* Optional members are mapped to sequences */
    if (m->modifiers & CORTO_OPTIONAL) {
        size = sizeof(DDS_SampleInfoSeq);
        alignment = CORTO_ALIGNMENT(DDS_SampleInfoSeq);
    } else {
        size = ospl_getDdsOffset(t, NULL, NULL);
        alignment = ospl_getDdsAlignment(t);
    }

    if (size > data->maxSize) {
        data->maxSize = size;
    }

    /* Compute current offset */
    if (data->offset) {
        data->offset = CORTO_ALIGN(data->offset, alignment);
    }

    /* Check if membernames match */
    if (data->memberName && !strcmp(corto_idof(info->is.member.t), data->memberName)) {
        data->result = data->offset;
        data->m = m;
        return 1; /* Stop serializing */
    } else {
        if (data->isUnion) {
            /* In unions, all members start after the discriminator */
            data->offset = sizeof(corto_uint32);
        } else {
            data->offset += size;
        }
    }

    return 0;
}

struct corto_serializer_s ospl_ddsOffsetSerializer(void) {
    struct corto_serializer_s s;
    corto_serializerInit(&s);
    s.access = CORTO_LOCAL;
    s.accessKind = CORTO_NOT;
    s.traceKind = CORTO_SERIALIZER_TRACE_ON_FAIL;
    s.optionalAction = CORTO_SERIALIZER_OPTIONAL_ALWAYS;
    s.program[CORTO_PRIMITIVE] = ospl_ddsOffset_primitive;
    s.program[CORTO_COLLECTION] = ospl_ddsOffset_collection;
    s.metaprogram[CORTO_MEMBER] = ospl_ddsOffset_item;
    return s;
}

corto_uint8 ospl_getDdsAlignment(corto_type type) {
    struct corto_serializer_s s = ospl_ddsAlignmentSerializer();
    corto_uint8 alignment = 0;
    corto_metaWalk(&s, type, &alignment);
    return alignment;
}

corto_uint32 ospl_getDdsOffset(corto_type type, corto_string memberName, corto_member *m) {
    struct corto_serializer_s s = ospl_ddsOffsetSerializer();
    ddsOffset_t walkData;

    walkData.memberName = memberName;
    walkData.alignment = ospl_getDdsAlignment(type);
    walkData.offset = 0;
    walkData.result = 0;
    walkData.maxSize = 0;
    walkData.m = NULL;
    walkData.isUnion = (type->kind == CORTO_COMPOSITE) &&
                       (corto_interface(type)->kind == CORTO_UNION);

    if (!corto_metaWalk(&s, type, &walkData) && memberName) {
        corto_seterr("member '%s' not found in DDS type '%s'",
            memberName,
            corto_fullpath(NULL, type));
        goto error;
    }

    if (!memberName) {
        if (walkData.isUnion) {
            walkData.result = walkData.maxSize + sizeof(corto_int32);
            walkData.result = CORTO_ALIGN(walkData.result, walkData.alignment);
        } else {
            walkData.result = CORTO_ALIGN(walkData.offset, walkData.alignment);
        }
    }

    if (m) {
        *m = walkData.m;
    }

    return walkData.result;
error:
    return -1;
}

/* Primitive values */
corto_int16 DDS__ospl_primitive(corto_serializer s, corto_value* v, void* userData) {
    corto_type t = corto_value_getType(v);
    ospl_serdata *data = userData;

    switch(corto_primitive(t)->kind) {
    case CORTO_TEXT:
        ospl_stringInsert(
            data->program,
            data->src_base_offset + data->src_offset,
            data->dst_base_offset + data->dst_offset);
        break;
    default:
        ospl_copyInsert(
            data->program,
            data->src_base_offset + data->src_offset,
            data->dst_base_offset + data->dst_offset,
            corto_type_sizeof(t));
        break;
    }
    return 0;
}

/* Reference values */
corto_int16 DDS__ospl_reference(corto_serializer s, corto_value* v, void* userData) {
    ospl_serdata *data = userData;
    ospl_referenceInsert(
        data->program,
        data->src_base_offset + data->src_offset,
        data->dst_base_offset + data->dst_offset);
    return 0;
}

/* Composite value */
corto_int16 DDS__ospl_composite(corto_serializer s, corto_value* v, void *userData) {
    corto_interface t = corto_interface(corto_value_getType(v));
    corto_int16 result = 0;

    if (v->kind == CORTO_MEMBER) {
        ospl_serdata *data = userData;
        ospl_serdata nested = *data;
        corto_member member = v->is.member.t;
        corto_member srcMember = NULL;

        /* Lookup member in type from type */
        ospl_getDdsOffset(data->srcType, corto_idof(member), &srcMember);

        nested.dst_base_offset = data->dst_base_offset + data->dst_offset;
        nested.src_base_offset = data->src_base_offset + data->src_offset;
        nested.dst_offset = 0;
        nested.src_offset = 0;
        nested.srcType = srcMember->type;
        nested.dstType = corto_type(t);

        /* Serialize nested type */
        if (t->kind != CORTO_UNION) {
            result = corto_serializeMembers(s, v, &nested);
        } else {
            /* Serialize union */
            ospl_copyUnionCase *cases = corto_alloc(t->members.length * sizeof(ospl_copyUnionCase));
            corto_int32 i; for (i = 0; i < t->members.length; i++) {
                corto_case c = corto_case(t->members.buffer[i]);
                corto_type t = corto_member(c)->type;

                /* Find or create a program for converting srcType => dstType */
                ospl_copyElement *found = ospl_getProgram(data, t, t);
                if (!found) {
                    goto error;
                }
                cases[i].discriminator = c->discriminator;
                cases[i].program = *found;
            }
            ospl_unionInsert(
                data->program,
                nested.src_base_offset,
                nested.dst_base_offset,
                cases,
                t->members.length,
                /* There are no cases where alignment between corto types and
                 * DDS types is different */
                corto_type(t)->alignment);
        }
    } else {
        result = corto_serializeMembers(s, v, userData);
    }

    return result;
error:
    return -1;
}

/* Collection value */
corto_int16 DDS__ospl_collection(corto_serializer s, corto_value* v, void *userData) {
    corto_type t = corto_value_getType(v);
    corto_int16 result = 0;
    ospl_serdata *data = userData;

    switch(corto_collection(t)->kind) {
    case CORTO_ARRAY:
        /* Serialize elements of array */
        result = corto_serializeElements(s, v, userData);
        break;
    case CORTO_SEQUENCE:
    case CORTO_LIST: {
        corto_type dstType = corto_collection(t)->elementType;
        corto_type srcType = NULL;

        /* Lookup member in type */
        if (v->kind == CORTO_MEMBER) {
            corto_member member = v->is.member.t;
            corto_member srcMember = NULL;
            ospl_getDdsOffset(data->srcType, corto_idof(member), &srcMember);
            srcType = corto_collection(srcMember->type)->elementType;
        } else {
            srcType = dstType;
        }

        /* Find or create a program for converting srcType => dstType */
        ospl_copyElement *found = ospl_getProgram(data, srcType, dstType);
        if (!found) {
            goto error;
        }

        /* Insert sequence instruction */
        if (corto_collection(t)->kind == CORTO_SEQUENCE) {
            ospl_sequenceInsert(
                data->program,
                data->src_base_offset + data->src_offset,
                data->dst_base_offset + data->dst_offset,
                found);
        }else if (corto_collection(t)->kind == CORTO_LIST) {
            ospl_listInsert(
                data->program,
                data->src_base_offset + data->src_offset,
                data->dst_base_offset + data->dst_offset,
                found);
        }
        break;
    }
        break;
    case CORTO_MAP:
        corto_error("DDS__dataserializer: maps are not yet supported.");
        goto error;
        break;
    }

    return result;
error:
    return -1;
}

corto_int16 DDS__ospl_item(corto_serializer s, corto_value *info, void *userData) {
    ospl_serdata *data = userData;
    corto_member m = info->is.member.t;
    data->dst_offset = m->offset;
    data->src_offset = ospl_getDdsOffset(data->srcType, corto_idof(m), NULL);

    if (((corto_int32)data->src_offset < 0)) {
        corto_error("cannot find offset for member '%s' in '%s'",
          corto_idof(m),
          corto_fullpath(NULL, data->srcType));
        goto error;
    }

    /* Optional members have their own instruction */
    if (m->modifiers & CORTO_OPTIONAL) {
        ospl_copyElement *found;

        if (!(found = ospl_findType(data, m->type, m->type))) {
            ospl_copyElement e = ospl_copyProgramCompile(m->type, m->type);
            if (e.program) {
                found = corto_alloc(sizeof(ospl_copyElement));
                *found = e;
            } else {
                corto_seterr("failed to create a program for optional member '%s' (type='%s')",
                  corto_fullpath(NULL, m),
                  corto_fullpath(NULL, m->type));
                goto error;
            }
        }
        ospl_optionalInsert(
            data->program,
            data->src_base_offset + data->src_offset,
            data->dst_base_offset + data->dst_offset,
            found);
    } else {
        corto_serializeValue(s, info, userData);
    }
    return 0;
error:
    return -1;
}

struct corto_serializer_s ospl_copyProgramSerializer(void) {
    struct corto_serializer_s s;
    corto_serializerInit(&s);
    s.access = CORTO_LOCAL;
    s.accessKind = CORTO_NOT;
    s.traceKind = CORTO_SERIALIZER_TRACE_ON_FAIL;
    s.optionalAction = CORTO_SERIALIZER_OPTIONAL_ALWAYS;
    s.program[CORTO_PRIMITIVE] = DDS__ospl_primitive;
    s.program[CORTO_COLLECTION] = DDS__ospl_collection;
    s.program[CORTO_COMPOSITE] = DDS__ospl_composite;
    s.metaprogram[CORTO_MEMBER] = DDS__ospl_item;
    s.reference = DDS__ospl_reference;
    return s;
}

/* Print program */
void DDS__osplPrint(void *instr) {
    static corto_uint32 indent = 0;

    do {
        /* Print indentation */
        printf("%*s", indent * 3, "");

        switch(*(ospl_opKind*)instr) {
        case OSPL_OP_COPY: {
            ospl_opCopy *op = instr;
            printf("COPY (from)%d (to)%d (size)%d\n", op->op.from, op->op.to, op->size);
            instr = CORTO_OFFSET(instr, sizeof(ospl_opCopy));
            break;
        }
        case OSPL_OP_STRING: {
            ospl_opString *op = instr;
            printf("STRING (from)%d (to)%d\n", op->op.from, op->op.to);
            instr = CORTO_OFFSET(instr, sizeof(ospl_opString));
            break;
        }
        case OSPL_OP_REFERENCE: {
            ospl_opReference *op = instr;
            printf("REFERENCE (from)%d (to)%d\n", op->op.from, op->op.to);
            instr = CORTO_OFFSET(instr, sizeof(ospl_opReference));
            break;
        }
        case OSPL_OP_UNION: {
            ospl_opUnion *op = instr;
            printf("UNION (from)%d (to)%d\n", op->op.from, op->op.to);
            corto_int32 i; for (i = 0; i < op->caseCount; i ++) {
                ospl_copyUnionCase *c = &op->cases[i];
                corto_int32 d; for (d = 0; d < c->discriminator.length; d++) {
                    printf("   %d -> %s\n", c->discriminator.buffer[d], corto_idof(c->program.srcType));
                }
            }
            instr = CORTO_OFFSET(instr, sizeof(ospl_opUnion));
            break;
        }
        case OSPL_OP_OPTIONAL: {
            ospl_opSequence *op = instr;
            printf("OPTIONAL (from)%d (to)%d (type)%s \n", op->op.from, op->op.to, corto_fullpath(NULL, op->program.dstType));
            indent++;
            DDS__osplPrint(op->program.program);
            indent--;
            instr = CORTO_OFFSET(instr, sizeof(ospl_opOptional));
            break;
        }
        case OSPL_OP_SEQUENCE: {
            ospl_opSequence *op = instr;
            printf("SEQUENCE (from)%d (to)%d (type)%s \n", op->op.from, op->op.to, corto_fullpath(NULL, op->program.dstType));
            indent++;
            DDS__osplPrint(op->program.program);
            indent--;
            instr = CORTO_OFFSET(instr, sizeof(ospl_opSequence));
            break;
        }
        case OSPL_OP_LIST: {
            ospl_opList *op = instr;
            printf("LIST (from)%d (to)%d (type)%s \n", op->op.from, op->op.to, corto_fullpath(NULL, op->program.dstType));
            indent++;
            DDS__osplPrint(op->program.program);
            indent--;
            instr = CORTO_OFFSET(instr, sizeof(ospl_opList));
            break;
        }
        case OSPL_OP_STOP:
            printf("STOP\n");
            break;
        default:
            corto_critical("DDS__osplPrint: invalid data-serializer program operation.");
            break;
        }
    } while (*(ospl_opKind*)instr != OSPL_OP_STOP);
}

static corto_uint32 ospl_copyOutOpSize(ospl_op *instr) {
    corto_uint32 result = 0;

    switch(instr->kind) {
    case OSPL_OP_COPY: {
        result = sizeof(ospl_opCopy);
        break;
    }
    case OSPL_OP_STRING: {
        result = sizeof(ospl_opString);
        break;
    }
    case OSPL_OP_REFERENCE: {
        result = sizeof(ospl_opReference);
        break;
    }
    case OSPL_OP_OPTIONAL: {
        result = sizeof(ospl_opOptional);
        break;
    }
    case OSPL_OP_SEQUENCE: {
        result = sizeof(ospl_opSequence);
        break;
    }
    case OSPL_OP_LIST: {
        result = sizeof(ospl_opList);
        break;
    }
    case OSPL_OP_UNION: {
        result = sizeof(ospl_opUnion);
        break;
    }
    case OSPL_OP_STOP:
        result = sizeof(ospl_op);
        break;
    default:
        corto_critical("ospl_copyOutSize: invalid data-serializer program operation.");
        break;
    }

    return result;
}

/* Combine copy-instructions where possible */
static void ospl_serdataOptimizeFields(ospl_serdata *root) {
    corto_iter iter, instrIter;
    ospl_op *instr, *start;
    ospl_serdata *data;
    corto_ll optimized;
    corto_uint32 lastOffset = 0;

    /* Loop compiled programs */
    iter = corto_llIter(root->compiled);
    while(corto_iterHasNext(&iter)) {
        data = corto_iterNext(&iter);
        start = NULL;
        optimized = corto_llNew();

        /* Loop instructions of program */
        instrIter = corto_llIter(data->program);
        while(corto_iterHasNext(&instrIter)) {
            instr = corto_iterNext(&instrIter);
            if (!start) {
                if (instr->kind == OSPL_OP_COPY) {
                    start = instr;
                    lastOffset = start->from + ((ospl_opCopy*)start)->size;
                }else {
                    ospl_instrClone(optimized, instr);
                    data->size += ospl_copyOutOpSize(instr);
                }
            } else {
                corto_bool isCopy = instr->kind == OSPL_OP_COPY;

                /* If from-start != to-start the block can no longer be copied with
                 * a single copy instruction, thus insert a copy for the previous
                 * instructions and start a new block. */
                if (!isCopy || ((instr->from - start->from) != (instr->to - start->to))) {
                    ospl_copyInsert(optimized, start->from, start->to, lastOffset - start->from);
                    data->size += sizeof(ospl_opCopy);
                    start = isCopy ? instr : NULL;
                }
                if (isCopy) {
                    lastOffset = instr->from + ((ospl_opCopy*)instr)->size;
                } else if (!isCopy) {
                    ospl_instrClone(optimized, instr);
                    data->size += ospl_copyOutOpSize(instr);
                }
            }
        }
        if (start) {
            ospl_copyInsert(optimized, start->from, start->to, instr->from - start->from + ((ospl_opCopy*)instr)->size);
            data->size += sizeof(ospl_opCopy);
        }
        data->program = optimized; /* TODO: Memory leakage */
    }
}

void ospl_serdataToArraySingle(ospl_serdata *data) {
    corto_iter instrIter;
    ospl_op *instr;
    corto_uint32 offset=0;
    corto_uint32 instrSize=0;

    if (!data->result) {
        /* Create instruction array */
        data->result = corto_alloc(data->size);

        /* Loop instructions of program, copy in array */
        instrIter = corto_llIter(data->program);
        while(corto_iterHasNext(&instrIter)) {
            instr = corto_iterNext(&instrIter);
            instrSize = ospl_copyOutOpSize(instr);
            memcpy(CORTO_OFFSET(data->result, offset), instr, instrSize);
            offset += instrSize;
        }
    }
}

/* Translate from list to instruction-array */
void ospl_serdataToArray(ospl_serdata *root) {
    corto_iter iter;
    ospl_serdata *data;

    /* Loop compiled programs */
    iter = corto_llIter(root->compiled);
    while(corto_iterHasNext(&iter)) {
        data = corto_iterNext(&iter);
        ospl_serdataToArraySingle(data);
    }
}

/* Parse dot-member expression and return offset */
corto_uint32 ospl_copyOutGetMemberOffset(
    corto_type type,
    corto_string key,
    corto_type *type_out)
{
    corto_id buff;
    char *ptr = buff, *prev = buff;
    corto_uint32 offset = 0;
    corto_type currentType = type;
    corto_member m = NULL;

    strcpy(buff, key);
    while ((ptr = strchr(ptr + 1, '.'))) {
        *ptr = '\0';
        offset += ospl_getDdsOffset(currentType, prev, &m);
        if (!m) {
            corto_seterr("key '%s' not found in type '%s'",
                prev, corto_fullpath(NULL, type));
            goto error;
        }
        currentType = m->type;
        prev = ptr + 1;
    }

    if (*key) {
        offset += ospl_getDdsOffset(currentType, prev, &m);
        if (type_out) {
            *type_out = m->type;
        }
    }

    return offset;
error:
    return -1;
}

/* Build array of key offsets and key types based on key expression */
corto_int16 ospl_copyOutGetKeyOffsets(ospl_copyProgram program, corto_string keys) {

    program->nKeys = 0;
    if (keys) {
        corto_id buff;
        char *ptr = buff, *prev = buff;
        corto_uint32 offset;
        corto_type keyType = NULL;

        strcpy(buff, keys);
        while ((ptr = strchr(ptr + 1, ','))) {
            *ptr = '\0';
            offset = ospl_copyOutGetMemberOffset(program->base.srcType, prev, &keyType);
            if (offset == -1) {
                goto error;
            }
            program->key_types[program->nKeys] = keyType;
            program->key_offsets[program->nKeys] = offset;
            program->nKeys ++;
            prev = ptr + 1;
        }

        if (*keys) {
            offset = ospl_copyOutGetMemberOffset(program->base.srcType, prev, &keyType);
            if (offset == -1) {
                goto error;
            }
            program->key_types[program->nKeys] = keyType;
            program->key_offsets[program->nKeys] = offset;
            program->nKeys ++;
        }
    }

    return 0;
error:
    return -1;
}

void ospl_copyProgram_getKeys(
    ospl_copyProgram program,
    void* src,
    void **ptrs_out,
    corto_type *types_out,
    int *nKeys_out)
{
    corto_uint32 i = 0;
    *nKeys_out = program->nKeys;

    for (i = 0; i < program->nKeys; i++) {
        ptrs_out[i] = CORTO_OFFSET(src, program->key_offsets[i]);
        types_out[i] = program->key_types[i];
    }
}

corto_string ospl_copyProgram_keyString(
    ospl_copyProgram program,
    corto_id result,
    void* src)
{
    void *ptrs[OSPL_MAX_KEYS];
    corto_type types[OSPL_MAX_KEYS];
    int nKeys, i;
    result[0] = '\0';

    ospl_copyProgram_getKeys(program, src, ptrs, types, &nKeys);

    for (i = 0; i < nKeys; i ++) {
        char *str = NULL;
        if (!corto_instanceof(corto_text_o, types[i])) {
            str = corto_strp(ptrs[i], types[i], 0);
        } else {
            str = corto_strdup(*(corto_string*)ptrs[i]);
        }
        if (i) {
            strcat(result, "_");
        }
        strcat(result, str);
        corto_dealloc(str);
    }

    return result;
}

/* Compile program for type */
ospl_copyElement ospl_copyProgramCompile(corto_type srcType, corto_type dstType) {
    ospl_serdata *data;
    struct corto_serializer_s s;
    ospl_copyElement result = {.program = NULL};

    s = ospl_copyProgramSerializer();

    data = ospl_serdataNew(srcType, dstType, NULL);

    /* Walk over dstType, insert instructions to corresponding fields in type */
    if (corto_metaWalk(&s, dstType, data)) {
        corto_seterr("creating program(%s=>%s) failed: %s",
          corto_fullpath(NULL, srcType),
          corto_fullpath(NULL, dstType),
          corto_lasterr());
        goto error;
    }
    /* Insert STOP */
    ospl_stopInsert(data->program);

    /* Combine copy instructions where possible */
    ospl_serdataOptimizeFields(data);

    /* Convert program to array */
    ospl_serdataToArray(data);

    /*printf("%s=>%s program (optimized):\n", corto_fullpath(NULL, srcType), corto_fullpath(NULL, dstType));
    DDS__osplPrint(data->result);
    printf("\n");*/

    /* Convert program from list to array */
    result.program = data->result;
    result.srcType = srcType;
    result.dstType = dstType;
    result.ddsSize = ospl_getDdsOffset(srcType, NULL, NULL); /* Compute DDS type size */

    ospl_serdataFree(data);

  error:
    return result;
}

/* Create a new copy-program for a type */
ospl_copyProgram _ospl_copyProgramNew(corto_type srcType, corto_type dstType, corto_string keys) {
    ospl_copyProgram result = NULL;
    ospl_copyElement e = ospl_copyProgramCompile(srcType, dstType);
    if(!e.program) {
        goto error;
    }

    result = corto_alloc(sizeof(struct ospl_copyProgram));
    result->base = e;

    /* Obtain key offsets */
    ospl_copyOutGetKeyOffsets(result, keys);

    return result;
error:
    return NULL;
}

static corto_int16 ospl_copyUnion(
    ospl_opUnion *op,
    void *dst,
    void *src,
    corto_int16 ___ (*copy)(ospl_copyElement *p, void *dst, void *src))
{
    corto_int32 srcD = *(corto_int32*)src;
    ospl_copyUnionCase *defaultCase = NULL;
    corto_bool set = FALSE;
    corto_int32 i; for (i = 0; i < op->caseCount; i ++) {
        ospl_copyUnionCase *c = &op->cases[i];
        if (!c->discriminator.length) {
            defaultCase = c;
        }
        corto_int32 d; for (d = 0; d < c->discriminator.length; d++) {
            if (srcD == c->discriminator.buffer[d]) {
                set = TRUE;
                if (copy(&c->program,
                    CORTO_OFFSET(dst, op->offset),
                    CORTO_OFFSET(src, op->offset)))
                {
                    goto error;
                }
                break;
            }
        }
    }
    if (!set && defaultCase) {
        if (copy(&defaultCase->program,
            CORTO_OFFSET(dst, op->offset),
            CORTO_OFFSET(src, op->offset)))
        {
            goto error;
        }
    } else if (!set) {
        corto_error("invalid discriminator '%d'", srcD);
        goto error;
    }
    *(corto_int32*)dst = srcD;

    return 0;
error:
    return -1;
}

/* Run program to copy from DDS to corto */
static corto_int16 ospl_copyOutElement(ospl_copyElement *program, void *o, void *sample) {
    ospl_op *instr;

    instr = program->program;
    do {
        void *src = CORTO_OFFSET(sample, instr->from);
        void *dst = CORTO_OFFSET(o, instr->to);
        switch(instr->kind) {
        case OSPL_OP_COPY: {
            ospl_opCopy *op = (ospl_opCopy*)instr;
            memcpy(dst, src, op->size);
            instr = CORTO_OFFSET(instr, sizeof(ospl_opCopy));
            break;
        }
        case OSPL_OP_STRING: {
            corto_setstr(dst, *(corto_string*)src);
            instr = CORTO_OFFSET(instr, sizeof(ospl_opString));
            break;
        }
        case OSPL_OP_REFERENCE: {
            corto_object ref = corto_resolve(NULL, *(corto_string*)src);
            if (!ref) {
                corto_seterr("invalid reference '%s' in DDS object", ref);
                goto error;
            }
            corto_setref(dst, ref);
            instr = CORTO_OFFSET(instr, sizeof(ospl_opReference));
            break;
        }
        case OSPL_OP_UNION: {
            if (ospl_copyUnion((ospl_opUnion*)instr, dst, src, ospl_copyOutElement)) {
                goto error;
            }
            instr = CORTO_OFFSET(instr, sizeof(ospl_opUnion));
            break;
        }
        case OSPL_OP_OPTIONAL: {
            ospl_opOptional *op = (ospl_opOptional*)instr;
            DDS_SampleInfoSeq *seq = src;
            if (!seq->_length) {
                if (*(void**)dst) {
                    corto_deinitp(*(void**)dst, op->program.dstType);
                    corto_dealloc(*(void**)dst);
                    *(void**)dst = NULL;
                }
            } else {
                if (!*(void**)dst) {
                    *(void**)dst = corto_calloc(op->program.dstType->size);
                }
                ospl_copyOutElement(&op->program, *(void**)dst, (void*)seq->_buffer);
            }
            instr = CORTO_OFFSET(instr, sizeof(ospl_opOptional));
            break;
        }
        case OSPL_OP_SEQUENCE: {
            ospl_opSequence *op = (ospl_opSequence*)instr;
            DDS_SampleInfoSeq *src = CORTO_OFFSET(sample, instr->from);
            corto_objectseq *dst = CORTO_OFFSET(o, instr->to);
            corto_uint32 dstSize = op->program.dstType->size;

            /* If target buffer is smaller, grow buffer with zero'd memory */
            if (dst->length < src->_length) {
                dst->buffer = corto_realloc(dst->buffer, dstSize * src->_length);
                memset(
                  CORTO_OFFSET(dst->buffer, dstSize * dst->length),
                  0,
                  (src->_length - dst->length) * dstSize);

            /* If target buffer is larger, deinit elements, and shrink buffer */
            } else if (dst->length > src->_length) {
                corto_int32 i; for (i = src->_length; i < dst->length; i++) {
                    corto_deinitp(CORTO_OFFSET(dst->buffer, dstSize * i), op->program.dstType);
                }
                dst->buffer = corto_realloc(dst->buffer, dstSize * src->_length);
            }

            dst->length = src->_length;

            void *srcPtr, *dstPtr;
            corto_uint32 i; for (i = 0; i < dst->length; i++) {
                srcPtr = CORTO_OFFSET(src->_buffer, i * op->program.ddsSize);
                dstPtr = CORTO_OFFSET(dst->buffer, i * op->program.dstType->size);
                ospl_copyOutElement(&op->program, dstPtr, srcPtr);
            }

            instr = CORTO_OFFSET(instr, sizeof(ospl_opSequence));
            break;
        }
        case OSPL_OP_LIST: {
            //printf("/// LIST (from)%d (to)%d (size)%d\n", op->op.from, op->op.to, op->elementSize);
            instr = CORTO_OFFSET(instr, sizeof(ospl_opList));
            break;
        }
        case OSPL_OP_STOP:
            break;
        default:
            corto_critical("ospl_copyOut: invalid data-serializer program operation.");
            break;
        }
    } while (instr->kind != OSPL_OP_STOP);

    return 0;
error:
    return -1;
}

corto_int16 ospl_copyOut(ospl_copyProgram program, corto_object *dst, void *src) {
    corto_object o;
    corto_bool newObject = FALSE;

    if (!dst) {
        corto_seterr("invalid destination pointer");
        goto error;
    }

    if (*dst) {
        o = *dst;
    } else {
        o = corto_declare(program->base.dstType);
        newObject = TRUE;
    }

    if (ospl_copyOutElement(&program->base, o, src)) {
        goto error;
    }

    if (newObject) {
        if (corto_define(o)) {
            corto_seterr("define failed: %s", corto_lasterr());
            goto error;
        }
        *dst = o;
    }

    return 0;
error:
    if (newObject) {
        corto_delete(o);
    }
    return -1;
}

/* Run program to copy from Corto to DDS */
static corto_int16 ospl_copyInElement(ospl_copyElement *program, void *sample, void *value) {
    ospl_op *instr;

    /* Reverse program, copy from 'to' to 'from' */

    instr = program->program;
    do {
        void *src = CORTO_OFFSET(value, instr->from);
        void *dst = CORTO_OFFSET(sample, instr->to);
        switch(instr->kind) {
        case OSPL_OP_COPY: {
            ospl_opCopy *op = (ospl_opCopy*)instr;
            memcpy(CORTO_OFFSET(sample, instr->from), CORTO_OFFSET(value, instr->to), op->size);
            instr = CORTO_OFFSET(instr, sizeof(ospl_opCopy));
            break;
        }
        case OSPL_OP_STRING: {
            corto_setstr(CORTO_OFFSET(sample, instr->from), *(corto_string*)CORTO_OFFSET(value, instr->to));
            instr = CORTO_OFFSET(instr, sizeof(ospl_opString));
            break;
        }
        case OSPL_OP_REFERENCE: {
            corto_id id;
            corto_object *ref = CORTO_OFFSET(value, instr->to);
            if (*ref) {
                corto_fullpath(id, *ref);
            } else {
                strcpy(id, "null");
            }
            corto_setstr(CORTO_OFFSET(sample, instr->from), id);
            instr = CORTO_OFFSET(instr, sizeof(ospl_opReference));
            break;
        }
        case OSPL_OP_UNION: {
            if (ospl_copyUnion((ospl_opUnion*)instr, dst, src, ospl_copyInElement)) {
                goto error;
            }
            instr = CORTO_OFFSET(instr, sizeof(ospl_opUnion));
            break;
        }
        case OSPL_OP_OPTIONAL: {
            ospl_opOptional *op = (ospl_opOptional*)instr;
            DDS_SampleInfoSeq *dst = CORTO_OFFSET(sample, instr->from);
            void **src = CORTO_OFFSET(value, instr->to);

            if (*src) {
                dst->_length = 1;
                dst->_maximum = 1;
                dst->_buffer = corto_calloc(op->program.ddsSize);
                ospl_copyInElement(&op->program, dst->_buffer, *src);
            } else {
                dst->_length = 0;
                dst->_maximum = 0;
                dst->_buffer = NULL;
            }
            dst->_release = FALSE;

            instr = CORTO_OFFSET(instr, sizeof(ospl_opOptional));
            break;
        }
        case OSPL_OP_SEQUENCE: {
            ospl_opSequence *op = (ospl_opSequence*)instr;
            DDS_SampleInfoSeq *dst = CORTO_OFFSET(sample, instr->from);
            corto_objectseq *src = CORTO_OFFSET(value, instr->to);
            dst->_length = src->length;
            dst->_maximum = src->length;
            dst->_release = FALSE;
            dst->_buffer = corto_calloc(src->length * op->program.ddsSize);

            void *srcPtr, *dstPtr;
            corto_uint32 i; for (i = 0; i < src->length; i++) {
                srcPtr = CORTO_OFFSET(src->buffer, i * op->program.dstType->size);
                dstPtr = CORTO_OFFSET(dst->_buffer, i * op->program.ddsSize);
                ospl_copyInElement(&op->program, dstPtr, srcPtr);
            }

            instr = CORTO_OFFSET(instr, sizeof(ospl_opSequence));
            break;
        }
        case OSPL_OP_LIST: {
            instr = CORTO_OFFSET(instr, sizeof(ospl_opList));
            break;
        }
        case OSPL_OP_STOP:
            break;
        default:
            corto_critical("ospl_copyIn: invalid data-serializer program operation.");
            break;
        }
    } while (instr->kind != OSPL_OP_STOP);

    return 0;
error:
    return -1;
}

corto_int16 ospl_copyIn(ospl_copyProgram program, void *sample, corto_object src) {
    if (!sample) {
        corto_seterr("invalid destination sample");
        goto error;
    }
    ospl_copyInElement(&program->base, sample, src);
    return 0;
error:
    return -1;
}

/* Free DDS sample */
static corto_int16 ospl_copyFreeElement(ospl_copyElement *program, void *sample, void *dummy) {
    ospl_op *instr;

    instr = program->program;
    do {
        switch(instr->kind) {
        case OSPL_OP_COPY: {
            instr = CORTO_OFFSET(instr, sizeof(ospl_opCopy));
            break;
        }
        case OSPL_OP_STRING: {
            corto_string str = *(corto_string*)CORTO_OFFSET(sample, instr->from);
            if (str) {
                corto_dealloc(str);
            }
            instr = CORTO_OFFSET(instr, sizeof(ospl_opString));
            break;
        }
        case OSPL_OP_REFERENCE: {
            corto_string str = *(corto_string*)CORTO_OFFSET(sample, instr->from);
            if (str) {
                corto_dealloc(str);
            }
            instr = CORTO_OFFSET(instr, sizeof(ospl_opReference));
            break;
        }
        case OSPL_OP_UNION:
            if (ospl_copyUnion((ospl_opUnion*)instr, sample, NULL, ospl_copyFreeElement)) {
                goto error;
            }
            break;
        case OSPL_OP_OPTIONAL:
        case OSPL_OP_SEQUENCE: {
            ospl_opSequence *op = (ospl_opSequence*)instr;
            DDS_SampleInfoSeq *seq = CORTO_OFFSET(sample, instr->from);
            void *ptr;
            corto_uint32 i; for (i = 0; i < seq->_length; i++) {
                ptr = CORTO_OFFSET(seq->_buffer, i * op->program.ddsSize);
                if (ospl_copyFreeElement(&op->program, ptr, NULL)) {
                    goto error;
                }
            }
            corto_dealloc(seq->_buffer);

            instr = CORTO_OFFSET(instr, sizeof(ospl_opSequence));
            break;
        }
        case OSPL_OP_LIST: {
            //printf("/// LIST (from)%d (to)%d (size)%d\n", op->op.from, op->op.to, op->elementSize);
            instr = CORTO_OFFSET(instr, sizeof(ospl_opList));
            break;
        }
        case OSPL_OP_STOP:
            break;
        default:
            corto_critical("ospl_copyOut: invalid data-serializer program operation.");
            break;
        }
    } while (instr->kind != OSPL_OP_STOP);

    return 0;
error:
    return -1;
}

corto_int16 ospl_copyFree(ospl_copyProgram program, void *sample) {
    corto_int16 result = ospl_copyFreeElement(&program->base, sample, NULL);
    corto_dealloc(sample);
    return result;
}

/* Allocate space for a DDS sample */
void* ospl_copyAlloc(ospl_copyProgram program) {
    return corto_calloc(program->base.ddsSize);
}

corto_uint32 ospl_copyProgram_getDdsSize(ospl_copyProgram program) {
    return program->base.ddsSize;
}
