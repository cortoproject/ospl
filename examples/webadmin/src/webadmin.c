#include "webadmin.h"

#define GREY    "\033[0;37m"
#define NORMAL  "\033[0;49m"

int webadminMain(int argc, char *argv[]) {
    printf("Vortex web-monitor v0.1\n");
    printf("  OSPL_URI      = %s'%s'%s\n", GREY, *ospl_uri_o, NORMAL);
    printf("  domainName    = %s'%s'%s\n", GREY,*ospl_domainName_o, NORMAL);
    printf("  domainId      = %s%d%s\n", GREY,*ospl_domainId_o, NORMAL);
    printf("  sharedMemory  = %s%s%s\n", GREY,ospl_singleProcess_o ? "false" : "true", NORMAL);
    printf("  admin address = %shttp://localhost:9090%s\n\n", GREY, NORMAL);

    /* Create scope that will contain discovery database */
    corto_voidCreateChild_auto(root_o, db);

    /* Create OpenSplice health monitor */
    ospl_MonitorCreateChild_auto(root_o, monitor, db, NULL);
    if (!monitor) {
        goto error;
    }

    /* Create corto admin */
    admin_serverCreate(9090);

    /* Keep alive */
    while (1) {
        corto_sleep(1, 0);
    }

    return 0;
error:
    corto_error("%s", corto_lasterr);
    return -1;
}
