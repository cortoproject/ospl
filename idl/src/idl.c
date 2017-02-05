/* $CORTO_GENERATED
 *
 * idl.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/idl/idl.h>

/* $header() */
corto_threadKey idl_PARSER_KEY;

/* Parse an IDL file */
int idl_loadFile(corto_string file, int argc, char* argv[], void* udata) {
    idl_Parser p;
    CORTO_UNUSED(udata);
    CORTO_UNUSED(argc);
    CORTO_UNUSED(argv);

    /* Parse IDL */
    p = idl_ParserCreate(file, NULL);
    if (!p) {
        goto error;
    }
    
    idl_Parser_parse(p);
    corto_release(p);

    return 0;
error:
    return -1;
}
/* $end */

int idlMain(int argc, char *argv[]) {
/* $begin(main) */
    /* Insert code that must be run when component is loaded */
    CORTO_UNUSED(argc);
    CORTO_UNUSED(argv);
    if (corto_threadTlsKey(&idl_PARSER_KEY, NULL)) {
        return -1;
    }

    /* Register corto extension */
    corto_loaderRegister("idl", idl_loadFile, NULL);

    return 0;
/* $end */
}
