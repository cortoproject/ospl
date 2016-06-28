#include "demo.h"

/* A demo project to show simple usage of the dynamic OpenSplice API */

void onCreate(corto_object this, corto_object o) {
    char *value = corto_str(o, 0);
    printf("CREATE %s => (%s)%s\n", corto_idof(o), corto_idof(corto_typeof(o)), value);
    corto_dealloc(value);
}

void onUpdate(corto_object this, corto_object o) {
    char *value = corto_str(o, 0);
    printf("UPDATE %s => (%s)%s\n", corto_idof(o), corto_idof(corto_typeof(o)), value);
    corto_dealloc(value);
}

void onDelete(corto_object this, corto_object o) {
    char *value = corto_str(o, 0);
    printf("DELETE %s => (%s)%s\n", corto_idof(o), corto_idof(corto_typeof(o)), value);
    corto_dealloc(value);
}

int demoMain(int argc, char *argv[]) {
    /* Create scope to hold topic instances */
    corto_voidCreateChild_auto(root_o, db);

    /* Create connector to CMParticipant */
    ospl_ConnectorCreate(db, "__BUILT-IN PARTITION__.CMParticipant", NULL);

    /* Create observers for CREATE, UPDATE and DELETE events */
    corto_observerCreate(CORTO_ON_DEFINE|CORTO_ON_TREE, db, onCreate);
    corto_observerCreate(CORTO_ON_UPDATE|CORTO_ON_TREE, db, onUpdate);
    corto_observerCreate(CORTO_ON_DELETE|CORTO_ON_TREE, db, onDelete);

    /* Keep alive */
    while (1) {
        corto_sleep(1, 0);
    }

    return 0;
}
