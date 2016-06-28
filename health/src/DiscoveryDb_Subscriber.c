/* $CORTO_GENERATED
 *
 * DiscoveryDb_Subscriber.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/health/health.h>

corto_int16 _ospl_DiscoveryDb_Subscriber_construct(
    ospl_DiscoveryDb_Subscriber this)
{
/* $begin(ospl/health/DiscoveryDb/Subscriber/construct) */

    ospl_DiscoveryDb_Object(this)->db->subscriberCount ++;
    return 0;
/* $end */
}

corto_void _ospl_DiscoveryDb_Subscriber_destruct(
    ospl_DiscoveryDb_Subscriber this)
{
/* $begin(ospl/health/DiscoveryDb/Subscriber/destruct) */

    ospl_DiscoveryDb_Object(this)->db->subscriberCount --;

/* $end */
}
