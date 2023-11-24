/*
 * control_logic.c
 *
 *  Created on: 13 de out de 2023
 *      Author: willj
 */
#include "F28x_Project.h"
#include "control_logic.h"

//
// struct variables initialization
//

volatile struct EPWM_REGS *EPWM[TOTAL_EPWM] = { &EPwm1Regs, &EPwm2Regs,
                                                &EPwm3Regs, &EPwm4Regs,
                                                &EPwm5Regs, &EPwm6Regs,
                                                &EPwm7Regs, &EPwm8Regs,
                                                &EPwm9Regs, &EPwm10Regs,
                                                &EPwm11Regs, &EPwm12Regs };

volatile struct ADC_REGS *ADC[TOTAL_ADC] = { &AdcaRegs, &AdcbRegs, &AdccRegs,
                                             &AdcdRegs };

volatile Uint16 *ADC_RESULTS[TOTAL_ADC][TOTAL_ADC_RESULTS] = {
        { &AdcaResultRegs.ADCRESULT0, &AdcaResultRegs.ADCRESULT1,
          &AdcaResultRegs.ADCRESULT2, &AdcaResultRegs.ADCRESULT3,
          &AdcaResultRegs.ADCRESULT4, &AdcaResultRegs.ADCRESULT5,
          &AdcaResultRegs.ADCRESULT6, &AdcaResultRegs.ADCRESULT7,
          &AdcaResultRegs.ADCRESULT8, &AdcaResultRegs.ADCRESULT9,
          &AdcaResultRegs.ADCRESULT10, &AdcaResultRegs.ADCRESULT11,
          &AdcaResultRegs.ADCRESULT12, &AdcaResultRegs.ADCRESULT13,
          &AdcaResultRegs.ADCRESULT14, &AdcaResultRegs.ADCRESULT15 },
        { &AdcbResultRegs.ADCRESULT0, &AdcbResultRegs.ADCRESULT1,
          &AdcbResultRegs.ADCRESULT2, &AdcbResultRegs.ADCRESULT3,
          &AdcbResultRegs.ADCRESULT4, &AdcbResultRegs.ADCRESULT5,
          &AdcbResultRegs.ADCRESULT6, &AdcbResultRegs.ADCRESULT7,
          &AdcbResultRegs.ADCRESULT8, &AdcbResultRegs.ADCRESULT9,
          &AdcbResultRegs.ADCRESULT10, &AdcbResultRegs.ADCRESULT11,
          &AdcbResultRegs.ADCRESULT12, &AdcbResultRegs.ADCRESULT13,
          &AdcbResultRegs.ADCRESULT14, &AdcbResultRegs.ADCRESULT15 },
        { &AdccResultRegs.ADCRESULT0, &AdccResultRegs.ADCRESULT1,
          &AdccResultRegs.ADCRESULT2, &AdccResultRegs.ADCRESULT3,
          &AdccResultRegs.ADCRESULT4, &AdccResultRegs.ADCRESULT5,
          &AdccResultRegs.ADCRESULT6, &AdccResultRegs.ADCRESULT7,
          &AdccResultRegs.ADCRESULT8, &AdccResultRegs.ADCRESULT9,
          &AdccResultRegs.ADCRESULT10, &AdccResultRegs.ADCRESULT11,
          &AdccResultRegs.ADCRESULT12, &AdccResultRegs.ADCRESULT13,
          &AdccResultRegs.ADCRESULT14, &AdccResultRegs.ADCRESULT15 },
        { &AdcdResultRegs.ADCRESULT0, &AdcdResultRegs.ADCRESULT1,
          &AdcdResultRegs.ADCRESULT2, &AdcdResultRegs.ADCRESULT3,
          &AdcdResultRegs.ADCRESULT4, &AdcdResultRegs.ADCRESULT5,
          &AdcdResultRegs.ADCRESULT6, &AdcdResultRegs.ADCRESULT7,
          &AdcdResultRegs.ADCRESULT8, &AdcdResultRegs.ADCRESULT9,
          &AdcdResultRegs.ADCRESULT10, &AdcdResultRegs.ADCRESULT11,
          &AdcdResultRegs.ADCRESULT12, &AdcdResultRegs.ADCRESULT13,
          &AdcdResultRegs.ADCRESULT14, &AdcdResultRegs.ADCRESULT15 } };

//
//  Function Prototypes
//

void initializeDsp()
{

    InitSysCtrl();
    InitGpio();
    DINT;
    InitPieCtrl();
    IER = 0x0000;
    IFR = 0x0000;
    InitPieVectTable();

}

void configureIbcPfm(struct IBC_PFM_VARIABLES *ibcPfmVariables)
{

    initializeEpwms(ibcPfmVariables);

    initializeAdcInterrupts(ibcPfmVariables);

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    for (int i = 0; i < MAX_ADC; i++)
    {
        if (ibcPfmVariables->adc[i].enable)
        {
            configureADC(&ibcPfmVariables->adc[i]);
        }
    }

    configureAdcEPWM(ibcPfmVariables);

    for (int i = 0; i < MAX_EPWM; i++)
    {
        if (ibcPfmVariables->epwm[i].enable)
        {
            configureEpwm(&ibcPfmVariables->epwm[i]);
        }
    }

    //Enable group 1 interrupts (ADCx1)
    // Enable Global interrupt INTM
    // Enable Global realtime interrupt DBGM
    IER |= M_INT1;
    EINT;
    ERTM;

    switch (ibcPfmVariables->epwm[0].module)
    {
    case ADCA_MODULE:
        PieCtrlRegs.PIEIER1.bit.INTx1 = 1;
        break;
    case ADCB_MODULE:
        PieCtrlRegs.PIEIER1.bit.INTx2 = 1;
        break;
    case ADCC_MODULE:
        PieCtrlRegs.PIEIER1.bit.INTx3 = 1;
        break;
    case ADCD_MODULE:
        PieCtrlRegs.PIEIER1.bit.INTx6 = 1;
        break;
    }

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    EPwm7Regs.ETSEL.bit.SOCAEN = TB_ENABLE;  //enable SOCA
    EPwm7Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; //unfreeze, and enter up count mode

}

Uint32 frequencyToPeriod10ns(float frequency)
{
    return (Uint32) __divf32(100000000, frequency);
}
