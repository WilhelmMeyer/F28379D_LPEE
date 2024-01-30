// Não alterar a freqûencia desse código de forma online, podem ocorrer desincronismos entre os PWMs.

//
// Included Files
//
#include "F28x_Project.h"
#include "control_logic.h"
#include "control_logic_epwm.h"

//
// Defines
//
#define VOLTAGE_OUTPUT_COEF 0.126037735849

//
// Globals
//
struct IBC_PFM_VARIABLES ibcPfmVariables;

Uint32 pulseFrequency = 300000;
Uint32 acquisitionFrequency = 25000;
Uint32 superpositionDelay10ns = 0; // Leonardo - Agora tem uma lógica que altera esse valor automaticamente para sempre ser 1% da razão ciclica.
Uint32 flagMode = 1; // Escolha do modo: 0 normal; 1 Sobreposição; 2 Deadtime
Uint16 flagSync = 0;
Uint16 exit = 0;

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
    Uint32 acquisitionPeriod = frequencyToPeriod10ns(acquisitionFrequency);
    Uint32 pulsePeriod = roundNo(frequencyToPeriod10ns(pulseFrequency));

    //
    // Initialize global variables
    //
    ibcPfmVariables.adcInterruptionFunction = &interruptionFunction;
//
    updateSamplingPeriodIbcPfm(&ibcPfmVariables, acquisitionPeriod);

    //
    // Set adc channels and enable SoC for oversample;
    //
    setAdc1PerModule(&ibcPfmVariables.adc[0], ADCA4_CHANNEL);

    configureAdcAquisition(&ibcPfmVariables.adc[0], VOLTAGE_OUTPUT_COEF, 0,
                           100000, 2000);

//    configureMasterEpwm(&ibcPfmVariables.epwm[2]);
//
//    configureMasterEpwm(&ibcPfmVariables.epwm[3]);
//
//    configureMasterEpwm(&ibcPfmVariables.epwm[4]);
//
//    configureMasterEpwm(&ibcPfmVariables.epwm[5]);

    configureIbcPfmEpwm(&ibcPfmVariables.epwm[2],
    EPWM_SC_PFM_SUPERPOSITION,
                        EPWM3_MODULE, EPWM3_LINK, pulsePeriod, 0, 0, 0);

    configureIbcPfmEpwm(&ibcPfmVariables.epwm[3],
    EPWM_SC_PFM_PHASE_SUPERPOSITION,
                        EPWM4_MODULE,
                        EPWM4_LINK,
                        pulsePeriod, 0, 0, 0);

    configureIbcPfmEpwm(&ibcPfmVariables.epwm[4],
    EPWM_SC_PFM_PHASE_SUPERPOSITION,
                        EPWM5_MODULE,
                        EPWM5_LINK,
                        pulsePeriod, 0, 0, 0);

    configureIbcPfmEpwm(&ibcPfmVariables.epwm[5],
    EPWM_SC_PFM_PHASE_SUPERPOSITION,
                        EPWM6_MODULE,
                        EPWM6_LINK,
                        pulsePeriod, 0, 0, 0);

}

//
// interruption function acquisition
//
void interruptionFunction(void)
{

    Uint32 period = roundNo(frequencyToPeriod10ns(pulseFrequency));

    if (exit == 0 && flagSync == 0)
    {
        updateEpwmConfiguration(&ibcPfmVariables.epwm[2],
        EPWM_SC_PFM_SUPERPOSITION);

        updateEpwmConfiguration(&ibcPfmVariables.epwm[3],
        EPWM_SC_PFM_PHASE_SUPERPOSITION);

        updateEpwmConfiguration(&ibcPfmVariables.epwm[4],
        EPWM_SC_PFM_PHASE_SUPERPOSITION);

        updateEpwmConfiguration(&ibcPfmVariables.epwm[5],
        EPWM_SC_PFM_PHASE_SUPERPOSITION);

        flagSync = 1;
    }

    // Caso o usuario queira superposição entre os PWM
    if (flagMode == 1)
    {
        superpositionDelay10ns = (Uint32) roundNo(0.02 * period);

        if (superpositionDelay10ns <= 30)
        {
            superpositionDelay10ns = 30;
        }

        updateEpwmPOLSELIbcPfm(&ibcPfmVariables.epwm[2], 0x1);
        updateEpwmPOLSELIbcPfm(&ibcPfmVariables.epwm[3], 0x1);
        updateEpwmPOLSELIbcPfm(&ibcPfmVariables.epwm[4], 0x1);
        updateEpwmPOLSELIbcPfm(&ibcPfmVariables.epwm[5], 0x1);
        updateEpwmOUTSWAPIbcPfm(&ibcPfmVariables.epwm[2], 0x3);
        updateEpwmOUTSWAPIbcPfm(&ibcPfmVariables.epwm[3], 0x3);
        updateEpwmOUTSWAPIbcPfm(&ibcPfmVariables.epwm[4], 0x3);
        updateEpwmOUTSWAPIbcPfm(&ibcPfmVariables.epwm[5], 0x3);

        updateEpwmDEDB_MODEIbcPfm(&ibcPfmVariables.epwm[5], 0);

        updateEpwmREDIbcPfm(&ibcPfmVariables.epwm[5], 0);

        updateEpwmFEDIbcPfm(&ibcPfmVariables.epwm[5],
                            (period * 0.25) + superpositionDelay10ns);
    }
// Caso o usuario queira deadband entre os PWM
    else if (flagMode == 2)
    {
        superpositionDelay10ns = (Uint32) roundNo(0.015 * period);

        if (superpositionDelay10ns <= 20)
        {
            superpositionDelay10ns = 20;
        }

        updateEpwmPOLSELIbcPfm(&ibcPfmVariables.epwm[2], 0x2);
        updateEpwmPOLSELIbcPfm(&ibcPfmVariables.epwm[3], 0x2);
        updateEpwmPOLSELIbcPfm(&ibcPfmVariables.epwm[4], 0x2);
        updateEpwmPOLSELIbcPfm(&ibcPfmVariables.epwm[5], 0x0);
        updateEpwmOUTSWAPIbcPfm(&ibcPfmVariables.epwm[2], 0x0);
        updateEpwmOUTSWAPIbcPfm(&ibcPfmVariables.epwm[3], 0x0);
        updateEpwmOUTSWAPIbcPfm(&ibcPfmVariables.epwm[4], 0x0);
        updateEpwmOUTSWAPIbcPfm(&ibcPfmVariables.epwm[5], 0x3);

        updateEpwmDEDB_MODEIbcPfm(&ibcPfmVariables.epwm[5], 1);

        updateEpwmREDIbcPfm(&ibcPfmVariables.epwm[5], superpositionDelay10ns);

        updateEpwmFEDIbcPfm(&ibcPfmVariables.epwm[5], (period * 0.25));
    }
//Caso o usuario queira exatamente 50% nos PWM
    else
    {
        superpositionDelay10ns = 0;

        updateEpwmPOLSELIbcPfm(&ibcPfmVariables.epwm[2], 0x1);
        updateEpwmPOLSELIbcPfm(&ibcPfmVariables.epwm[3], 0x1);
        updateEpwmPOLSELIbcPfm(&ibcPfmVariables.epwm[4], 0x1);
        updateEpwmPOLSELIbcPfm(&ibcPfmVariables.epwm[5], 0x1);
        updateEpwmOUTSWAPIbcPfm(&ibcPfmVariables.epwm[2], 0x3);
        updateEpwmOUTSWAPIbcPfm(&ibcPfmVariables.epwm[3], 0x3);
        updateEpwmOUTSWAPIbcPfm(&ibcPfmVariables.epwm[4], 0x3);
        updateEpwmOUTSWAPIbcPfm(&ibcPfmVariables.epwm[5], 0x3);

        updateEpwmDEDB_MODEIbcPfm(&ibcPfmVariables.epwm[5], 0);

        updateEpwmREDIbcPfm(&ibcPfmVariables.epwm[5], superpositionDelay10ns);

        updateEpwmFEDIbcPfm(&ibcPfmVariables.epwm[5],
                            (period * 0.25) + superpositionDelay10ns);
    }

    updateEpwmPeriodIbcPfm(&ibcPfmVariables.epwm[2], period);

    updateEpwmPeriodAndPhaseIbcPfm(&ibcPfmVariables.epwm[3], period,
                                   (period * 0.5));

    updateEpwmPeriodAndPhaseIbcPfm(&ibcPfmVariables.epwm[4], period,
                                   period * 0.25);

    updateEpwmPeriodNormal(&ibcPfmVariables.epwm[5], period);

    updateComparatorA(&ibcPfmVariables.epwm[5], period * 0.75);

    updateComparatorB(&ibcPfmVariables.epwm[5], period);

    updateEpwmDelayIbcPfm(&ibcPfmVariables.epwm[2], superpositionDelay10ns);

    updateEpwmDelayIbcPfm(&ibcPfmVariables.epwm[3], superpositionDelay10ns);

    updateEpwmDelayIbcPfm(&ibcPfmVariables.epwm[4], superpositionDelay10ns);

//    updateAnalogValueFiltered(&ibcPfmVariables.adc[0]);

//    if (flagSync == 1)
//    {
//        configureOSHTSYNCEpwm(&ibcPfmVariables.epwm[2]);
//        configureOSHTSYNCEpwm(&ibcPfmVariables.epwm[3]);
//        configureOSHTSYNCEpwm(&ibcPfmVariables.epwm[4]);
//        configureOSHTSYNCEpwm(&ibcPfmVariables.epwm[5]);
//
//        flagSync = 0;
//    }
    if (exit == 1 && flagSync == 1)
    {

//        EPwm3Regs.TZCLR.bit.OST = 1;
//
//        EPwm4Regs.TZCLR.bit.OST = 1;

        updateEpwmConfiguration(&ibcPfmVariables.epwm[2], EPWM_ALWAYS_OFF);

        updateEpwmConfiguration(&ibcPfmVariables.epwm[3], EPWM_ALWAYS_OFF);

        updateEpwmConfiguration(&ibcPfmVariables.epwm[4], EPWM_ALWAYS_OFF);

        updateEpwmConfiguration(&ibcPfmVariables.epwm[5], EPWM_ALWAYS_OFF);

        exit = 0;

        flagSync = 0;

        asm (" ESTOP0");

    }
}

//
// user defined functions
//
