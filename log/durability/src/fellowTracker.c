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

durability_fellowTracker_master _durability_fellowTracker_findMaster(
    durability_fellowTracker* this,
    corto_string nameSpace)
{
/* $begin(ospl/log/durability/fellowTracker/findMaster) */
    corto_iter it = corto_llIter(this->fellows);
    durability_fellowTracker_master result = NULL;

    /* Find the fellow who was last master for specified namespace */
    while (corto_iterHasNext(&it)) {
        durability_fellowTracker_fellow f = corto_iterNext(&it);
        corto_iter itMaster = corto_llIter(f->master);
        while(corto_iterHasNext(&itMaster)) {
            durability_fellowTracker_master m = corto_iterNext(&itMaster);
            if (!strcmp(m->_namespace, nameSpace)) {
                if (!result) {
                    result = m;
                } else {
                    if (result->t.from.sec < m->t.from.sec) {
                        result = m;
                    }
                }
            }
        }
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

corto_int16 _durability_fellowTracker_lostMaster(
    durability_fellowTracker* this,
    corto_string nameSpace,
    corto_time *time)
{
/* $begin(ospl/log/durability/fellowTracker/lostMaster) */

    /* Logfile doesn't provide fellowId in message that indicates a new master
     * needs to be found, so look for namespace in all fellows */
    durability_fellowTracker_master m = 
        durability_fellowTracker_findMaster(this, nameSpace);
    
    if (!m) {
        corto_seterr("fellowTracker/newMaster: could not find existing master for nameSpace %s", nameSpace);
        goto error;
    }

    m->t.to = *time;

    return 0;
error:
    return -1;
/* $end */
}

corto_void _durability_fellowTracker_new(
    durability_fellowTracker* this,
    corto_int32 fellowId,
    corto_bool me,
    corto_time *time)
{
/* $begin(ospl/log/durability/fellowTracker/new) */
    corto_time dummy = {0, 0};

    durability_fellowTracker_fellow f = 
        durability_fellowTracker_find(this, fellowId);

    if (!f) {
        f = durability_fellowTracker_fellowCreate(fellowId, me, NULL, NULL);
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

corto_int16 _durability_fellowTracker_newMaster(
    durability_fellowTracker* this,
    corto_int32 fellowId,
    corto_string nameSpace,
    corto_time *time)
{
/* $begin(ospl/log/durability/fellowTracker/newMaster) */
    corto_time dummy = {0, 0};

    durability_fellowTracker_fellow f = 
        durability_fellowTracker_find(this, fellowId);

    if (!f) {
        corto_seterr("fellowTracker/newMaster: master %d for nameSpace %s unknown", fellowId, nameSpace);
        goto error;
    }

    durability_period p = {*time, dummy};
    durability_fellowTracker_master master = durability_fellowTracker_masterCreate(&p, nameSpace);
    corto_llAppend(f->master, master);

    /* Add nameSpace to list of namespaces (if not yet added) */
    corto_iter it = corto_llIter(this->nameSpaces);
    corto_bool found = FALSE;
    while (corto_iterHasNext(&it)) {
        corto_string s = corto_iterNext(&it);
        if (!strcmp(s, nameSpace)) {
            found = TRUE;
            break;
        }
    }

    if (!found) {
        corto_llAppend(this->nameSpaces, corto_strdup(nameSpace));
    }

    return 0;
error:
    return -1;
/* $end */
}

corto_void _durability_fellowTracker_stop(
    durability_fellowTracker* this,
    corto_time *time)
{
/* $begin(ospl/log/durability/fellowTracker/stop) */
    corto_iter it = corto_llIter(this->fellows);

    /* Set 'to' field for alive fellows to termination timestamp */
    while (corto_iterHasNext(&it)) {
        durability_fellowTracker_fellow f = corto_iterNext(&it);
        durability_period *p = corto_llLast(f->alive);
        if (p->to.sec == 0) {
            p->to = *time;
        }

        corto_iter itMaster = corto_llIter(f->master);
        while(corto_iterHasNext(&itMaster)) {
            durability_fellowTracker_master m = corto_iterNext(&itMaster);
            if (m->t.to.sec == 0) {
                m->t.to = *time;
            }
        }
    }

/* $end */
}
