/* $CORTO_GENERATED
 *
 * admin.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/admin.h>

/* $header() */
/* Enter code outside of main here. */
/* $end */

int adminMain(int argc, char *argv[]) {
/* $begin(main) */

    /* Setup connector in root (creates participant, get domainId from OSPL_URI) */
    ospl_MountCreate("/", "*");

    /* Create corto admin on port 9090 */
    admin_serverCreate(9090);

    /* Keep alive */
    while (TRUE) corto_sleep(1, 0);

    return 0;
/* $end */
}
