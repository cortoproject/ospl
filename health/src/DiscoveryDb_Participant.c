/* $CORTO_GENERATED
 *
 * DiscoveryDb_Participant.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/health/health.h>

corto_int16 _ospl_DiscoveryDb_Participant_construct(
    ospl_DiscoveryDb_Participant this)
{
/* $begin(ospl/health/DiscoveryDb/Participant/construct) */

    ospl_DiscoveryDb_Object(this)->db->participantCount ++;
    return 0;
/* $end */
}

corto_void _ospl_DiscoveryDb_Participant_destruct(
    ospl_DiscoveryDb_Participant this)
{
/* $begin(ospl/health/DiscoveryDb/Participant/destruct) */

    ospl_DiscoveryDb_Object(this)->db->participantCount --;

/* $end */
}
