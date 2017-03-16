#ifndef DDDS_METAXML_H
#define DDDS_METAXML_H

#include "corto/corto.h"

#ifdef __cplusplus
extern "C" {
#endif

corto_string ospl_metaXmlGet(corto_type type);
corto_int16 ospl_metaXmlParse(corto_string xml, corto_string type, corto_string keys);

#ifdef __cplusplus
}
#endif
#endif
