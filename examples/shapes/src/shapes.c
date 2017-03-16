/* $CORTO_GENERATED
 *
 * shapes.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/shapes.h>

/* $header() */
void onUpdate(corto_object this, corto_eventMask event, corto_result *shape, corto_subscriber subscriber) {
    printf("%s %s = %s\n", shape->parent, shape->id, corto_result_getText(shape));
}
/* $end */

int shapesMain(int argc, char *argv[]) {
/* $begin(main) */
    if (argc < 3) {
        printf("Usage: shapes [topic] [color]\n");
        printf("  Example: shapes Circle RED\n");
        return -1;
    }

    /* Create connector in root for default partition (creates participant) */
    ospl_ConnectorCreate(root_o, NULL);

    /* Create table with topic name and ShapeType type (creates topic) */
    corto_object topic = corto_tablescopeCreateChild(root_o, argv[1], ShapeType_o);

    /* Subscribe for instances in topic (creates reader) */
    corto_subscribe(CORTO_ON_UPDATE, "/", "%s/", argv[1]).contentType("text/json").callback(onUpdate);

    /* Forward declare shape with specified color (don't publish value yet) */
    ShapeType *s = ShapeTypeDeclareChild(topic, argv[2]);

    /* Make the shape turn in circles (creates writer, writes values to DDS) */
    corto_float32 t;
    for (t = 0; 1; t += 0.01) {
        ShapeTypeUpdate(s, cos(t) * 100, sin(t) * 100, 20);
        corto_sleep(0, 500000000);
    }

    return 0;
/* $end */
}
