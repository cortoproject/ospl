#include "mqttclient.h"

void onUpdate(corto_object this, corto_object o) {
    printf("Update %s = %s\n", corto_idof(o), corto_contentof(NULL, "text/corto", o));
}

int mqttclientMain(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: mqttclient <sensor_id> [unit]\n");
        printf("  Example: mqttclient my_sensor Cel\n");
        return -1;
    }

    /* Connect GenericSensor topic */
    mqtt_ConnectorCreateChild_auto(
        root_o,                /* create connector in root */
        mqttx,                 /* name of connector */
        NULL,                  /* store instances in scope of connector */
        "/ipso/GenericSensor", /* type */
        "GenericSensor",       /* topic */
        "corto.io:1883"        /* host */
    );

    /* Observe updates in connector scope, where instances are stored */
    corto_observerCreate(CORTO_ON_UPDATE|CORTO_ON_SCOPE, mqttx, onUpdate);

    /* Create generic sensor */
    ipso_GenericSensor *s = ipso_GenericSensorCreateChild(mqttx, argv[1], 0, SetCond(argc == 3, argv[2]));

    /* Publish waveform */
    corto_float32 t = 0;
    while (1) {
        ipso_GenericSensorUpdate(s, cos(t += 0.5) * 100, SetCond(argc == 3, argv[2]));
        corto_sleep(0, 500000000);
    }

    return 0;
}
