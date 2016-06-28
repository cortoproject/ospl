/* $CORTO_GENERATED
 *
 * DiscoveryDb_Publisher.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/health/health.h>

corto_int16 _ospl_DiscoveryDb_Publisher_construct(
    ospl_DiscoveryDb_Publisher this)
{
/* $begin(ospl/health/DiscoveryDb/Publisher/construct) */

    ospl_DiscoveryDb_Object(this)->db->publisherCount ++;
    return 0;
/* $end */
}

corto_void _ospl_DiscoveryDb_Publisher_destruct(
    ospl_DiscoveryDb_Publisher this)
{
/* $begin(ospl/health/DiscoveryDb/Publisher/destruct) */

    ospl_DiscoveryDb_Object(this)->db->publisherCount --;

/* $end */
}
