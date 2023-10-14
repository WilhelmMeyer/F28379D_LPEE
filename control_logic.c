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
void InitEPwm1Example(void);
void InitEPwm2Example(void);
void configureAdcEPWM(void);
void initializeDcDcIbcPfm(void);

void configureDcDcIbcPfm(struct IBC_PFM_VARIABLES *dcdcIbcPfmVariables)
{
    //
    // Configure the ADC and power it up
    //
    configureADC(dcdcIbcPfmVariables);

    //
    // Configure the ePWM
    //
    configureAdcEPWM();

    //
    // For this example, only initialize the ePWM
    //
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    //
    // Setup the ADC for ePWM triggered conversions on channel 0
    //
    setupAdcEpwm(dcdcIbcPfmVariables);
    initializeDcDcIbcPfm();

    //Enable group 1 interrupts
    // Enable Global interrupt INTM
    // Enable Global realtime interrupt DBGM
    IER |= M_INT1;
    EINT;
    ERTM;

    //
    // Enable ADCA INTn in the PIE: Group 1 interrupt 1
    //
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    EPwm7Regs.ETSEL.bit.SOCAEN = TB_ENABLE;  //enable SOCA
    EPwm7Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; //unfreeze, and enter up count mode

}

//
// InitEPwm1Example - Initialize EPWM1 values
//
void initializeDcDcIbcPfm()
{
    //
    // Setup TBCLK
    //
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // Count up
    EPwm1Regs.TBPRD = 2000;       // Set timer period
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;    // Disable phase loading
    EPwm1Regs.TBPHS.bit.TBPHS = 0x0000;        // Phase is 0
    EPwm1Regs.TBCTR = 0x0000;                  // Clear counter
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV2;   // Clock ratio to SYSCLKOUT
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV2;

    //
    // Setup shadow register load on ZERO
    //
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    //
    // Set Compare values
    //
    EPwm1Regs.CMPA.bit.CMPA = 1000;     // Set compare A value
    EPwm1Regs.CMPB.bit.CMPB = 1000;     // Set Compare B value

    //
    // Set actions
    //
    EPwm1Regs.AQCTLA.bit.ZRO = AQ_SET;            // Set PWM1A on Zero
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;          // Clear PWM1A on event A,
                                                  // up count

    EPwm1Regs.AQCTLB.bit.ZRO = AQ_SET;            // Set PWM1B on Zero
    EPwm1Regs.AQCTLB.bit.CAU = AQ_CLEAR;          // Clear PWM1B on event B,
                                                  // up count

    EPwm1Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
    EPwm1Regs.DBCTL.bit.POLSEL = DB_ACTV_LOC;
    EPwm1Regs.DBCTL.bit.IN_MODE = DBA_ALL;
    EPwm1Regs.DBRED.bit.DBRED = 50;
    EPwm1Regs.DBFED.bit.DBFED = 50;

}

