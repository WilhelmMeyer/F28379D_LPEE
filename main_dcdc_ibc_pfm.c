//
// Included Files
//
#include "F28x_Project.h"
#include "control_logic.h"

//
// Defines
//

//
// Globals
//
struct IBC_PFM_VARIABLES ibcPfmVariables;

//
//  Function Prototypes
//
void interruptionFunction(void);
void initializeVariables(void);

//
// Main
//
void main(void)
{
    initializeDsp();

    initializeVariables();

    configureIbcPfm(&ibcPfmVariables);

    //
    // IDLE loop. Just sit and loop forever (optional):
    //
    for (;;)
    {
        asm ("  NOP");
    }
}

void initializeVariables(void)
{
    //
    // Initialize global variables
    //
    ibcPfmVariables.adcInterruptionFunction = &interruptionFunction;

    //
    // Set adc channels and enable SoC for oversample;
    //
    setAdc1PerModule(&ibcPfmVariables.adcs[0], ADCA4_CHANNEL);

    ibcPfmVariables.epwms[0].period10ns = 1999;
    ibcPfmVariables.epwms[0].comparatorA10ns = 999;
    ibcPfmVariables.epwms[0].comparatorB10ns = 999;
    ibcPfmVariables.epwms[0].deadbandEnable = 1;
    ibcPfmVariables.epwms[0].risingEdgeDelay10ns = 49;
    ibcPfmVariables.epwms[0].fallingEdgeDelay10ns = 49;
    ibcPfmVariables.epwms[0].module = EPWM1_MODULE;
    ibcPfmVariables.epwms[0].enable = EPWM_ENABLE;

    ibcPfmVariables.adcPeriod10ns = 2000;
    ibcPfmVariables.adcComparatorA10ns = 1000;
}

//
// interruption function
//
void interruptionFunction(void)
{

}
