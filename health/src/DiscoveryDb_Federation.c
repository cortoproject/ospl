/* $CORTO_GENERATED
 *
 * DiscoveryDb_Federation.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/health/health.h>

corto_int16 _ospl_DiscoveryDb_Federation_construct(
    ospl_DiscoveryDb_Federation this)
{
/* $begin(ospl/health/DiscoveryDb/Federation/construct) */

    ospl_DiscoveryDb_Object(this)->db->federationCount ++;
    return 0;
/* $end */
}

corto_void _ospl_DiscoveryDb_Federation_destruct(
    ospl_DiscoveryDb_Federation this)
{
/* $begin(ospl/health/DiscoveryDb/Federation/destruct) */

    ospl_DiscoveryDb_Object(this)->db->federationCount --;

/* $end */
}
