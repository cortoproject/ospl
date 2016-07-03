/* $CORTO_GENERATED
 *
 * BaseType.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/ospl.h>

corto_int16 _ospl_BaseType_init(
    ospl_BaseType* this)
{
/* $begin(ospl/BaseType/init) */

    if (!corto_checkAttr(this, CORTO_ATTR_SCOPED)) {
        corto_seterr("DDS instances must be scoped objects");
        goto error;
    }

    /* If object is created locally, set keyvalues from ID. If data comes from a
     * remote source, keys are set by ospl deserializer.
     *
     * TODO: objects that are serialized from remote don't need to set keys
     * each time they are deserialized. */
    if (!corto_ownerof(this)) {
        corto_id keys;
        sprintf(keys, "{%s}", corto_idof(this));

        /* Serialize strings to object */
        corto_fromStr(&this, keys);
    }

    return 0;
error:
    return -1;
/* $end */
}
