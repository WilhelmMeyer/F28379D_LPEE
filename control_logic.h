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
#define MAX_EPWM 14
#define MAX_ADC 5

//max for the application
#define MAX_ADCS 8
#define MAX_EPWMS 6

extern volatile struct EPWM_REGS *EPWM[MAX_EPWM];
extern volatile struct ADC_REGS *ADC[MAX_ADC];

struct ADC_VARIABLES
{
    Uint16 adcChannel;
    Uint16 adcResults;
    Uint16 socEnable;
    Uint16 adcModule;
};

struct EPWM_VARIABLES
{
    Uint16 epwmChannel;
    Uint16 period10ns;
    Uint16 comparatorA10ns;
    Uint16 comparatorB10ns;
    Uint16 deadbandEnable;
    Uint16 risingEdgeDelay10ns;
    Uint16 fallingEdgeDelay10ns;
    Uint16 epwmModule;
};

struct IBC_PFM_VARIABLES
{
    void (*adcInterruptionFunction)(void);
    struct ADC_VARIABLES adcs[MAX_ADCS];
    struct EPWM_VARIABLES epwms[MAX_EPWMS];
    Uint16 adcPeriod10ns;
    Uint16 adcComparatorA10ns;
};

inline void initializeDsp()
{

    InitSysCtrl();
    InitGpio();

}

inline void initializeInterrupts()
{

    // Disable CPU interrupts
    DINT;

    // Initialize the PIE control registers to their default state.
    InitPieCtrl();

    // Disable CPU interrupts and clear all CPU interrupt flags:
    IER = 0x0000;
    IFR = 0x0000;

    // Initialize the PIE vector table with pointers to the shell Interrupt
    InitPieVectTable();

}

extern void configureIbcPfm(struct IBC_PFM_VARIABLES *dcdcIbcPfmVariables);
extern void configureADC(struct IBC_PFM_VARIABLES *dcdcIbcPfmPointer);
extern void initializeEpwms(struct IBC_PFM_VARIABLES *ibcPfmVariables);
extern void configureEpwms(struct IBC_PFM_VARIABLES *ibcPfmVariables);

#endif /* CONTROL_LOGIC_H_ */
