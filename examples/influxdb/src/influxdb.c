/* $CORTO_GENERATED
 *
 * influxdb.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/influxdb.h>

int influxdbMain(int argc, char *argv[]) {
/* $begin(main) */
    printf("Vortex influxdb bridge v0.1\n");

    /* Connect all user topics */
    ospl_ConnectorCreateChild_auto(
        root_o,                 /* create connector in root */
        osplx,                  /* name of connector */
        NULL,                   /* store topics in osplx (default) */
        NULL,                   /* discover type from DDS */
        NULL,                   /* default policy */
        "*.^DCPS*|CM*|d_*|q_*", /* topic (no builtins) */
        NULL                    /* discover keylist from DDS */
    );

    /* Create influxdb connector */
    influxdb_ConnectorCreate_auto(
        influxdb,                 /* name */
        osplx,                    /* connect to DDS data */
        "sampleRate=1",           /* store at most 1 update per second per object */
        "http://localhost:8086",  /* hostname */
        "mydb"                    /* database name */
    );

    /* Keep alive */
    while (1) {
        corto_sleep(1, 0);
    }

    return 0;
/* $end */
}
