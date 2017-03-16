/* ospl.h
 *
 * This file contains generated code. Do not modify!
 */

#ifndef OSPL_H
#define OSPL_H

#include <corto/corto.h>
#include <corto/corto.h>
#include <ospl/_project.h>
#include <corto/ext/idl/idl.h>
#include <corto/fmt/xml/xml.h>
#include <corto/core/c/c.h>
#include <corto/lang/c/c.h>

/* $header() */
#include "dds_dcps.h"
#include <ospl/Copy.h>
#include <ospl/MetaXml.h>

#define OSPL_MAX_KEYS (32)

extern DDS_DomainParticipant ospl_dp;
extern DDS_Topic ospl_topic_DCPSTopic;
extern DDS_Subscriber ospl_sub_builtin;
/* $end */

#include <ospl/_type.h>
#include <ospl/_load.h>
#include <ospl/_api.h>

/* $body() */
/* Enter code that requires types here */
/* $end */

#ifdef __cplusplus
extern "C" {
#endif

#define ospl_ddsInit() _ospl_ddsInit()
OSPL_EXPORT
corto_int16 _ospl_ddsInit(void);

#define ospl_fromMetaXml(xml, type, keys) _ospl_fromMetaXml(xml, type, keys)
OSPL_EXPORT
corto_int16 _ospl_fromMetaXml(
    corto_string xml,
    corto_string type,
    corto_string keys);

#define ospl_getKeylist(type) _ospl_getKeylist(corto_struct(type))
OSPL_EXPORT
corto_string _ospl_getKeylist(
    corto_struct type);

#define ospl_registerTopic(topicName, type) _ospl_registerTopic(topicName, corto_type(type))
OSPL_EXPORT
DDS_Topic _ospl_registerTopic(
    corto_string topicName,
    corto_type type);

#define ospl_registerTypeForTopic(topicName) _ospl_registerTypeForTopic(topicName)
OSPL_EXPORT
ospl_DCPSTopic _ospl_registerTypeForTopic(
    corto_string topicName);

#define ospl_toMetaXml(type) _ospl_toMetaXml(corto_struct(type))
OSPL_EXPORT
corto_string _ospl_toMetaXml(
    corto_struct type);

#define ospl_waitForTopic(pattern) _ospl_waitForTopic(pattern)
OSPL_EXPORT
ospl_DCPSTopic _ospl_waitForTopic(
    corto_string pattern);

#include <ospl/Connector.h>
#include <ospl/Connector_Connection.h>
#include <ospl/DCPSTopic.h>

#ifdef __cplusplus
}
#endif

#endif

