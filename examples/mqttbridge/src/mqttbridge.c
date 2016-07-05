#include "mqttbridge.h"
void printstats(corto_object o) {
    corto_mountstats s = corto_mount(o)->sent;
    corto_mountstats r = corto_mount(o)->received;
    printf("[%s] SENT:%lu RECEIVED:%lu",
      corto_idof(o), s.updates + s.deletes, r.updates + r.deletes);
}

int mqttbridgeMain(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: mqttbridge <broker> <topic>\n");
        printf("  Example: mqttbridge corto.io:1883 Circle\n");
        return -1;
    }

    corto_verbosity(CORTO_TRACE);

    /* Create scope for instances */
    corto_object topicScope = corto_voidCreateChild(root_o, argv[2]);

    /* Connect OpenSplice topic */
    ospl_ConnectorCreateChild_auto(
        root_o,                /* create connector in root */
        osplx,                 /* name of connector */
        topicScope,            /* store data here */
        NULL,                  /* automatically discover type */
        argv[2],               /* topic */
        NULL                   /* automatically discover keys */
    );

    /* Wait until DDS connector has discovered the topic type */
    while (!corto_mount(osplx)->type) {
        corto_sleep(0, 500000000);
    }

    /* Connect MQTT topic */
    mqtt_ConnectorCreateChild_auto(
        root_o,                   /* create connector in root */
        mqttx,                    /* name of connector */
        topicScope,               /* store data here */
        corto_mount(osplx)->type, /* use a fixed type (piggyback DDS type) */
        argv[2],                  /* topic */
        argv[1]                   /* broker */
    );

    /* Keep alive, print statistics */
    while (TRUE) {
        printstats(osplx); printf("    "); printstats(mqttx); printf("\n");
        corto_sleep(0, 100000000);
    }

    return 0;
}
