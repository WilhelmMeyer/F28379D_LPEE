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
    for (int i = 0; i < MAX_EPWM; i++)
    {
        if (ibcPfmVariables->epwms[i].enable
                && ibcPfmVariables->epwms[i].module == EPWM1_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
            InitEPwm1Gpio();
        }
        else if (ibcPfmVariables->epwms[i].enable
                && ibcPfmVariables->epwms[i].module == EPWM2_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM2 = 1;
            InitEPwm2Gpio();
        }
        else if (ibcPfmVariables->epwms[i].enable
                && ibcPfmVariables->epwms[i].module == EPWM3_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM3 = 1;
            InitEPwm3Gpio();
        }
        else if (ibcPfmVariables->epwms[i].enable
                && ibcPfmVariables->epwms[i].module == EPWM4_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM4 = 1;
            InitEPwm4Gpio();
        }
        else if (ibcPfmVariables->epwms[i].enable
                && ibcPfmVariables->epwms[i].module == EPWM5_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM5 = 1;
            InitEPwm5Gpio();
        }
        else if (ibcPfmVariables->epwms[i].enable
                && ibcPfmVariables->epwms[i].module == EPWM6_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM6 = 1;
            InitEPwm6Gpio();
        }

    }

}

void configureEpwm(struct EPWM_VARIABLES epwm)
{

    (*EPWM[epwm.module]).TBCTL.bit.CTRMODE = TB_COUNT_UP; // Count up
    (*EPWM[epwm.module]).TBPRD = epwm.period10ns;       // Set timer period
    if (epwm.module == EPWM1_MODULE)
    {
        (*EPWM[epwm.module]).TBCTL.bit.PHSEN = TB_DISABLE; // Disable phase loading
        (*EPWM[epwm.module]).TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // Disable phase loading
    }
    else
    {
        (*EPWM[epwm.module]).TBCTL.bit.PHSEN = TB_ENABLE;
        (*EPWM[epwm.module]).TBCTL.bit.SYNCOSEL = TB_SYNC_IN;
    }
    (*EPWM[epwm.module]).TBPHS.bit.TBPHS = 0x0000;        // Phase is 0
    (*EPWM[epwm.module]).TBCTR = 0x0000;                  // Clear counter
    (*EPWM[epwm.module]).TBCTL.bit.HSPCLKDIV = TB_DIV1; // Clock ratio to SYSCLKOUT
    (*EPWM[epwm.module]).TBCTL.bit.CLKDIV = TB_DIV1;

    //
    // Setup shadow register load on ZERO
    //
    (*EPWM[epwm.module]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    (*EPWM[epwm.module]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    (*EPWM[epwm.module]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    (*EPWM[epwm.module]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    //
    // Set Compare values
    //
    (*EPWM[epwm.module]).CMPA.bit.CMPA = epwm.comparatorA10ns; // Set compare A value
    (*EPWM[epwm.module]).CMPB.bit.CMPB = epwm.comparatorB10ns; // Set Compare B value

    //
    // Set actions
    //
    (*EPWM[epwm.module]).AQCTLA.bit.ZRO = AQ_SET;       // Set PWM1A on Zero
    (*EPWM[epwm.module]).AQCTLA.bit.CAU = AQ_CLEAR; // Clear PWM1A on event A,
    // up count

    (*EPWM[epwm.module]).AQCTLB.bit.CAU = AQ_SET;       // Set PWM1B on Zero
    (*EPWM[epwm.module]).AQCTLB.bit.ZRO = AQ_CLEAR; // Clear PWM1B on event B,
    // up count

    if (epwm.epwmConfiguration == EPWM_SC_PFM_SUPERPOSITION)
    {
        (*EPWM[epwm.module]).DBCTL.bit.IN_MODE = DBA_ALL;
        (*EPWM[epwm.module]).DBCTL.bit.POLSEL = DB_ACTV_LOC;
        (*EPWM[epwm.module]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
        (*EPWM[epwm.module]).DBCTL.bit.OUTSWAP = DB_OUTSWAP_BA;
        (*EPWM[epwm.module]).DBRED.bit.DBRED = epwm.risingEdgeDelay10ns;
        (*EPWM[epwm.module]).DBFED.bit.DBFED = epwm.fallingEdgeDelay10ns;
    }
    else if (epwm.epwmConfiguration == EPWM_SC_PFM_DEADBAND)
    {
        (*EPWM[epwm.module]).DBCTL.bit.IN_MODE = DBA_ALL;
        (*EPWM[epwm.module]).DBCTL.bit.POLSEL = DB_ACTV_HIC;
        (*EPWM[epwm.module]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
        (*EPWM[epwm.module]).DBCTL.bit.OUTSWAP = DB_OUTSWAP_AB;
        (*EPWM[epwm.module]).DBRED.bit.DBRED = epwm.risingEdgeDelay10ns;
        (*EPWM[epwm.module]).DBFED.bit.DBFED = epwm.fallingEdgeDelay10ns;
    }
}

void updatePeriod(struct EPWM_VARIABLES *epwm, Uint16 period10ns)
{
    epwm->period10ns = period10ns;
    (*EPWM[epwm->module]).TBPRD = epwm->period10ns;
}

void updateComparatorA(struct EPWM_VARIABLES *epwm, Uint16 comparatorA10ns)
{
    epwm->comparatorA10ns = comparatorA10ns;
    (*EPWM[epwm->module]).CMPA.bit.CMPA = epwm->comparatorA10ns;
}

void updateComparatorB(struct EPWM_VARIABLES *epwm, Uint16 comparatorB10ns)
{
    epwm->comparatorB10ns = comparatorB10ns;
    (*EPWM[epwm->module]).CMPB.bit.CMPB = epwm->comparatorB10ns;
}

void updatePeriodIbcPfm(struct EPWM_VARIABLES *epwm, Uint16 period10ns)
{
    Uint16 comparatorA10ns = period10ns >> 1;
    updatePeriod(epwm, period10ns);
    updateComparatorA(epwm, comparatorA10ns);
}
