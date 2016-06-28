/* $CORTO_GENERATED
 *
 * DiscoveryDb_DataReader.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/health/health.h>

corto_int16 _ospl_DiscoveryDb_DataReader_construct(
    ospl_DiscoveryDb_DataReader this)
{
/* $begin(ospl/health/DiscoveryDb/DataReader/construct) */

    ospl_DiscoveryDb_Object(this)->db->dataReaderCount ++;
    return 0;
/* $end */
}

corto_void _ospl_DiscoveryDb_DataReader_destruct(
    ospl_DiscoveryDb_DataReader this)
{
/* $begin(ospl/health/DiscoveryDb/DataReader/destruct) */

    ospl_DiscoveryDb_Object(this)->db->dataReaderCount --;

/* $end */
}
