#include "osplmon.h"

#define BLACK   "\033[1;30m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define WHITE   "\033[1;37m"
#define GREY    "\033[0;37m"
#define NORMAL  "\033[0;49m"
#define BOLD    "\033[1;49m"

static corto_object dbroot;

void printIndent(corto_object o) {
    corto_object p = o;
    while ((p = corto_parentof(p)) && (p != dbroot)) {
        printf(" ");
    }
}

char* column(corto_id buff, char* str, corto_uint32 width) {
    strcpy(buff, str);
    int i; for (i = strlen(buff); i < width; i++) {
        strcat(buff, " ");
    }
    return buff;
}

char* color(corto_object o) {
    if (corto_instanceof(ospl_DiscoveryDb_Object_o, o)) {
        ospl_DiscoveryDb_Object obj = o;
        switch(obj->state) {
        case Ospl_Initializing:
        case Ospl_Degraded:
            return YELLOW;
        case Ospl_Error:
            return RED;
        case Ospl_Operational:
            return GREEN;
        case Ospl_Unknown:
        case Ospl_Offline:
        case Ospl_Idle:
        default:
            return GREY;
        }
    }
    return GREEN;
}

void onCreate(corto_object this, corto_object o) {
    corto_id b;
    printIndent(o);
    printf("%s> %s '%s'%s\n",
      color(o), column(b, corto_idof(corto_typeof(o)), 12), corto_pathname(NULL, dbroot, o, "/"), NORMAL);
}

void onUpdate(corto_object this, corto_object o) {
    corto_id b;
    char *value = ospl_DiscoveryDb_Object_value(o);
    printIndent(o);
    printf("%s! %s '%s' => %s%s\n",
      color(o), column(b, corto_idof(corto_typeof(o)), 12), corto_pathname(NULL, dbroot, o, "/"), value, NORMAL);
    corto_dealloc(value);
}

void onDelete(corto_object this, corto_object o) {
    corto_id b;
    printIndent(o);
    printf("%sX %s '%s'%s\n",
      color(o), column(b, corto_idof(corto_typeof(o)), 12), corto_pathname(NULL, dbroot, o, "/"), NORMAL);
}

int osplmonMain(int argc, char *argv[]) {

    corto_voidCreateChild_auto(root_o, db);
    ospl_MonitorCreateChild_auto(root_o, monitor, db);
    if (!monitor) {
        goto error;
    }

    dbroot = db;

    corto_observerCreate_auto(createObserver, CORTO_ON_DEFINE|CORTO_ON_TREE, db, onCreate);
    corto_observerCreate_auto(updateObserver, CORTO_ON_UPDATE|CORTO_ON_TREE, db, onUpdate);
    corto_observerCreate_auto(deleteObserver, CORTO_ON_DELETE|CORTO_ON_TREE, db, onDelete);

    while(1) { corto_sleep(1, 0); }

    return 0;
error:
    corto_error("%s", corto_lasterr());
    return -1;
}
