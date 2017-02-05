/* $CORTO_GENERATED
 *
 * parser.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/durability.h>

corto_void _durability_parser__matched(
    durability_parser this,
    corto_object node)
{
/* $begin(ospl/log/durability/parser/_matched) */

    printf("%s\n", corto_contentof(NULL, "text/corto-color", node));

/* $end */
}

corto_void _durability_parser_fellowLost(
    durability_parser this,
    durability_fellowLost *node)
{
/* $begin(ospl/log/durability/parser/fellowLost) */
/* $end */
}

corto_void _durability_parser_fellowNew(
    durability_parser this,
    durability_fellowNew *node)
{
/* $begin(ospl/log/durability/parser/fellowNew) */
/* $end */
}

corto_void _durability_parser_me(
    durability_parser this,
    durability_me *node)
{
/* $begin(ospl/log/durability/parser/me) */
    printf("My address = %d\n", node->fellow);
/* $end */
}
