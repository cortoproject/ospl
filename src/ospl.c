/* $CORTO_GENERATED
 *
 * ospl.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/ospl.h>

/* $header() */
DDS_DomainParticipant ospl_dp;
DDS_Topic ospl_topic_DCPSTopic;
DDS_Subscriber ospl_sub_builtin;
DDS_DataReader ospl_reader_DCPSTopic;
DDS_DataReader ospl_reader_DCPSTopicMon;
DDS_ReadCondition ospl_readCondition_DCPSTopic;
DDS_WaitSet ospl_waitSet_DCPSTopic;
ospl_copyProgram ospl_copyout_DCPSTopic;

static corto_bool ospl_init = FALSE;
static corto_uint8 OSPL_KEY_ANNOTATE;

typedef struct ospl_annotation {
    corto_bool optional;
    corto_object actualType;
} ospl_annotation;

static void ospl_annotationFree(void *o) {
    corto_dealloc(o);
}

static ospl_annotation* ospl_annotationGet(corto_object o, corto_bool create) {
    ospl_annotation *result = corto_olsGet(o, OSPL_KEY_ANNOTATE);
    if (!result && create) {
        result = corto_calloc(sizeof(ospl_annotation));
        corto_olsSet(o, OSPL_KEY_ANNOTATE, result);
    }
    return result;
}

/* $end */

corto_type _ospl_actualType(
    corto_object type)
{
/* $begin(ospl/actualType) */
    corto_type result;

    if (corto_instanceof(ospl_Typedef_o, type)) {
        result = ospl_Typedef_actualType(type);
    } else {
        result = type;
    }

    return corto_type(result);
/* $end */
}

corto_void _ospl_annotateActualType(
    corto_object o,
    corto_object t)
{
/* $begin(ospl/annotateActualType) */
    ospl_annotation  *a = ospl_annotationGet(o, TRUE);
    a->actualType = t;
/* $end */
}

corto_object _ospl_annotateGetActualType(
    corto_object o)
{
/* $begin(ospl/annotateGetActualType) */
    ospl_annotation  *a = ospl_annotationGet(o, FALSE);
    if (!a) {
        return NULL;
    }
    return a->actualType;
/* $end */
}

corto_bool _ospl_annotateGetOptional(
    corto_object o)
{
/* $begin(ospl/annotateGetOptional) */
    ospl_annotation  *a = ospl_annotationGet(o, FALSE);
    if (!a) {
        return FALSE;
    }
    return a->optional;
/* $end */
}

corto_void _ospl_annotateOptional(
    corto_object o,
    corto_bool optional)
{
/* $begin(ospl/annotateOptional) */
    ospl_annotation  *a = ospl_annotationGet(o, TRUE);
    a->optional = optional;
/* $end */
}

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
    ospl_copyout_DCPSTopic = ospl_copyProgramNew(type, ospl_DCPSTopic_o, keys);

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

    corto_trace("ospl: creating DomainParticipant");
    ospl_dp = DDS_DomainParticipantFactory_create_participant(
        DDS_TheParticipantFactory,
        *ospl_domainId_o,
        DDS_PARTICIPANT_QOS_DEFAULT,
        NULL,
        DDS_STATUS_MASK_NONE);
    if(!ospl_dp) {
        corto_error("ospl: create_participant failed for domain 0");
        goto error;
    }

    if (ospl_loadXml("kernelModule::v_topicInfo", "key.localId,key.systemId", "DCPSTopicType.xml")) {
        corto_error("ospl: %s", corto_lasterr());
        goto error;
    }

    DDS_Duration_t timeout = {10, 0};
    ospl_topic_DCPSTopic = DDS_DomainParticipant_find_topic(ospl_dp, "DCPSTopic", &timeout);
    if (!ospl_topic_DCPSTopic) {
        corto_error("ospl: couldn't find DCPSTopic (timeout = 10 seconds)");
        goto error;
    }

    ospl_sub_builtin = DDS_DomainParticipant_get_builtin_subscriber(ospl_dp);
    if (!ospl_sub_builtin) {
        corto_error("ospl: couldn't obtain builtin subscriber");
        goto error;
    }

    corto_trace("ospl: creating reader for DCPSTopic");
    ospl_reader_DCPSTopic = DDS_Subscriber_create_datareader(
        ospl_sub_builtin,
        ospl_topic_DCPSTopic,
        DDS_DATAREADER_QOS_USE_TOPIC_QOS,
        NULL,
        DDS_STATUS_MASK_NONE);
    if (!ospl_reader_DCPSTopic) {
        corto_error("ospl: failed to create DCPSTopic reader");
        goto error;
    }
    ospl_reader_DCPSTopicMon = DDS_Subscriber_create_datareader(
        ospl_sub_builtin,
        ospl_topic_DCPSTopic,
        DDS_DATAREADER_QOS_USE_TOPIC_QOS,
        NULL,
        DDS_STATUS_MASK_NONE);
    if (!ospl_reader_DCPSTopic) {
        corto_error("ospl: failed to create DCPSTopicMon reader");
        goto error;
    }

    corto_trace("ospl: create readcondition for DCPSTopic");
    ospl_readCondition_DCPSTopic = DDS_DataReader_create_readcondition(
        ospl_reader_DCPSTopicMon,
        DDS_NOT_READ_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE,
        DDS_ALIVE_INSTANCE_STATE);
    if (!ospl_readCondition_DCPSTopic) {
      corto_error("ospl: failed to create DCPSTopic readcondition");
      goto error;
    }

    corto_trace("ospl: create waitset for DCPSTopic");
    ospl_waitSet_DCPSTopic = DDS_WaitSet__alloc();
    if (DDS_WaitSet_attach_condition(ospl_waitSet_DCPSTopic, ospl_readCondition_DCPSTopic) != DDS_RETCODE_OK) {
        corto_error("ospl: failed to create DCPSTopic waitset");
        goto error;
    }

    corto_ok("ospl: connected to domain %d (%s)",
        *ospl_domainId_o,
        *ospl_domainName_o);

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

DDS_Topic _ospl_registerTopic(
    corto_string topicName,
    corto_type type,
    corto_string keys)
{
/* $begin(ospl/registerTopic) */
    corto_id typeName;
    DDS_Topic topic = NULL;

    corto_trace("ospl: registering topic '%s' with type '%s' and keys '%s'",
        topicName, type, keys);

    /* Get metadescriptor */
    corto_string xml = ospl_toMetaXml(type);

    /* Create typename */
    corto_path(typeName, root_o, type, "::");

    /* Obtain typesupport */
    DDS_TypeSupport ts = DDS_TypeSupport__alloc(typeName, keys, xml);

    /* Inject type */
    DDS_ReturnCode_t result = DDS__FooTypeSupport_register_type(ts, ospl_dp, typeName);
    if (result != DDS_RETCODE_OK) {
        corto_seterr("failed to inject type '%s'", typeName);
        goto error;
    }

    /* Create topic */
    topic = DDS_DomainParticipant_create_topic(
        ospl_dp,
        topicName,
        typeName,
        DDS_TOPIC_QOS_DEFAULT,
        NULL,
        0);
    if (!topic){
        corto_seterr(
            "failed to create topic '%s' with type %s)", topicName, typeName);
        goto error;
    }

    return topic;
error:
    return NULL;
/* $end */
}

ospl_DCPSTopic _ospl_registerTypeForTopic(
    corto_string topicName,
    corto_string keys)
{
/* $begin(ospl/registerTypeForTopic) */
    DDS_sequence sampleSeq = corto_calloc(sizeof(DDS_SampleInfoSeq));
    DDS_SampleInfoSeq *infoSeq = DDS_SampleInfoSeq__alloc();
    infoSeq->_release = FALSE;
    corto_uint32 i = 0, tries = 0;

    ospl_DCPSTopic sample = ospl_DCPSTopicCreate(NULL, NULL, NULL, NULL);
    ospl_DCPSTopic result = NULL;

    /* Try at most 10 times. This function should only be called after
     * find_topic has successfully returned. */
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
                i * ospl_copyProgram_getDdsSize(ospl_copyout_DCPSTopic));

            /* Copy out sample */
            ospl_copyOut(ospl_copyout_DCPSTopic, (void**)&sample, ddsSample);

            /* Check if this is the sample that matches the topic */
            if (!strcmp(sample->name, topicName)) {

                corto_trace("ospl: found topic '%s' with type '%s' and keys '%s'",
                    topicName, sample->type_name, sample->key_list);

                /* Validate that keys are same as what is requested */
                if (keys && strcmp(keys, sample->key_list)) {
                    corto_seterr("requested keys ('%s') for topic '%s' do not match discovered keys ('%s')",
                      keys,
                      topicName,
                      sample->key_list);
                    goto error;
                }

                /* Try to load type first from package repository. This ensures
                 * that if the corto type differs from the DDS type, a consistent
                 * mapping is used across applications */
                corto_object t = corto_resolve(NULL, sample->type_name);
                if (!t) {
                    corto_trace("ospl: type '%s' not found in package repo, loading from DDS", sample->type_name);

                    /* Not a problem, inject the type from DDS */
                    if (ospl_fromMetaXml(sample->meta_data)) {
                        corto_seterr("can't inject metadata for '%s': %s",
                            sample->name,
                            corto_lasterr());
                        goto error;
                    }
                } else {
                    /* Ensure that the type is defined */
                    while (!corto_checkState(t, CORTO_DEFINED)) {
                        corto_sleep(0, 100000000);
                    }

                    /* The type has been loaded from the package repository. The
                     * DDS type will be mapped to this type */
                    corto_release(t);
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

ospl_DCPSTopic _ospl_waitForTopic(
    corto_string pattern)
{
/* $begin(ospl/waitForTopic) */
    DDS_Duration_t timeout = DDS_DURATION_INFINITE;
    DDS_ConditionSeq *guardSeq = DDS_ConditionSeq__alloc();
    DDS_sequence sampleSeq = corto_calloc(sizeof(DDS_SampleInfoSeq));
    DDS_SampleInfoSeq *infoSeq = DDS_SampleInfoSeq__alloc();
    DDS_ReturnCode_t status;

    status = DDS_WaitSet_wait(ospl_waitSet_DCPSTopic, guardSeq, &timeout);
    if (status != DDS_RETCODE_OK) {
        corto_seterr("failed to wait for DCPSTopic");
        goto error;
    }

    /* Read one sample at a time */
    ospl_DCPSTopic sample = ospl_DCPSTopicCreate(NULL, NULL, NULL, NULL);
    do {
        DDS_ReturnCode_t status = DDS_DataReader_read(
            ospl_reader_DCPSTopicMon,
            sampleSeq,
            infoSeq,
            1,
            DDS_NOT_READ_SAMPLE_STATE,
            DDS_NEW_VIEW_STATE,
            DDS_ALIVE_INSTANCE_STATE);
        if (status) {
            corto_seterr("failed to read from DCPSTopic");
            goto error;
        }

        ospl_copyOut(ospl_copyout_DCPSTopic, (void**)&sample, sampleSeq->_buffer);
        DDS_DataReader_return_loan(ospl_reader_DCPSTopic, sampleSeq, infoSeq);
    } while (fnmatch(pattern, sample->name, 0));

    DDS_free(guardSeq);
    corto_dealloc(sampleSeq);
    DDS_free(infoSeq);

    return sample;
error:
    return NULL;
/* $end */
}

int osplMain(int argc, char* argv[]) {
/* $begin(main) */
    char *uri = corto_getenv("OSPL_URI");
    corto_setstr(ospl_uri_o, uri);

    OSPL_KEY_ANNOTATE = corto_olsKey(ospl_annotationFree);

    /* Parse configuration to obtain domainId */
    if (uri) {
        char *xml = corto_fileLoad(uri + strlen("file://"));
        corto_xmlreader reader = corto_xmlMemoryReaderNew(xml, "OpenSplice");
        corto_xmlnode root = corto_xmlreaderRoot(reader);
        corto_xmlnode domainNode = corto_xmlnodeFind(root, "Domain");
        corto_xmlnode name = corto_xmlnodeFind(domainNode, "Name");
        corto_xmlnode Id = corto_xmlnodeFind(domainNode, "Id");
        corto_xmlnode SP = corto_xmlnodeFind(domainNode, "SingleProcess");
        if (name) {
            corto_setstr(ospl_domainName_o, corto_xmlnodeStr(name));
            *ospl_domainId_o = atoi(corto_xmlnodeStr(Id));
            *ospl_singleProcess_o = !strcmp(corto_xmlnodeStr(SP), "true");
        }
        corto_xmlreaderFree(reader);
    }

    /* Create corto package loader so that connectors can load types from
     * package repository */
    corto_loaderCreate();

    return 0;
/* $end */
}
