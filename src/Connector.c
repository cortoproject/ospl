/* $CORTO_GENERATED
 *
 * Connector.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/ospl.h>

/* $header(ospl/Connector/construct) */
typedef enum ospl_CrudKind {
    Ospl_Create,
    Ospl_Update,
    Ospl_Delete
} ospl_CrudKind;

ospl_CrudKind ospl_DdsToCrudKind(DDS_ViewStateKind vs, DDS_InstanceStateKind is) {
    ospl_CrudKind result;

    if (is == DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE) {
        result = Ospl_Delete;
    } else if(vs == DDS_NEW_VIEW_STATE) {
        result = Ospl_Create;
    } else if(is == DDS_ALIVE_INSTANCE_STATE) {
        result = Ospl_Update;
    }

    return result;
}

corto_object ospl_ConnectorGetObject(ospl_Connector this, corto_string key) {
    corto_object result = corto_lookup(corto_mount(this)->mount, key);
    if (!result) {
        result = corto_declareChild(corto_mount(this)->mount, key, this->type);
    }
    return result;
}

void ospl_connectorOnDataAvailable(ospl_Connector this, DDS_DataReader reader) {
    DDS_sequence sampleSeq = corto_calloc(sizeof(DDS_SampleInfoSeq));
    sampleSeq->_release = FALSE;
    DDS_SampleInfoSeq *infoSeq = DDS_SampleInfoSeq__alloc();
    infoSeq->_release = FALSE;
    corto_uint32 i = 0;
    corto_uint32 sampleSize = ospl_copyOutProgram_getDdsSize(this->program);

    DDS_ReturnCode_t status = DDS_DataReader_take(
        reader,
        sampleSeq,
        infoSeq,
        DDS_LENGTH_UNLIMITED,
        DDS_ANY_SAMPLE_STATE,
        DDS_ANY_VIEW_STATE,
        DDS_ANY_INSTANCE_STATE);
    if ((status != DDS_RETCODE_OK) && (status != DDS_RETCODE_NO_DATA)) {
        corto_error("failed to read from '%s'", this->partitionTopic);
        goto error;
    }

    corto_id key;
    for (i = 0; i < sampleSeq->_length; i++) {
        void *ptr = CORTO_OFFSET(sampleSeq->_buffer, i * sampleSize);
        ospl_copyOutProgram_keyString(this->program, key, ptr);

        corto_object o = ospl_ConnectorGetObject(this, key);

        switch (ospl_DdsToCrudKind(infoSeq->_buffer[i].view_state, infoSeq->_buffer[i].instance_state)) {\
        case Ospl_Create:
        case Ospl_Update:
            if (corto_checkState(o, CORTO_DEFINED)) {
                corto_updateBegin(o);
            }
            ospl_copyOut(this->program, (void**)&o, ptr);
            if (corto_checkState(o, CORTO_DEFINED)) {
                corto_updateEnd(o);
            } else {
                if (corto_define(o)) {
                    corto_error("failed to define '%s' for '%s'",
                        corto_fullpath(NULL, o),
                        this->partitionTopic);
                    goto error;
                }
            }
            break;
        case Ospl_Delete:
            corto_delete(o);
            break;
        }
    }

error:
    return;
}


void* ospl_ConnectorThread(void *arg)
{
    DDS_Duration_t timeout = {1, 0};
    ospl_Connector this = arg;
    ospl_DCPSTopic dcpsTopicSample = NULL;

    corto_trace("[ospl] waiting for topic '%s'", this->topic);

    /* Wait for topic until found or until connector is stopping */
    while (!this->ddsTopic && !this->quit) {
        this->ddsTopic = DDS_DomainParticipant_find_topic(
          ospl_dp,
          this->topic,
          &timeout);
    }

    corto_trace("[ospl] registering type for topic '%s'", this->topic);

    if (!(dcpsTopicSample = ospl_registerTypeForTopic(this->topic))) {
        corto_error("failed to register type for topic '%s': %s",
            this->topic,
            corto_lasterr());
        goto error;
    }

    corto_type src_type = corto_resolve(NULL, dcpsTopicSample->type_name);
    if (!src_type) {
        corto_error("failed to find '%s' after it has been inserted (topic = '%s')",
            dcpsTopicSample->type_name,
            dcpsTopicSample->name);
        goto error;
    }

    /* If destination type hasn't been set, use the topic type */
    if (!this->type) {
        corto_setref(&this->type, src_type);
    }

    corto_trace("[ospl] creating entities for reading '%s'", this->partitionTopic);

    /* Create subscriber for partition */
    DDS_SubscriberQos *qos = DDS_SubscriberQos__alloc();
    qos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
    qos->partition.name._buffer[0] = DDS_string_dup(this->partition);
    qos->partition.name._length = 1;
    qos->partition.name._maximum = 1;
    qos->entity_factory.autoenable_created_entities = TRUE;
    this->ddsSub = DDS_DomainParticipant_create_subscriber(ospl_dp, qos, NULL, DDS_STATUS_MASK_NONE);
    if (!this->ddsSub) {
        corto_error("failed to create subscriber for partition '%s'", this->partition);
        goto error;
    }
    DDS_free(qos);

    /* Create copyout program */
    this->program = ospl_copyOutProgramNew(src_type, this->type, dcpsTopicSample->key_list);
    corto_release(src_type);

    /* Setup listener */
    struct DDS_DataReaderListener listener;
    listener.on_data_available = (void (*)(void *, DDS_DataReader)) ospl_connectorOnDataAvailable;
    listener.on_requested_deadline_missed = NULL;
    listener.on_requested_incompatible_qos = NULL;
    listener.on_sample_rejected = NULL;
    listener.on_liveliness_changed = NULL;
    listener.on_subscription_matched = NULL;
    listener.on_sample_lost = NULL;
    listener.listener_data = this;

    /* Create datareader for topic */
    this->ddsReader = DDS_Subscriber_create_datareader(
        this->ddsSub,
        this->ddsTopic,
        DDS_DATAREADER_QOS_USE_TOPIC_QOS,
        &listener,
        DDS_DATA_AVAILABLE_STATUS);
    if (!this->ddsReader) {
        corto_error("failed to create reader for '%s'", this->partitionTopic);
        goto error;
    }

    /* Cleanup */
    corto_delete(dcpsTopicSample);

    corto_trace("[ospl] listening to '%s'", this->partitionTopic);

error:
    return NULL;
}
/* $end */
corto_int16 _ospl_Connector_construct(
    ospl_Connector this)
{
/* $begin(ospl/Connector/construct) */
    corto_id partition;
    corto_id topic;
    char *dot = NULL;

    if (ospl_ddsInit()) {
        goto error;
    }

    if (!this->partitionTopic || !strlen(this->partitionTopic)) {
        corto_seterr("invalid partitiontopic specified");
        goto error;
    }

    /* If partitionTopic doesn't contain a dot, assume default partition */
    char *ptr;
    if ((ptr = strchr(this->partitionTopic, '.'))) {
        /* Partition may contain dots, so find the last dot */
        do {
            dot = ptr;
        } while ((ptr = strchr(ptr + 1, '.')));
    }

    if (dot) {
        strcpy(partition, this->partitionTopic);
        partition[dot - this->partitionTopic] = '\0';
        strcpy(topic, &partition[dot - this->partitionTopic] + 1);
    } else {
        partition[0] = '\0';
        strcpy(topic, this->partitionTopic);
    }

    corto_setstr(&this->topic, topic);
    corto_setstr(&this->partition, partition);

    if (!corto_mount(this)->mount && corto_checkAttr(this, CORTO_ATTR_SCOPED)) {
        corto_setref(&corto_mount(this)->mount, this);
    }

    /* Start thread for reading */
    this->thread = (corto_word)corto_threadNew(ospl_ConnectorThread, this);

    return corto_mount_construct(this);
error:
    return -1;
/* $end */
}

corto_void _ospl_Connector_destruct(
    ospl_Connector this)
{
/* $begin(ospl/Connector/destruct) */

    this->quit = TRUE;

/* $end */
}
