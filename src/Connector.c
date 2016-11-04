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
        result = corto_declareChild(corto_mount(this)->mount, key, this->dstType);
        corto_claim(result);
    }
    return result;
}

void ospl_connectorOnDataAvailable(ospl_Connector this, DDS_DataReader reader) {
    DDS_sequence sampleSeq = corto_calloc(sizeof(DDS_SampleInfoSeq));
    sampleSeq->_release = FALSE;
    DDS_SampleInfoSeq *infoSeq = DDS_SampleInfoSeq__alloc();
    infoSeq->_release = FALSE;
    corto_uint32 i = 0;
    corto_uint32 sampleSize = ospl_copyProgram_getDdsSize(this->program);

    corto_object prevOwner = corto_setOwner(this);

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

    for (i = 0; i < sampleSeq->_length; i++) {
        void *ptr = CORTO_OFFSET(sampleSeq->_buffer, i * sampleSize);
        corto_id key;
        ospl_copyProgram_keyString(this->program, key, ptr);

        corto_object o = ospl_ConnectorGetObject(this, key);

        /* Only update objects owned by the connector, or if they haven't been
         * defined yet (in which case ConnectorGetObject just created it)
         *
         * If an object is owned by the connector, it means that it has been
         * created by the connector (in a thread that invoked corto_setOwner with
         * the connector). If the connector doesn't own the object, it should
         * not generate an event for it. Typically that is the result of
         * reading back its own write, or a rogue remote writer.
         *
         * Ownership assumes a model where each object has exactly one
         * process that owns it. The infrastructure (DDS) might in reality have
         * multiple processes for redundancy purposes (w/EXCLUSIVE ownership).
         * While outside of the scope of Corto, it doesn't break the mechanism.
         *
         * Even when multiple processes are publishing the same instances (for
         * redundancy purposes) it would be undesirable when process A would
         * overwrite the state of process B.
         */
        if (corto_owned(o)) {
            switch (ospl_DdsToCrudKind(infoSeq->_buffer[i].view_state, infoSeq->_buffer[i].instance_state)) {\
            case Ospl_Create:
            case Ospl_Update:
                if (corto_updateBegin(o)) {
                    corto_error("failed to start updating '%s' for '%s'",
                        corto_fullpath(NULL, o),
                        this->partitionTopic);
                    goto error;
                }

                ospl_copyOut(this->program, (void**)&o, ptr);
                if (corto_updateEnd(o)) {
                    corto_error("failed to update '%s' for '%s'",
                        corto_fullpath(NULL, o),
                        this->partitionTopic);
                    goto error;
                }

                break;
            case Ospl_Delete:
                corto_delete(o);
                break;
            }
        }

        corto_release(o);
    }

    status = DDS_DataReader_return_loan(reader, sampleSeq, infoSeq);
    if (status) {
        corto_error("failed to return loan for '%s'", this->partitionTopic);
        goto error;
    }
    corto_dealloc(sampleSeq);
    DDS_free(infoSeq);

error:
    corto_setOwner(prevOwner);
    return;
}

/* Create copy program */
corto_int16 ospl_ConnectorInitProgram(
    ospl_Connector this,
    corto_type dstType,
    corto_type srcType,
    corto_string keys)
{
    this->program = ospl_copyProgramNew(ospl_actualType(srcType), ospl_actualType(dstType), keys);
    return this->program == NULL ? -1 : 0;
}

/* Setup connector to use existing topic */
corto_int16 ospl_ConnectorInitializeExistingTopic(ospl_Connector this, DDS_Topic topic)
{
    ospl_DCPSTopic dcpsTopicSample = NULL;

    corto_lock(this);
    if (this->ddsTopic) {
        goto alreadyInitialized;
    }

    if (!(dcpsTopicSample = ospl_registerTypeForTopic(this->topic, this->keys))) {
        corto_error("failed to register type for topic '%s': %s",
            this->topic,
            corto_lasterr());
        goto error;
    }

    corto_type srcType = corto_resolve(NULL, dcpsTopicSample->type_name);
    if (!srcType) {
        corto_error("failed to find '%s' after it has been inserted (topic = '%s')",
            dcpsTopicSample->type_name,
            dcpsTopicSample->name);
        goto error;
    }

    /* If destination type hasn't been set, use the topic type */
    if (!corto_observer(this)->type) {
        corto_id src_typeId;
        corto_fullpath(src_typeId, srcType);
        corto_setstr(&corto_observer(this)->type, src_typeId);
    }

    /* If destination keylist hasn't been set, use the topic type */
    if (!this->keys) {
        corto_setstr(&this->keys, dcpsTopicSample->key_list);
    }

    this->dstType = corto_resolve(NULL, corto_observer(this)->type);
    if (!this->dstType) {
        corto_error("failed to find type '%s' for topic '%s'",
            corto_observer(this)->type,
            dcpsTopicSample->name);
        goto error;
    }

    if (ospl_ConnectorInitProgram(this, this->dstType, srcType, dcpsTopicSample->key_list)) {
        goto error;
    }
    corto_release(srcType);
    corto_delete(dcpsTopicSample);

    this->ddsTopic = topic;

    corto_unlock(this);

alreadyInitialized:
    return 0;
error:
    return -1;
}

/* Setup connector with new topic */
corto_int16 ospl_ConnectorInitializeNewTopic(ospl_Connector this) {
    corto_type t = corto_resolve(NULL, corto_observer(this)->type);
    if (!t) {
        corto_seterr("could not resolve type '%s' for topic '%s'",
          corto_observer(this)->type,
          this->topic);
        goto error;
    }

    corto_trace("ospl: registering type for new topic '%s'", this->topic);

    corto_lock(this);

    if (this->ddsTopic) {
        goto alreadyInitialized;
    }

    corto_trace("ospl: topic '%s' already initialized by connector thread", this->topic);

    DDS_Topic topic = ospl_registerTopic(this->topic, t, this->keys);
    if (topic) {
        /* If inserting new topic, destination and source type are the same */
        if (ospl_ConnectorInitProgram(this, t, t, this->keys)) {
            goto error;
        }
        this->dstType = t;
    } else {
        corto_unlock(this);
        goto error;
    }

    this->ddsTopic = topic;

    corto_unlock(this);

alreadyInitialized:
    return 0;
error:
    return -1;
}

corto_int16 ospl_ConnectorCreateWriter(ospl_Connector this) {
    /* Lock object, to prevent connector thread and this thread from inserting
     * type and topic at the same time */
    DDS_Duration_t timeout = {0, 100000000};

    /* Try to find topic. The type provided to the connector could be a
     * projection type and not the actual type that is available in DDS.
     * Discovering the type upfront will insert the actual type in
     * corto, after which the correct conversion program can be created.
     *
     * If no existing type is available in DDS at the time the application wants
     * to start writing, insert the type known to the connector. */
    DDS_Topic topic = DDS_DomainParticipant_find_topic(
      ospl_dp,
      this->topic,
      &timeout);

    if (!topic) {
        if (ospl_ConnectorInitializeNewTopic(this)) {
            goto error;
        }
    } else {
        /* If a topic is found, the connector thread must've seen it as well and
         * is should insert the topic. Wait until the topic has been
         * created */
        while (!this->ddsTopic && !this->quit) {
            corto_trace("waiting for topic '%s' to be initialized", this->topic);
            corto_sleep(0, 100000000);
        }
        if (!this->ddsTopic) {
            /* Thread is quitting, exit */
            goto error;
        }
    }

    if (this->ddsTopic) {
        corto_trace("ospl: creating entities for writing '%s'", this->partitionTopic);

        /* Create subscriber for partition */
        DDS_PublisherQos *qos = DDS_PublisherQos__alloc();
        qos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
        qos->partition.name._buffer[0] = DDS_string_dup(this->partition);
        qos->partition.name._length = 1;
        qos->partition.name._maximum = 1;
        qos->entity_factory.autoenable_created_entities = TRUE;
        this->ddsPub = DDS_DomainParticipant_create_publisher(ospl_dp, qos, NULL, DDS_STATUS_MASK_NONE);
        if (!this->ddsPub) {
            corto_error("failed to create publisher for partition '%s'", this->partition);
            goto error;
        }
        DDS_free(qos);

        /* Create datawriter for topic */
        this->ddsWriter = DDS_Publisher_create_datawriter(
            this->ddsPub,
            this->ddsTopic,
            DDS_DATAWRITER_QOS_USE_TOPIC_QOS,
            NULL,
            0);
        if (!this->ddsWriter) {
            corto_error("failed to create writer for '%s'", this->partitionTopic);
            goto error;
        }

        corto_ok("ospl: ready to write '%s'", this->partitionTopic);
    } else {
        goto error;
    }

    return 0;
error:
    return -1;
}

void* ospl_ConnectorThread(void *arg)
{
    ospl_Connector this = arg;
    DDS_Topic topic = NULL;
    DDS_Duration_t timeout = {1, 0};

    corto_trace("ospl: waiting for topic '%s'", this->topic);

    /* Wait for topic until found or until connector is stopping */
    while (!topic && !this->quit) {
        topic = DDS_DomainParticipant_find_topic(
          ospl_dp,
          this->topic,
          &timeout);
    }

    if (!topic) {
        /* Stopping connector, exit thread */
        goto error;
    }

    /* Initialize connector with existing topic */
    if (ospl_ConnectorInitializeExistingTopic(this, topic)) {
        goto error;
    }

    corto_trace("ospl: creating entities for reading '%s'", this->partitionTopic);

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

    corto_ok("ospl: listening to '%s'", this->partitionTopic);

error:
    return NULL;
}

void* ospl_ConnectorMatcherThread(void *arg)
{
    ospl_Connector this = arg;
    ospl_DCPSTopic dcpsTopicSample = NULL;

    corto_trace("ospl: monitor topics that match '%s'", this->topic);

    while ((dcpsTopicSample = ospl_waitForTopic(this->topic))) {
        corto_id topic;

        if (this->partition) {
            sprintf(topic, "%s.%s", this->partition, dcpsTopicSample->name);
        } else {
            sprintf(topic, "%s", dcpsTopicSample->name);
        }
        corto_trace("ospl: creating connector for '%s'", topic);

        ospl_ConnectorCreateChild(
            this,                  /* create connector in root connector */
            dcpsTopicSample->name, /* name of topic */
            NULL,                  /* store instances in scope of connector */
            NULL,                  /* discover type from DDS*/
            NULL,                  /* default policy */
            topic,                 /* topic */
            NULL                   /* discover keylist from DDS */
        );

        corto_ok("ospl: created connector for '%s'", topic);

        corto_delete(dcpsTopicSample);
    }

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
        corto_setstr(&this->partitionTopic, "*.*");
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
    corto_trace("ospl: start connector for %s.%s", this->topic, this->partition);

    /* Start thread for reading */
    if (strchr(this->topic, '*') || strchr(this->topic, '?')) {
        this->thread = (corto_word)corto_threadNew(ospl_ConnectorMatcherThread, this);
    } else {
        this->thread = (corto_word)corto_threadNew(ospl_ConnectorThread, this);
    }

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

corto_void _ospl_Connector_onDelete(
    ospl_Connector this,
    corto_object observable)
{
/* $begin(ospl/Connector/onDelete) */
    if (!this->ddsWriter) {
        if (ospl_ConnectorCreateWriter(this)) {
            if (!this->quit) {
                corto_error("error: %s", corto_lasterr());
            }
            return;
        }
    }

    corto_assert(this->ddsWriter != NULL, "writer should exist");

    void *sample = ospl_copyAlloc(this->program);
    ospl_copyIn(this->program, sample, observable);

    DDS_ReturnCode_t status = DDS_DataWriter_dispose(this->ddsWriter, sample, DDS_HANDLE_NIL);
    if ((status != DDS_RETCODE_OK) && (status != DDS_RETCODE_NO_DATA)) {
        corto_error("failed to write to '%s'", this->partitionTopic);
    }

    ospl_copyFree(this->program, sample);

/* $end */
}

corto_void _ospl_Connector_onUpdate(
    ospl_Connector this,
    corto_object observable)
{
/* $begin(ospl/Connector/onUpdate) */
    /* If this is a wildcard connector, don't do anything */
    if (this->topic && strchr(this->topic, '*')) {
        return;
    }

    if (!this->ddsWriter) {
        if (ospl_ConnectorCreateWriter(this)) {
            if (!this->quit) {
                corto_error("error: %s", corto_lasterr());
            }
            return;
        }
    }

    corto_assert(this->ddsWriter != NULL, "writer should exist");

    void *sample = ospl_copyAlloc(this->program);
    ospl_copyIn(this->program, sample, observable);

    DDS_ReturnCode_t status = DDS_DataWriter_write(this->ddsWriter, sample, DDS_HANDLE_NIL);
    if ((status != DDS_RETCODE_OK) && (status != DDS_RETCODE_NO_DATA)) {
        corto_error("failed to write to '%s'", this->partitionTopic);
    }

    ospl_copyFree(this->program, sample);

/* $end */
}
