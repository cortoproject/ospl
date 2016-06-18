/* $CORTO_GENERATED
 *
 * ospl.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/ospl.h>

/* $header() */
#include "ospl/MetaXml.h"
#include "ospl/CopyOut.h"

DDS_DomainParticipant ospl_dp;
DDS_Topic ospl_topic_DCPSTopic;
DDS_Subscriber ospl_sub_builtin;
DDS_DataReader ospl_reader_DCPSTopic;
ospl_copyOutProgram ospl_copyout_DCPSTopic;

static corto_bool ospl_init = FALSE;
/* $end */

/* $header(ospl/ddsInit) */
static corto_int16 ospl_loadXml(
    corto_string name,
    corto_string keys,
    corto_string file)
{
    corto_string xmlFile;
    corto_asprintf(&xmlFile, "%s/%s", OSPL_ETC, file);

    corto_string xml = corto_fileLoad(xmlFile);
    if (!xml) {
        corto_seterr("failed to load '%s'", xmlFile);
        goto error;
    }

    /* Inject type into corto */
    if (ospl_fromMetaXml(xml)) {
        corto_seterr("can't parse '%s': %s",
            xmlFile,
            corto_lasterr());
        goto error;
    }

    /* Inject type into DDS */
    DDS_TypeSupport ts = DDS_TypeSupport__alloc(
        name,
        keys,
        xml);

    DDS_ReturnCode_t result = DDS__FooTypeSupport_register_type(ts, ospl_dp, name);
    if(result != DDS_RETCODE_OK) {
        corto_seterr(
          "failed to inject type '%s'", name);
        goto error;
    }

    corto_dealloc(xml);

    /* Create program for copying data from DDS v_topicInfo to corto */
    corto_object type = corto_resolve(NULL, "kernelModule/v_topicInfo");
    if (!type) {
        corto_seterr("failed to resolve 'kernelModule/v_topicInfo'");
        goto error;
    }
    ospl_copyout_DCPSTopic = ospl_copyOutProgramNew(type, ospl_DCPSTopic_o, keys);

    return 0;
error:
    return -1;
}
/* $end */
corto_int16 _ospl_ddsInit(void)
{
/* $begin(ospl/ddsInit) */
    if (ospl_init) {
        return 0;
    }

    /* Not thread safe */
    ospl_init = TRUE;

    corto_trace("[ospl] creating DomainParticipant");
    ospl_dp = DDS_DomainParticipantFactory_create_participant(
        DDS_TheParticipantFactory,
        0,
        DDS_PARTICIPANT_QOS_DEFAULT,
        NULL,
        DDS_STATUS_MASK_NONE);
    if(!ospl_dp) {
        corto_error("[ospl] create_participant failed for domain 0");
        goto error;
    }

    corto_trace("[ospl] injecting DCPSTopic type");
    if (ospl_loadXml("kernelModule::v_topicInfo", "key.localId,key.systemId", "DCPSTopicType.xml")) {
        corto_error("[ospl] %s", corto_lasterr());
        goto error;
    }

    corto_trace("[ospl] creating DCPSTopic proxy");
    DDS_Duration_t timeout = {10, 0};
    ospl_topic_DCPSTopic = DDS_DomainParticipant_find_topic(ospl_dp, "DCPSTopic", &timeout);
    if (!ospl_topic_DCPSTopic) {
        corto_error("[ospl] couldn't find DCPSTopic (timeout = 10 seconds)");
        goto error;
    }

    corto_trace("[ospl] obtaining built-in subscriber");
    ospl_sub_builtin = DDS_DomainParticipant_get_builtin_subscriber(ospl_dp);
    if (!ospl_sub_builtin) {
        corto_error("[ospl] couldn't obtain builtin subscriber");
        goto error;
    }

    corto_trace("[ospl] creating reader for DCPSTopic");
    ospl_reader_DCPSTopic = DDS_Subscriber_create_datareader(
        ospl_sub_builtin,
        ospl_topic_DCPSTopic,
        DDS_DATAREADER_QOS_USE_TOPIC_QOS,
        NULL,
        DDS_STATUS_MASK_NONE);
    if (!ospl_reader_DCPSTopic) {
        corto_error("[ospl] failed to create DCPSTopic reader");
        goto error;
    }

    corto_trace("[ospl] initialized");

    return 0;
error:
    return -1;
/* $end */
}

corto_int16 _ospl_fromMetaXml(
    corto_string xml)
{
/* $begin(ospl/fromMetaXml) */

    if (ospl_metaXmlParse(xml)) {
        goto error;
    }

    return 0;
error:
    return -1;
/* $end */
}

ospl_DCPSTopic _ospl_registerTypeForTopic(
    corto_string topicName)
{
/* $begin(ospl/registerTypeForTopic) */
    DDS_sequence sampleSeq = corto_calloc(sizeof(DDS_SampleInfoSeq));
    sampleSeq->_release = FALSE;
    DDS_SampleInfoSeq *infoSeq = DDS_SampleInfoSeq__alloc();
    infoSeq->_release = FALSE;
    corto_uint32 i = 0, tries = 0;

    ospl_DCPSTopic sample = ospl_DCPSTopicCreate(NULL, NULL, NULL, NULL);
    ospl_DCPSTopic result = NULL;

    /* Try at most 10 times */
    do {
        DDS_ReturnCode_t status = DDS_DataReader_read(
            ospl_reader_DCPSTopic,
            sampleSeq,
            infoSeq,
            DDS_LENGTH_UNLIMITED,
            DDS_ANY_SAMPLE_STATE,
            DDS_ANY_VIEW_STATE,
            DDS_ALIVE_INSTANCE_STATE);
        if (status) {
            corto_seterr("failed to read from DCPSTopic");
            goto error;
        }

        for (i = 0; i < sampleSeq->_length; i++) {
            void *ddsSample = CORTO_OFFSET(
                sampleSeq->_buffer,
                i * ospl_copyOutProgram_getDdsSize(ospl_copyout_DCPSTopic));

            /* Copy out sample */
            ospl_copyOut(ospl_copyout_DCPSTopic, (void**)&sample, ddsSample);

            /* Check if this is the sample that matches the topic */
            if (!strcmp(sample->name, topicName)) {

                /* Inject type into corto */
                if (ospl_fromMetaXml(sample->meta_data)) {
                    corto_seterr("can't inject metadata for '%s': %s",
                        sample->name,
                        corto_lasterr());
                    goto error;
                }

                /* Register type with DDS */
                DDS_TypeSupport ts = DDS_TypeSupport__alloc(
                    sample->type_name,
                    sample->key_list,
                    sample->meta_data);

                DDS_ReturnCode_t status = DDS__FooTypeSupport_register_type(ts, ospl_dp, sample->type_name);
                if(status != DDS_RETCODE_OK) {
                    corto_seterr(
                      "failed to inject type '%s'", sample->name);
                    goto error;
                }
                result = sample;
                break;
            }
        }

        DDS_DataReader_return_loan(ospl_reader_DCPSTopic, sampleSeq, infoSeq);
        tries ++;

        if (!result) {
            corto_sleep(1, 0);
        }
    } while ((tries < 10) && (!result));

    corto_dealloc(sampleSeq);
    DDS_free(infoSeq);

    return result;
error:
    return NULL;
/* $end */
}

corto_string _ospl_toMetaXml(
    corto_struct type)
{
/* $begin(ospl/toMetaXml) */

    return ospl_metaXmlGet(corto_type(type));
/* $end */
}

int osplMain(int argc, char* argv[]) {
/* $begin(main) */
    return 0;
/* $end */
}
