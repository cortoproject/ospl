/* $CORTO_GENERATED
 *
 * DiscoveryDb_DataWriter.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/health/health.h>

corto_int16 _ospl_DiscoveryDb_DataWriter_construct(
    ospl_DiscoveryDb_DataWriter this)
{
/* $begin(ospl/health/DiscoveryDb/DataWriter/construct) */

    ospl_DiscoveryDb_Object(this)->db->dataWriterCount ++;
    return 0;
/* $end */
}

corto_void _ospl_DiscoveryDb_DataWriter_destruct(
    ospl_DiscoveryDb_DataWriter this)
{
/* $begin(ospl/health/DiscoveryDb/DataWriter/destruct) */

    ospl_DiscoveryDb_Object(this)->db->dataWriterCount --;

/* $end */
}
