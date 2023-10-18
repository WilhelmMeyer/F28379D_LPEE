/*
 * control_logic.h
 *
 *  Created on: 13 de out de 2023
 *      Author: willj
 */
#include "F2837xD_device.h"
#include "F2837xD_adc.h"
#include "control_logic_adc.h"
#include "control_logic_epwm.h"

#ifndef CONTROL_LOGIC_H_
#define CONTROL_LOGIC_H_

//max total
#define TOTAL_EPWM 13
#define TOTAL_ADC 4

//max for the application
#define MAX_ADC 8
#define MAX_EPWM 6

extern volatile struct EPWM_REGS *EPWM[TOTAL_EPWM];
extern volatile struct ADC_REGS *ADC[TOTAL_ADC];

struct ADC_VARIABLES
{
    Uint16 channel;
    Uint16 results;
    Uint16 overSample;
    Uint16 module;
    Uint16 enable;
};

struct EPWM_VARIABLES
{
    Uint16 channel;
    Uint16 period10ns;
    Uint16 comparatorA10ns;
    Uint16 comparatorB10ns;
    Uint16 epwmConfiguration;
    Uint16 risingEdgeDelay10ns;
    Uint16 fallingEdgeDelay10ns;
    Uint16 module;
    Uint16 enable;
};

struct IBC_PFM_VARIABLES
{
    void (*adcInterruptionFunction)(void);
    struct ADC_VARIABLES adcs[MAX_ADC];
    struct EPWM_VARIABLES epwms[MAX_EPWM];
    Uint16 adcPeriod10ns;
    Uint16 adcComparatorA10ns;
};

inline void initializeDsp()
{

    InitSysCtrl();
    InitGpio();
    DINT;
    InitPieCtrl();
    IER = 0x0000;
    IFR = 0x0000;
    InitPieVectTable();

}

extern void initializeAdcInterrupts(struct IBC_PFM_VARIABLES *ibcPfmVariables);

extern void configureIbcPfm(struct IBC_PFM_VARIABLES *ibcPfmVariables);

extern void configureADC(struct ADC_VARIABLES *adc);

extern void configureAdcEPWM(struct IBC_PFM_VARIABLES *ibcPfmVariables);

extern void initializeEpwms(struct IBC_PFM_VARIABLES *ibcPfmVariables);

extern void configureEpwm(struct EPWM_VARIABLES epwm);

extern void setAdc1PerModule(struct ADC_VARIABLES *adc, Uint16 channel);

extern void setAdc2PerModule(struct ADC_VARIABLES *adc1, Uint16 channel1,
                             struct ADC_VARIABLES *adc2, Uint16 channel2);

extern void setAdc4PerModule(struct ADC_VARIABLES *adc1, Uint16 channel1,
                             struct ADC_VARIABLES *adc2, Uint16 channel2,
                             struct ADC_VARIABLES *adc3, Uint16 channel3,
                             struct ADC_VARIABLES *adc4, Uint16 channel4);

extern void updatePeriodIbcPfm(struct EPWM_VARIABLES *epwm, Uint16 period10ns);

#endif /* CONTROL_LOGIC_H_ */
