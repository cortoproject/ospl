#include "webbridge.h"

#define GREY    "\033[0;37m"
#define NORMAL  "\033[0;49m"

int webbridgeMain(int argc, char *argv[]) {
    printf("Vortex web-monitor v0.1\n");
    printf("  OSPL_URI      = %s'%s'%s\n", GREY, *ospl_uri_o, NORMAL);
    printf("  domainName    = %s'%s'%s\n", GREY,*ospl_domainName_o, NORMAL);
    printf("  domainId      = %s%d%s\n", GREY,*ospl_domainId_o, NORMAL);
    printf("  sharedMemory  = %s%s%s\n", GREY,ospl_singleProcess_o ? "false" : "true", NORMAL);
    printf("  admin address = %shttp://localhost:9090%s\n\n", GREY, NORMAL);

    /* Create OpenSplice health monitor */
    ospl_MonitorCreateChild_auto(root_o, db, NULL, NULL);

    /* Connect GenericSensor topic */
    ospl_ConnectorCreateChild_auto(
        root_o,                /* create connector in root */
        GenericSensor,         /* name of connector */
        NULL,                  /* store instances in scope of connector */
        "/ipso/GenericSensor", /* type */
        NULL,                  /* default policy */
        "GenericSensor",       /* topic */
        "id"                   /* keylist */
    );

    /* Create corto admin */
    admin_serverCreate(9090);

    /* Keep alive */
    while (1) {
        corto_sleep(1, 0);
    }

    return 0;
}
