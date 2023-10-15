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
    ibcPfmVariables.adcInterruptionFunction =
            &interruptionFunction;

    //
    // Set adc channels and enable SoC for oversample;
    //
    ibcPfmVariables.adcs[0].adcChannel = ADCA4_CHANNEL;
    ibcPfmVariables.adcs[0].socEnable = SOC_ALL_ENABLE;

    ibcPfmVariables.adcs[1].adcChannel = ADCB4_CHANNEL;
    ibcPfmVariables.adcs[1].socEnable = SOC_ALL_ENABLE;

    ibcPfmVariables.adcs[2].adcChannel = ADCC4_CHANNEL;
    ibcPfmVariables.adcs[2].socEnable = SOC_ALL_ENABLE;

    ibcPfmVariables.adcs[3].adcChannel = ADCD14_CHANNEL;
    ibcPfmVariables.adcs[3].socEnable = SOC0_ENABLE | SOC1_ENABLE
            | SOC2_ENABLE | SOC3_ENABLE | SOC4_ENABLE | SOC5_ENABLE
            | SOC6_ENABLE | SOC7_ENABLE;

    ibcPfmVariables.adcs[4].adcChannel = ADCD15_CHANNEL;
    ibcPfmVariables.adcs[4].socEnable = SOC8_ENABLE | SOC9_ENABLE
            | SOC10_ENABLE | SOC11_ENABLE | SOC12_ENABLE | SOC13_ENABLE
            | SOC14_ENABLE | SOC15_ENABLE;

    for (int i = 0; i < MAX_EPWMS; i++)
    {
        ibcPfmVariables.epwms[i].period10ns = 1999;
        ibcPfmVariables.epwms[i].comparatorA10ns = 999;
        ibcPfmVariables.epwms[i].comparatorB10ns = 999;
        ibcPfmVariables.epwms[i].deadbandEnable = 1;
        ibcPfmVariables.epwms[i].risingEdgeDelay10ns = 49;
        ibcPfmVariables.epwms[i].fallingEdgeDelay10ns = 49;
        ibcPfmVariables.epwms[i].epwmModule = i+1;
    }
}

//
// interruption function acquisition
//
void interruptionFunction(void)
{

}
