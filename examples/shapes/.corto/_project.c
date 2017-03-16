/* _project.c
 *
 * This file is generated. Do not modify.
 */

#include <include/shapes.h>

int shapesMain(int argc, char* argv[]);

int shapes_load(void);

#ifdef __cplusplus
extern "C"
#endif
SHAPETYPE_EXPORT int main(int argc, char* argv[]) {
    corto_start();
    if (corto_load("/corto/core/c", 0, NULL)) return -1;
    if (corto_load("/corto/lang/c", 0, NULL)) return -1;
    if (corto_load("/corto/ext/idl", 0, NULL)) return -1;
    if (corto_load("/ospl", 0, NULL)) return -1;
    if (shapes_load()) return -1;
    if (shapesMain(argc, argv)) return -1;
    corto_stop();
    return 0;
}

