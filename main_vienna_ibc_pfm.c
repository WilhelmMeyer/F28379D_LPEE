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

Uint32 pulseFrequency = 50000;

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
    setAdc1PerModule(&ibcPfmVariables.adcs[1], ADCB4_CHANNEL);
    setAdc1PerModule(&ibcPfmVariables.adcs[2], ADCC4_CHANNEL);
    setAdc2PerModule(&ibcPfmVariables.adcs[3], ADCD14_CHANNEL,
                     &ibcPfmVariables.adcs[4], ADCD15_CHANNEL);

    for (int i = 0; i < MAX_EPWM; i++)
    {
        ibcPfmVariables.epwms[i].period10ns = 1999;
        ibcPfmVariables.epwms[i].comparatorA10ns = 999;
        ibcPfmVariables.epwms[i].comparatorB10ns = 999;
        ibcPfmVariables.epwms[i].epwmConfiguration = EPWM_SC_PFM_SUPERPOSITION;
        ibcPfmVariables.epwms[i].risingEdgeDelay10ns = 49;
        ibcPfmVariables.epwms[i].fallingEdgeDelay10ns = 49;
        ibcPfmVariables.epwms[i].module = i;
        ibcPfmVariables.epwms[i].enable = EPWM_ENABLE;
    }

    ibcPfmVariables.adcPeriod10ns = 2000;
    ibcPfmVariables.adcComparatorA10ns = 1000;
}

//
// interruption function acquisition
//
void interruptionFunction(void)
{
    for (int i = 0; i < MAX_EPWM; i++)
    {
        updatePeriodIbcPfm(&ibcPfmVariables.epwms[i],
                           100000000 / pulseFrequency);
    }
}
