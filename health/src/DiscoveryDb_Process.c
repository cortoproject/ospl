/* $CORTO_GENERATED
 *
 * DiscoveryDb_Process.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/health/health.h>

corto_int16 _ospl_DiscoveryDb_Process_construct(
    ospl_DiscoveryDb_Process this)
{
/* $begin(ospl/health/DiscoveryDb/Process/construct) */
    ospl_DiscoveryDb_Object(this)->db->processCount ++;
    return 0;
/* $end */
}

corto_void _ospl_DiscoveryDb_Process_destruct(
    ospl_DiscoveryDb_Process this)
{
/* $begin(ospl/health/DiscoveryDb/Process/destruct) */

    ospl_DiscoveryDb_Object(this)->db->processCount --;

/* $end */
}

corto_string _ospl_DiscoveryDb_Process_nameof(
    ospl_DiscoveryDb_Process this)
{
/* $begin(ospl/health/DiscoveryDb/Process/nameof) */

    return corto_strdup(this->processName);

/* $end */
}
