//
// Included Files
//
#include "F28x_Project.h"
#include "control_logic.h"

//
// Defines
//
#define VOLTAGE_PHASE_A_COEF 0.126037735849
#define VOLTAGE_PHASE_A_OFFSET -0.5

#define VOLTAGE_OUTPUT_COEF 0.126037735849

//
// Globals
//
struct IBC_PFM_VARIABLES ibcPfmVariables;
struct RMS_CALCULATION rmsVoltageA;

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

    Uint32 period10ns = frequencyToPeriod10ns(pulseFrequency);

    updateSamplingPeriodIbcPfm(&ibcPfmVariables, period10ns);

    //
    // Set adc channels and enable SoC for oversample;
    //
    setAdc1PerModule(&ibcPfmVariables.adc[0], ADCA4_CHANNEL);
    setAdc1PerModule(&ibcPfmVariables.adc[1], ADCB4_CHANNEL);
    setAdc1PerModule(&ibcPfmVariables.adc[2], ADCC4_CHANNEL);
    setAdc2PerModule(&ibcPfmVariables.adc[3], ADCD14_CHANNEL,
                     &ibcPfmVariables.adc[4], ADCD15_CHANNEL);

    configureAdcAquisition(&ibcPfmVariables.adc[0], VOLTAGE_OUTPUT_COEF, 0,
                           100000, period10ns);
    configureAdcAquisition(&ibcPfmVariables.adc[1], VOLTAGE_OUTPUT_COEF, 0,
                           100000, period10ns);

    configureAdcAquisition(&ibcPfmVariables.adc[2], VOLTAGE_PHASE_A_COEF,
    VOLTAGE_PHASE_A_OFFSET,
                           100000, period10ns);
    configureAdcAquisition(&ibcPfmVariables.adc[3], VOLTAGE_PHASE_A_COEF,
    VOLTAGE_PHASE_A_OFFSET,
                           100000, period10ns);
    configureAdcAquisition(&ibcPfmVariables.adc[4], VOLTAGE_PHASE_A_COEF,
    VOLTAGE_PHASE_A_OFFSET,
                           100000, period10ns);

    configureRmsCalculation(&rmsVoltageA, 60, period10ns);

    for (int i = 0; i < MAX_EPWM; i++)
    {
        configureIbcPfmEpwm(&ibcPfmVariables.epwm[i],
        EPWM_SC_PFM_SUPERPOSITION,
                            i, EPWM1_LINK,
                            frequencyToPeriod10ns(pulseFrequency),
                            superpositionDelay10ns);

    }

}

//
// interruption function acquisition
//
void interruptionFunction(void)
{
    Uint32 period = frequencyToPeriod10ns(pulseFrequency);

    updateEpwmPeriodIbcPfm(&ibcPfmVariables, period);

    for (int i = 0; i < 5; i++)
    {
        updateAnalogValueFiltered(&ibcPfmVariables.adc[i]);
    }

    rmsCalculation(&rmsVoltageA, ibcPfmVariables.adc[2].filteredValue);

    if (ibcPfmVariables.adc[2].filteredValue > 0)
    {
        updateEpwmConfiguration(&ibcPfmVariables.epwm[0],
        EPWM_SC_PFM_SUPERPOSITION);
    }
    else
    {

    }

}

//
// user defined functions
//

