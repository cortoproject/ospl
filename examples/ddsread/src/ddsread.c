#include "ddsread.h"

void onUpdate(corto_object this, corto_object o) {
    printf("Update %s = %s\n", corto_idof(o), corto_contentof(NULL, "text/corto", o));
}

int ddsreadMain(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ddsread <topic>\n");
        printf("  Example: ddsclient durabilityPartition.d_status\n");
        return -1;
    }

    /* Connect GenericSensor topic */
    ospl_ConnectorCreateChild_auto(
        root_o,                /* create connector in root */
        osplx,                 /* name of connector */
        NULL,                  /* store instances in scope of connector */
        NULL,                  /* type */
        NULL,                  /* default policy */
        argv[1],               /* topic */
        NULL                   /* keylist */
    );

    /* Observe updates in connector scope, where instances are stored */
    corto_observerCreate(CORTO_ON_UPDATE|CORTO_ON_SCOPE, osplx, onUpdate);

    while (TRUE) {
        corto_sleep(1, 0);
    }

    return 0;
}
