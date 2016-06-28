/* $CORTO_GENERATED
 *
 * DiscoveryDb_Object.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/health/health.h>

corto_uint32 _ospl_DiscoveryDb_Object_count(
    ospl_DiscoveryDb_Object this,
    corto_type type)
{
/* $begin(ospl/health/DiscoveryDb/Object/count) */
    corto_uint32 count = 0;
    corto_objectseq scope = corto_scopeClaim(this);
    corto_objectseqForeach(scope, o) {
        if (corto_instanceof(type, o)) {
            count ++;
        }
        if (corto_scopeSize(o)) {
            count += ospl_DiscoveryDb_Object_count(o, type);
        }
    }
    corto_scopeRelease(scope);
    return count;
/* $end */
}

corto_string _ospl_DiscoveryDb_Object_value_v(
    ospl_DiscoveryDb_Object this)
{
/* $begin(ospl/health/DiscoveryDb/Object/value) */

    return NULL;

/* $end */
}
