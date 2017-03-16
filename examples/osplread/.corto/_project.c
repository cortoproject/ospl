/* _project.c
 *
 * This file is generated. Do not modify.
 */

#include <include/read.h>

int readMain(int argc, char* argv[]);

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char* argv[]) {
    corto_start();
    if (corto_load("/corto/core/c", 0, NULL)) return -1;
    if (corto_load("/corto/lang/c", 0, NULL)) return -1;
    if (corto_load("/ospl", 0, NULL)) return -1;
    if (readMain(argc, argv)) return -1;
    corto_stop();
    return 0;
}

