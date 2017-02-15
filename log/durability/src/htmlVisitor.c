/* $CORTO_GENERATED
 *
 * htmlVisitor.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/durability.h>

corto_void _durability_htmlVisitor__matched(
    durability_htmlVisitor this,
    durability_parser parser,
    corto_object node)
{
/* $begin(ospl/log/durability/htmlVisitor/_matched) */
    corto_value t, o = corto_value_object(node, NULL);

    if (!corto_value_getMember(&o, "header.t", &t)) {
        this->lastReportedTime = *(corto_time*)corto_value_getPtr(&t);
    } else if (!corto_value_getMember(&o, "t", &t)) {
        corto_lasterr(); /* Catch error from previous getMember */
        this->lastReportedTime = *(corto_time*)corto_value_getPtr(&t);
    }

/* $end */
}

corto_void _durability_htmlVisitor_destruct(
    durability_htmlVisitor this)
{
/* $begin(ospl/log/durability/htmlVisitor/destruct) */
    corto_string tmpl = corto_fileLoad("index.tmpl");
    FILE *f = fopen("index.html", "w");
    if (!f) {
        corto_error("cannot open index.html: %s", corto_lasterr);
    } else {
        /* Set timestamps of fellows that are still alive */
        durability_fellowTracker_stop(&this->tracker, &this->lastReportedTime);

        char *output = corto_replace(tmpl, "$data", corto_contentof(NULL, "text/json", this));
        fprintf(f, "%s", output);
        corto_dealloc(output);
        fclose(f);
    }

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

    durability_fellowTracker_new(&this->tracker, node->fellow, FALSE, &node->header.t);

/* $end */
}

corto_void _durability_htmlVisitor_me(
    durability_htmlVisitor this,
    durability_parser parser,
    durability_me *node)
{
/* $begin(ospl/log/durability/htmlVisitor/me) */

    durability_fellowTracker_new(&this->tracker, node->fellow, TRUE, &node->header.t);
    this->isMe = node->fellow;

/* $end */
}

corto_void _durability_htmlVisitor_nsIAmMaster(
    durability_htmlVisitor this,
    durability_parser parser,
    durability_nsIAmMaster *node)
{
/* $begin(ospl/log/durability/htmlVisitor/nsIAmMaster) */

    durability_fellowTracker_newMaster(&this->tracker, this->isMe, node->_namespace, &node->header.t);

/* $end */
}

corto_void _durability_htmlVisitor_nsMasterConfirmed(
    durability_htmlVisitor this,
    durability_parser parser,
    durability_nsMasterConfirmed *node)
{
/* $begin(ospl/log/durability/htmlVisitor/nsMasterConfirmed) */

    if (node->fellow) {
        durability_fellowTracker_newMaster(&this->tracker, node->fellow, node->_namespace, &node->header.t);
    }

/* $end */
}

corto_void _durability_htmlVisitor_nsMasterFinding(
    durability_htmlVisitor this,
    durability_parser parser,
    durability_nsMasterFinding *node)
{
/* $begin(ospl/log/durability/htmlVisitor/nsMasterFinding) */

    durability_fellowTracker_lostMaster(&this->tracker, node->_namespace, &node->header.t);

/* $end */
}

corto_void _durability_htmlVisitor_terminating(
    durability_htmlVisitor this,
    durability_parser parser,
    durability_terminating *node)
{
/* $begin(ospl/log/durability/htmlVisitor/terminating) */

    durability_fellowTracker_stop(&this->tracker, &node->t);

/* $end */
}
