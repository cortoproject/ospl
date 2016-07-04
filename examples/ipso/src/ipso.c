#include "ipso.h"

void onUpdate(corto_object this, corto_object sensor) {
    corto_string value = corto_str(sensor, 0);
    printf("Update %s = %s\n", corto_idof(sensor), value);
    corto_dealloc(value);
}

int ipsoMain(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ipso <sensor_id> [unit]\n");
        printf("  Example: ipso my_sensor Cel\n");
        return -1;
    }

    /* Connect GenericSensor topic */
    ospl_ConnectorCreateChild_auto(
        root_o,                /* create connector in root */
        connector,             /* object name of connector */
        NULL,                  /* store instances in scope of connector */
        "GenericSensor",       /* topic */
        "/ipso/GenericSensor", /* type */
        "id"                   /* keylist */
    );

    /* Observe updates in connector scope */
    corto_observerCreate(CORTO_ON_UPDATE|CORTO_ON_SCOPE, connector, onUpdate);

    /* Create generic sensor */
    ipso_GenericSensor *s = ipso_GenericSensorCreateChild(connector, argv[1], 0, SetCond(argc == 3, argv[2]));

    /* Publish waveform */
    corto_float32 t = 0;
    while (1) {
        ipso_GenericSensorUpdate(s, cos(t += 0.5) * 100, SetCond(argc == 3, argv[2]));
        corto_sleep(0, 500000000);
    }

    return 0;
}
