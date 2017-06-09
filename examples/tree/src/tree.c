/* $CORTO_GENERATED
 *
 * tree.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/tree.h>

/* $header() */
void onUpdate(corto_object this, corto_eventMask event, corto_object o, corto_observer observer) {
    corto_value v = corto_value_value(&event, corto_eventMask_o);
    char *eventStr = corto_value_contentof(&v, "text/corto");
    corto_info("%s: %s: '%s' %s", eventStr + 3,
        corto_fullpath(NULL, corto_parentof(o)),
        corto_idof(o),
        corto_contentof(NULL, "text/json", o));
    corto_dealloc(eventStr);
}
/* $end */

int treeMain(int argc, char *argv[]) {
/* $begin(main) */
    char *key = NULL;
    tree_Car instance = NULL;
    tree_Car_Engine engine = NULL;
    tree_Car_Wheel fl, fr, bl, br;

    if (argc == 1) {
        corto_info("starting 'ospl/tree' in reader mode (specify 'id' argument to start writer)");
    } else {
        corto_info("starting 'ospl/tree' in reader/writer mode");
        key = argv[1];
    }

    /* Create connector in root for default partition */
    ospl_Connector participant = ospl_ConnectorCreate("/", NULL);

    /* Instantiate table with name 'Car' and type 'tree/Car' (creates topic) */
    corto_object topic = corto_tableinstanceCreateChild(root_o, "Car", tree_Car_o);

    if (key) {
        /* If key is provided, create new car instance */
        instance = tree_CarDeclareChild(topic, key);

        /* Lookup singleton Engine object */
        engine = corto_lookup(instance, "Engine");

        /* Populate 'Wheel' table */
        fl = corto_declareChild(instance, "Wheel/FrontLeft", tree_Car_Wheel_o);
        fr = corto_declareChild(instance, "Wheel/FrontRight", tree_Car_Wheel_o);
        bl = corto_declareChild(instance, "Wheel/BackLeft", tree_Car_Wheel_o);
        br = corto_declareChild(instance, "Wheel/BackRight", tree_Car_Wheel_o);
    }

    /* Subscribe for instances in topic (creates reader) */
    corto_observe(CORTO_ON_UPDATE|CORTO_ON_TREE, topic).callback(onUpdate);

    corto_define(instance);

    while (1) {
        if (instance) {
            tree_CarUpdate(instance, 10);
            tree_Car_WheelUpdate(fl, 100);
            tree_Car_WheelUpdate(fr, 200);
            tree_Car_WheelUpdate(bl, 150);
            tree_Car_WheelUpdate(br, 300);
        }

        corto_sleep(1, 0);
    }

    return 0;
/* $end */
}
