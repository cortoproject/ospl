/* $CORTO_GENERATED
 *
 * fellowTracker.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/durability.h>

durability_fellowTracker_fellow _durability_fellowTracker_find(
    durability_fellowTracker* this,
    corto_int32 fellowId)
{
/* $begin(ospl/log/durability/fellowTracker/find) */
    corto_iter it = corto_llIter(this->fellows);
    durability_fellowTracker_fellow result = NULL;

    while (corto_iterHasNext(&it)) {
        durability_fellowTracker_fellow f = corto_iterNext(&it);
        if (f->id == fellowId) {
            result = f;
            break;
        }
    }

    if (result) {
        corto_claim(result);
    }

    return result;
/* $end */
}

corto_int16 _durability_fellowTracker_lost(
    durability_fellowTracker* this,
    corto_int32 fellowId,
    corto_time *time)
{
/* $begin(ospl/log/durability/fellowTracker/lost) */
    durability_fellowTracker_fellow f = 
        durability_fellowTracker_find(this, fellowId);

    if (!f) {
        corto_seterr("fellowTracker/lost: fellow %d does not exist", fellowId);
        goto error;
    }

    durability_period *p = corto_llLast(f->alive);
    if (p->to.sec || p->to.nanosec) {
        corto_seterr("fellowTracker/lost: fellow %d is not alive", fellowId);
        goto error;
    }

    p->to = *time;

    corto_release(f);

    return 0;
error:
    return -1;
/* $end */
}

corto_void _durability_fellowTracker_new(
    durability_fellowTracker* this,
    corto_int32 fellowId,
    corto_time *time)
{
/* $begin(ospl/log/durability/fellowTracker/new) */
    corto_time dummy = {0, 0};

    durability_fellowTracker_fellow f = 
        durability_fellowTracker_find(this, fellowId);

    if (!f) {
        f = durability_fellowTracker_fellowCreate(fellowId, NULL);
        corto_claim(f);
        corto_llAppend(this->fellows, f);
    }

    durability_periodAssign(
        durability_periodListAppendAlloc(f->alive),
        time,
        &dummy
    );

    corto_release(f);

/* $end */
}
