/* $CORTO_GENERATED
 *
 * DiscoveryDb.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/health/health.h>

/* $header() */
corto_object ospl_DiscoveryDb_findFederation(
    corto_object root,
    corto_uint32 systemId)
{
    ospl_DiscoveryDb_Federation federation = NULL;
    corto_objectseq nodes = corto_scopeClaim(root);
    corto_objectseqForeach(nodes, n) {
        corto_objectseq federations = corto_scopeClaim(n);
        corto_objectseqForeach(federations, f) {
            if (ospl_DiscoveryDb_Federation(f)->systemId == systemId) {
                federation = f;
                break;
            }
        }
        corto_scopeRelease(federations);
        if (federation) {
            break;
        }
    }
    corto_scopeRelease(nodes);

    return federation;
}

corto_object ospl_DiscoveryDb_findEntityWalk(corto_object f, corto_uint32 localId) {
    corto_object result = NULL;
    corto_objectseq scope = corto_scopeClaim(f);
    corto_objectseqForeach(scope, o) {
        if (corto_instanceof(ospl_DiscoveryDb_Entity_o, o)) {
            if (ospl_DiscoveryDb_Entity(o)->localId == localId) {
                result = o;
                break;
            }
        }
        if (corto_scopeSize(o)) {
            result = ospl_DiscoveryDb_findEntityWalk(o, localId);
        }
        if (result) {
            break;
        }
    }
    corto_scopeRelease(scope);
    return result;
}

corto_object ospl_DiscoveryDb_findEntity(
    ospl_DiscoveryDb this,
    corto_uint32 systemId,
    corto_uint32 localId)
{
    /* Find federation for systemId */
    ospl_DiscoveryDb_Federation f = ospl_DiscoveryDb_findFederation(this->mount, systemId);
    if (!f) {
        corto_seterr("failed to find federation '%x'", systemId);
        goto error;
    }

    /* Crawl federation for entity with specified localId */
    return ospl_DiscoveryDb_findEntityWalk(f, localId);
error:
    return NULL;
}
/* $end */

corto_void _ospl_DiscoveryDb_deleteEntity(
    ospl_DiscoveryDb this,
    corto_uint32 systemId,
    corto_uint32 localId)
{
/* $begin(ospl/health/DiscoveryDb/deleteEntity) */

    corto_object e = ospl_DiscoveryDb_findEntity(
        this,
        systemId,
        localId);
    if (e) {
        corto_delete(e);
    }

/* $end */
}

corto_void _ospl_DiscoveryDb_deleteParticipant(
    ospl_DiscoveryDb this,
    corto_uint32 systemId,
    corto_uint32 localId)
{
/* $begin(ospl/health/DiscoveryDb/deleteParticipant) */

    corto_object participant = ospl_DiscoveryDb_findEntity(
        this,
        systemId,
        localId
    );

    /* Cleanup participant, and federation, process and node if they become
     * empty */
    if (participant) {
        ospl_DiscoveryDb_Object(participant)->state = Ospl_Offline;
        corto_object process = corto_parentof(participant);
        if (corto_scopeSize(process) == 1) {
            ospl_DiscoveryDb_Object(process)->state = Ospl_Offline;
            corto_object federation = corto_parentof(process);
            if (corto_scopeSize(federation) == 1) {
                ospl_DiscoveryDb_Object(federation)->state = Ospl_Offline;
                corto_object node = corto_parentof(federation);
                if (corto_scopeSize(node) == 1) {
                    ospl_DiscoveryDb_Object(node)->state = Ospl_Offline;
                    corto_delete(node);
                } else {
                    corto_delete(federation);
                }
            } else {
                corto_delete(process);
            }
        } else {
            corto_delete(participant);
        }
    }

/* $end */
}

corto_string _ospl_DiscoveryDb_getProductAttr(
    corto_string xml,
    corto_string attr)
{
/* $begin(ospl/health/DiscoveryDb/getProductAttr) */
    corto_xmlreader reader = corto_xmlMemoryReaderNew(xml, "Product");
    corto_xmlnode root = corto_xmlreaderRoot(reader);
    corto_xmlnode node = corto_xmlnodeFind(root, attr);
    corto_string result = NULL;
    if (node) {
        result = corto_xmlnodeStr(node);
    }
    corto_xmlreaderFree(reader);
    return result;
/* $end */
}

corto_bool _ospl_DiscoveryDb_updateDataReader(
    ospl_DiscoveryDb this,
    corto_uint32 systemId,
    corto_uint32 localId,
    corto_uint32 subscriberId,
    corto_string name)
{
/* $begin(ospl/health/DiscoveryDb/updateDataReader) */
    corto_bool result = FALSE;

    corto_object subscriber = ospl_DiscoveryDb_findEntity(
        this, systemId, subscriberId);

    if (subscriber) {
        result = TRUE;
        char id[16]; sprintf(id, "%d", localId);

        ospl_DiscoveryDb_DataReaderCreateChild(
            subscriber, id, this, localId, name);
    }

    /* Return TRUE if object was inserted, FALSE if rejected */
    return result;
/* $end */
}

corto_bool _ospl_DiscoveryDb_updateDataWriter(
    ospl_DiscoveryDb this,
    corto_uint32 systemId,
    corto_uint32 localId,
    corto_uint32 publisherId,
    corto_string name)
{
/* $begin(ospl/health/DiscoveryDb/updateDataWriter) */
    corto_bool result = FALSE;

    corto_object publisher = ospl_DiscoveryDb_findEntity(
        this, systemId, publisherId);

    if (publisher) {
        result = TRUE;
        char id[16]; sprintf(id, "%d", localId);

        ospl_DiscoveryDb_DataWriterCreateChild(
            publisher, id, this, localId, name);
    }

    /* Return TRUE if object was inserted, FALSE if rejected */
    return result;
/* $end */
}

corto_void _ospl_DiscoveryDb_updateDurability(
    ospl_DiscoveryDb this,
    corto_uint32 systemId,
    ospl_DurabilityState state)
{
/* $begin(ospl/health/DiscoveryDb/updateDurability) */
    ospl_DiscoveryDb_Federation f = ospl_DiscoveryDb_findFederation(this->mount, systemId);
    if (f) {
        if (f->durabilityService) {
            ospl_DiscoveryDb_Durability_setState(f->durabilityService, state);
        }
    }

/* $end */
}

corto_bool _ospl_DiscoveryDb_updateParticipant(
    ospl_DiscoveryDb this,
    corto_uint32 systemId,
    corto_uint32 localId,
    corto_string productXml)
{
/* $begin(ospl/health/DiscoveryDb/updateParticipant) */
    corto_bool result = FALSE;
    corto_string node = ospl_DiscoveryDb_getProductAttr(productXml, "NodeName");
    corto_string pid = ospl_DiscoveryDb_getProductAttr(productXml, "PID");
    corto_string federation = ospl_DiscoveryDb_getProductAttr(productXml, "FederationId");
    corto_string participant; corto_asprintf(&participant, "%x", localId);

    /* Find or create node */
    corto_object node_o = corto_lookup(this->mount, node);
    if (!node_o) {
        node_o = ospl_DiscoveryDb_NodeCreateChild(this->mount, node, this);
    }

    /* Find or create federation */
    corto_object federation_o = corto_lookup(node_o, federation);
    if (!federation_o) {
        federation_o = ospl_DiscoveryDb_FederationCreateChild(
            node_o,
            federation,
            this,
            systemId);
    }

    /* Find or create process */
    corto_object process_o = corto_lookup(federation_o, pid);
    if (!process_o) {
        corto_string name = ospl_DiscoveryDb_getProductAttr(
            productXml,
            "ExecName");
        process_o = ospl_DiscoveryDb_ProcessCreateChild(
            federation_o,
            pid,
            this,
            atoi(pid),
            name);
    }

    /* Find or create participant */
    corto_object participant_o = corto_lookup(process_o, participant);
    if (!participant_o) {
        corto_string name = ospl_DiscoveryDb_getProductAttr(productXml, "ParticipantName");
        corto_uint32 serviceType = atoi(ospl_DiscoveryDb_getProductAttr(productXml, "ServiceType"));
        result = TRUE;
        switch(serviceType) {
        case 4: /* Soap service */
            ospl_DiscoveryDb_SoapCreateChild(
                process_o,
                participant,
                this,
                localId,
                name);
            break;
        case 5: /* DDSI service */
            ospl_DiscoveryDb_DdsiCreateChild(
                process_o,
                participant,
                this,
                localId,
                name);
            break;
        case 8: /* Durability service */
            ospl_DiscoveryDb_DurabilityCreateChild(
                process_o,
                participant,
                this,
                localId,
                name);
            break;
        case 11: /* Splice daemon */
            ospl_DiscoveryDb_SplicedCreateChild(
                process_o,
                participant,
                this,
                localId,
                name);
            break;
        default:
            if (!serviceType) {
                ospl_DiscoveryDb_ParticipantCreateChild(
                    process_o,
                    participant,
                    this,
                    localId,
                    name);
            } else {
                ospl_DiscoveryDb_ServiceCreateChild(
                    process_o,
                    participant,
                    this,
                    localId,
                    name);
            }
            break;
        }
    }

    corto_dealloc(node);
    corto_dealloc(pid);
    corto_dealloc(federation);

    return result;
/* $end */
}

corto_bool _ospl_DiscoveryDb_updatePublisher(
    ospl_DiscoveryDb this,
    corto_uint32 systemId,
    corto_uint32 localId,
    corto_uint32 participantId,
    corto_string name)
{
/* $begin(ospl/health/DiscoveryDb/updatePublisher) */
    corto_bool result = FALSE;

    corto_object participant = ospl_DiscoveryDb_findEntity(
        this, systemId, participantId);

    if (participant) {
        result = TRUE;
        char id[16]; sprintf(id, "%d", localId);

        ospl_DiscoveryDb_PublisherCreateChild(
            participant, id, this, localId, name);
    }

    /* Return TRUE if object was inserted, FALSE if rejected */
    return result;
/* $end */
}

corto_bool _ospl_DiscoveryDb_updateSubscriber(
    ospl_DiscoveryDb this,
    corto_uint32 systemId,
    corto_uint32 localId,
    corto_uint32 participantId,
    corto_string name)
{
/* $begin(ospl/health/DiscoveryDb/updateSubscriber) */
    corto_bool result = FALSE;

    corto_object participant = ospl_DiscoveryDb_findEntity(
        this, systemId, participantId);

    if (participant) {
        result = TRUE;
        char id[16]; sprintf(id, "%d", localId);

        ospl_DiscoveryDb_SubscriberCreateChild(
            participant, id, this, localId, name);
    }

    /* Return TRUE if object was inserted, FALSE if rejected */
    return result;
/* $end */
}
