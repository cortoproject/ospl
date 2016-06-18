/* $CORTO_GENERATED
 *
 * Connector.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/ospl.h>

/* $header(ospl/Connector/construct) */
void* ospl_ConnectorThread(void *arg)
{
    DDS_Duration_t timeout = {1, 0};
    ospl_Connector this = arg;

    corto_trace("[ospl] waiting for topic '%s'", this->topic);

    /* Wait for topic until found or until connector is stopping */
    while (!this->ddsTopic && !this->quit) {
        this->ddsTopic = DDS_DomainParticipant_find_topic(
          ospl_dp,
          this->topic,
          &timeout);
    }

    corto_trace("[ospl] registering type for topic '%s'", this->topic);

    if (ospl_registerTypeForTopic(this->topic)) {
        corto_error("failed to register type for topic '%s': %s",
            this->topic,
            corto_lasterr());
        goto error;
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

    /* Create datareader for topic */
    this->ddsReader = DDS_Subscriber_create_datareader(
        this->ddsSub,
        this->ddsTopic,
        DDS_DATAREADER_QOS_USE_TOPIC_QOS,
        NULL,
        DDS_STATUS_MASK_NONE);
    if (!this->ddsReader) {
        corto_error("failed to create reader for '%s'", this->partitionTopic);
        goto error;
    }

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
