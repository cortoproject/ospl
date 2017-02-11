/* $CORTO_GENERATED
 *
 * durability.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/durability.h>

int durabilityMain(int argc, char *argv[]) {
/* $begin(main) */

    if (argc < 2) {
        corto_seterr("invalid number of arguments\n Example: corto run ospl/durability durability.log\n");
        goto error;
    }

    /* Create parser */
    durability_parser parser = durability_parserCreate();
    if (!parser) {
        goto error;
    }

    /* Create visitor */
    durability_htmlVisitor visitor = durability_htmlVisitorCreate();
    if (!visitor) {
        goto error;
    }

    /* Feed file into parser */
    if (x_parseFile(parser, argv[1], visitor)) {
        goto error;
    }

    corto_delete(visitor);
    corto_delete(parser);

    return 0;
error:
    corto_error("durability: %s", corto_lasterr());
    return -1;
/* $end */
}
