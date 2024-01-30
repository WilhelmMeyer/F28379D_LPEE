/*
 * control_logic_epwm.c
 *
 *  Created on: 14 de out de 2023
 *      Author: willj
 */
#include "F28x_Project.h"
#include "control_logic.h"
#include "math.h"

int roundNo(double num)
{
    return num >= 0 ? (double) (num + 0.5) : (double) (num - 0.5);
}

void initializeEpwms(struct IBC_PFM_VARIABLES *ibcPfmVariables)
{
    for (int i = 0; i < MAX_EPWM; i++)
    {
        if (ibcPfmVariables->epwm[i].enable
                && ibcPfmVariables->epwm[i].module == EPWM1_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
            InitEPwm1Gpio();
        }
        else if (ibcPfmVariables->epwm[i].enable
                && ibcPfmVariables->epwm[i].module == EPWM2_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM2 = 1;
            InitEPwm2Gpio();
        }
        else if (ibcPfmVariables->epwm[i].enable
                && ibcPfmVariables->epwm[i].module == EPWM3_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM3 = 1;
            InitEPwm3Gpio();
        }
        else if (ibcPfmVariables->epwm[i].enable
                && ibcPfmVariables->epwm[i].module == EPWM4_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM4 = 1;
            InitEPwm4Gpio();
        }
        else if (ibcPfmVariables->epwm[i].enable
                && ibcPfmVariables->epwm[i].module == EPWM5_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM5 = 1;
            InitEPwm5Gpio();
        }
        else if (ibcPfmVariables->epwm[i].enable
                && ibcPfmVariables->epwm[i].module == EPWM6_MODULE)
        {
            CpuSysRegs.PCLKCR2.bit.EPWM6 = 1;
            InitEPwm6Gpio();
        }

    }

}

void configureMasterEpwm(struct EPWM_VARIABLES *epwm)
{
    (*EPWM[epwm->module]).GLDCTL2.bit.GFRCLD = 0x1;
    (*EPWM[epwm->module]).GLDCTL2.bit.OSHTLD = 0x1;
    (*EPWM[epwm->module]).GLDCTL.bit.GLDMODE = 0x0;
}

void configureSlaveEpwm(struct EPWM_VARIABLES *epwm)
{

//    (*EPWM[epwm->module]).SYNCSELECT.EPWM3SYNCIN;
}

// Configuration to turn Oneshot sync mode enabled
void configureOneShotEpwm(struct EPWM_VARIABLES *epwm)
{
//    (*EPWM[epwm->module]).TBCTL.bit.PRDLD = 0;
//    (*EPWM[epwm->module]).TBCTL2.bit.PRDLDSYNC = 2;
//    (*EPWM[epwm->module]).TBCTL2.bit.SYNCOSELX = 1;
//    (*EPWM[epwm->module]).TBCTL2.bit.OSHTSYNC = 1;
//    (*EPWM[epwm->module]).TBCTL2.bit.OSHTSYNCMODE = 1;

    (*EPWM[epwm->module]).GLDCTL2.bit.GFRCLD = 0x0;
    (*EPWM[epwm->module]).GLDCTL2.bit.OSHTLD = 0x1;
    (*EPWM[epwm->module]).GLDCTL.bit.GLDMODE = 0x0;

//    (*EPWM[epwm->module]).EPWMXLINK.bit.GLDCTL2LINK = 0x2;
//    (*EPWM[epwm->module]).GLDCTL.bit.GLDMODE = 0011;
//
//
//    (*EPWM[epwm->module]).CMPCTL.bit.LOADASYNC = 0010;
//    (*EPWM[epwm->module]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;
//    (*EPWM[epwm->module]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;
//    (*EPWM[epwm->module]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
//    (*EPWM[epwm->module]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

}

void configureOSHTSYNCEpwm(struct EPWM_VARIABLES *epwm)
{
    (*EPWM[epwm->module]).TBCTL2.bit.OSHTSYNC = 0x1;
    (*EPWM[epwm->module]).TBCTL2.bit.OSHTSYNCMODE = 0x1;
}

void configureEpwm(struct EPWM_VARIABLES *epwm)
{

    (*EPWM[epwm->module]).TBCTL.bit.CTRMODE = TB_COUNT_UP; // Count up
    (*EPWM[epwm->module]).TBPRD = epwm->period10ns;       // Set timer period
    if (epwm->module == EPWM1_MODULE)
    {
        (*EPWM[epwm->module]).TBCTL.bit.PHSEN = TB_DISABLE; // Disable phase loading
        (*EPWM[epwm->module]).TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;
    }
    else
    {
        (*EPWM[epwm->module]).TBCTL.bit.PHSEN = TB_ENABLE;
        (*EPWM[epwm->module]).TBCTL.bit.SYNCOSEL = TB_SYNC_IN;
        (*EPWM[epwm->module]).TBCTL2.bit.SYNCOSELX = 00;
    }
    (*EPWM[epwm->module]).TBPHS.bit.TBPHS = 0x0000;        // Phase is 0
    (*EPWM[epwm->module]).TBCTR = 0x0000;                  // Clear counter
    (*EPWM[epwm->module]).TBCTL.bit.HSPCLKDIV = TB_DIV1; // Clock ratio to SYSCLKOUT
    (*EPWM[epwm->module]).TBCTL.bit.CLKDIV = TB_DIV1;

    //
    // Setup shadow register load on ZERO
    //
    (*EPWM[epwm->module]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    (*EPWM[epwm->module]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    (*EPWM[epwm->module]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    (*EPWM[epwm->module]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    //
    // Set Compare values
    //
    (*EPWM[epwm->module]).CMPA.bit.CMPA = epwm->comparatorA10ns; // Set compare A value
    (*EPWM[epwm->module]).CMPB.bit.CMPB = epwm->comparatorB10ns; // Set Compare B value

    //
    // Set actions
    //
    updateEpwmConfiguration(epwm, epwm->configuration);
}

void updatePeriod(struct EPWM_VARIABLES *epwm, Uint16 period10ns)
{
    epwm->period10ns = period10ns;
    if (epwm->followUp == epwm->module)
        (*EPWM[epwm->module]).TBPRD = epwm->period10ns;
}

void updateComparatorA(struct EPWM_VARIABLES *epwm, Uint16 comparatorA10ns)
{
    epwm->comparatorA10ns = roundNo(comparatorA10ns);
    if (epwm->followUp == epwm->module)
        (*EPWM[epwm->module]).CMPA.bit.CMPA = epwm->comparatorA10ns;
}

void updateDutyCycleA(struct EPWM_VARIABLES *epwm, double dutyCycle)
{
    epwm->comparatorA10ns = (Uint16) roundNo(epwm->period10ns * dutyCycle);
    if (epwm->followUp == epwm->module)
        (*EPWM[epwm->module]).CMPA.bit.CMPA = epwm->comparatorA10ns;
}

void updateDutyCycleB(struct EPWM_VARIABLES *epwm, double dutyCycle,
                      Uint16 phase)
{
    epwm->comparatorB10ns = ((Uint16) (epwm->period10ns * dutyCycle)) + phase;
    if (epwm->followUp == epwm->module)
        (*EPWM[epwm->module]).CMPB.bit.CMPB = epwm->comparatorB10ns;
}

void updateComparatorB(struct EPWM_VARIABLES *epwm, Uint16 comparatorB10ns)
{
    epwm->comparatorB10ns = roundNo(comparatorB10ns);
    (*EPWM[epwm->module]).CMPB.bit.CMPB = epwm->comparatorB10ns;
}

void updateAllEpwmPeriodIbcPfm(struct IBC_PFM_VARIABLES *ibcPfmVariables,
                               Uint16 period10ns)
{
    Uint16 comparatorA10ns = period10ns >> 1;
    for (int i = 0; i < MAX_EPWM; i++)
    {
        updatePeriod(&ibcPfmVariables->epwm[i], period10ns);
        updateComparatorA(&ibcPfmVariables->epwm[i], comparatorA10ns);
    }
}

void updateEpwmPeriodIbcPfm(struct EPWM_VARIABLES *epwm, Uint16 period10ns)
{
    Uint16 comparatorA10ns = roundNo(period10ns * 0.5);

    updatePeriod(epwm, period10ns);
    updateComparatorA(epwm, comparatorA10ns);

}

void updateEpwmPeriodNormal(struct EPWM_VARIABLES *epwm, Uint16 period10ns)
{
    updatePeriod(epwm, period10ns);
}

void updateEpwmPeriodAndPhaseIbcPfm(struct EPWM_VARIABLES *epwm,
                                    Uint16 period10ns, Uint16 phase10ns)
{
    Uint16 comparatorA10ns = roundNo(phase10ns);
    Uint16 comparatorB10ns = roundNo(phase10ns + (period10ns * 0.5));

    updatePeriod(epwm, period10ns);
    updateComparatorA(epwm, comparatorA10ns);
    updateComparatorB(epwm, comparatorB10ns);
}

void updateEpwmDelayIbcPfm(struct EPWM_VARIABLES *epwm, Uint16 delay10ns)
{
    epwm->risingEdgeDelay10ns = delay10ns;
    epwm->fallingEdgeDelay10ns = delay10ns;
    (*EPWM[epwm->module]).DBRED.bit.DBRED = epwm->risingEdgeDelay10ns;
    (*EPWM[epwm->module]).DBFED.bit.DBFED = epwm->fallingEdgeDelay10ns;
}

void updateEpwmREDIbcPfm(struct EPWM_VARIABLES *epwm, Uint16 delay10ns)
{
    epwm->risingEdgeDelay10ns = delay10ns;
    (*EPWM[epwm->module]).DBRED.bit.DBRED = epwm->risingEdgeDelay10ns;
}

void updateEpwmFEDIbcPfm(struct EPWM_VARIABLES *epwm, Uint16 delay10ns)
{
    epwm->fallingEdgeDelay10ns = delay10ns;
    (*EPWM[epwm->module]).DBFED.bit.DBFED = epwm->fallingEdgeDelay10ns;
}
void updateEpwmPOLSELIbcPfm(struct EPWM_VARIABLES *epwm, Uint16 mode)
{
    epwm->POLSEL = mode;
    (*EPWM[epwm->module]).DBCTL.bit.POLSEL = epwm->POLSEL;
}
void updateEpwmOUTSWAPIbcPfm(struct EPWM_VARIABLES *epwm, Uint16 mode)
{
    epwm->OUTSWAP = mode;
    (*EPWM[epwm->module]).DBCTL.bit.OUTSWAP = epwm->OUTSWAP;
}

void updateEpwmDEDB_MODEIbcPfm(struct EPWM_VARIABLES *epwm, Uint16 mode)
{
    epwm->DEDB_MODE = mode;
    (*EPWM[epwm->module]).DBCTL.bit.DEDB_MODE = epwm->OUTSWAP;
}

void configureIbcPfmEpwm(struct EPWM_VARIABLES *epwm, Uint16 epwmConfiguration,
                         Uint16 epwmModule, Uint16 followUp, Uint32 period10ns,
                         Uint32 comparatorA10ns, Uint32 comparatorB10ns,
                         Uint32 delay10ns)
{
    period10ns--; // fix epwm TBPRD count

    epwm->period10ns = period10ns;
    epwm->comparatorA10ns = comparatorA10ns;
    epwm->comparatorB10ns = comparatorB10ns;
    epwm->configuration = epwmConfiguration;
    epwm->risingEdgeDelay10ns = delay10ns;
    epwm->fallingEdgeDelay10ns = delay10ns;
    epwm->module = epwmModule;
    epwm->followUp = followUp;
    epwm->enable = EPWM_ENABLE;
}

void updateEpwmConfiguration(struct EPWM_VARIABLES *epwm,
                             Uint16 epwmConfiguration)
{

    epwm->configuration = epwmConfiguration;

    if (epwm->configuration == EPWM_ALWAYS_OFF)
    {
        (*EPWM[epwm->module]).AQCTLA.bit.ZRO = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLA.bit.PRD = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLA.bit.CAU = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLA.bit.CAD = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLA.bit.CBU = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLA.bit.CBD = AQ_CLEAR;

        (*EPWM[epwm->module]).AQCTLB.bit.ZRO = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLB.bit.PRD = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLB.bit.CAU = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLB.bit.CAD = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLB.bit.CBU = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLB.bit.CBD = AQ_CLEAR;

    }
    else if (epwm->configuration == EPWM_ALWAYS_ON)
    {
        (*EPWM[epwm->module]).AQCTLA.bit.ZRO = AQ_SET;
        (*EPWM[epwm->module]).AQCTLA.bit.PRD = AQ_SET;
        (*EPWM[epwm->module]).AQCTLA.bit.CAU = AQ_SET;
        (*EPWM[epwm->module]).AQCTLA.bit.CAD = AQ_SET;
        (*EPWM[epwm->module]).AQCTLA.bit.CBU = AQ_SET;
        (*EPWM[epwm->module]).AQCTLA.bit.CBD = AQ_SET;

        (*EPWM[epwm->module]).AQCTLB.bit.ZRO = AQ_SET;
        (*EPWM[epwm->module]).AQCTLB.bit.PRD = AQ_SET;
        (*EPWM[epwm->module]).AQCTLB.bit.CAU = AQ_SET;
        (*EPWM[epwm->module]).AQCTLB.bit.CAD = AQ_SET;
        (*EPWM[epwm->module]).AQCTLB.bit.CBU = AQ_SET;
        (*EPWM[epwm->module]).AQCTLB.bit.CBD = AQ_SET;

    }
    else if (epwm->configuration == EPWM_SC_PFM
            || epwm->configuration == EPWM_SC_PFM_SUPERPOSITION
            || epwm->configuration == EPWM_SC_PFM_DEADBAND)
    {

        (*EPWM[epwm->module]).AQCTLA.bit.ZRO = AQ_SET;
        (*EPWM[epwm->module]).AQCTLA.bit.PRD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.CAU = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLA.bit.CAD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.CBU = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.CBD = AQ_NO_ACTION;

        (*EPWM[epwm->module]).AQCTLB.bit.ZRO = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLB.bit.PRD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLB.bit.CAU = AQ_SET;
        (*EPWM[epwm->module]).AQCTLB.bit.CAD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLB.bit.CBU = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLB.bit.CBD = AQ_NO_ACTION;
    }
    else if (epwm->configuration == EPWM_SIMPLE_PWM)
    {

        (*EPWM[epwm->module]).AQCTLA.bit.ZRO = AQ_SET;
        (*EPWM[epwm->module]).AQCTLA.bit.PRD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.CAU = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLA.bit.CAD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.CBU = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.CBD = AQ_NO_ACTION;

    }
    else if (epwm->configuration == EPWM_DUAL_PWM)
    {

        (*EPWM[epwm->module]).AQCTLA.bit.ZRO = AQ_SET;
        (*EPWM[epwm->module]).AQCTLA.bit.PRD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.CAU = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLA.bit.CAD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.CBU = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.CBD = AQ_NO_ACTION;

        (*EPWM[epwm->module]).AQCTLB.bit.ZRO = AQ_SET;
        (*EPWM[epwm->module]).AQCTLB.bit.PRD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLB.bit.CAU = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLB.bit.CAD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLB.bit.CBU = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLB.bit.CBD = AQ_NO_ACTION;

    }
    else if (epwm->configuration == EPWM_DUAL_PWM_SECOND)
    {

        (*EPWM[epwm->module]).AQCTLA.bit.ZRO = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.PRD = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLA.bit.CAU = AQ_SET;
        (*EPWM[epwm->module]).AQCTLA.bit.CAD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.CBU = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.CBD = AQ_NO_ACTION;

        (*EPWM[epwm->module]).AQCTLB.bit.ZRO = AQ_SET;
        (*EPWM[epwm->module]).AQCTLB.bit.PRD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLB.bit.CAU = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLB.bit.CAD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLB.bit.CBU = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLB.bit.CBD = AQ_NO_ACTION;

    }
    else if (epwm->configuration == EPWM_SIMPLE_PWM_PHASE)
    {

        (*EPWM[epwm->module]).AQCTLA.bit.ZRO = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.PRD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.CAU = AQ_SET;
        (*EPWM[epwm->module]).AQCTLA.bit.CAD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.CBU = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLA.bit.CBD = AQ_NO_ACTION;

    }

    else if (epwm->configuration == EPWM_SC_PFM_PHASE
            || epwm->configuration == EPWM_SC_PFM_PHASE_SUPERPOSITION
            || epwm->configuration == EPWM_SC_PFM_PHASE_DEADBAND)
    {
        (*EPWM[epwm->module]).AQCTLA.bit.ZRO = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.PRD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.CAU = AQ_SET;
        (*EPWM[epwm->module]).AQCTLA.bit.CAD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLA.bit.CBU = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLA.bit.CBD = AQ_NO_ACTION;

        (*EPWM[epwm->module]).AQCTLB.bit.ZRO = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLB.bit.PRD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLB.bit.CAU = AQ_CLEAR;
        (*EPWM[epwm->module]).AQCTLB.bit.CAD = AQ_NO_ACTION;
        (*EPWM[epwm->module]).AQCTLB.bit.CBU = AQ_SET;
        (*EPWM[epwm->module]).AQCTLB.bit.CBD = AQ_NO_ACTION;
    }

    if (epwm->configuration == EPWM_SC_PFM_SUPERPOSITION
            || epwm->configuration == EPWM_SC_PFM_PHASE_SUPERPOSITION)
    {
        (*EPWM[epwm->module]).DBCTL.bit.IN_MODE = DBA_ALL;
        (*EPWM[epwm->module]).DBCTL.bit.POLSEL = DB_ACTV_LOC;
        (*EPWM[epwm->module]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
        (*EPWM[epwm->module]).DBCTL.bit.OUTSWAP = DB_OUTSWAP_BA;
        (*EPWM[epwm->module]).DBRED.bit.DBRED = epwm->risingEdgeDelay10ns;
        (*EPWM[epwm->module]).DBFED.bit.DBFED = epwm->fallingEdgeDelay10ns;
    }
    else if (epwm->configuration == EPWM_DUAL_PWM)
    {
        (*EPWM[epwm->module]).DBCTL.bit.IN_MODE = DBB_RED_DBA_FED;
        (*EPWM[epwm->module]).DBCTL.bit.DEDB_MODE = 1;
        (*EPWM[epwm->module]).DBCTL.bit.POLSEL = DB_ACTV_HI;
        (*EPWM[epwm->module]).DBCTL.bit.OUT_MODE = DBB_ENABLE;
        (*EPWM[epwm->module]).DBCTL.bit.OUTSWAP = DB_OUTSWAP_AB;
        (*EPWM[epwm->module]).DBRED.bit.DBRED = epwm->risingEdgeDelay10ns;
        (*EPWM[epwm->module]).DBFED.bit.DBFED = epwm->fallingEdgeDelay10ns;
    }

    else if (epwm->configuration == EPWM_DUAL_PWM_SECOND)
    {
        (*EPWM[epwm->module]).DBCTL.bit.IN_MODE = DBB_RED_DBA_FED;
        (*EPWM[epwm->module]).DBCTL.bit.DEDB_MODE = 0;
        (*EPWM[epwm->module]).DBCTL.bit.POLSEL = DB_ACTV_HIC;
        (*EPWM[epwm->module]).DBCTL.bit.OUT_MODE = DB_ACTV_LO;
        (*EPWM[epwm->module]).DBCTL.bit.OUTSWAP = DB_OUTSWAP_BA;
        (*EPWM[epwm->module]).DBRED.bit.DBRED = epwm->risingEdgeDelay10ns;
        (*EPWM[epwm->module]).DBFED.bit.DBFED = epwm->fallingEdgeDelay10ns;
    }

    else if (epwm->configuration == EPWM_SIMPLE_PWM_PHASE)
    {
        (*EPWM[epwm->module]).DBCTL.bit.IN_MODE = DBB_RED_DBA_FED;
        (*EPWM[epwm->module]).DBCTL.bit.DEDB_MODE = 0;
        (*EPWM[epwm->module]).DBCTL.bit.POLSEL = DB_ACTV_HI;
        (*EPWM[epwm->module]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
        (*EPWM[epwm->module]).DBCTL.bit.OUTSWAP = DB_OUTSWAP_BA;
        (*EPWM[epwm->module]).DBRED.bit.DBRED = epwm->risingEdgeDelay10ns;
        (*EPWM[epwm->module]).DBFED.bit.DBFED = epwm->fallingEdgeDelay10ns;
    }

    else if (epwm->configuration == EPWM_SC_PFM_DEADBAND
            || epwm->configuration == EPWM_SC_PFM_PHASE_DEADBAND)
    {
        (*EPWM[epwm->module]).DBCTL.bit.IN_MODE = DBA_ALL;
        (*EPWM[epwm->module]).DBCTL.bit.POLSEL = DB_ACTV_HIC;
        (*EPWM[epwm->module]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
        (*EPWM[epwm->module]).DBCTL.bit.OUTSWAP = DB_OUTSWAP_AB;
        (*EPWM[epwm->module]).DBRED.bit.DBRED = epwm->risingEdgeDelay10ns;
        (*EPWM[epwm->module]).DBFED.bit.DBFED = epwm->fallingEdgeDelay10ns;
    }
    else
    {
        (*EPWM[epwm->module]).DBCTL.bit.OUT_MODE = DB_DISABLE;
    }

}
