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
  typedef void* ospl_copyProgram;
#else
  typedef struct ospl_copyProgram* ospl_copyProgram;
#endif


ospl_copyProgram _ospl_copyProgramNew(corto_type type, corto_type view);
void ospl_copyProgramFree(ospl_copyProgram program);

#define ospl_copyProgramNew(type, view)\
  _ospl_copyProgramNew(corto_type(type), corto_type(view))

corto_int16 ospl_copyOut(ospl_copyProgram program, corto_object *dst, void *src);
corto_int16 ospl_copyIn(ospl_copyProgram program, void *dst, corto_object src);
void* ospl_copyAlloc(ospl_copyProgram program);
corto_int16 ospl_copyFree(ospl_copyProgram program, void *sample);

corto_uint32 ospl_copyProgram_getDdsSize(ospl_copyProgram program);
void ospl_copyProgram_getKeys(
    ospl_copyProgram program,
    void* src,
    void **ptrs_out,
    corto_type *types_out,
    int *nKeys_out);

corto_string ospl_copyProgram_keyString(
    ospl_copyProgram program,
    corto_id result,
    void* src);

#ifdef __cplusplus
}
#endif

#endif /* DDS__DATASERIALIZER_H_ */
