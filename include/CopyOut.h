/*
 * DDS__dataserializer.h
 *
 *  Created on: Feb 22, 2013
 *      Author: sander
 */

#ifndef OSPL_COPYOUT_H
#define OSPL_COPYOUT_H

#include "ospl/ospl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ospl_copyOutProgram* ospl_copyOutProgram;

ospl_copyOutProgram _ospl_copyOutProgramNew(corto_type type, corto_type view);
#define ospl_copyOutProgramNew(type, view)\
  _ospl_copyOutProgramNew(corto_type(type), corto_type(view))

corto_int16 ospl_copyOut(ospl_copyOutProgram program, corto_object *dst, void *src);
corto_uint32 ospl_copyOutProgram_getDdsSize(ospl_copyOutProgram program);

#ifdef __cplusplus
}
#endif

#endif /* DDS__DATASERIALIZER_H_ */
