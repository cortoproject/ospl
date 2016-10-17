/* $CORTO_GENERATED
 *
 * Monitor.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/health/health.h>

corto_void _ospl_Monitor_addPending(
    ospl_Monitor this,
    corto_object sample)
{
/* $begin(ospl/health/Monitor/addPending) */
    corto_object copy = NULL;
    corto_bool found = FALSE;

    corto_lock(this);
    corto_objectListForeach(this->pending, e) {
        if (corto_compare(sample, e) == CORTO_EQ) {
            found = TRUE;
            break;
        }
    }

    if (!found) {
        corto_copy(&copy, sample);
        corto_objectListInsert(this->pending, copy);
    }
    corto_unlock(this);

/* $end */
}

corto_int16 _ospl_Monitor_construct(
    ospl_Monitor this)
{
/* $begin(ospl/health/Monitor/construct) */
    /* Split up instances of different topics over different scopes */

    corto_trace("ospl: start monitor");

    corto_object d_status = corto_voidCreateChild(this, ".d_status");
    corto_object d_sampleChain = corto_voidCreateChild(this, ".d_sampleChain");
    corto_object d_nameSpaces = corto_voidCreateChild(this, ".d_nameSpaces");
    corto_object CMParticipant = corto_voidCreateChild(this, ".CMParticipant");
    corto_object CMPublisher = corto_voidCreateChild(this, ".CMPublisher");
    corto_object CMSubscriber = corto_voidCreateChild(this, ".CMSubscriber");
    corto_object CMDataWriter = corto_voidCreateChild(this, ".CMDataWriter");
    corto_object CMDataReader = corto_voidCreateChild(this, ".CMDataReader");

    /* Store discovery database in self if no mount is provided */
    if (!this->mount && corto_checkAttr(this, CORTO_ATTR_SCOPED)) {
        corto_setref(&this->mount, this);
    }

    /* Create discovery database manager */
    this->db = ospl_DiscoveryDbCreate(this->mount);

    /* Subscribe to participants */
    corto_listen(
      this,
      ospl_Monitor_participant_onUpdate_o,
      CORTO_ON_UPDATE|CORTO_ON_DEFINE|CORTO_ON_SCOPE,
      CMParticipant,
      NULL);

    corto_listen(
      this,
      ospl_Monitor_participant_onDelete_o,
      CORTO_ON_DELETE|CORTO_ON_SCOPE,
      CMParticipant,
      NULL);

    /* Subscribe to subscribers */
    corto_listen(
      this,
      ospl_Monitor_subscriber_onUpdate_o,
      CORTO_ON_UPDATE|CORTO_ON_DEFINE|CORTO_ON_SCOPE,
      CMSubscriber,
      NULL);

    corto_listen(
      this,
      ospl_Monitor_subscriber_onDelete_o,
      CORTO_ON_DELETE|CORTO_ON_SCOPE,
      CMSubscriber,
      NULL);

    /* Subscribe to publishers */
    corto_listen(
      this,
      ospl_Monitor_publisher_onUpdate_o,
      CORTO_ON_UPDATE|CORTO_ON_DEFINE|CORTO_ON_SCOPE,
      CMPublisher,
      NULL);

    corto_listen(
      this,
      ospl_Monitor_publisher_onDelete_o,
      CORTO_ON_DELETE|CORTO_ON_SCOPE,
      CMPublisher,
      NULL);

    /* Subscribe to datawriters */
    corto_listen(
      this,
      ospl_Monitor_dataWriter_onUpdate_o,
      CORTO_ON_UPDATE|CORTO_ON_DEFINE|CORTO_ON_SCOPE,
      CMDataWriter,
      NULL);

    corto_listen(
      this,
      ospl_Monitor_dataWriter_onDelete_o,
      CORTO_ON_DELETE|CORTO_ON_SCOPE,
      CMDataWriter,
      NULL);

    /* Subscribe to datareaders */
    corto_listen(
      this,
      ospl_Monitor_dataReader_onUpdate_o,
      CORTO_ON_UPDATE|CORTO_ON_DEFINE|CORTO_ON_SCOPE,
      CMDataReader,
      NULL);

    corto_listen(
      this,
      ospl_Monitor_dataReader_onDelete_o,
      CORTO_ON_DELETE|CORTO_ON_SCOPE,
      CMDataReader,
      NULL);

    /* Subscribe to durability topics */
    corto_listen(
      this,
      ospl_Monitor_durabilityStatus_onUpdate_o,
      CORTO_ON_DEFINE|CORTO_ON_UPDATE|CORTO_ON_SCOPE,
      d_status,
      NULL);

    corto_listen(
      this,
      ospl_Monitor_durabilitySampleChain_onUpdate_o,
      CORTO_ON_DEFINE|CORTO_ON_UPDATE|CORTO_ON_SCOPE,
      d_sampleChain,
      NULL);

    corto_listen(
      this,
      ospl_Monitor_durabilityNameSpaces_onUpdate_o,
      CORTO_ON_DEFINE|CORTO_ON_UPDATE|CORTO_ON_SCOPE,
      d_nameSpaces,
      NULL);

    /* Connect to topics */
    this->d_statusX = ospl_ConnectorCreate(
        d_status,
        "/ospl/health/Monitor/DurabilityStatus",
        NULL,
        "durabilityPartition.d_status",
        NULL);

    this->d_sampleChainX = ospl_ConnectorCreate(
        d_sampleChain,
        "/ospl/health/Monitor/DurabilitySampleChain",
        NULL,
        "durabilityPartition.d_sampleChain",
        NULL);

    this->d_nameSpacesX = ospl_ConnectorCreate(
        d_nameSpaces,
        "/ospl/health/Monitor/DurabilityNameSpaces",
        NULL,
        "durabilityPartition.d_nameSpaces",
        NULL);
/*
    this->DCPSHeartbeatX = ospl_ConnectorCreate(
        DCPSHeartbeat,
        "__BUILT-IN PARTITION__.DCPSHeartbeat",
        NULL);
*/
    this->CMParticipantX = ospl_ConnectorCreate(
        CMParticipant,
        "/ospl/health/Monitor/CMParticipant",
        NULL,
        "__BUILT-IN PARTITION__.CMParticipant",
        NULL);

    this->CMPublisherX = ospl_ConnectorCreate(
        CMPublisher,
        "/ospl/health/Monitor/CMPublisher",
        NULL,
        "__BUILT-IN PARTITION__.CMPublisher",
        NULL);

    this->CMSubscriberX = ospl_ConnectorCreate(
        CMSubscriber,
        "/ospl/health/Monitor/CMSubscriber",
        NULL,
        "__BUILT-IN PARTITION__.CMSubscriber",
        NULL);

    this->CMDataWriterX = ospl_ConnectorCreate(
        CMDataWriter,
        "/ospl/health/Monitor/CMDataWriter",
        NULL,
        "__BUILT-IN PARTITION__.CMDataWriter",
        NULL);

    this->CMDataReaderX = ospl_ConnectorCreate(
        CMDataReader,
        "/ospl/health/Monitor/CMDataReader",
        NULL,
        "__BUILT-IN PARTITION__.CMDataReader",
        NULL);

    corto_trace("ospl: monitor running");

    return 0;
/* $end */
}

corto_void _ospl_Monitor_dataReader_onDelete(
    ospl_Monitor this,
    ospl_Monitor_CMDataReader r)
{
/* $begin(ospl/health/Monitor/dataReader_onDelete) */

    ospl_DiscoveryDb_deleteEntity(
        this->db,
        r->key.systemId,
        r->key.localId);

/* $end */
}

corto_void _ospl_Monitor_dataReader_onUpdate(
    ospl_Monitor this,
    ospl_Monitor_CMDataReader r)
{
/* $begin(ospl/health/Monitor/dataReader_onUpdate) */

    if (!ospl_DiscoveryDb_updateDataReader(
        this->db,
        r->key.systemId,
        r->key.localId,
        r->subscriber_key.localId,
        r->name))
    {
        ospl_Monitor_addPending(this, r);
    }

/* $end */
}

corto_void _ospl_Monitor_dataWriter_onDelete(
    ospl_Monitor this,
    ospl_Monitor_CMDataWriter w)
{
/* $begin(ospl/health/Monitor/dataWriter_onDelete) */

    ospl_DiscoveryDb_deleteEntity(
        this->db,
        w->key.systemId,
        w->key.localId);

/* $end */
}

corto_void _ospl_Monitor_dataWriter_onUpdate(
    ospl_Monitor this,
    ospl_Monitor_CMDataWriter w)
{
/* $begin(ospl/health/Monitor/dataWriter_onUpdate) */

    if (!ospl_DiscoveryDb_updateDataWriter(
        this->db,
        w->key.systemId,
        w->key.localId,
        w->publisher_key.localId,
        w->name))
    {
        ospl_Monitor_addPending(this, w);
    }

/* $end */
}

corto_void _ospl_Monitor_destruct(
    ospl_Monitor this)
{
/* $begin(ospl/health/Monitor/destruct) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _ospl_Monitor_durabilityNameSpaces_onUpdate(
    ospl_Monitor this,
    ospl_Monitor_DurabilityNameSpaces o)
{
/* $begin(ospl/health/Monitor/durabilityNameSpaces_onUpdate) */

    ospl_NameSpacesEvent *e = ospl_NameSpacesEventCreate(
        o->name,
        o->parentMsg.senderAddress.systemId,
        o->master.systemId,
        o->masterConfirmed);

    if (ospl_Monitor_nameSpacesEventIsNew(this, e)) {
        ospl_Monitor_eventActionCall(&this->onEvent, (ospl_Event*)e);
    }

    corto_release(e);

/* $end */
}

corto_void _ospl_Monitor_durabilitySampleChain_onUpdate(
    ospl_Monitor this,
    ospl_Monitor_DurabilitySampleChain o)
{
/* $begin(ospl/health/Monitor/durabilitySampleChain_onUpdate) */

    ospl_AlignEvent *e = ospl_AlignEventCreate(
        o->parentMsg.senderAddress.systemId,
        o->addresseesCount,
        0);

    ospl_Monitor_eventActionCall(&this->onEvent, (ospl_Event*)e);

    corto_delete(e);

/* $end */
}

corto_void _ospl_Monitor_durabilityStatus_onUpdate(
    ospl_Monitor this,
    ospl_Monitor_DurabilityStatus s)
{
/* $begin(ospl/health/Monitor/durabilityStatus_onUpdate) */
    ospl_DiscoveryDb_updateDurability(
        this->db,
        s->parentMsg.senderAddress.systemId,
        s->parentMsg.senderState);

/* $end */
}

corto_bool _ospl_Monitor_nameSpacesEventIsNew(
    ospl_Monitor this,
    ospl_NameSpacesEvent *e)
{
/* $begin(ospl/health/Monitor/nameSpacesEventIsNew) */
    corto_bool newEvent = FALSE, found = FALSE;

    corto_objectListForeach(this->nameSpacesEvents, prev) {
        if (e->source == ospl_NameSpacesEvent(prev)->source) {
            found = TRUE;
            if (corto_compare(e, prev) != CORTO_EQ) {
                corto_llReplace(this->nameSpacesEvents, prev, e);
                corto_claim(e);
                corto_release(prev);
                newEvent = TRUE;
            }
            break;
        }
    }

    if (!found) {
        corto_llInsert(this->nameSpacesEvents, e);
        corto_claim(e);
        newEvent = TRUE;
    }

    return newEvent;
/* $end */
}

corto_void _ospl_Monitor_participant_onDelete(
    ospl_Monitor this,
    ospl_Monitor_CMParticipant p)
{
/* $begin(ospl/health/Monitor/participant_onDelete) */

    ospl_DiscoveryDb_deleteParticipant(
        this->db,
        p->key.systemId,
        p->key.localId);

/* $end */
}

corto_void _ospl_Monitor_participant_onUpdate(
    ospl_Monitor this,
    ospl_Monitor_CMParticipant p)
{
/* $begin(ospl/health/Monitor/participant_onUpdate) */

    if (ospl_DiscoveryDb_updateParticipant(
        this->db,
        p->key.systemId,
        p->key.localId,
        p->product.value))
    {
        ospl_Monitor_resumePending(
          this,
          ospl_Monitor_CMSubscriber_o,
          ospl_Monitor_subscriber_onUpdate_o);
        ospl_Monitor_resumePending(
          this,
          ospl_Monitor_CMPublisher_o,
          ospl_Monitor_publisher_onUpdate_o);
    }

/* $end */
}

corto_void _ospl_Monitor_publisher_onDelete(
    ospl_Monitor this,
    ospl_Monitor_CMPublisher p)
{
/* $begin(ospl/health/Monitor/publisher_onDelete) */

    ospl_DiscoveryDb_deleteEntity(
        this->db,
        p->key.systemId,
        p->key.localId);

/* $end */
}

corto_void _ospl_Monitor_publisher_onUpdate(
    ospl_Monitor this,
    ospl_Monitor_CMPublisher p)
{
/* $begin(ospl/health/Monitor/publisher_onUpdate) */

    if (!ospl_DiscoveryDb_updatePublisher(
        this->db,
        p->key.systemId,
        p->key.localId,
        p->participant_key.localId,
        p->name))
    {
        ospl_Monitor_addPending(this, p);
    } else {
        ospl_Monitor_resumePending(
          this,
          ospl_Monitor_CMDataWriter_o,
          ospl_Monitor_dataWriter_onUpdate_o);
    }

/* $end */
}

corto_void _ospl_Monitor_resumePending(
    ospl_Monitor this,
    corto_type t,
    corto_observer o)
{
/* $begin(ospl/health/Monitor/resumePending) */

    /* Copy list. Lock must be released while iterating, and because a remove
     * occurs during the walk, this could invalidate iterators in other
     * threads */
    corto_lock(this);
    corto_ll copy = corto_llCopy(this->pending);
    corto_unlock(this);

    corto_objectListForeach(copy, e) {
        if (corto_typeof(e) == t) {
            /* Remove from pending list. It is guaranteed that no other thread
             * will attempt to delete objects of the same type from this list,
             * so the entity is guaranteed to still be in the list. */
            corto_lock(this);
            corto_llRemove(this->pending, e);
            corto_unlock(this);

            corto_call(corto_function(o), NULL, this, e);
            corto_release(e);
        }
    }

    corto_llFree(copy);

/* $end */
}

corto_void _ospl_Monitor_subscriber_onDelete(
    ospl_Monitor this,
    ospl_Monitor_CMSubscriber s)
{
/* $begin(ospl/health/Monitor/subscriber_onDelete) */

    ospl_DiscoveryDb_deleteEntity(
        this->db,
        s->key.systemId,
        s->key.localId);

/* $end */
}

corto_void _ospl_Monitor_subscriber_onUpdate(
    ospl_Monitor this,
    ospl_Monitor_CMSubscriber s)
{
/* $begin(ospl/health/Monitor/subscriber_onUpdate) */

    if (!ospl_DiscoveryDb_updateSubscriber(
        this->db,
        s->key.systemId,
        s->key.localId,
        s->participant_key.localId,
        s->name))
    {
        ospl_Monitor_addPending(this, s);
    } else {
        ospl_Monitor_resumePending(
          this,
          ospl_Monitor_CMDataReader_o,
          ospl_Monitor_dataReader_onUpdate_o);
    }

/* $end */
}
