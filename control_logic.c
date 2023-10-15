/*
 * control_logic.c
 *
 *  Created on: 13 de out de 2023
 *      Author: willj
 */
#include "F28x_Project.h"
#include "control_logic.h"

//
//  Function Prototypes
//

volatile struct EPWM_REGS *EPWM[MAX_EPWM] = { 0, &EPwm1Regs, &EPwm2Regs,
                                              &EPwm3Regs, &EPwm4Regs,
                                              &EPwm5Regs, &EPwm6Regs,
                                              &EPwm7Regs, &EPwm8Regs,
                                              &EPwm9Regs, &EPwm10Regs,
                                              &EPwm11Regs, &EPwm12Regs };

volatile struct ADC_REGS *ADC[MAX_ADC] = { 0, &AdcaRegs, &AdcbRegs, &AdccRegs,
                                           &AdcdRegs };

void configureIbcPfm(struct IBC_PFM_VARIABLES *ibcPfmVariables)
{

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

//    configureADC(ibcPfmVariables);
    configureEpwms(ibcPfmVariables);

    //Enable group 1 interrupts
    // Enable Global interrupt INTM
    // Enable Global realtime interrupt DBGM
    IER |= M_INT1;
    EINT;
    ERTM;

    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

//    EPwm7Regs.ETSEL.bit.SOCAEN = TB_ENABLE;  //enable SOCA
//    EPwm7Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; //unfreeze, and enter up count mode

}

