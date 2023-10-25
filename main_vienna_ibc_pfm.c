//
// Included Files
//
#include "F28x_Project.h"
#include "control_logic.h"

//
// Defines
//
#define VOLTAGE_PHASE_A_COEF 0.126037735849
#define VOLTAGE_PHASE_A_OFFSET 2200

#define VOLTAGE_OUTPUT_COEF 0.126037735849

//
// Globals
//
struct IBC_PFM_VARIABLES ibcPfmVariables;
struct RMS_CALCULATION rmsVoltageA;
struct RMS_CALCULATION rmsVoltageB;
struct RMS_CALCULATION rmsVoltageC;

struct ADC_VARIABLES *outputVoltage1 = &ibcPfmVariables.adc[0];
struct ADC_VARIABLES *outputVoltage2 = &ibcPfmVariables.adc[1];
struct ADC_VARIABLES *voltageA = &ibcPfmVariables.adc[2];
struct ADC_VARIABLES *voltageB = &ibcPfmVariables.adc[3];
struct ADC_VARIABLES *voltageC = &ibcPfmVariables.adc[4];

struct EPWM_VARIABLES *epwmIbcPfm1S1S3 = &ibcPfmVariables.epwm[0];
struct EPWM_VARIABLES *epwmIbcPfm1S2S4 = &ibcPfmVariables.epwm[1];
struct EPWM_VARIABLES *epwmIbcPfm2S1S3 = &ibcPfmVariables.epwm[2];
struct EPWM_VARIABLES *epwmIbcPfm2S2S4 = &ibcPfmVariables.epwm[3];
struct EPWM_VARIABLES *epwmIbcPfm3S1S3 = &ibcPfmVariables.epwm[4];
struct EPWM_VARIABLES *epwmIbcPfm3S2S4 = &ibcPfmVariables.epwm[5];

float semiCycleHisterese = 0.1;
int holdCmdSemiCycleA = 0;
int holdCmdSemiCycleB = 0;
int holdCmdSemiCycleC = 0;

Uint32 pulseFrequency = 50000;
Uint32 superpositionDelay10ns = 20;

//
//  Function Prototypes
//
void interruptionFunction(void);
void initializeVariables(void);
Uint32 frequencyToPeriod10ns(float frequency);
void viennaIbcPfmModulationStrategy(void);

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
    setAdc1PerModule(outputVoltage1, ADCA4_CHANNEL);
    setAdc1PerModule(outputVoltage2, ADCB4_CHANNEL);
    setAdc1PerModule(voltageA, ADCC4_CHANNEL);
    setAdc2PerModule(voltageB, ADCD14_CHANNEL, voltageC, ADCD15_CHANNEL);

    configureAdcAquisition(outputVoltage1, VOLTAGE_OUTPUT_COEF, 0, 100000,
                           period10ns);
    configureAdcAquisition(outputVoltage2, VOLTAGE_OUTPUT_COEF, 0, 100000,
                           period10ns);

    configureAdcAquisition(voltageA, VOLTAGE_PHASE_A_COEF,
    VOLTAGE_PHASE_A_OFFSET,
                           100000, period10ns);
    configureAdcAquisition(voltageB, VOLTAGE_PHASE_A_COEF,
    VOLTAGE_PHASE_A_OFFSET,
                           100000, period10ns);
    configureAdcAquisition(voltageC, VOLTAGE_PHASE_A_COEF,
    VOLTAGE_PHASE_A_OFFSET,
                           100000, period10ns);

    configureRmsCalculation(&rmsVoltageA, 60, period10ns);
    configureRmsCalculation(&rmsVoltageB, 60, period10ns);
    configureRmsCalculation(&rmsVoltageC, 60, period10ns);

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

    rmsCalculation(&rmsVoltageA, voltageA->filteredValue);
    rmsCalculation(&rmsVoltageB, voltageB->filteredValue);
    rmsCalculation(&rmsVoltageC, voltageC->filteredValue);

    viennaIbcPfmModulationStrategy();
}

//
// user defined functions
//
void viennaIbcPfmModulationStrategy()
{
    Uint16 semiCycleA = rmsVoltageA.sine > 0;

    if (rmsVoltageA.value < 20)
    {
        updateEpwmConfiguration(epwmIbcPfm1S1S3,
        EPWM_SC_PFM_SUPERPOSITION);
        updateEpwmConfiguration(epwmIbcPfm1S2S4,
        EPWM_SC_PFM_SUPERPOSITION);
    }
    else
    {
        if (!semiCycleA && rmsVoltageA.sine > -semiCycleHisterese
                && !holdCmdSemiCycleA)
        {
            holdCmdSemiCycleA = 1;

            updateEpwmConfiguration(epwmIbcPfm1S1S3,
            EPWM_SC_PFM_SUPERPOSITION);
            updateEpwmConfiguration(epwmIbcPfm1S2S4,
            EPWM_ALWAYS_ON);
        }
        else if (semiCycleA && rmsVoltageA.sine < semiCycleHisterese
                && !holdCmdSemiCycleA)
        {
            holdCmdSemiCycleA = 1;

            updateEpwmConfiguration(epwmIbcPfm1S1S3,
            EPWM_ALWAYS_ON);
            updateEpwmConfiguration(epwmIbcPfm1S2S4,
            EPWM_SC_PFM_SUPERPOSITION);
        }
        else if (rmsVoltageA.sine > semiCycleHisterese
                || rmsVoltageA.sine < -semiCycleHisterese)
        {
            holdCmdSemiCycleA = 0;
        }
    }

    Uint16 semiCycleB = rmsVoltageB.sine > 0;

    if (rmsVoltageB.value < 20)
    {
        updateEpwmConfiguration(epwmIbcPfm2S1S3,
        EPWM_SC_PFM_SUPERPOSITION);
        updateEpwmConfiguration(epwmIbcPfm2S2S4,
        EPWM_SC_PFM_SUPERPOSITION);
    }
    else
    {
        if (!semiCycleB && rmsVoltageB.sine > -semiCycleHisterese
                && !holdCmdSemiCycleB)
        {
            holdCmdSemiCycleB = 1;

            updateEpwmConfiguration(epwmIbcPfm2S1S3,
            EPWM_SC_PFM_SUPERPOSITION);
            updateEpwmConfiguration(epwmIbcPfm2S2S4,
            EPWM_ALWAYS_ON);
        }
        else if (semiCycleB && rmsVoltageB.sine < semiCycleHisterese
                && !holdCmdSemiCycleB)
        {
            holdCmdSemiCycleB = 1;

            updateEpwmConfiguration(epwmIbcPfm2S1S3,
            EPWM_ALWAYS_ON);
            updateEpwmConfiguration(epwmIbcPfm2S2S4,
            EPWM_SC_PFM_SUPERPOSITION);
        }
        else if (rmsVoltageB.sine > semiCycleHisterese
                || rmsVoltageB.sine < -semiCycleHisterese)
        {
            holdCmdSemiCycleB = 0;
        }
    }

    Uint16 semiCycleC = rmsVoltageC.sine > 0;

    if (rmsVoltageC.value < 20)
    {
        updateEpwmConfiguration(epwmIbcPfm3S1S3,
        EPWM_SC_PFM_SUPERPOSITION);
        updateEpwmConfiguration(epwmIbcPfm3S2S4,
        EPWM_SC_PFM_SUPERPOSITION);
    }
    else
    {
        if (!semiCycleC && rmsVoltageC.sine > -semiCycleHisterese
                && !holdCmdSemiCycleC)
        {
            holdCmdSemiCycleC = 1;

            updateEpwmConfiguration(epwmIbcPfm3S1S3,
            EPWM_SC_PFM_SUPERPOSITION);
            updateEpwmConfiguration(epwmIbcPfm3S2S4,
            EPWM_ALWAYS_ON);
        }
        else if (semiCycleC && rmsVoltageC.sine < semiCycleHisterese
                && !holdCmdSemiCycleC)
        {
            holdCmdSemiCycleC = 1;

            updateEpwmConfiguration(epwmIbcPfm3S1S3,
            EPWM_ALWAYS_ON);
            updateEpwmConfiguration(epwmIbcPfm3S2S4,
            EPWM_SC_PFM_SUPERPOSITION);
        }
        else if (rmsVoltageC.sine > semiCycleHisterese
                || rmsVoltageC.sine < -semiCycleHisterese)
        {
            holdCmdSemiCycleC = 0;
        }
    }
}
