//
// Included Files
//
#include "F28x_Project.h"
#include "control_logic.h"

//
// Defines
//
#define VOLTAGE_OUTPUT_COEF 0.126037735849

//
// Globals
//
struct IBC_PFM_VARIABLES ibcPfmVariables;

Uint32 pulseFrequency = 50000;
Uint32 superpositionDelay10ns = 20;

//
//  Function Prototypes
//
void interruptionFunction(void);
void initializeVariables(void);
Uint32 frequencyToPeriod10ns(float frequency);

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

    updateSamplingPeriodIbcPfm(&ibcPfmVariables,
                               frequencyToPeriod10ns(pulseFrequency));

    //
    // Set adc channels and enable SoC for oversample;
    //
    setAdc1PerModule(&ibcPfmVariables.adc[0], ADCA4_CHANNEL);

    configureAdcAquisition(&ibcPfmVariables.adc[0], VOLTAGE_OUTPUT_COEF, 0,
                           100000, 2000);

    configureIbcPfmEpwm(&ibcPfmVariables.epwm[0],
    EPWM_SC_PFM_SUPERPOSITION,
                        EPWM1_MODULE, EPWM1_LINK,
                        frequencyToPeriod10ns(pulseFrequency),
                        superpositionDelay10ns);

}

//
// interruption function acquisition
//
void interruptionFunction(void)
{
    Uint32 period = frequencyToPeriod10ns(pulseFrequency);

    updateEpwmPeriodIbcPfm(&ibcPfmVariables, period);

    updateAnalogValueFiltered(&ibcPfmVariables.adc[0]);

}

//
// user defined functions
//

