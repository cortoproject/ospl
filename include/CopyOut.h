/*
 * DDS__dataserializer.h
 *
 *  Created on: Feb 22, 2013
 *      Author: sander
 */

#ifndef OSPL_COPYOUT_H
#define OSPL_COPYOUT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ospl_copyOutProgram* ospl_copyOutProgram;

ospl_copyOutProgram _ospl_copyOutProgramNew(corto_type type, corto_type view, corto_string keys);
#define ospl_copyOutProgramNew(type, view, keys)\
  _ospl_copyOutProgramNew(corto_type(type), corto_type(view), keys)

corto_int16 ospl_copyOut(ospl_copyOutProgram program, corto_object *dst, void *src);
corto_uint32 ospl_copyOutProgram_getDdsSize(ospl_copyOutProgram program);
void ospl_copyOutProgram_getKeys(
    ospl_copyOutProgram program,
    void* src,
    void **ptrs_out,
    corto_type *types_out,
    int *nKeys_out);

corto_string ospl_copyOutProgram_keyString(
    ospl_copyOutProgram program,
    corto_id result,
    void* src);

#ifdef __cplusplus
}
#endif

#endif /* DDS__DATASERIALIZER_H_ */
