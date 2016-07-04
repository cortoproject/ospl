/* ospl.h
 *
 * This file contains generated code. Do not modify!
 */

#ifndef OSPL_H
#define OSPL_H

#include <corto/corto.h>
#include <ospl/_interface.h>
/* $header() */
#include "dds_dcps.h"
#include <ospl/Copy.h>
#include <ospl/MetaXml.h>

#define OSPL_MAX_KEYS (32)

extern DDS_DomainParticipant ospl_dp;
extern DDS_Topic ospl_topic_DCPSTopic;
extern DDS_Subscriber ospl_sub_builtin;
/* $end */

#include <corto/fmt/xml/xml.h>

#include <ospl/_type.h>
#include <ospl/_api.h>
#include <ospl/_meta.h>

#ifdef __cplusplus
extern "C" {
#endif


OSPL_EXPORT corto_type _ospl_actualType(
    corto_object type);
#define ospl_actualType(type) _ospl_actualType(type)

OSPL_EXPORT corto_void _ospl_annotateActualType(
    corto_object o,
    corto_object t);
#define ospl_annotateActualType(o, t) _ospl_annotateActualType(o, t)

OSPL_EXPORT corto_object _ospl_annotateGetActualType(
    corto_object o);
#define ospl_annotateGetActualType(o) _ospl_annotateGetActualType(o)

OSPL_EXPORT corto_bool _ospl_annotateGetOptional(
    corto_object o);
#define ospl_annotateGetOptional(o) _ospl_annotateGetOptional(o)

OSPL_EXPORT corto_void _ospl_annotateOptional(
    corto_object o,
    corto_bool optional);
#define ospl_annotateOptional(o, optional) _ospl_annotateOptional(o, optional)

OSPL_EXPORT corto_int16 _ospl_ddsInit(void);
#define ospl_ddsInit() _ospl_ddsInit()

OSPL_EXPORT corto_int16 _ospl_fromMetaXml(
    corto_string xml);
#define ospl_fromMetaXml(xml) _ospl_fromMetaXml(xml)

OSPL_EXPORT DDS_Topic _ospl_registerTopic(
    corto_string topicName,
    corto_type type,
    corto_string keys);
#define ospl_registerTopic(topicName, type, keys) _ospl_registerTopic(topicName, corto_type(type), keys)

OSPL_EXPORT ospl_DCPSTopic _ospl_registerTypeForTopic(
    corto_string topicName,
    corto_string keys);
#define ospl_registerTypeForTopic(topicName, keys) _ospl_registerTypeForTopic(topicName, keys)

OSPL_EXPORT corto_string _ospl_toMetaXml(
    corto_struct type);
#define ospl_toMetaXml(type) _ospl_toMetaXml(corto_struct(type))
#include <ospl/BaseType.h>
#include <ospl/Connector.h>
#include <ospl/DCPSTopic.h>
#include <ospl/Member.h>
#include <ospl/Typedef.h>

#ifdef __cplusplus
}
#endif
#endif

