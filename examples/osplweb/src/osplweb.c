/* $CORTO_GENERATED
 *
 * osplweb.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/osplweb.h>

/* $header() */
#define GREY    "\033[0;37m"
#define NORMAL  "\033[0;49m"
/* $end */

int osplwebMain(int argc, char *argv[]) {
/* $begin(main) */
    printf("Vortex web bridge v0.1\n");
    printf("  OSPL_URI      = %s'%s'%s\n", GREY, *ospl_uri_o, NORMAL);
    printf("  domainName    = %s'%s'%s\n", GREY,*ospl_domainName_o, NORMAL);
    printf("  domainId      = %s%d%s\n", GREY,*ospl_domainId_o, NORMAL);
    printf("  sharedMemory  = %s%s%s\n", GREY,ospl_singleProcess_o ? "false" : "true", NORMAL);
    printf("  admin address = %shttp://localhost:9090%s\n\n", GREY, NORMAL);

    /* Create OpenSplice health monitor */
    ospl_MonitorCreateChild_auto(root_o, osplmon, NULL, NULL);

    /* Connect all DDS topics */
    ospl_ConnectorCreateChild_auto(
        root_o,                /* create connector in root */
        osplx,                 /* name of connector */
        NULL,                  /* store instances in scope of connector */
        NULL,                  /* discover type from DDS*/
        NULL,                  /* default policy */
        "*.*",                 /* topic */
        NULL                   /* discover keylist from DDS */
    );

    /* Create corto admin */
    admin_serverCreate(9090);

    /* Keep alive */
    while (1) {
        corto_sleep(1, 0);
    }

    return 0;
/* $end */
}
