#include "osplmon.h"

int osplmonMain(int argc, char *argv[]) {

    printf("[osplmon] starting monitor\n");

    if (!ospl_ConnectorCreateChild(root_o, "monitor", NULL, "durabilityPartition.d_status", NULL)) {
        corto_error("osplmon: %s", corto_lasterr());
        goto error;
    }

    printf("[osplmon] active\n");

    while(1) {
        corto_sleep(1, 0);
    }

    return 0;
error:
    return -1;
}
