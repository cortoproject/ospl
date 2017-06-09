/* $CORTO_GENERATED
 *
 * Mount.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/ospl.h>

int16_t _ospl_Mount_construct(
    ospl_Mount this)
{
/* $begin(ospl/Mount/construct) */

    if (ospl_ddsInit()) {
        goto error;
    }

    if (!this->partition) {
        this->partition = corto_strdup("*");
    }

    /* Filter out built-in topics */
    corto_ptr_setstr(&corto_subscriber(this)->query.select, "^DCPS*|d_*|CM*|q_*");
    corto_ptr_setstr(&corto_subscriber(this)->query.from, this->from);
    corto_ptr_setstr(&corto_subscriber(this)->query.type, "tableinstance");

    corto_mount_setContentType(this, "text/corto");

    if (!corto_mount_construct(this)) {
        corto_ok("ospl: monitoring corto subscriptions for domain %d, partition '%s'", 
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

void _ospl_Mount_destruct(
    ospl_Mount this)
{
/* $begin(ospl/Mount/destruct) */

    /* << Insert implementation >> */

/* $end */
}

/* $header(ospl/Mount/onQuery) */
typedef struct ospl_Mount_iterData_t {
    corto_iter it;
    corto_ll results;
} ospl_Mount_iterData_t;
static int hasNext(corto_iter *it) {
    ospl_Mount_iterData_t *ctx = it->ctx;
    return corto_iter_hasNext(&ctx->it);
}
static void* next(corto_iter *it) {
    ospl_Mount_iterData_t *ctx = it->ctx;
    return corto_iter_next(&ctx->it);
}
static void release(corto_iter *it) {
    ospl_Mount_iterData_t *ctx = it->ctx;
    corto_iter_release(&ctx->it);
    corto_ll_free(ctx->results);
}
/* $end */
corto_resultIter _ospl_Mount_onQuery(
    ospl_Mount this,
    corto_query *query)
{
/* $begin(ospl/Mount/onQuery) */
    if (!strcmp(query->from, ".")) {
        ospl_DCPSTopicList topics = ospl_getTopics(corto_subscriber(this)->query.select);
        corto_iter it = corto_ll_iter(topics);
        corto_iter result;
        corto_resultList results = corto_ll_new();

        while (corto_iter_hasNext(&it)) {
            char *type;
            ospl_DCPSTopic topic = corto_iter_next(&it);

            if (corto_match(query->select, topic->name)) {
                type = ospl_cortoId(topic->type_name);
                corto_resultAssign(
                    corto_resultListAppendAlloc(results),
                    topic->name,
                    NULL,
                    ".",
                    "tableinstance",
                    (corto_word)"{}",
                    TRUE
                );
                corto_dealloc(type);
            }

        }

        corto_ptr_deinit(&topics, ospl_DCPSTopicList_o);

        /* Prepare iterator */
        ospl_Mount_iterData_t *data = corto_alloc(sizeof(ospl_Mount_iterData_t));
        data->results = results;
        data->it = corto_ll_iterAlloc(results);
        result.ctx = data;
        result.hasNext = hasNext;
        result.next = next;
        result.release = release;

        return result;
    } else {
        return CORTO_ITER_EMPTY;
    }
/* $end */
}

/* $header(ospl/Mount/onSubscribe) */
typedef struct ospl_Mount_topicMonitor_t {
    ospl_Mount this;
    corto_query query;
    corto_thread thread;
    bool quit;
    DDS_GuardCondition guard;
} ospl_Mount_topicMonitor_t;
static void* ospl_Mount_topicMonitor(void *data) {
    ospl_Mount_topicMonitor_t *ctx = data;
    corto_time timeout = {2147483647, 2147483647};

    corto_trace("ospl: starting topic monitor (filter = '%s')",
        ctx->query.select);

    while (!ctx->quit) {
        ospl_DCPSTopic sample = ospl_waitForTopic(
            corto_subscriber(ctx->this)->query.select, 
            &timeout,
            ctx->guard);

        if (sample) {
            if (corto_match(ctx->query.select, sample->name)) {
                corto_mount_publish(
                    ctx->this,
                    CORTO_ON_DEFINE,
                    ".",
                    sample->name,
                    "tableinstance",
                    (corto_word)"{}"
                );
            }

            corto_delete(sample);
        }
    }

    return NULL;
}
/* $end */
uintptr_t _ospl_Mount_onSubscribe(
    ospl_Mount this,
    corto_query *query,
    uintptr_t ctx)
{
/* $begin(ospl/Mount/onSubscribe) */
    uintptr_t result = 0;

    corto_trace("ospl: subscribe for ('%s', '%s')",
        query->select,
        query->from);

    /* If a subscriber subscribes for the root of the mount, create topic
     * listener subscription. */
    if (!strcmp(query->from, ".")) {
        ospl_Mount_topicMonitor_t *data = corto_calloc(sizeof(ospl_Mount_topicMonitor_t));
    
        data->this = this;
        data->quit = false;
        data->guard = DDS_GuardCondition__alloc();
        if (!data->guard) {
            corto_error("ospl: failed to create GuardCondition for thread monitor");
            corto_dealloc(data);
            goto error;
        }

        corto_ptr_copy(&data->query, corto_query_o, query);

        corto_thread t = corto_threadNew(ospl_Mount_topicMonitor, data);
        if (!t) {
            corto_error("ospl: failed to start topic monitor thread");
        }

        data->thread = t;

        result = (uintptr_t)data;
    } else {
        /* For nested queries, create Topic connectors */
        corto_object prev = corto_setOwner(this);
        result = (uintptr_t)ospl_Mount_TopicCreate(this, query->from);
        corto_setOwner(prev);
    }

    return result;
error:
    return 0;
/* $end */
}

void _ospl_Mount_onUnsubscribe(
    ospl_Mount this,
    corto_query *query,
    uintptr_t ctx)
{
/* $begin(ospl/Mount/onUnsubscribe) */

    /* Quit topic listener */
    if (!strcmp(query->from, ".")) {
        void *out;
        ospl_Mount_topicMonitor_t *data = (ospl_Mount_topicMonitor_t *)ctx;
        corto_trace("ospl: stopping topic monitor");

        /* Signal thread should stop */
        data->quit = true;
        DDS_ReturnCode_t status = DDS_GuardCondition_set_trigger_value(data->guard, TRUE);
        if (status != DDS_RETCODE_OK) {
            corto_error("failed to interrupt topic monitor");
        }

        /* Wait for thread to finish */
        corto_threadJoin(data->thread, &out);

        /* Cleanup resources */
        corto_ptr_deinit(&data->query, corto_query_o);
        DDS_free(data->guard);
        corto_dealloc(data);

        corto_trace("ospl: topic monitor stopped");
    } else {
        corto_delete((corto_object)ctx);
    }

/* $end */
}
