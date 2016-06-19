/* $CORTO_GENERATED
 *
 * Monitor.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/health/health.h>

corto_int16 _ospl_Monitor_construct(
    ospl_Monitor this)
{
/* $begin(ospl/health/Monitor/construct) */
    corto_voidCreateChild_auto(this, d_status);
    corto_voidCreateChild_auto(this, DCPSHeartbeat);
    corto_voidCreateChild_auto(this, CMParticipant);
    corto_voidCreateChild_auto(this, CMPublisher);
    corto_voidCreateChild_auto(this, CMSubscriber);
    corto_voidCreateChild_auto(this, CMDataWriter);
    corto_voidCreateChild_auto(this, CMDataReader);

    if (!this->mount) {
        corto_setref(&this->mount, this);
    }

    this->db = ospl_DiscoveryDbCreate(this->mount);

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

    corto_listen(
      this,
      ospl_Monitor_durabilityStatus_onUpdate_o,
      CORTO_ON_DEFINE|CORTO_ON_UPDATE|CORTO_ON_SCOPE,
      d_status,
      NULL);

    /* Setup connectors to topics */
    this->d_statusX = ospl_ConnectorCreate(
        d_status,
        "durabilityPartition.d_status",
        ospl_Monitor_DurabilityStatus_o);
/*
    this->DCPSHeartbeatX = ospl_ConnectorCreate(
        DCPSHeartbeat,
        "__BUILT-IN PARTITION__.DCPSHeartbeat",
        NULL);
*/
    this->CMParticipantX = ospl_ConnectorCreate(
        CMParticipant,
        "__BUILT-IN PARTITION__.CMParticipant",
        ospl_Monitor_CMParticipant_o);

    /*this->CMPublisherX = ospl_ConnectorCreate(
        CMPublisher,
        "__BUILT-IN PARTITION__.CMPublisher",
        NULL);

    this->CMSubscriberX = ospl_ConnectorCreate(
        CMSubscriber,
        "__BUILT-IN PARTITION__.CMSubscriber",
        NULL);

    this->CMDataWriterX = ospl_ConnectorCreate(
        CMDataWriter,
        "__BUILT-IN PARTITION__.CMDataWriter",
        NULL);

    this->CMDataReaderX = ospl_ConnectorCreate(
        CMDataReader,
        "__BUILT-IN PARTITION__.CMDataReader",
        NULL);*/

    return 0;
/* $end */
}

corto_void _ospl_Monitor_destruct(
    ospl_Monitor this)
{
/* $begin(ospl/health/Monitor/destruct) */

    /* << Insert implementation >> */

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

    ospl_DiscoveryDb_updateParticipant(
        this->db,
        p->key.systemId,
        p->key.localId,
        p->product.value);

/* $end */
}
