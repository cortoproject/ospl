/* $CORTO_GENERATED
 *
 * read.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/read.h>

/* $header() */
void onUpdate(corto_object this, corto_eventMask mask, corto_result *instance, corto_subscriber subscriber) {
    corto_info("%s %s = %s", instance->parent, instance->id, corto_result_getText(instance));
}
/* $end */

int readMain(int argc, char *argv[]) {
/* $begin(main) */
    if (argc < 2) {
        printf("Usage: read [partition] <topic>\n");
        printf("  Example: corto run ospl/read Circle\n");
        return -1;
    }

    char *partition = argc == 2 ? NULL : argv[1];
    char *topic = argc == 2 ? argv[1] : argv[2];

    /* Setup connector in root (creates participant, get domainId from OSPL_URI) */
    ospl_ConnectorCreate(root_o, partition);

    /* Subscribe for updates in topic (wait & subscribe for topic from DDS) */
    corto_subscribe(CORTO_ON_UPDATE, "/", "%s/", topic).contentType("text/json").callback(onUpdate);

    /* Keep alive */
    while (TRUE) corto_sleep(1, 0);

    return 0;
/* $end */
}
