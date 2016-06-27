/* $CORTO_GENERATED
 *
 * DiscoveryDb_Entity.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/health/health.h>

corto_uint32 _ospl_DiscoveryDb_Entity_count(
    ospl_DiscoveryDb_Entity this,
    corto_type type)
{
/* $begin(ospl/health/DiscoveryDb/Entity/count) */
    corto_uint32 count = 0;
    corto_objectseq scope = corto_scopeClaim(this);
    corto_objectseqForeach(scope, o) {
        if (corto_instanceof(type, o)) {
            count ++;
        }
        if (corto_scopeSize(o)) {
            count += _ospl_DiscoveryDb_Entity_count(o, type);
        }
    }
    corto_scopeRelease(scope);
    return count;
/* $end */
}

corto_string _ospl_DiscoveryDb_Entity_nameof(
    ospl_DiscoveryDb_Entity this)
{
/* $begin(ospl/health/DiscoveryDb/Entity/nameof) */

    return corto_strdup(this->entityName);

/* $end */
}
