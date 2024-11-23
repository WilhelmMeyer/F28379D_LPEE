//
// Included Files
//
#include "F28x_Project.h"
#include "control_logic.h"

//
// Defines
//
#define VOLTAGE_PHASE_A_COEF 0.1282

#define VOLTAGE_PHASE_A_OFFSET 1880

#define VOLTAGE_OUTPUT_COEF_V1P 0.135414 //0.11337

//#define MINIMUM_PERIOD_10NS 400 //250000 Hz
//#define MAXIMUM_PERIOD_10NS 3332 // 30000 Hz //1562 // 64000 Hz

#define MINIMUM_PERIOD 0.000004 //250000 Hz
#define MAXIMUM_PERIOD 0.0000333 //30000 Hz //0.00001562 //64000 Hz

#define PROPORTIONAL_GAIN 0.000000123//0.000000078//-0.000001
#define INTEGRATIVE_TIME 0.026//0.019//0.068//0.0000000008

//
// Globals
//
Uint16 controllOperation = OPEN_LOOP; //OPEN_LOOP; //CLOSED LOOP
Uint32 switchingPeriod10ns = 500;
Uint32 switchingPeriod = 0.00002;

Uint16 ControlMode; //ControlMode = 1 closed loop; ControlMode = 0 open loop

struct PID_VARIABLES pid;

struct IBC_PFM_VARIABLES ibcPfmVariables;
struct RMS_CALCULATION rmsVoltageA;

//volatile struct ADC_VARIABLES *outputVoltage1p = &ibcPfmVariables.adc[0];
//volatile struct ADC_VARIABLES *voltageA = &ibcPfmVariables.adc[2];
struct ADC_VARIABLES *auxVoltage = &ibcPfmVariables.adc[0]; //variável auxiliar criada para o código entrar na interrupção e atualizar as leituras do adc
struct ADC_VARIABLES *outputVoltage1p = &ibcPfmVariables.adc[1];
struct ADC_VARIABLES *voltageA = &ibcPfmVariables.adc[2];

struct EPWM_VARIABLES *epwmIbcPfm1S1S2 = &ibcPfmVariables.epwm[0];
struct EPWM_VARIABLES *epwmIbcPfm1S3S4 = &ibcPfmVariables.epwm[1];

Uint32 pulseFrequencyHz = 54400; //121900;//90900;//70000;
Uint32 acquisitionFrequencyHz = 25000;
Uint32 deadbandDelay10ns = 0.0000001; //100ns //10;
//Uint32 superpositionDelay = 0.0000001; //100ns

double outputVoltage = 0;
double outputVoltageSetpoint = 130; //100;//400;

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
    ibcPfmVariables.adcInterruptionFunction = &interruptionFunction; // aponta a função de interrupção do firmware

    Uint32 period10ns = frequencyToPeriod10ns(pulseFrequencyHz);
    Uint32 acquisitionPeriod10ns = frequencyToPeriod10ns(
            acquisitionFrequencyHz);

    //updateSamplingPeriodIbcPfm(&ibcPfmVariables, acquisitionPeriod10ns);
    updateSamplingFrequencyIbcPfm(&ibcPfmVariables, acquisitionFrequencyHz);
    //
    // Set adc channels and enable SoC for oversample;
    //

    setAdc1PerModule(auxVoltage, ADCA4_CHANNEL); //Configuração de variável auxiliar somente para o firmware
    setAdc1PerModule(outputVoltage1p, ADCB4_CHANNEL); //Lê 16 vezes e calcula a média
    setAdc1PerModule(voltageA, ADCC4_CHANNEL);

    configureAdcAquisition(auxVoltage, VOLTAGE_OUTPUT_COEF_V1P, 0, 25000,
                           acquisitionFrequencyHz); //Configuração de variável auxiliar somente para o firmware

    configureAdcAquisition(outputVoltage1p, VOLTAGE_OUTPUT_COEF_V1P, 0, 25000,
                           acquisitionFrequencyHz); //frequência de corte do filtro 25 kHz

    configureAdcAquisition(voltageA, VOLTAGE_PHASE_A_COEF,
    VOLTAGE_PHASE_A_OFFSET,
                           25000, acquisitionFrequencyHz); //ultimo parametro: acquisitionPeriod10ns

    configureRmsCalculation(&rmsVoltageA, 60, acquisitionFrequencyHz);

    configureIbcPfmEpwm(epwmIbcPfm1S1S2,
    EPWM_SC_PFM_DEADBAND,
                        EPWM1_MODULE, EPWM1_LINK, pulseFrequencyHz,
                        period10ns >> 1, deadbandDelay10ns); //dead band

    configureIbcPfmEpwm(epwmIbcPfm1S3S4,
    EPWM_SC_PFM_DEADBAND,
                        EPWM2_MODULE, EPWM1_LINK, pulseFrequencyHz,
                        period10ns >> 1, deadbandDelay10ns);

    pidConfiguration(&pid, PI_INCREMENTAL_ANTI_WINDUP, acquisitionFrequencyHz,
    PROPORTIONAL_GAIN,
                     INTEGRATIVE_TIME, 0, 0);

}

//
// interruption function acquisition
//
void interruptionFunction(void)
{
    updateAnalogValueFiltered(outputVoltage1p); //atualiza a variável analógica para armazenar a tensão de saída utilizada em malha fechada

    outputVoltage = outputVoltage1p->filteredValue;

    if (outputVoltage > 130)
    {
        controllOperation = CLOSED_LOOP;
        ControlMode = 1;
    }
//    else if (outputVoltage < 30)
//    {
//        controllOperation = OPEN_LOOP;
//        ControlMode = 0;
//    }

    if (controllOperation == OPEN_LOOP)
    {
        //ControlMode = 0;
        //switchingPeriod10ns = frequencyToPeriod10ns(pulseFrequency);
        switchingPeriod = frequencyToPeriod(pulseFrequencyHz);
        updatePidControllerOutput(&pid, controllOperation, switchingPeriod,
                                  outputVoltageSetpoint, outputVoltage,
                                  MAXIMUM_PERIOD,
                                  MINIMUM_PERIOD);
    }
    else if (controllOperation == CLOSED_LOOP)
    {
        //ControlMode = 1;
        switchingPeriod = updatePidControllerOutput(&pid, controllOperation,
                                                    switchingPeriod,
                                                    outputVoltageSetpoint,
                                                    outputVoltage,
                                                    MAXIMUM_PERIOD,
                                                    MINIMUM_PERIOD);
    }

    if (switchingPeriod > MAXIMUM_PERIOD)
        switchingPeriod = MAXIMUM_PERIOD;
    else if (switchingPeriod < MINIMUM_PERIOD)
        switchingPeriod = MINIMUM_PERIOD;

    updateAllEpwmPeriodIbcPfm(&ibcPfmVariables, switchingPeriod);

    updateAnalogValueFilteredAutoOffset(voltageA);

    rmsCalculation(&rmsVoltageA, voltageA->filteredValue);

}
