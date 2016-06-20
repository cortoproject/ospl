/* $CORTO_GENERATED
 *
 * DiscoveryDb_Durability.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <ospl/health/health.h>

corto_int16 _ospl_DiscoveryDb_Durability_init(
    ospl_DiscoveryDb_Durability this)
{
/* $begin(ospl/health/DiscoveryDb/Durability/init) */

    ospl_DiscoveryDb_Object(this)->state = Ospl_Initializing;

    /* Set service in federation */
    ospl_DiscoveryDb_Federation f = corto_parentof(this);
    corto_setref(&f->durabilityService, this);

    return 0;
/* $end */
}

corto_void _ospl_DiscoveryDb_Durability_setState(
    ospl_DiscoveryDb_Durability this,
    ospl_DurabilityState state)
{
/* $begin(ospl/health/DiscoveryDb/Durability/setState) */

    if (this->state != state) {
        corto_updateBegin(this);
        this->state = state;
        if (this->state == OSPL_COMPLETE) {
            ospl_DiscoveryDb_Object(this)->state = Ospl_Operational;
        } else if (this->state == OSPL_TERMINATING) {
            ospl_DiscoveryDb_Object(this)->state = Ospl_Degraded;
        } else {
            ospl_DiscoveryDb_Object(this)->state = Ospl_Initializing;            
        }
        corto_updateEnd(this);
    }

/* $end */
}

corto_string _ospl_DiscoveryDb_Durability_value(
    ospl_DiscoveryDb_Durability this)
{
/* $begin(ospl/health/DiscoveryDb/Durability/value) */

    return corto_strp(&this->state, ospl_DurabilityState_o, 0);

/* $end */
}
