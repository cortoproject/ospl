/* $CORTO_GENERATED
 *
 * Mount_Topic.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/ospl.h>

/* $header() */
#include "CheckStatus.h"
#define TOPIC_NAME(c) c->topicName
#define PARTITION_NAME(c) (c->rootMount->partition[0] ? c->rootMount->partition : "<DEFAULT>")
typedef enum ospl_CrudKind {
    Ospl_Create,
    Ospl_Update,
    Ospl_Delete
} ospl_CrudKind;

static ospl_CrudKind ospl_DdsToCrudKind(DDS_ViewStateKind vs, DDS_InstanceStateKind is) {
    ospl_CrudKind result = Ospl_Update;

    if (is == DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE) {
        result = Ospl_Delete;
    } else if(vs == DDS_NEW_VIEW_STATE) {
        result = Ospl_Create;
    } else if(is == DDS_ALIVE_INSTANCE_STATE) {
        result = Ospl_Update;
    }

    return result;
}

static corto_object ospl_MountGetObject(ospl_Mount_Topic this, corto_string key, ospl_CrudKind state, int *err) {
    *err = 0;
    corto_object result = corto_lookup(corto_mount(this)->mount, key);
    if (!result && state != Ospl_Delete) {
        result = corto_declareChild(corto_mount(this)->mount, key, this->topic->type);
        if (result) {
            corto_claim(result);
        } else {
            *err = 1;
            corto_error("ospl: failed to declare '%s' for '%s.%s': %s", 
                key, 
                PARTITION_NAME(this),
                TOPIC_NAME(this),
                corto_lasterr());
        }
    }
    return result;
}

static void ospl_connectionOnDataAvailable(ospl_Mount_Topic this, DDS_DataReader reader) {
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
        corto_error("ospl: failed to read from '%s.%s'", 
            PARTITION_NAME(this),
            TOPIC_NAME(this));
        goto error;
    }

    for (i = 0; i < sampleSeq->_length; i++) {
        void *ptr = CORTO_OFFSET(sampleSeq->_buffer, i * sampleSize);
        corto_id key;
        ospl_copyProgram_keyString(this->program, key, ptr);

        ospl_CrudKind state = 
            ospl_DdsToCrudKind(
                infoSeq->_buffer[i].view_state, 
                infoSeq->_buffer[i].instance_state);

        int err = 0;
        corto_object o = ospl_MountGetObject(this, key, state, &err);
        if (err) {
            goto error;
        }

        /* Only update objects owned by the connector, or if they haven't been
         * defined yet (in which case MountGetObject just created it)
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

        if (o && corto_owned(o)) {

            /* Always generate update event for valid data, even if data is
             * disposed. */
            if (infoSeq->_buffer[i].valid_data) {
                if (corto_updateBegin(o)) {
                    corto_error("ospl: insert '%s' failed for '%s.%s': %s",
                        corto_fullpath(NULL, o),
                        PARTITION_NAME(this),
                        TOPIC_NAME(this),
                        corto_lasterr());
                    goto error;
                }

                if (ospl_copyOut(this->program, (void**)&o, ptr)) {
                    corto_error("ospl: insert '%s' failed for '%s.%s': %s",
                        corto_fullpath(NULL, o),
                        PARTITION_NAME(this),
                        TOPIC_NAME(this),
                        corto_lasterr());
                    corto_updateCancel(o);
                    goto error;
                }
                if (corto_updateEnd(o)) {
                    corto_error("ospl: failed to insert '%s' for '%s.%s': %s",
                        corto_fullpath(NULL, o),
                        PARTITION_NAME(this),
                        TOPIC_NAME(this),
                        corto_lasterr());
                    goto error;
                }
            }

            /* If data is disposed, delete object */
            if (state == Ospl_Delete) {
                corto_info("delete object");
                if (corto_delete(o)) {
                    corto_error("ospl: failed to delete '%s' for '%s.%s': %s",
                        corto_fullpath(NULL, o),
                        PARTITION_NAME(this),
                        TOPIC_NAME(this),
                        corto_lasterr());
                    goto error;
                }
            }
        } else {
            corto_debug("ospl: ignoring remote update for '%s' (application owns object)",
                corto_fullpath(NULL, o));
        }

        if (o) corto_release(o);
    }

    status = DDS_DataReader_return_loan(reader, sampleSeq, infoSeq);
    if (status) {
        corto_error("ospl: failed to return loan for '%s.%s'",
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
static corto_int16 ospl_Topic_initProgram(
    ospl_Mount_Topic this,
    corto_type dstType,
    corto_type srcType)
{
    this->program = ospl_copyProgramNew(idl_actualType(srcType), idl_actualType(dstType));
    return this->program == NULL ? -1 : 0;
}

/* Setup connection to existing topic */
static DDS_Topic ospl_Topic_setupTopic(
    ospl_Mount_Topic this, 
    char *topicName,
    corto_bool wait)
{
    ospl_DCPSTopic dcpsTopicSample = NULL;
    DDS_Topic topic = NULL;
    DDS_Duration_t timeout = {0, 100000000};
    int retriesAfterClash = 0;
    char *typeName = NULL;

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
        /* Even if corto has defined the topic, try to find it in DDS to ensure
         * that definitions don't clash */
        while (!topic && !this->quit && !corto_checkState(this->topic, CORTO_DEFINED)) {
            topic = DDS_DomainParticipant_find_topic(
              ospl_dp,
              topicName,
              &timeout);

            if (topic) {
                corto_ok("ospl: found topic '%s' in DDS", TOPIC_NAME(this));
            }

            /* If not waiting for topic to become available, Corto already has a
             * type defined for the tableinstance. Insert topic with Corto type. */
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
                corto_lasterr();
                corto_trace("ospl: definition of topic '%s' contains error or clashed, retrying", 
                    TOPIC_NAME(this));
                retriesAfterClash ++;
                if (retriesAfterClash > 5) {
                    corto_error("ospl: failed to insert topic '%s.%s'",
                        PARTITION_NAME(this), TOPIC_NAME(this));
                    goto error;
                }
            }
        }

    /* It is possible that the ospl_registerTopic function failed because a new topic was
     * inserted at by another app the same time. In that case, retry findTopic */
    } while (!topic && !this->quit);

    if (this->quit) {
        goto quit;
    }

    if (!(dcpsTopicSample = ospl_registerTypeForTopic(topicName))) {
        corto_error("failed to register type for topic '%s.%s': %s",
            PARTITION_NAME(this),
            TOPIC_NAME(this),
            corto_lasterr());
        goto error;
    }

    /* Resolve source type (potentially loaded from installed packages) */
    typeName = ospl_cortoId(dcpsTopicSample->type_name);
    corto_type srcType = corto_resolve(NULL, typeName);
    if (!srcType) {
        corto_error("failed to find '%s' after it has been inserted (topic = '%s')",
            dcpsTopicSample->type_name,
            dcpsTopicSample->name);
        goto error;
    }

    /* If table was not yet defined, assign its type and define it */
    if (!corto_checkState(this->topic, CORTO_DEFINED)) {
        corto_ptr_setref(&this->topic->type, srcType);
        /*
         * TODO: the topic will remain in a declared state (not defined) because
         * the topic was declared in another thread, and therefore this thread
         * can't define it. To fix this, the topic will have to be created in
         * this thread, though for that to happen, mounts will need to switch to
         * using an expression instead of an object as mount point.
         */
    }

    /* Create serializer that serializes from topic type to local type */
    if (ospl_Topic_initProgram(this, corto_type(this->topic->type), srcType)) {
        goto error;
    }

    this->ddsTopic = topic;

    corto_release(srcType);
    corto_delete(dcpsTopicSample);
    corto_dealloc(typeName);
    corto_ok("ospl: topic '%s' initialized in corto", TOPIC_NAME(this));

quit:
    return topic;
error:
    if (typeName) {
        corto_dealloc(typeName);
    }
    return NULL;
}

static corto_int16 ospl_Topic_createWriter(ospl_Mount_Topic this) {
    /* Wait for connector to initialize topic */
    corto_int32 waitCount = 0;
    while (!this->ddsTopic && !this->quit) {
        if (!waitCount) {
            corto_trace("ospl: writer waiting for initialization of topic '%s.%s'", 
                PARTITION_NAME(this), TOPIC_NAME(this));
        }
        corto_sleep(0, 100000000);
        waitCount ++;
        if (waitCount > 10) {
            corto_seterr("writer could not be created for topic '%s.%s' (missing type)", 
                PARTITION_NAME(this), TOPIC_NAME(this));
            return -1;
        }
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
        qos->partition.name._buffer[0] = DDS_string_dup(this->rootMount->partition);
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

        /* Get default datawriter QoS, then modify to offer most possible */
        DDS_DataWriterQos *dwQos = DDS_DataWriterQos__alloc();
        DDS_Publisher_get_default_datawriter_qos(this->ddsPub, dwQos);
        DDS_TopicQos topicQos;
        DDS_Topic_get_qos(this->ddsTopic, &topicQos);
        DDS_Publisher_copy_from_topic_qos(
            this->ddsPub,
            dwQos, 
            &topicQos);

        /* Offer maximum QoS */
        dwQos->durability.kind = DDS_PERSISTENT_DURABILITY_QOS;
        dwQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;

        /* Create datawriter for topic */
        this->ddsWriter = DDS_Publisher_create_datawriter(
            this->ddsPub,
            this->ddsTopic,
            dwQos,
            NULL,
            0);
        if (!this->ddsWriter) {
            corto_error("ospl: failed to create writer for '%s.%s'", 
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

static corto_int16 ospl_Topic_createEntities(
    ospl_Mount_Topic this, 
    char *topicName, DDS_Topic topic) 
{
    corto_trace("ospl: creating entities for reading '%s.%s'", 
        PARTITION_NAME(this), topicName);

    /* Create subscriber for partition */
    DDS_SubscriberQos *qos = DDS_SubscriberQos__alloc();
    qos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
    qos->partition.name._buffer[0] = DDS_string_dup(this->rootMount->partition);
    qos->partition.name._length = 1;
    qos->partition.name._maximum = 1;
    qos->entity_factory.autoenable_created_entities = TRUE;
    this->ddsSub = DDS_DomainParticipant_create_subscriber(ospl_dp, qos, NULL, DDS_STATUS_MASK_NONE);
    if (!this->ddsSub) {
        corto_error("failed to create subscriber for partition '%s'", this->rootMount->partition);
        goto error;
    }
    DDS_free(qos);


    /* Get default datareader QoS, then modify to be not as restrictive */
    DDS_DataReaderQos *drQos = DDS_DataReaderQos__alloc();
    DDS_Subscriber_get_default_datareader_qos(this->ddsSub, drQos);
    DDS_TopicQos topicQos;
    DDS_Topic_get_qos(topic, &topicQos);
    DDS_Subscriber_copy_from_topic_qos(
        this->ddsSub,
        drQos, 
        &topicQos);

    drQos->reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
    drQos->durability.kind = DDS_VOLATILE_DURABILITY_QOS;

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
        drQos,
        &listener,
        DDS_DATA_AVAILABLE_STATUS);
    if (!this->ddsReader) {
        corto_error("failed to create reader for '%s.%s'", 
            PARTITION_NAME(this), topicName);
        goto error;
    }

    DDS_Duration_t timeout = {0, 0};
    DDS_ReturnCode_t status = DDS_DataReader_wait_for_historical_data(this->ddsReader, &timeout);
    if (status != DDS_RETCODE_OK) {
        corto_error("DDS_DataReader_wait_for_historical_data failed");
        goto error;
    }

    DDS_free(drQos);

    return 0;
error:
    return -1;
}

static void ospl_Topic_getTopicName(
    ospl_Mount_Topic this, 
    char *buffer, 
    corto_object topic) 
{
    char *elems[CORTO_MAX_SCOPE_DEPTH];
    corto_uint8 elemCount = 0;

    corto_object parent = topic;
    do {
        if (corto_instanceof(corto_tableinstance_o, parent)) {
            elems[elemCount] = corto_idof(parent);
            elemCount ++;
        }
    } while ((parent = corto_parentof(parent)) && parent != this->rootMount && parent != root_o);

    buffer[0] = '\0';

    while (elemCount) {
        strcat(buffer, elems[elemCount - 1]);
        if (elemCount != 1) {
            strcat(buffer, "_");
        }
        elemCount --;
    }
}

static void* ospl_TopicThread(void *arg)
{
    ospl_Mount_Topic this = arg;
    DDS_Topic topic = NULL;

    /* Generate topic name with '/' replaced as '_' */
    corto_id topicName;
    ospl_Topic_getTopicName(this, topicName, this->topic);
    corto_ptr_setstr(&this->topicName, topicName);

    corto_trace("ospl: initializing monitor for '%s.%s'", 
        PARTITION_NAME(this), 
        topicName);

    /* If the topic tableinstance has not yet been defined, wait for the topic to be
     * created, and populate the tableinstance with the type inserted from ospl. */
    if (!corto_checkState(this->topic, CORTO_DEFINED)) {
        if (!(topic = ospl_Topic_setupTopic(this, topicName, TRUE))) {
            goto error;
        }

    /* If the topic tableinstance has been defined, create a new topic using the
     * type specified on the tableinstance. */
    } else {
        if (!(topic = ospl_Topic_setupTopic(this, topicName, FALSE))) {
            goto error;
        }
    }

    if (!topic || this->quit) {
        /* Stopping connector, exit thread */
        goto error;
    }

    /* Create DDS entities */
    if (ospl_Topic_createEntities(this, topicName, topic)) {
        goto error;
    }

    corto_ok("ospl: now monitoring '%s.%s'", PARTITION_NAME(this), topicName);
    return NULL;
error:
    corto_ok("ospl: interrupted monitor setup for '%s.%s': %s", PARTITION_NAME(this), topicName, corto_lasterr());
    return NULL;
}
/* $end */

int16_t _ospl_Mount_Topic_construct(
    ospl_Mount_Topic this)
{
/* $begin(ospl/Mount/Topic/construct) */

    /* Declare a table for the topic */
    corto_tableinstance table = corto_findOrDeclare(
        corto_mount(this->rootMount)->mount, 
        this->topicName, 
        corto_tableinstance_o);
    
    if (!table) {
        corto_error("ospl: failed to create table: %s", corto_lasterr());
        goto error;
    }

    if (!corto_instanceof(corto_tableinstance_o, table)) {
        corto_error("ospl: cannot connect '%s': singletons unsupported", this->topicName);
        goto error;
    }
    
    corto_ptr_setref(&corto_mount(this)->mount, table);
    corto_ptr_setref(&this->topic, table);

    if (corto_mount_construct(this)) {
        goto error;
    }

    /* Start thread for reading */
    this->thread = (corto_word)corto_threadNew(ospl_TopicThread, this);
    if (!this->thread) {
        goto error;
    }

    return 0;
error:
    return -1;
/* $end */
}

void _ospl_Mount_Topic_destruct(
    ospl_Mount_Topic this)
{
/* $begin(ospl/Mount/Topic/destruct) */
    DDS_ReturnCode_t status;
    corto_trace("ospl: stopping connector thread for topic '%s'",
        TOPIC_NAME(this));

    this->quit = TRUE;
    corto_threadJoin((corto_thread)this->thread, NULL);

    corto_trace("ospl: cleaning up DDS entities for topic '%s'",
        TOPIC_NAME(this));

    if (this->ddsReader) {
        status = DDS_Subscriber_delete_datareader(this->ddsSub, this->ddsReader);
        if (status != DDS_RETCODE_OK) {
            corto_error("ospl: failed to delete reader for '%s' (%s)",
                TOPIC_NAME(this),
                OSPL_ERROR_NAME[status]);
        }
    }
    if (this->ddsWriter) {
        status = DDS_Publisher_delete_datawriter(this->ddsPub, this->ddsWriter);
        if (status != DDS_RETCODE_OK) {
            corto_error("ospl: failed to delete writer for '%s' (%s)",
                TOPIC_NAME(this),
                OSPL_ERROR_NAME[status]);
        }
    }
    if (this->ddsSub) {
        status = DDS_DomainParticipant_delete_subscriber(ospl_dp, this->ddsSub);
        if (status != DDS_RETCODE_OK) {
            corto_error("ospl: failed to delete subscriber for '%s' (%s)",
                TOPIC_NAME(this),
                OSPL_ERROR_NAME[status]);
        }
    }
    if (this->ddsPub) {
        status = DDS_DomainParticipant_delete_publisher(ospl_dp, this->ddsPub);
        if (status != DDS_RETCODE_OK) {
            corto_error("ospl: failed to delete publisher for '%s' (%s)",
                TOPIC_NAME(this),
                OSPL_ERROR_NAME[status]);
        }
    }
    if (this->ddsTopic) {
        status = DDS_DomainParticipant_delete_topic(ospl_dp, this->ddsTopic);
        if (status != DDS_RETCODE_OK) {
            corto_error("ospl: failed to delete topic for '%s' (%s)",
                TOPIC_NAME(this),
                OSPL_ERROR_NAME[status]);
        }
    }

    if (this->program) {
        ospl_copyProgramFree(this->program);
    }

    if (this->topic) {
        if (corto_delete(this->topic)) {
            corto_error("ospl: failed to delete tableinstance for '%s': %s",
                TOPIC_NAME(this),
                corto_lasterr());
        }
    }

    corto_ok("ospl: stopped monitoring '%s'",
        TOPIC_NAME(this));

    corto_super_destruct(this);

/* $end */
}

void _ospl_Mount_Topic_onNotify(
    ospl_Mount_Topic this,
    corto_subscriberEvent *event)
{
/* $begin(ospl/Mount/Topic/onNotify) */

    if (!this->ddsWriter) {
        corto_trace("ospl: first update received for topic '%s' ('%s')",
            TOPIC_NAME(this),
            event->data.id);
        if (ospl_Topic_createWriter(this)) {
            if (!this->quit) {
                corto_warning("ospl: %s", corto_lasterr());
            }
            return;
        }
    }

    corto_assert(this->ddsWriter != NULL, "writer should exist");

    void *sample = ospl_copyAlloc(this->program);
    ospl_copyIn(this->program, sample, event->data.object);

    DDS_ReturnCode_t status = DDS_DataWriter_write(this->ddsWriter, sample, DDS_HANDLE_NIL);
    if ((status != DDS_RETCODE_OK) && (status != DDS_RETCODE_NO_DATA)) {
        corto_error("ospl: failed to write to '%s.%s'", 
            PARTITION_NAME(this),
            TOPIC_NAME(this));
    }

    ospl_copyFree(this->program, sample);

/* $end */
}
