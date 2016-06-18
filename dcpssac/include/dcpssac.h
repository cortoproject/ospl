
#ifndef CORTO_FMT_DCPSSAC_H
#define CORTO_FMT_DCPSSAC_H

/* Add include files here */

#ifdef __cplusplus
extern "C" {
#endif

corto_int16 dcpssac_toCorto(corto_object dst, void* src);
void* dcpssac_fromCorto(corto_object o);
void dcpssac_release(void* o);

#ifdef __cplusplus
}
#endif

#endif /* CORTO_FMT_DCPSSAC_H */
