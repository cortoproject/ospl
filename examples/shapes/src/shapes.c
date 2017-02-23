/* $CORTO_GENERATED
 *
 * shapes.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/shapes.h>

/* $header() */
void onSubscribe(corto_object this, corto_eventMask event, corto_result *shape, corto_subscriber subscriber) {
    printf("Update %s %s = %s\n", 
        shape->parent, 
        shape->id, 
        corto_result_getText(shape));
}
/* $end */

int shapesMain(int argc, char *argv[]) {
/* $begin(main) */
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

    /* Observe updates from any instance in specified topic */
    corto_subscribe(CORTO_ON_UPDATE, "/", "%s/*", argv[1])
        .contentType("text/json")
        .callback(onSubscribe);

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
/* $end */
}
