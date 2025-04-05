//
// Included Files
//
#include "F28x_Project.h"
#include "control_logic.h"
#include "spll_1ph_sogi.h"

//
// Defines
//
#define VOLTAGE_PHASE_A_COEF 0.1282
#define VOLTAGE_PHASE_B_COEF 0.1282
#define VOLTAGE_PHASE_C_COEF 0.1282
#define VOLTAGE_PHASE_A_OFFSET 1880
#define VOLTAGE_PHASE_B_OFFSET 1875
#define VOLTAGE_PHASE_C_OFFSET 1855

#define VOLTAGE_OUTPUT_COEF_V1P 0.1128
#define VOLTAGE_OUTPUT_COEF_V2N 0.1128

#define MINIMUM_PERIOD_10NS 300
#define MAXIMUM_PERIOD_10NS 1562 // 64000 kHz

#define PROPORTIONAL_GAIN -0.000001
#define INTEGRATIVE_TIME 0.0000000008

//
// Globals
//
Uint16 controllOperation = OPEN_LOOP;
Uint32 switchingPeriod10ns = 50000;

struct PID_VARIABLES pid;

SPLL_1PH_SOGI spllVoltageA;
SPLL_1PH_SOGI spllVoltageB;
SPLL_1PH_SOGI spllVoltageC;

struct IBC_PFM_VARIABLES ibcPfmVariables;
struct RMS_CALCULATION rmsVoltageA;
struct RMS_CALCULATION rmsVoltageB;
struct RMS_CALCULATION rmsVoltageC;

struct ADC_VARIABLES *outputVoltage1p = &ibcPfmVariables.adc[0];
struct ADC_VARIABLES *outputVoltage2n = &ibcPfmVariables.adc[1];
struct ADC_VARIABLES *voltageA = &ibcPfmVariables.adc[2];
struct ADC_VARIABLES *voltageB = &ibcPfmVariables.adc[3];
struct ADC_VARIABLES *voltageC = &ibcPfmVariables.adc[4];

struct EPWM_VARIABLES *epwmIbcPfm1S1S2 = &ibcPfmVariables.epwm[0];
struct EPWM_VARIABLES *epwmIbcPfm1S3S4 = &ibcPfmVariables.epwm[1];
struct EPWM_VARIABLES *epwmIbcPfm2S1S2 = &ibcPfmVariables.epwm[2];
struct EPWM_VARIABLES *epwmIbcPfm2S3S4 = &ibcPfmVariables.epwm[3];
struct EPWM_VARIABLES *epwmIbcPfm3S1S2 = &ibcPfmVariables.epwm[4];
struct EPWM_VARIABLES *epwmIbcPfm3S3S4 = &ibcPfmVariables.epwm[5];

double semiCycleHisterese = 0.01;

double minimumVoltageForModulationStrategy = 20;
Uint16 holdCmdSemiCycleA = 0;
Uint16 holdCmdSemiCycleB = 0;
Uint16 holdCmdSemiCycleC = 0;
Uint16 semiCycleA = 0;
Uint16 semiCycleB = 0;
Uint16 semiCycleC = 0;

double voltageASine;
double voltageATheta;
double voltageBSine;
double voltageBTheta;
double voltageCSine;
double voltageCTheta;

double voltageAThetaDelay = 1.13;
double voltageBThetaDelay = 1.13;
double voltageCThetaDelay = 1.13;

Uint32 pulseFrequency = 64000;
Uint32 acquisitionFrequency = 25000;
Uint32 superpositionDelay10ns = 10;

double outputVoltage = 0;
double outputVoltageSetpoint = 800;

//
//  Function Prototypes
//
void interruptionFunction(void);
void initializeVariables(void);
void viennaIbcPfmModulationStrategy(void);

void activateAndHoldIbcPfm(struct RMS_CALCULATION rmsVoltage, double theta,
                           double minimumVoltage, double histerese,
                           struct EPWM_VARIABLES *epwmPositiveCycle,
                           struct EPWM_VARIABLES *epwmNegativeCycle);

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
    Uint32 acquisitionPeriod10ns = frequencyToPeriod10ns(acquisitionFrequency);

    updateSamplingPeriodIbcPfm(&ibcPfmVariables, acquisitionPeriod10ns);

    //
    // Set adc channels and enable SoC for oversample;
    //
    setAdc1PerModule(outputVoltage1p, ADCA4_CHANNEL);
    setAdc1PerModule(outputVoltage2n, ADCB4_CHANNEL);
    setAdc2PerModule(voltageA, ADCC4_CHANNEL, voltageB, ADCC3_CHANNEL);
    setAdc1PerModule(voltageC, ADCD15_CHANNEL);

    configureAdcAquisition(outputVoltage1p, VOLTAGE_OUTPUT_COEF_V1P, 0, 25000,
                           acquisitionPeriod10ns);
    configureAdcAquisition(outputVoltage2n, VOLTAGE_OUTPUT_COEF_V2N, 0, 25000,
                           acquisitionPeriod10ns);

    configureAdcAquisition(voltageA, VOLTAGE_PHASE_A_COEF,
    VOLTAGE_PHASE_A_OFFSET,
                           500000, acquisitionPeriod10ns);
    configureAdcAquisition(voltageB, VOLTAGE_PHASE_B_COEF,
    VOLTAGE_PHASE_B_OFFSET,
                           500000, acquisitionPeriod10ns);
    configureAdcAquisition(voltageC, VOLTAGE_PHASE_C_COEF,
    VOLTAGE_PHASE_C_OFFSET,
                           500000, acquisitionPeriod10ns);

    configureRmsCalculation(&rmsVoltageA, 60, acquisitionPeriod10ns);
    configureRmsCalculation(&rmsVoltageB, 60, acquisitionPeriod10ns);
    configureRmsCalculation(&rmsVoltageC, 60, acquisitionPeriod10ns);

    for (int i = 0; i < MAX_EPWM; i++)
    {
        configureIbcPfmEpwm(&ibcPfmVariables.epwm[i],
                            EPWM_SC_PFM,
                            i, EPWM1_LINK, period10ns, period10ns >> 1,
                            0);

    }

    SPLL_1PH_SOGI_reset(&spllVoltageA);
    SPLL_1PH_SOGI_config(&spllVoltageA, 60, acquisitionFrequency,
                         (float32_t) (222.2862), (float32_t) (-222.034));
    SPLL_1PH_SOGI_coeff_calc(&spllVoltageA);

    SPLL_1PH_SOGI_reset(&spllVoltageB);
    SPLL_1PH_SOGI_config(&spllVoltageB, 60, acquisitionFrequency,
                         (float32_t) (222.2862), (float32_t) (-222.034));
    SPLL_1PH_SOGI_coeff_calc(&spllVoltageB);

    SPLL_1PH_SOGI_reset(&spllVoltageC);
    SPLL_1PH_SOGI_config(&spllVoltageC, 60, acquisitionFrequency,
                         (float32_t) (222.2862), (float32_t) (-222.034));
    SPLL_1PH_SOGI_coeff_calc(&spllVoltageC);

    pidConfiguration(&pid, PI_INCREMENTAL_ANTI_WINDUP, acquisitionPeriod10ns,
    PROPORTIONAL_GAIN,
                     INTEGRATIVE_TIME, 0, 0);

}

//
// interruption function acquisition
//
void interruptionFunction(void)
{
    updateAnalogValueFiltered(outputVoltage1p);
    updateAnalogValueFiltered(outputVoltage2n);

    outputVoltage = outputVoltage1p->filteredValue
            + outputVoltage2n->filteredValue;

    if (outputVoltage > 800)
    {
        controllOperation = CLOSED_LOOP;
    }

    if (controllOperation == OPEN_LOOP)
    {
        switchingPeriod10ns = frequencyToPeriod10ns(pulseFrequency);
        updatePidControllerOutput(&pid, controllOperation, switchingPeriod10ns,
                                  outputVoltageSetpoint, outputVoltage,
                                  MAXIMUM_PERIOD_10NS,
                                  MINIMUM_PERIOD_10NS);
    }
    else if (controllOperation == CLOSED_LOOP)
    {
        switchingPeriod10ns = updatePidControllerOutput(&pid, controllOperation,
                                                        switchingPeriod10ns,
                                                        -outputVoltageSetpoint,
                                                        -outputVoltage,
                                                        MAXIMUM_PERIOD_10NS,
                                                        MINIMUM_PERIOD_10NS);
    }

    if (switchingPeriod10ns > MAXIMUM_PERIOD_10NS)
        switchingPeriod10ns = MAXIMUM_PERIOD_10NS;
    else if (switchingPeriod10ns < MINIMUM_PERIOD_10NS)
        switchingPeriod10ns = MINIMUM_PERIOD_10NS;

    updateAllEpwmPeriodIbcPfm(&ibcPfmVariables, switchingPeriod10ns);

    updateAnalogValueFilteredAutoOffset(voltageA);
    updateAnalogValueFilteredAutoOffset(voltageB);
    updateAnalogValueFilteredAutoOffset(voltageC);

    rmsCalculation(&rmsVoltageA, voltageA->filteredValue);
    rmsCalculation(&rmsVoltageB, voltageB->filteredValue);
    rmsCalculation(&rmsVoltageC, voltageC->filteredValue);

    SPLL_1PH_SOGI_run(&spllVoltageA, rmsVoltageA.sine);
    voltageASine = spllVoltageA.sine;
    voltageATheta = spllVoltageA.theta + voltageAThetaDelay;
    if (voltageATheta > 6.28315401)
    {
        voltageATheta = voltageATheta - 6.28315401;
    }
    else if (voltageATheta < 0)
    {
        voltageATheta = 6.28315401 - voltageATheta;
    }

    SPLL_1PH_SOGI_run(&spllVoltageB, rmsVoltageB.sine);
    voltageBSine = spllVoltageB.sine;
    voltageBTheta = spllVoltageB.theta + voltageBThetaDelay;
    if (voltageBTheta > 6.28315401)
    {
        voltageBTheta = voltageBTheta - 6.28315401;
    }
    else if (voltageBTheta < 0)
    {
        voltageBTheta = 6.28315401 - voltageBTheta;
    }

    SPLL_1PH_SOGI_run(&spllVoltageC, rmsVoltageC.sine);
    voltageCSine = spllVoltageC.sine;
    voltageCTheta = spllVoltageC.theta + voltageCThetaDelay;
    if (voltageCTheta > 6.28315401)
    {
        voltageCTheta = voltageCTheta - 6.28315401;
    }
    else if (voltageCTheta < 0)
    {
        voltageCTheta = 6.28315401 - voltageCTheta;
    }

    viennaIbcPfmModulationStrategy();
}

//
// user defined functions
//
void viennaIbcPfmModulationStrategy()
{
    activateAndHoldIbcPfm(rmsVoltageA, voltageATheta,
                          minimumVoltageForModulationStrategy,
                          semiCycleHisterese, epwmIbcPfm1S1S2, epwmIbcPfm1S3S4);

    activateAndHoldIbcPfm(rmsVoltageB, voltageBTheta,
                          minimumVoltageForModulationStrategy,
                          semiCycleHisterese, epwmIbcPfm2S1S2, epwmIbcPfm2S3S4);

    activateAndHoldIbcPfm(rmsVoltageC, voltageCTheta,
                          minimumVoltageForModulationStrategy,
                          semiCycleHisterese, epwmIbcPfm3S1S2, epwmIbcPfm3S3S4);

}

void activateAndHoldIbcPfm(struct RMS_CALCULATION rmsVoltage, double theta,
                           double minimumVoltage, double histerese,
                           struct EPWM_VARIABLES *epwmPositiveCycle,
                           struct EPWM_VARIABLES *epwmNegativeCycle)
{
    if (rmsVoltage.value < minimumVoltage)
    {
        updateEpwmConfiguration(epwmPositiveCycle,
        EPWM_SC_PFM_SUPERPOSITION);
        updateEpwmConfiguration(epwmNegativeCycle,
        EPWM_SC_PFM_SUPERPOSITION);
    }
    else
    {
        if ((theta > (6.2831853 - 5*semiCycleHisterese) && theta < (6.2831853 - semiCycleHisterese))
          ||(theta > (3.1415926 - 5*semiCycleHisterese) && theta < (3.1415926 - semiCycleHisterese)))
        {
            updateEpwmConfiguration(epwmPositiveCycle,
            EPWM_ALWAYS_OFF);
            updateEpwmConfiguration(epwmNegativeCycle,
            EPWM_ALWAYS_OFF);

        }
        else if ((theta > semiCycleHisterese && theta < 1.57) || (theta > 6.2831853 + semiCycleHisterese))
        {
            updateEpwmConfiguration(epwmPositiveCycle,
            EPWM_SC_PFM_SUPERPOSITION);
            updateEpwmConfiguration(epwmNegativeCycle,
            EPWM_ALWAYS_ON);
        }
        else if (theta > (3.1415926536 + semiCycleHisterese) && theta < 4.71)
        {
            updateEpwmConfiguration(epwmPositiveCycle,
            EPWM_ALWAYS_ON);
            updateEpwmConfiguration(epwmNegativeCycle,
            EPWM_SC_PFM_SUPERPOSITION);
        }

    }
}
