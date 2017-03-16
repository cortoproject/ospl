/* $CORTO_GENERATED
 *
 * Connector.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/ospl.h>

corto_int16 _ospl_Connector_construct(
    ospl_Connector this)
{
/* $begin(ospl/Connector/construct) */

    if (ospl_ddsInit()) {
        goto error;
    }

    if (!this->partition) {
        this->partition = corto_strdup("");
    }

    if (!corto_mount_construct(this)) {
        corto_ok("ospl: monitoring subscriptions for domain %d, partition '%s'", 
            *ospl_domainId_o,
            this->partition[0] ? this->partition : "<DEFAULT>");
    } else {
        goto error;
    }

    return 0;
error:
    return -1;
/* $end */
}

corto_void _ospl_Connector_destruct(
    ospl_Connector this)
{
/* $begin(ospl/Connector/destruct) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _ospl_Connector_onNotify(
    ospl_Connector this,
    corto_eventMask event,
    corto_result *object)
{
/* $begin(ospl/Connector/onNotify) */

    /* << Insert implementation >> */

/* $end */
}

corto_resultIter _ospl_Connector_onRequest(
    ospl_Connector this,
    corto_request *request)
{
/* $begin(ospl/Connector/onRequest) */

    return CORTO_ITERATOR_EMPTY;

/* $end */
}

corto_word _ospl_Connector_onSubscribe(
    ospl_Connector this,
    corto_string parent,
    corto_string expr,
    corto_word ctx)
{
/* $begin(ospl/Connector/onSubscribe) */
    ospl_Connector_Connection result = NULL;

    /* 'parent' represents a topic name, 'expr' represents an instance key */

    if (!ctx) {
        /* Ignore 'expr' for now, future versions could create content filters based
         * on (simple) filters. */
        corto_ok("ospl: new subscription for '%s'", parent);

        /* First level of hierarchy is just for topics */
        if (strcmp(parent, ".")) {

            /* Create a connection for the specified parent (topic) */
            result = ospl_Connector_ConnectionCreate(this, parent);
            if (!result) {
                corto_error("ospl: failed to create connection for '%s': %s",
                    parent, corto_lasterr());
                goto error;
            }
        }
    }

error:
    return (corto_word)result;
/* $end */
}

corto_void _ospl_Connector_onUnsubscribe(
    ospl_Connector this,
    corto_string parent,
    corto_string expr,
    corto_word ctx)
{
/* $begin(ospl/Connector/onUnsubscribe) */

    corto_delete((corto_object)ctx);
    corto_ok("ospl: unsubscribed for '%s'", parent);

/* $end */
}
