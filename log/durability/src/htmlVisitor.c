/* $CORTO_GENERATED
 *
 * htmlVisitor.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/durability.h>

corto_void _durability_htmlVisitor_destruct(
    durability_htmlVisitor this)
{
/* $begin(ospl/log/durability/htmlVisitor/destruct) */

    printf("%s\n", corto_contentof(NULL, "text/json", this));

/* $end */
}

corto_void _durability_htmlVisitor_fellowLost(
    durability_htmlVisitor this,
    durability_parser parser,
    durability_fellowLost *node)
{
/* $begin(ospl/log/durability/htmlVisitor/fellowLost) */

    if (durability_fellowTracker_lost(&this->tracker, node->fellow, &node->header.t)) {
        corto_error("%.9d.%.9d %s", 
            node->header.t.sec,
            node->header.t.nanosec,
            corto_lasterr());
    }

/* $end */
}

corto_void _durability_htmlVisitor_fellowNew(
    durability_htmlVisitor this,
    durability_parser parser,
    durability_fellowNew *node)
{
/* $begin(ospl/log/durability/htmlVisitor/fellowNew) */

    durability_fellowTracker_new(&this->tracker, node->fellow, &node->header.t);

/* $end */
}
