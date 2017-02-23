/* $CORTO_GENERATED
 *
 * mqttbridge.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/mqttbridge.h>

/* $header() */
#define GREEN   "\033[0;32m"
#define MAGENTA "\033[1;35m"
#define GREY    "\033[0;37m"
#define NORMAL  "\033[0;49m"
#define BLUE    "\033[0;34m"
#define YELLOW  "\033[0;33m"

void erase(void){int i=80;for(;i;i--)putchar('\b');}
corto_uint64 count(corto_mountStats *s) {
    return s->updates + s->deletes;
}
char* printStats(corto_id buffer, corto_object m, int orientation) {
    corto_id send, x, recv, discard;
    corto_uint64 discarded = count(&corto_mount(m)->sentDiscarded);
    discard[0] = '\0';
    sprintf(send, "%s%lu%s", BLUE, count(&corto_mount(m)->sent), NORMAL);
    sprintf(x, "<-%s%s%s->", MAGENTA, corto_idof(m), NORMAL);
    sprintf(recv, "%s%lu%s", BLUE, count(&corto_mount(m)->received), NORMAL);
    if (discarded) {
        static float tick = 0; tick += 0.5;
        if (!orientation) sprintf(discard, " %c %s%lu%s", (char[]){'|', '\\', '-', '/'}[(int)tick % 4], GREY, discarded, NORMAL);
        else sprintf(discard, "%s%lu%s %c ", GREY, discarded, NORMAL, (char[]){'|', '/', '-', '\\'}[(int)tick % 4]);
    }
    if (!orientation) sprintf(buffer, "%s%s%s%s", send, discard, x, recv);
    else sprintf(buffer, "%s%s%s%s", recv, x, discard, send);
    return buffer;
}

/* $end */

int mqttbridgeMain(int argc, char *argv[]) {
/* $begin(main) */
    if (argc < 3) {
        printf("Usage: mqttbridge <broker> <topic>\n");
        printf("  Example: mqttbridge corto.io:1883 Circle\n");
        return -1;
    }

    /* Create scope for instances */
    corto_object topicScope = corto_voidCreateChild(root_o, argv[2]);

    /* Connect OpenSplice topic */
    ospl_ConnectorCreateChild_auto(
        root_o,                /* create connector in root */
        osplx,                 /* name of connector */
        topicScope,            /* store data here */
        NULL,                  /* automatically discover type */
        NULL,                  /* default policy */
        argv[2],               /* topic */
        NULL                   /* automatically discover keys */
    );
    if (!osplx) goto error;

    /* Wait until DDS connector has discovered the topic type */
    while (!corto_observer(osplx)->type) {
        corto_sleep(0, 500000000);
    }

    /* Connect MQTT topic */
    mqtt_ConnectorCreateChild_auto(
        root_o,                   /* create connector in root */
        mqttx,                    /* name of connector */
        topicScope,               /* store data here */
        corto_observer(osplx)->type, /* use a fixed type (piggyback DDS type) */
        "sampleRate=1",         /* limit sampleRate to 0.5Hz */
        argv[1]                   /* broker */
    );
    if (!mqttx) goto error;

    /* Keep alive, print statistics */
    while (TRUE) {
        corto_id b1, b2;
        corto_sleep(0, 80000000);
        printf("  (MQTT) %s  %s (DDS)", printStats(b1, mqttx, 0), printStats(b2, osplx, 1));
        erase();
        fflush(stdout);
    }

    return 0;
error:
    corto_error("mqttbridge: %s", corto_lasterr());
    return -1;
/* $end */
}
