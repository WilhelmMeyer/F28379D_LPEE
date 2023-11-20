//
// Included Files
//
#include "F28x_Project.h"
#include "control_logic.h"

//
// Defines
//
#define VOLTAGE_OUTPUT_COEF 0.003465
#define PROPORTIONAL_GAIN 0.3
#define INTEGRATIVE_TIME 0.001

//
// Globals
//
struct IBC_PFM_VARIABLES ibcPfmVariables;

struct ADC_VARIABLES *outputVoltage = &ibcPfmVariables.adc[0];

struct EPWM_VARIABLES *epwmS1 = &ibcPfmVariables.epwm[0];

struct PID_VARIABLES pid;

Uint32 pulseFrequency = 50000;
double dutyCycle = 0.2;
double dutyCycleUpperSaturationLimit = 0.5;
double dutyCycleLowerSaturationLimit = 0.001;

double outputVoltageSetpoint = 6.0;
Uint16 controllOperation = OPEN_LOOP;

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
    setAdc1PerModule(outputVoltage, ADCA4_CHANNEL);

    configureAdcAquisition(outputVoltage, VOLTAGE_OUTPUT_COEF, 0, 100000,
                           period10ns);

    configureIbcPfmEpwm(epwmS1,
    EPWM_SC_PFM,
                        EPWM1_MODULE, EPWM1_LINK, period10ns, period10ns >> 2,
                        0);

    pidConfiguration(&pid, PI_INCREMENTAL_ANTI_WINDUP, period10ns,
    PROPORTIONAL_GAIN,
                     INTEGRATIVE_TIME, 0, 0);

}

//
// interruption function acquisition
//
void interruptionFunction(void)
{
    updateAnalogValueFiltered(outputVoltage);

    dutyCycle = updatePidControllerOutput(&pid, controllOperation, dutyCycle,
                                          outputVoltageSetpoint,
                                          outputVoltage->filteredValue,
                                          dutyCycleUpperSaturationLimit,
                                          dutyCycleLowerSaturationLimit);

    updateDutyCycleA(epwmS1, dutyCycle);

}

//
// user defined functions
//
