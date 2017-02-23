/* $CORTO_GENERATED
 *
 * sequencemon.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/sequencemon.h>

/* $header() */
static corto_ll federations = NULL;
static corto_ll aliveFederations = NULL;
static corto_id master;

#define GREEN   "\033[1;32m"
#define NORMAL  "\033[0;49m"

void onUpdate(corto_object this, corto_eventMask event, corto_object observable, corto_observer observer) {
    if (corto_instanceof(ospl_DiscoveryDb_Federation_o, observable)) {
        if (!corto_llHasObject(federations, observable)) {
            if (corto_llHasObject(federations, NULL)) {
                corto_llReplace(federations, NULL, observable);
            } else {
                corto_llAppend(federations, observable);
            }
        }
    }
}

void onDelete(corto_object this, corto_eventMask event, corto_object observable, corto_observer observer) {
    if (corto_instanceof(ospl_DiscoveryDb_Federation_o, observable)) {
        corto_llReplace(federations, observable, NULL);
    }
}

void onEvent(ospl_Event *e) {
     if (corto_instanceof(ospl_NameSpacesEvent_o, e)) {
        if (ospl_NameSpacesEvent(e)->masterConfirmed) {
            sprintf(master, "%x", ospl_NameSpacesEvent(e)->master);
        }
    }
}
/* $end */

int sequencemonMain(int argc, char *argv[]) {
/* $begin(main) */
    federations = corto_llNew();
    aliveFederations = corto_llNew();

    /* Create scope that will contain discovery database */
    corto_voidCreateChild_auto(root_o, db);

    /* Create delegate for capturing events */
    ospl_Monitor_eventActionInitC_auto(onEventDelegate, onEvent);

    /* Create OpenSplice health monitor */
    ospl_MonitorCreateChild_auto(root_o, monitor, db, &onEventDelegate);
    if (!monitor) {
        goto error;
    }

    /* Observe updates in connector scope, where instances are stored */
    corto_observe(CORTO_ON_DEFINE|CORTO_ON_UPDATE|CORTO_ON_TREE, db).callback(onUpdate);
    corto_observe(CORTO_ON_DELETE|CORTO_ON_TREE, db).callback(onDelete);

    corto_int32 i = 0;
    while (1) {
        corto_iter it = corto_llIter(federations);
        corto_int32 count = 0;

        while (corto_iterHasNext(&it)) {
            corto_object f = corto_iterNext(&it);
            corto_bool isMaster = f && !strcmp(corto_idof(f), master);
            if (isMaster) printf("%s", GREEN);

            if (!(i % 10)) {
                printf("%11s", f ? corto_idof(f) : "");
            } else {
                printf("%*s%s%*s", 7, " ", f ? (isMaster?"M":"|") : " ", 3, " ");
            }

            if (isMaster) printf("%s", NORMAL);
            count++;
        }

        if (count) {
            printf("\n");
            i++;
        }

        corto_sleep(1, 0);
    }

    return 0;
error:
    return -1;
/* $end */
}
