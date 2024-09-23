//
// Included Files
//
#include "F28x_Project.h"
#include "control_logic.h"

//
// Defines
//
#define VOLTAGE_OUTPUT_COEF 0.0186037735849

//
// Globals
//
struct IBC_PFM_VARIABLES ibcPfmVariables;

Uint32 acquisitionFrequency = 25000;
Uint32 pulseFrequency = 160000;
Uint32 deadbandDelay10ns = 11;
Uint16 controllOperation = OPEN_LOOP;

//
// Vari�veis MPPT
//

double potenciaSolarAnterior = 0.0;
double potenciaSolar = 0.0;
double deltaPotenciaSolar = 0.0;

double tensaoSolar = 0.0;
double tensaoSolarSomada = 0.0;
double tensaoSolarMedia = 0.0;
double correnteSolarIdeal = 0.0;
double correnteSolarAprimorada = 0.0;
Uint32 periodoChaveamento = 0.0;
Uint32 periodoChaveamentoAnterior = 0.0;
double deltaPeriodoChaveamento = 0.0;
double indutor4Lm = 0.0000851;
double deltaAumentaPeriodo = 1.04;
double deltaDiminuiPeriodo = 0.94;
double histeresePotenciaSolarSuperior = 0.75;
double histeresePotenciaSolarInferior = -3.75;
Uint32 contadorMediaMovelPotenciaSolar = 0;
Uint32 limiteContadorMediaMovelPotenciaSolar = 35000;
Uint32 periodoTransicao1PeriodoEficaz = 25; //pi*sqrt(Lm(Cs+Cd)) (10 ns)
double correnteRodaLivre = 0.954; //0.954

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
    Uint32 acquisitionPeriod10ns = frequencyToPeriod10ns(acquisitionFrequency);

    updateSamplingPeriodIbcPfm(&ibcPfmVariables, acquisitionPeriod10ns);

    //
    // Set adc channels and enable SoC for oversample;
    //
    setAdc1PerModule(&ibcPfmVariables.adc[0], ADCA4_CHANNEL);

    configureAdcAquisition(&ibcPfmVariables.adc[0], VOLTAGE_OUTPUT_COEF, 0,
                           100000, 2000);

    configureIbcPfmEpwm(&ibcPfmVariables.epwm[0],
    EPWM_SC_PFM_DEADBAND,
                        EPWM1_MODULE, EPWM1_LINK, period10ns, period10ns >> 1,
                        deadbandDelay10ns);

}

//
// interruption function acquisition
//
void interruptionFunction(void)
{

    updateAnalogValueFiltered(&ibcPfmVariables.adc[0]);

    tensaoSolar = ibcPfmVariables.adc[0].filteredValue;

    if (controllOperation == OPEN_LOOP)
    {



    }
    else
    {

        if (contadorMediaMovelPotenciaSolar
                >= limiteContadorMediaMovelPotenciaSolar)
        {
            contadorMediaMovelPotenciaSolar = 0;

            tensaoSolarMedia = tensaoSolarSomada
                    / (limiteContadorMediaMovelPotenciaSolar >> 1);
            correnteSolarIdeal = (tensaoSolarMedia * periodoChaveamento * 1e-8)
                    / indutor4Lm;     //corrente de entrada calculada
            correnteSolarAprimorada = ((tensaoSolarMedia
                    * (periodoChaveamento - periodoTransicao1PeriodoEficaz)
                    * 1e-8 / indutor4Lm) - 2 * correnteRodaLivre);
            potenciaSolar = tensaoSolarMedia * correnteSolarAprimorada;

            deltaPotenciaSolar = potenciaSolar - potenciaSolarAnterior;
            deltaPeriodoChaveamento = periodoChaveamento
                    - periodoChaveamentoAnterior;

            tensaoSolarSomada = 0;
            periodoChaveamentoAnterior = periodoChaveamento;

            if (deltaPotenciaSolar > histeresePotenciaSolarSuperior) //Variação potência +
            {
                if (deltaPeriodoChaveamento > 0) // Variação frequência + e variação período -
                {
                    periodoChaveamento = periodoChaveamento
                            * deltaAumentaPeriodo; //frequência diminui e perído aumenta
                }
                else // Variação frequência - e variação período +
                {
                    periodoChaveamento = periodoChaveamento
                            * deltaDiminuiPeriodo; //frequência aumenta e perído diminui
                }
                potenciaSolarAnterior = potenciaSolar;
            }
            else if (deltaPotenciaSolar < histeresePotenciaSolarInferior) //Variação potência -
            {
                if (deltaPeriodoChaveamento > 0) // Variação frequência + e variação período -
                {
                    periodoChaveamento = periodoChaveamento
                            * deltaDiminuiPeriodo; //frequência aumenta e perído diminui
                }
                else // Variação frequência - e variação período +
                {
                    periodoChaveamento = periodoChaveamento
                            * deltaAumentaPeriodo; //frequência diminui e perído aumenta
                }
                potenciaSolarAnterior = potenciaSolar;
            }

        }
        if (contadorMediaMovelPotenciaSolar
                > limiteContadorMediaMovelPotenciaSolar >> 1)
        {
            tensaoSolarSomada += tensaoSolar;
        }
        //Pmed=Pmed+P;
        contadorMediaMovelPotenciaSolar++; //incrementa o contador
        pulseFrequency = 100000000 / periodoChaveamento;

    }

    periodoChaveamento = frequencyToPeriod10ns(pulseFrequency);

    if(periodoChaveamento > 2222) periodoChaveamento = 2222;
    else if(periodoChaveamento < 640)periodoChaveamento = 640;

    updateEpwmPeriodIbcPfm(&ibcPfmVariables.epwm[0], periodoChaveamento);

}

//
// user defined functions
//

