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


struct AQUISITION_VARIABLES
{
    Uint16 adcChannel;
    Uint16 adcResults;
    volatile struct ADC_REGS *adcRegs;
};

struct EPWM_VARIABLES
{
    Uint16 epwmChannel;
    Uint16 period10ns;
    Uint16 comparatorA;
    Uint16 comparatorB;
    Uint16 deadbandEnable;
    Uint16 risingEdgeDelay;
    Uint16 fallingEdgeDelay;
    volatile struct EPWM_REGS *ePwmRegs;
};

struct IBC_PFM_VARIABLES
{
    PINT isrPointer;
    struct AQUISITION_VARIABLES aquisition[5];
    struct EPWM_VARIABLES epwm[6];
    Uint16 epwmEnable;
};



inline void initializeDsp(){

    InitSysCtrl();
    InitGpio();

}

inline void initializeEpwms(struct IBC_PFM_VARIABLES *ibcPfmVariables){

    //
    // Enable PWM1
    //
        CpuSysRegs.PCLKCR2.bit.EPWM1=1;
        CpuSysRegs.PCLKCR2.bit.EPWM2=1;

    //
    // For this case just init GPIO pins for ePWM1
    // These functions are in the F2837xD_EPwm.c file
    //
        InitEPwm1Gpio();
        InitEPwm2Gpio();

}


inline void initializeInterrupts(){
    //
    // Step 3. Clear all interrupts and initialize PIE vector table:
    // Disable CPU interrupts
    //
        DINT;

    //
    // Initialize the PIE control registers to their default state.
    // The default state is all PIE interrupts disabled and flags
    // are cleared.
    // This function is found in the F2837xD_PieCtrl.c file.
    //
        InitPieCtrl();

    //
    // Disable CPU interrupts and clear all CPU interrupt flags:
    //
        IER = 0x0000;
        IFR = 0x0000;

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    // This will populate the entire table, even if the interrupt
    // is not used in this example.  This is useful for debug purposes.
    // The shell ISR routines are found in F2837xD_DefaultIsr.c.
    // This function is found in F2837xD_PieVect.c.
    //
        InitPieVectTable();

}


extern void configureDcDcIbcPfm(struct IBC_PFM_VARIABLES *dcdcIbcPfmVariables);
extern void configureADC(struct IBC_PFM_VARIABLES *dcdcIbcPfmPointer);
extern void setupAdcEpwm(struct IBC_PFM_VARIABLES *dcdcIbcPfmPointer);




#endif /* CONTROL_LOGIC_H_ */
