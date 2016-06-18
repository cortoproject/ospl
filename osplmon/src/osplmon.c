#include "osplmon.h"

int osplmonMain(int argc, char *argv[]) {

    if (!ospl_ConnectorCreateChild(root_o, "monitor", NULL, "durabilityPartition.d_status", NULL)) {
        goto error;
    }

    while(1) { corto_sleep(1, 0); }

    return 0;
error:
    corto_error("%s", corto_lasterr());
    return -1;
}
