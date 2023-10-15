/*
 * control_logic_epwm.c
 *
 *  Created on: 14 de out de 2023
 *      Author: willj
 */
#include "F28x_Project.h"
#include "control_logic.h"

void initializeEpwms(struct IBC_PFM_VARIABLES *ibcPfmVariables)
{
    for (int i = 0; i < MAX_EPWMS; i++)
    {
        if (ibcPfmVariables->epwms[i].epwmModule == EPWM1_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
            InitEPwm1Gpio();
        }
        else if (ibcPfmVariables->epwms[i].epwmModule == EPWM2_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM2 = 1;
            InitEPwm2Gpio();
        }
        else if (ibcPfmVariables->epwms[i].epwmModule == EPWM3_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM3 = 1;
            InitEPwm3Gpio();
        }
        else if (ibcPfmVariables->epwms[i].epwmModule == EPWM4_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM4 = 1;
            InitEPwm4Gpio();
        }
        else if (ibcPfmVariables->epwms[i].epwmModule == EPWM5_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM5 = 1;
            InitEPwm5Gpio();
        }
        else if (ibcPfmVariables->epwms[i].epwmModule == EPWM6_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM6 = 1;
            InitEPwm6Gpio();
        }

    }

}

void configureEachEpwm(struct EPWM_VARIABLES epwm)
{

    (*EPWM[epwm.epwmModule]).TBCTL.bit.CTRMODE = TB_COUNT_UP; // Count up
    (*EPWM[epwm.epwmModule]).TBPRD = epwm.period10ns;       // Set timer period
    (*EPWM[epwm.epwmModule]).TBCTL.bit.PHSEN = TB_DISABLE; // Disable phase loading
    (*EPWM[epwm.epwmModule]).TBPHS.bit.TBPHS = 0x0000;        // Phase is 0
    (*EPWM[epwm.epwmModule]).TBCTR = 0x0000;                  // Clear counter
    (*EPWM[epwm.epwmModule]).TBCTL.bit.HSPCLKDIV = TB_DIV1; // Clock ratio to SYSCLKOUT
    (*EPWM[epwm.epwmModule]).TBCTL.bit.CLKDIV = TB_DIV1;

    //
    // Setup shadow register load on ZERO
    //
    (*EPWM[epwm.epwmModule]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    (*EPWM[epwm.epwmModule]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    (*EPWM[epwm.epwmModule]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    (*EPWM[epwm.epwmModule]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    //
    // Set Compare values
    //
    (*EPWM[epwm.epwmModule]).CMPA.bit.CMPA = epwm.comparatorA10ns; // Set compare A value
    (*EPWM[epwm.epwmModule]).CMPB.bit.CMPB = epwm.comparatorB10ns; // Set Compare B value

    //
    // Set actions
    //
    (*EPWM[epwm.epwmModule]).AQCTLA.bit.ZRO = AQ_SET;       // Set PWM1A on Zero
    (*EPWM[epwm.epwmModule]).AQCTLA.bit.CAU = AQ_CLEAR; // Clear PWM1A on event A,
    // up count

    (*EPWM[epwm.epwmModule]).AQCTLB.bit.ZRO = AQ_SET;       // Set PWM1B on Zero
    (*EPWM[epwm.epwmModule]).AQCTLB.bit.CAU = AQ_CLEAR; // Clear PWM1B on event B,
    // up count

    if (epwm.deadbandEnable)
    {
        (*EPWM[epwm.epwmModule]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
        (*EPWM[epwm.epwmModule]).DBCTL.bit.POLSEL = DB_ACTV_LOC;
        (*EPWM[epwm.epwmModule]).DBCTL.bit.IN_MODE = DBA_ALL;
        (*EPWM[epwm.epwmModule]).DBRED.bit.DBRED = epwm.risingEdgeDelay10ns;
        (*EPWM[epwm.epwmModule]).DBFED.bit.DBFED = epwm.fallingEdgeDelay10ns;
    }

}

void configureEpwms(struct IBC_PFM_VARIABLES *ibcPfmVariables)
{
    for (int i = 0; i < MAX_EPWMS; i++)
    {
        if (ibcPfmVariables->epwms[i].epwmModule != 0)
        {
            configureEachEpwm(ibcPfmVariables->epwms[i]);
        }
    }
}
