/*
 * DDS__dataserializer.c
 *
 *  Created on: Feb 22, 2013
 *      Author: sander
 */

#include "ospl/ospl.h"

/* CopyOut program structures */
typedef enum ospl_opKind {
    OSPL_OP_COPY,
    OSPL_OP_STRING,
    OSPL_OP_REFERENCE, /* References are mapped to strings */
    OSPL_OP_SEQUENCE,
    OSPL_OP_LIST,
    OSPL_OP_STOP
}ospl_opKind;

typedef struct ospl_op {
    ospl_opKind kind;
    corto_uint32 from;
    corto_uint32 to;
}ospl_op;

typedef struct ospl_opCopy {
    ospl_op op;
    corto_uint32 size;
}ospl_opCopy;

typedef struct ospl_opSequence {
    ospl_op op;
    corto_uint32 elementSize;
    void *elementInstr; /* If elementInstr is NULL, the sequence elements have
    					   no resources, and can be copied using a single memcopy.

    					   In the first passes, this pointer contains the serializerdata
    					   with the unoptimized instruction list. This is eventually
    					   replaced with an optimized instruction-array.  */
}ospl_opSequence;

typedef struct ospl_opList {
    ospl_op op;
    corto_uint32 elementSize;
    void *elementInstr; /* If elementInstr is NULL, the sequence elements have
                           no resources, and can be copied using a single memcopy

                           In the first passes, this pointer contains the serializerdata
                           with the unoptimized instruction list. This is eventually
                           replaced with an optimized instruction-array.  */
}ospl_opList;

typedef struct ospl_opString {
    ospl_op op;
}ospl_opString;

typedef struct ospl_opReference {
    ospl_op op;
}ospl_opReference;

struct ospl_copyOutProgram {
    void* program;
    corto_type src_type;
    corto_type dst_type;
    corto_uint32 ddsSize;
    corto_uint32 key_offsets[OSPL_MAX_KEYS];
    corto_type key_types[OSPL_MAX_KEYS];
    corto_uint32 nKeys;
};

typedef struct ospl_serdata ospl_serdata;
struct ospl_serdata {
    corto_ll program;
    corto_type src_type;
    corto_type dst_type;
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

/* Find compiled type (only root-type and types used in sequences appear in this list */
ospl_serdata *DDS__ospl_findType(ospl_serdata *data, corto_type type) {
    corto_iter iter;
    ospl_serdata *found = NULL;

    iter = corto_llIter(data->program);
    while(corto_iterHasNext(&iter)) {
        found = corto_iterNext(&iter);
        if (found->src_type == type) {
            break;
        }else {
            found = NULL;
        }
    }

    return found;
}

ospl_serdata *ospl_serdataNew(corto_type src_type, corto_type dst_type, corto_ll compiled) {
    ospl_serdata *data;

    data = corto_alloc(sizeof(ospl_serdata));
    data->src_type = src_type;
    data->dst_type = dst_type;
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
void ospl_copyInsert(corto_ll program, corto_uint32 from, corto_uint32 to, corto_uint32 size) {
    ospl_opCopy *instr;
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

/* Insert copy-operation for sequences */
void ospl_sequenceInsert(corto_ll program, corto_uint32 from, corto_uint32 to, corto_uint32 elementSize, void *elementInstr) {
    ospl_opSequence *instr;
    instr = corto_alloc(sizeof(ospl_opSequence));
    instr->op.kind = OSPL_OP_SEQUENCE;
    instr->op.from = from;
    instr->op.to = to;
    instr->elementSize = elementSize;
    instr->elementInstr = elementInstr;
    corto_llAppend(program, instr);
}

/* Insert copy-operation for sequences */
void ospl_listInsert(corto_ll program, corto_uint32 from, corto_uint32 to, corto_uint32 elementSize, void* elementInstr) {
    ospl_opList *instr;
    instr = corto_alloc(sizeof(ospl_opList));
    instr->op.kind = OSPL_OP_LIST;
    instr->op.from = from;
    instr->op.to = to;
    instr->elementSize = elementSize;
    instr->elementInstr = elementInstr;
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
    case OSPL_OP_SEQUENCE:
        ospl_sequenceInsert(program, op->from, op->to, ((ospl_opSequence*)op)->elementSize, ((ospl_opSequence*)op)->elementInstr);
        break;
    case OSPL_OP_LIST:
        ospl_listInsert(program, op->from, op->to, ((ospl_opList*)op)->elementSize, ((ospl_opList*)op)->elementInstr);
        break;
    case OSPL_OP_STOP:
        ospl_stopInsert(program);
        break;
    }
}

corto_int16 ospl_ddsAlignment_item(
    corto_serializer s,
    corto_value *info,
    void *data)
{
    corto_uint8 *alignment = data;
    corto_type t = corto_value_getType(info);

    /* Alignment is the same as corto types, except for sequences */

    if ((t->kind == CORTO_COLLECTION) && (corto_collection(t)->kind != CORTO_ARRAY)) {
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
} ddsOffset_t;

corto_int16 ospl_ddsOffset_item(
    corto_serializer s,
    corto_value *info,
    void *userData)
{
    ddsOffset_t *data = userData;
    corto_type t = corto_value_getType(info);
    corto_uint32 size = corto_type_sizeof(t);

    if ((t->kind == CORTO_COLLECTION) && (corto_collection(t)->kind != CORTO_ARRAY)) {
        size = sizeof(DDS_SampleInfoSeq);
    }

    /* Compute current offset */
    if (data->offset) {
        data->offset = CORTO_ALIGN(data->offset, t->alignment);
    }

    /* Check if membernames match */
    if (data->memberName && !strcmp(corto_idof(info->is.member.t), data->memberName)) {
        data->result = data->offset;
        data->m = info->is.member.t;
        return 1; /* Stop serializing */
    } else {
        data->offset += size;
    }

    return 0;
}

struct corto_serializer_s ospl_ddsOffsetSerializer(void) {
    struct corto_serializer_s s;
    corto_serializerInit(&s);
    s.access = CORTO_LOCAL;
    s.accessKind = CORTO_NOT;
    s.traceKind = CORTO_SERIALIZER_TRACE_ON_FAIL;
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
    walkData.m = NULL;

    if (!corto_metaWalk(&s, type, &walkData) && memberName) {
        corto_seterr("member '%s' not found in DDS type '%s'",
            memberName,
            corto_fullpath(NULL, type));
        goto error;
    }

    if (!memberName) {
        walkData.result = CORTO_ALIGN(walkData.offset, walkData.alignment);
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
        ospl_stringInsert(data->program, data->src_offset, data->dst_offset);
        break;
    default:
        ospl_copyInsert(data->program, data->src_offset, data->dst_offset, corto_type_sizeof(t));
        break;
    }
    return 0;
}

/* Reference values */
corto_int16 DDS__ospl_reference(corto_serializer s, corto_value* v, void* userData) {
    ospl_serdata *data = userData;
    ospl_referenceInsert(data->program, data->src_offset, data->dst_offset);
    return 0;
}

/* Collection value */
corto_int16 DDS__ospl_composite(corto_serializer s, corto_value* v, void *userData) {

    if (v->kind == CORTO_MEMBER) {
        ospl_serdata *data = userData;
        ospl_serdata nested = *data;
        corto_type t = corto_value_getType(v);
        corto_member member = v->is.member.t;
        corto_member srcMember = NULL;

        /* Lookup member in type */
        ospl_getDdsOffset(data->src_type, corto_idof(member), &srcMember);

        nested.dst_base_offset = data->dst_base_offset + data->dst_offset;
        nested.src_base_offset = data->src_base_offset + data->src_offset;
        nested.dst_offset = 0;
        nested.src_offset = 0;
        nested.src_type = srcMember->type;
        nested.dst_type = t;

        /* Serialize nested type */
        return corto_serializeMembers(s, v, &nested);
    } else {
        return corto_serializeMembers(s, v, userData);
    }
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
        ospl_serdata *found;
        corto_type elementType = corto_collection(t)->elementType;

        /* Check if elementType already has been serialized - in case of
         * cyclic types. */
        if (!(found = DDS__ospl_findType(data, elementType))) {
            if (elementType->hasResources) {
                found = ospl_serdataNew(elementType, elementType, data->compiled);

                /* Serialize elementType */
                if (corto_metaWalk(s, elementType, found)) {
                    goto error;
                }
            }else {
                found = NULL;
            }
        }

        /* Insert sequence instruction */
        if (corto_collection(t)->kind == CORTO_SEQUENCE) {
            ospl_sequenceInsert(data->program, data->src_offset, data->dst_offset, corto_type_sizeof(elementType), found);
        }else if (corto_collection(t)->kind == CORTO_LIST) {
            ospl_listInsert(data->program, data->src_offset, data->dst_offset, corto_type_sizeof(elementType), found);
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
    data->dst_offset = data->dst_base_offset + m->offset;
    data->src_offset = data->src_base_offset + ospl_getDdsOffset(data->src_type, corto_idof(m), NULL);
    return corto_serializeValue(s, info, userData);
}

struct corto_serializer_s ospl_copyOutProgramSerializer(void) {
    struct corto_serializer_s s;
    corto_serializerInit(&s);
    s.access = CORTO_LOCAL;
    s.accessKind = CORTO_NOT;
    s.traceKind = CORTO_SERIALIZER_TRACE_ON_FAIL;
    s.program[CORTO_PRIMITIVE] = DDS__ospl_primitive;
    s.program[CORTO_COLLECTION] = DDS__ospl_collection;
    s.program[CORTO_COMPOSITE] = DDS__ospl_composite;
    s.metaprogram[CORTO_MEMBER] = DDS__ospl_item;
    s.reference = DDS__ospl_reference;
    return s;
}

/* Print program */
void DDS__osplPrint(corto_ll program) {
    corto_iter iter;
    void *instr;
    static corto_uint32 indent = 0;

    iter = corto_llIter(program);
    while(corto_iterHasNext(&iter)) {
        instr = corto_iterNext(&iter);

        /* Print indentation */
        printf("%*s", indent * 3, "");

        switch(*(ospl_opKind*)instr) {
        case OSPL_OP_COPY: {
            ospl_opCopy *op = instr;
            printf("COPY (from)%d (to)%d (size)%d\n", op->op.from, op->op.to, op->size);
            break;
        }
        case OSPL_OP_STRING: {
            ospl_opString *op = instr;
            printf("STRING (from)%d (to)%d\n", op->op.from, op->op.to);
            break;
        }
        case OSPL_OP_REFERENCE: {
            ospl_opReference *op = instr;
            printf("REFERENCE (from)%d (to)%d\n", op->op.from, op->op.to);
            break;
        }
        case OSPL_OP_SEQUENCE: {
            ospl_opSequence *op = instr;
            printf("SEQUENCE (from)%d (to)%d (elementSize)%d \n", op->op.from, op->op.to, op->elementSize);
            if (op->elementInstr) {
                indent++;
                DDS__osplPrint(((ospl_serdata*)op->elementInstr)->program);
                indent--;
            }
            break;
        }
        case OSPL_OP_LIST: {
            ospl_opList *op = instr;
            printf("LIST (from)%d (to)%d (size)%d\n", op->op.from, op->op.to, op->elementSize);
            break;
        }
        case OSPL_OP_STOP:
            printf("STOP\n");
            break;
        default:
            corto_critical("DDS__osplPrint: invalid data-serializer program operation.");
            break;
        }
    }
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
    case OSPL_OP_SEQUENCE: {
        result = sizeof(ospl_opSequence);
        break;
    }
    case OSPL_OP_LIST: {
        result = sizeof(ospl_opList);
        break;
    }
    case OSPL_OP_STOP:
        result = sizeof(ospl_op);
        break;
    default:
        corto_critical("ospl_copyOut: invalid data-serializer program operation.");
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
                }else {
                    ospl_instrClone(optimized, instr);
                    data->size += sizeof(ospl_opCopy);
                }
            } else {
                corto_bool isCopy = instr->kind == OSPL_OP_COPY;

                if (isCopy) {
                    lastOffset = instr->from + ((ospl_opCopy*)instr)->size;
                }

                /* If from-start != to-start the block can no longer be copied with
                 * a single copy instruction, thus insert a copy for the previous
                 * instructions and start a new block. */
                if (!isCopy || ((instr->from - start->from) != (instr->to - start->to))) {
                    ospl_copyInsert(optimized, start->from, start->to, lastOffset - start->from);
                    data->size += sizeof(ospl_opCopy);
                    start = isCopy ? instr : NULL;
                }
                if (!isCopy) {
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
corto_int16 ospl_copyOutGetKeyOffsets(ospl_copyOutProgram program, corto_string keys) {
    corto_id buff;
    char *ptr = buff, *prev = buff;
    corto_uint32 offset;
    corto_type keyType = NULL;

    program->nKeys = 0;
    strcpy(buff, keys);
    while ((ptr = strchr(ptr + 1, ','))) {
        *ptr = '\0';
        offset = ospl_copyOutGetMemberOffset(program->src_type, prev, &keyType);
        if (offset == -1) {
            goto error;
        }
        program->key_types[program->nKeys] = keyType;
        program->key_offsets[program->nKeys] = offset;
        program->nKeys ++;
        prev = ptr + 1;
    }

    if (*keys) {
        offset = ospl_copyOutGetMemberOffset(program->src_type, prev, &keyType);
        if (offset == -1) {
            goto error;
        }
        program->key_types[program->nKeys] = keyType;
        program->key_offsets[program->nKeys] = offset;
        program->nKeys ++;
    }

    return 0;
error:
    return -1;
}

void ospl_copyOutProgram_getKeys(
    ospl_copyOutProgram program,
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

corto_string ospl_copyOutProgram_keyString(
    ospl_copyOutProgram program,
    corto_id result,
    void* src)
{
    void *ptrs[OSPL_MAX_KEYS];
    corto_type types[OSPL_MAX_KEYS];
    int nKeys, i;
    result[0] = '\0';

    ospl_copyOutProgram_getKeys(program, src, ptrs, types, &nKeys);

    for (i = 0; i < nKeys; i ++) {
        char *str = corto_strp(ptrs[i], types[i], 0);
        if (i) {
            strcat(result, "_");
        }
        strcat(result, str);
        corto_dealloc(str);
    }

    return result;
}

/* Create a new copy-program for a type */
ospl_copyOutProgram _ospl_copyOutProgramNew(corto_type src_type, corto_type dst_type, corto_string keys) {
    ospl_serdata *data;
    struct corto_serializer_s s;
    ospl_copyOutProgram result = NULL;

    s = ospl_copyOutProgramSerializer();

    data = ospl_serdataNew(src_type, dst_type, NULL);

    /* Walk over dst_type, insert instructions to corresponding fields in type */
    corto_metaWalk(&s, dst_type, data);
    /* Insert STOP */
    ospl_stopInsert(data->program);

    /*printf("%s unoptimized:\n", corto_fullpath(NULL, src_type));
    DDS__osplPrint(data->program);
    printf("\n"); */

    /* Combine copy instructions where possible */
    ospl_serdataOptimizeFields(data);

    /*printf("%s optimized:\n", corto_fullpath(NULL, src_type));
    DDS__osplPrint(data->program);
    printf("\n"); */

    /* Convert program from list to array */
    ospl_serdataToArray(data);

    result = corto_alloc(sizeof(struct ospl_copyOutProgram));
    result->program = data->result;
    result->src_type = src_type;
    result->dst_type = dst_type;
    result->ddsSize = ospl_getDdsOffset(src_type, NULL, NULL); /* Compute DDS type size */

    /* Obtain key offsets */
    ospl_copyOutGetKeyOffsets(result, keys);

    /* Cleanup data */
    ospl_serdataFree(data);

    return result;
}

/* Run program */
corto_int16 ospl_copyOut(ospl_copyOutProgram program, corto_object *dst, void *src) {
    ospl_op *instr;
    corto_object o;
    corto_bool newObject = FALSE;

    if (!dst) {
        corto_seterr("invalid destination pointer");
        goto error;
    }

    if (*dst) {
        o = *dst;
    } else {
        o = corto_declare(program->dst_type);
        newObject = TRUE;
    }

    instr = program->program;
    do {
        switch(instr->kind) {
        case OSPL_OP_COPY: {
            ospl_opCopy *op = (ospl_opCopy*)instr;
            memcpy(CORTO_OFFSET(o, instr->to), CORTO_OFFSET(src, instr->from), op->size);
            instr = CORTO_OFFSET(instr, sizeof(ospl_opCopy));
            break;
        }
        case OSPL_OP_STRING: {
            corto_setstr(CORTO_OFFSET(o, instr->to), *(corto_string*)CORTO_OFFSET(src, instr->from));
            instr = CORTO_OFFSET(instr, sizeof(ospl_opString));
            break;
        }
        case OSPL_OP_REFERENCE: {
            corto_object ref = corto_resolve(NULL, *(corto_string*)CORTO_OFFSET(src, instr->from));
            if (!ref) {
                corto_seterr("invalid reference '%s' in DDS object", ref);
                goto error;
            }
            corto_setref(CORTO_OFFSET(o, instr->to), ref);
            instr = CORTO_OFFSET(instr, sizeof(ospl_opReference));
            break;
        }
        case OSPL_OP_SEQUENCE: {
            ospl_opSequence *op = (ospl_opSequence*)instr;
            printf("/// SEQUENCE (from)%d (to)%d (elementSize)%d \n", op->op.from, op->op.to, op->elementSize);
            instr = CORTO_OFFSET(instr, sizeof(ospl_opSequence));
            break;
        }
        case OSPL_OP_LIST: {
            ospl_opList *op = (ospl_opList*)instr;
            printf("/// LIST (from)%d (to)%d (size)%d\n", op->op.from, op->op.to, op->elementSize);
            instr = CORTO_OFFSET(instr, sizeof(ospl_opList));
            break;
        }
        case OSPL_OP_STOP:
            printf("STOP\n");
            break;
        default:
            corto_critical("ospl_copyOut: invalid data-serializer program operation.");
            break;
        }
    } while (instr->kind != OSPL_OP_STOP);

    if (newObject) {
        if (corto_define(o)) {
            corto_seterr("define failed: %s", corto_lasterr());
            goto error;
        }
        *dst = o;
    }

    return 0;
error:
    return -1;
}

corto_uint32 ospl_copyOutProgram_getDdsSize(ospl_copyOutProgram program) {
    return program->ddsSize;
}
