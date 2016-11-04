#include "shapes.h"

void onUpdate(corto_object this, corto_eventMask event, corto_object shape, corto_observer observer) {
    printf("Update %s %s = %s\n",
      corto_idof(corto_parentof(shape)), corto_idof(shape), corto_contentof(NULL, "text/json", shape));
}

int shapesMain(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: shapes [topic] [color]\n");
        printf("  Example: shapes Circle RED\n");
        return -1;
    }

    /* Connect shape topic */
    ospl_Connector connector = ospl_ConnectorCreateChild(
        root_o,       /* create connector in root */
        argv[1],      /* object name of connector (same as topic) */
        NULL,         /* store instances in scope of connector */
        "/ShapeType", /* type */
        NULL,         /* default policy */
        argv[1],      /* topic */
        "color"       /* keylist */
    );

    /* Observe updates in connector scope */
    corto_observe(CORTO_ON_UPDATE|CORTO_ON_SCOPE, connector).callback(onUpdate);

    /* Create shape */
    ShapeType *s = ShapeTypeDeclareChild(connector, argv[2]);

    /* Make the shape turn in circles */
    corto_float32 t = 0;
    while (1) {
        ShapeTypeUpdate(s, cos(t) * 100, sin(t) * 100, 20);
        corto_sleep(0, 400000000);
        t += 0.01;
    }

    return 0;
}
