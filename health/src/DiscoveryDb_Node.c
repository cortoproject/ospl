/* $CORTO_GENERATED
 *
 * DiscoveryDb_Node.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/health/health.h>

corto_int16 _ospl_DiscoveryDb_Node_construct(
    ospl_DiscoveryDb_Node this)
{
/* $begin(ospl/health/DiscoveryDb/Node/construct) */

    ospl_DiscoveryDb_Object(this)->db->nodeCount ++;
    return 0;
/* $end */
}

corto_void _ospl_DiscoveryDb_Node_destruct(
    ospl_DiscoveryDb_Node this)
{
/* $begin(ospl/health/DiscoveryDb/Node/destruct) */

    ospl_DiscoveryDb_Object(this)->db->nodeCount --;

/* $end */
}
