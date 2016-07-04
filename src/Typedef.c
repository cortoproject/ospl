/* $CORTO_GENERATED
 *
 * Typedef.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/ospl.h>

corto_type _ospl_Typedef_actualType(
    ospl_Typedef this)
{
/* $begin(ospl/Typedef/actualType) */
    corto_object result = this->type;

    while (corto_instanceof(ospl_Typedef_o, result)) {
        result = ospl_Typedef(result)->type;
    }

    return corto_type(result);
/* $end */
}
