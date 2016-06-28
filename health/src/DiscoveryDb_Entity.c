/* $CORTO_GENERATED
 *
 * DiscoveryDb_Entity.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/health/health.h>

corto_string _ospl_DiscoveryDb_Entity_nameof(
    ospl_DiscoveryDb_Entity this)
{
/* $begin(ospl/health/DiscoveryDb/Entity/nameof) */

    return corto_strdup(this->entityName);

/* $end */
}
