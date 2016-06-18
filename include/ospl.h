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


OSPL_EXPORT corto_int16 _ospl_ddsInit(void);
#define ospl_ddsInit() _ospl_ddsInit()

OSPL_EXPORT corto_int16 _ospl_fromMetaXml(
    corto_string xml);
#define ospl_fromMetaXml(xml) _ospl_fromMetaXml(xml)

OSPL_EXPORT corto_int16 _ospl_registerTypeForTopic(
    corto_string topicName);
#define ospl_registerTypeForTopic(topicName) _ospl_registerTypeForTopic(topicName)

OSPL_EXPORT corto_string _ospl_toMetaXml(
    corto_struct type);
#define ospl_toMetaXml(type) _ospl_toMetaXml(corto_struct(type))
#include <ospl/Connector.h>
#include <ospl/DCPSTopic.h>
#include <ospl/Typedef.h>

#ifdef __cplusplus
}
#endif
#endif

