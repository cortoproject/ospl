/* $CORTO_GENERATED
 *
 * fellowTracker_fellow.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/durability.h>

durability_fellowTracker_master _durability_fellowTracker_fellow_findMaster(
    durability_fellowTracker_fellow this,
    corto_string nameSpace)
{
/* $begin(ospl/log/durability/fellowTracker/fellow/findMaster) */
    corto_iter it = corto_llIter(this->master);
    durability_fellowTracker_master result = NULL;

    while (corto_iterHasNext(&it)) {
        durability_fellowTracker_master m = corto_iterNext(&it);
        if (!strcmp(m->_namespace, nameSpace)) {
            result = m;
            /*break; find the last occurance of the master */
        }
    }

    return result;
/* $end */
}
