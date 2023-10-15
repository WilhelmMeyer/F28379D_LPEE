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

    initializeInterrupts();

    initializeVariables();

    initializeEpwms(&ibcPfmVariables);

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
    ibcPfmVariables.adcs[0].adcChannel = ADCA4_CHANNEL;
    ibcPfmVariables.adcs[0].socEnable = SOC_ALL_ENABLE;
    ibcPfmVariables.adcs[0].adcModule = ADCA_MODULE;

    ibcPfmVariables.epwms[0].period10ns = 1999;
    ibcPfmVariables.epwms[0].comparatorA10ns = 999;
    ibcPfmVariables.epwms[0].comparatorB10ns = 999;
    ibcPfmVariables.epwms[0].deadbandEnable = 1;
    ibcPfmVariables.epwms[0].risingEdgeDelay10ns = 49;
    ibcPfmVariables.epwms[0].fallingEdgeDelay10ns = 49;
    ibcPfmVariables.epwms[0].epwmModule = EPWM3_MODULE;

}

//
// interruption function acquisition
//
void interruptionFunction(void)
{

}
