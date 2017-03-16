/* $CORTO_GENERATED
 *
 * Connector_Connection.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/ospl.h>

/* $header() */
#define TOPIC_NAME(c) corto_path(NULL, corto_mount(c->rootConnector)->mount, c->topic, "_")
#define PARTITION_NAME(c) (c->rootConnector->partition[0] ? c->rootConnector->partition : "<DEFAULT>")
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

corto_object ospl_ConnectorGetObject(ospl_Connector_Connection this, corto_string key) {
    corto_object result = corto_lookup(corto_mount(this)->mount, key);
    if (!result) {
        result = corto_declareChild(corto_mount(this)->mount, key, this->topic->type);
        if (result) {
            corto_claim(result);
        } else {
            corto_error("ospl: failed to declare '%s' for '%s.%s': %s", 
                key, 
                PARTITION_NAME(this),
                TOPIC_NAME(this),
                corto_lasterr());
        }
    }
    return result;
}

void ospl_connectionOnDataAvailable(ospl_Connector_Connection this, DDS_DataReader reader) {
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
        corto_error("failed to read from '%s.%s'", 
            PARTITION_NAME(this),
            TOPIC_NAME(this));
        goto error;
    }

    for (i = 0; i < sampleSeq->_length; i++) {
        void *ptr = CORTO_OFFSET(sampleSeq->_buffer, i * sampleSize);
        corto_id key;
        ospl_copyProgram_keyString(this->program, key, ptr);

        corto_object o = ospl_ConnectorGetObject(this, key);
        if (!o) {
            goto error;
        }

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
                    corto_error("failed to start updating '%s' for '%s.%s'",
                        corto_fullpath(NULL, o),
                        PARTITION_NAME(this),
                        TOPIC_NAME(this));
                    goto error;
                }

                ospl_copyOut(this->program, (void**)&o, ptr);
                if (corto_updateEnd(o)) {
                    corto_error("failed to update '%s' for '%s.%s'",
                        corto_fullpath(NULL, o),
                        PARTITION_NAME(this),
                        TOPIC_NAME(this));
                    goto error;
                }

                break;
            case Ospl_Delete:
                if (corto_delete(o)) {
                    corto_error("failed to delete '%s' for '%s.%s'",
                        corto_fullpath(NULL, o),
                        PARTITION_NAME(this),
                        TOPIC_NAME(this));
                    goto error;
                }
                break;
            }
        }

        corto_release(o);
    }

    status = DDS_DataReader_return_loan(reader, sampleSeq, infoSeq);
    if (status) {
        corto_error("failed to return loan for '%s.%s'",
            PARTITION_NAME(this),
            TOPIC_NAME(this));
        goto error;
    }
    corto_dealloc(sampleSeq);
    DDS_free(infoSeq);

error:
    corto_setOwner(prevOwner);
    return;
}

/* Create copyin-copyout program */
corto_int16 ospl_Connection_initProgram(
    ospl_Connector_Connection this,
    corto_type dstType,
    corto_type srcType)
{
    this->program = ospl_copyProgramNew(idl_actualType(srcType), idl_actualType(dstType));
    return this->program == NULL ? -1 : 0;
}

/* Setup connection to existing topic */
DDS_Topic ospl_Connection_setupTopic(
    ospl_Connector_Connection this, 
    char *topicName,
    corto_bool wait)
{
    ospl_DCPSTopic dcpsTopicSample = NULL;
    DDS_Topic topic = NULL;
    DDS_Duration_t timeout = {0, 100000000};

    if (!wait && !corto_checkState(this->topic, CORTO_DEFINED)) {
        corto_error("cannot create new topic '%s' without a type",
            topicName);
        goto error;
    }

    if (wait) {
        corto_trace("ospl: waiting for type of topic '%s'", TOPIC_NAME(this));
    } else {
        corto_trace("ospl: checking for existing version of topic '%s'", TOPIC_NAME(this));
    }

    /* Wait for topic until found, connector is stopping or corto table is defined */
    do {
        while (!topic && !this->quit && !corto_checkState(this->topic, CORTO_DEFINED)) {
            topic = DDS_DomainParticipant_find_topic(
              ospl_dp,
              topicName,
              &timeout);

            if (topic) {
                corto_trace("ospl: found existing topic '%s'", TOPIC_NAME(this));
            }

            /* If not waiting for topic to become available, Corto already has a
             * type defined for the tablescope. Insert topic with Corto type. */
            if (!wait) {
                break;
            }
        }

        if (this->quit) {
            goto quit;
        }

        /* If topic was not found, register new topic with local type */
        if (!topic) {
            corto_trace("ospl: creating new topic '%s' with type '%s'", 
                TOPIC_NAME(this), 
                corto_fullpath(NULL, this->topic->type));
            topic = ospl_registerTopic(topicName, this->topic->type);
            if (!topic) {
                corto_trace("ospl: definition of topic '%s' clashed, retrying", 
                    TOPIC_NAME(this));   
            }
        }

    /* It is possible that the ospl_registerTopic function failed because a new topic was
     * inserted at by another app the same time. In that case, retry findTopic */
    } while (!topic && !this->quit);

    if (this->quit) {
        goto quit;
    }

    if (!(dcpsTopicSample = ospl_registerTypeForTopic(topicName))) {
        corto_error("failed to register type for topic '%s': %s",
            this->topic,
            corto_lasterr());
        goto error;
    }

    /* Resolve source type (potentially loaded from installed packages) */
    corto_type srcType = corto_resolve(NULL, dcpsTopicSample->type_name);
    if (!srcType) {
        corto_error("failed to find '%s' after it has been inserted (topic = '%s')",
            dcpsTopicSample->type_name,
            dcpsTopicSample->name);
        goto error;
    }

    /* If table was not yet defined, assign its type and define it */
    if (!corto_checkState(this->topic, CORTO_DEFINED)) {
        corto_setref(&this->topic->type, srcType);
        /*
         * TODO: the topic will remain in a declared state (not defined) because
         * the topic was declared in another thread, and therefore this thread
         * can't define it. To fix this, the topic will have to be created in
         * this thread, though for that to happen, mounts will need to switch to
         * using an expression instead of an object as mount point.
        */
    }

    /* Create serializer that serializes from topic type to local type */
    if (ospl_Connection_initProgram(this, corto_type(this->topic->type), srcType)) {
        goto error;
    }

    this->ddsTopic = topic;

    corto_release(srcType);
    corto_delete(dcpsTopicSample);
    corto_ok("ospl: topic '%s' initialized", TOPIC_NAME(this));

quit:
    return topic;
error:
    return NULL;
}

corto_int16 ospl_Connection_createWriter(ospl_Connector_Connection this) {
    /* Wait for connector to initialize topic */
    while (!this->ddsTopic && !this->quit) {
        corto_trace("ospl: writer waiting for initialization of topic '%s'", TOPIC_NAME(this));
        corto_sleep(0, 100000000);
    }
    if (!this->ddsTopic) {
        /* Thread is quitting, exit */
        goto error;
    }

    if (this->ddsTopic) {
        corto_trace("ospl: creating entities for writing '%s.%s'",
            PARTITION_NAME(this),
            TOPIC_NAME(this));

        /* Create subscriber for partition */
        DDS_PublisherQos *qos = DDS_PublisherQos__alloc();
        qos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
        qos->partition.name._buffer[0] = DDS_string_dup(this->rootConnector->partition);
        qos->partition.name._length = 1;
        qos->partition.name._maximum = 1;
        qos->entity_factory.autoenable_created_entities = TRUE;
        this->ddsPub = DDS_DomainParticipant_create_publisher(ospl_dp, qos, NULL, DDS_STATUS_MASK_NONE);
        if (!this->ddsPub) {
            corto_error("failed to create publisher for partition '%s'", 
                PARTITION_NAME(this));
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
            corto_error("failed to create writer for '%s.%s'", 
                PARTITION_NAME(this),
                TOPIC_NAME(this));
            goto error;
        }

        corto_ok("ospl: ready to write '%s.%s'", 
            PARTITION_NAME(this),
            TOPIC_NAME(this));
    } else {
        goto error;
    }

    return 0;
error:
    return -1;
}

corto_int16 ospl_Connection_createEntities(
    ospl_Connector_Connection this, 
    char *topicName, DDS_Topic topic) 
{
    corto_trace("ospl: creating entities for reading '%s.%s'", 
        PARTITION_NAME(this), topicName);

    /* Create subscriber for partition */
    DDS_SubscriberQos *qos = DDS_SubscriberQos__alloc();
    qos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
    qos->partition.name._buffer[0] = DDS_string_dup(this->rootConnector->partition);
    qos->partition.name._length = 1;
    qos->partition.name._maximum = 1;
    qos->entity_factory.autoenable_created_entities = TRUE;
    this->ddsSub = DDS_DomainParticipant_create_subscriber(ospl_dp, qos, NULL, DDS_STATUS_MASK_NONE);
    if (!this->ddsSub) {
        corto_error("failed to create subscriber for partition '%s'", this->rootConnector->partition);
        goto error;
    }
    DDS_free(qos);

    /* Setup listener */
    struct DDS_DataReaderListener listener;
    listener.on_data_available = (void (*)(void *, DDS_DataReader)) ospl_connectionOnDataAvailable;
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
        corto_error("failed to create reader for '%s.%s'", 
            PARTITION_NAME(this), topicName);
        goto error;
    }

    return 0;
error:
    return -1;
}

void* ospl_ConnectionThread(void *arg)
{
    ospl_Connector_Connection this = arg;
    DDS_Topic topic = NULL;

    /* Generate topic name with '/' replaced as '_' */
    corto_id topicName;
    corto_path(topicName, corto_mount(this->rootConnector)->mount, this->topic, "_");

    corto_trace("ospl: initializing monitor for '%s.%s'", 
        PARTITION_NAME(this), 
        topicName);

    /* If the topic tablescope has not yet been defined, wait for the topic to be
     * created, and populate the tablescope with the type inserted from ospl. */
    if (!corto_checkState(this->topic, CORTO_DEFINED)) {
        if (!(topic = ospl_Connection_setupTopic(this, topicName, TRUE))) {
            goto error;
        }

    /* If the topic tablescope has been defined, create a new topic using the
     * type specified on the tablescope. */
    } else {
        if (!(topic = ospl_Connection_setupTopic(this, topicName, FALSE))) {
            goto error;
        }
    }

    if (!topic || this->quit) {
        /* Stopping connector, exit thread */
        goto error;
    }

    /* Create DDS entities */
    if (ospl_Connection_createEntities(this, topicName, topic)) {
        goto error;
    }

    corto_ok("ospl: now monitoring '%s.%s'", PARTITION_NAME(this), topicName);
    return NULL;
error:
    corto_ok("ospl: interrupted monitor setup for '%s.%s': %s", PARTITION_NAME(this), topicName, corto_lasterr());
    return NULL;
}
/* $end */

corto_int16 _ospl_Connector_Connection_construct(
    ospl_Connector_Connection this)
{
/* $begin(ospl/Connector/Connection/construct) */

    /* Declare a table for the topic */
    corto_tablescope table = corto_declareChild(
        corto_mount(this->rootConnector)->mount, 
        this->topicName, 
        corto_tablescope_o);
    
    if (!table) {
        corto_error("ospl: failed to create table: %s", corto_lasterr());
        goto error;
    }
    
    //corto_setstr(&corto_subscriber(this)->parent, this->topicName);
    corto_setref(&corto_mount(this)->mount, table);
    corto_setref(&this->topic, table);

    /* Start thread for reading */
    this->thread = (corto_word)corto_threadNew(ospl_ConnectionThread, this);

    return corto_mount_construct(this);
error:
    return -1;
/* $end */
}

corto_void _ospl_Connector_Connection_destruct(
    ospl_Connector_Connection this)
{
/* $begin(ospl/Connector/Connection/destruct) */

    this->quit = TRUE;
    corto_threadJoin((corto_thread)this->thread, NULL);

/* $end */
}

corto_void _ospl_Connector_Connection_onNotify(
    ospl_Connector_Connection this,
    corto_eventMask event,
    corto_result *object)
{
/* $begin(ospl/Connector/Connection/onNotify) */

    if (!this->ddsWriter) {
        corto_trace("ospl: initializing writer for '%s.%s'",
            PARTITION_NAME(this),
            TOPIC_NAME(this));
        if (ospl_Connection_createWriter(this)) {
            if (!this->quit) {
                corto_error("error: %s", corto_lasterr());
            }
            return;
        }
    }

    corto_assert(this->ddsWriter != NULL, "writer should exist");

    void *sample = ospl_copyAlloc(this->program);
    ospl_copyIn(this->program, sample, object->object);

    DDS_ReturnCode_t status = DDS_DataWriter_write(this->ddsWriter, sample, DDS_HANDLE_NIL);
    if ((status != DDS_RETCODE_OK) && (status != DDS_RETCODE_NO_DATA)) {
        corto_error("failed to write to '%s.%s'", 
            PARTITION_NAME(this),
            TOPIC_NAME(this));
    }

    ospl_copyFree(this->program, sample);

/* $end */
}
