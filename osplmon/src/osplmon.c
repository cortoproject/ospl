#include "osplmon.h"

#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define GREY    "\033[0;37m"
#define NORMAL  "\033[0;49m"

static corto_object dbroot;
static corto_time timer;
static corto_bool printSummary;

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

void print(char action, corto_object o) {
    corto_id b;
    char *value = ospl_DiscoveryDb_Object_value(o);
    printIndent(o);
    if (value) {
        printf("%s%c %s '%s' => %s%s\n",
          color(o), action, column(b, corto_idof(corto_typeof(o)), 12), corto_pathname(NULL, dbroot, o, "/"), value, NORMAL);
        corto_dealloc(value);
    } else {
        printf("%s%c %s '%s'%s\n",
          color(o), action, column(b, corto_idof(corto_typeof(o)), 12), corto_pathname(NULL, dbroot, o, "/"), NORMAL);
    }
    corto_timeGet(&timer);
    printSummary = TRUE;
}

void onCreate(corto_object this, corto_object o) {
    print('>', o);
}

void onUpdate(corto_object this, corto_object o) {
    print('!', o);
}

void onDelete(corto_object this, corto_object o) {
    print('X', o);
}

void onEvent(ospl_Event *e) {
    printf("Event!\n");
}

int osplmonMain(int argc, char *argv[]) {
    printf("OpenSplice monitor v0.1\n");
    printf("  OSPL_URI      = %s'%s'%s\n", GREY, *ospl_uri_o, NORMAL);
    printf("  domainName    = %s'%s'%s\n", GREY,*ospl_domainName_o, NORMAL);
    printf("  domainId      = %s%d%s\n", GREY,*ospl_domainId_o, NORMAL);
    printf("  sharedMemory  = %s%s%s\n\n", GREY,ospl_singleProcess_o ? "false" : "true", NORMAL);

    corto_voidCreateChild_auto(root_o, db);
    ospl_Monitor_eventActionInitC_auto(onEventDelegate, onEvent);
    ospl_MonitorCreateChild_auto(root_o, monitor, db, &onEventDelegate);
    if (!monitor) {
        goto error;
    }

    dbroot = db;

    corto_observerCreate_auto(createObserver, CORTO_ON_DEFINE|CORTO_ON_TREE, db, onCreate);
    corto_observerCreate_auto(updateObserver, CORTO_ON_UPDATE|CORTO_ON_TREE, db, onUpdate);
    corto_observerCreate_auto(deleteObserver, CORTO_ON_DELETE|CORTO_ON_TREE, db, onDelete);

    while(1) {
        corto_time now;
        corto_timeGet(&now);

        if (printSummary && ((corto_timeToDouble(now) - corto_timeToDouble(timer)) > 12)) {
            printf("\n================================================================================\n");
            printf(" Summary: Nodes:%d Processes:%d Federations:%d Participants:%d\n",
              monitor->db->nodeCount,
              monitor->db->processCount,
              monitor->db->federationCount,
              monitor->db->participantCount);
            printf("================================================================================\n\n");
            printSummary = FALSE;
        }
        corto_sleep(1, 0);
    }

    return 0;
error:
    corto_error("%s", corto_lasterr());
    return -1;
}
