/*
 * control_logic_adc.c
 *
 *  Created on: 13 de out de 2023
 *      Author: willj
 */
#include "F28x_Project.h"
#include "control_logic.h"

//
// Defines
//

//
// Globals
//

//
//  Function Prototypes
//
interrupt void adcaInterruption(void);
interrupt void adcbInterruption(void);
interrupt void adccInterruption(void);
interrupt void adcdInterruption(void);

void setupAdcEpwm(struct ADC_VARIABLES adc);

void (*adcInterruptionFunction)(void);

void initializeAdcInterrupts(struct IBC_PFM_VARIABLES *ibcPfmVariables)
{


    if (!ibcPfmVariables->adcs[0].enable)
        return;

    EALLOW;

    adcInterruptionFunction = ibcPfmVariables->adcInterruptionFunction;
    switch (ibcPfmVariables->adcs[0].module)
    {
    case ADCA_MODULE:
        PieVectTable.ADCA1_INT = &adcaInterruption; //function for ADCA interrupt 1
        break;
    case ADCB_MODULE:
        PieVectTable.ADCB1_INT = &adcbInterruption; //function for ADCA interrupt 1
        break;
    case ADCC_MODULE:
        PieVectTable.ADCC1_INT = &adccInterruption; //function for ADCA interrupt 1
        break;
    case ADCD_MODULE:
        PieVectTable.ADCD1_INT = &adcdInterruption; //function for ADCA interrupt 1
        break;
    }

    EDIS;
}

void configureADC(struct ADC_VARIABLES *adc)
{
    if (!adc->enable)
        return;

    EALLOW;

    if (adc->channel < 0x10)
    {
        adc->module = ADCA_MODULE;
        (*ADC[adc->module]).ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
        AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    }
    else if (adc->channel < 0x20)
    {
        adc->module = ADCB_MODULE;
        (*ADC[adc->module]).ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
        AdcSetMode(ADC_ADCB, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    }
    else if (adc->channel < 0x30)
    {
        adc->module = ADCC_MODULE;
        (*ADC[adc->module]).ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
        AdcSetMode(ADC_ADCC, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    }
    else if (adc->channel < 0x40)
    {
        adc->module = ADCD_MODULE;
        (*ADC[adc->module]).ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
        AdcSetMode(ADC_ADCD, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    }

    //
    //write configurations
    //

    (*ADC[adc->module]).ADCCTL1.bit.INTPULSEPOS = 1;
    (*ADC[adc->module]).ADCCTL1.bit.ADCPWDNZ = 1;

    //
    //delay for 1ms to allow ADC time to power up
    //
    DELAY_US(1000);

    EDIS;

    setupAdcEpwm(*adc);
}

//
// ConfigureEPWM - Configure EPWM SOC and compare values
//
void configureAdcEPWM(struct IBC_PFM_VARIABLES *ibcPfmVariables)
{
    EALLOW;
    // Assumes ePWM clock is already enabled
    EPwm7Regs.ETSEL.bit.SOCAEN = TB_DISABLE;    // Enable SOC on A group
    EPwm7Regs.ETSEL.bit.SOCASEL = 4;   // Select SOC on up-count
    EPwm7Regs.ETPS.bit.SOCAPRD = 1;       // Generate pulse on 1st event
    EPwm7Regs.CMPA.bit.CMPA = ibcPfmVariables->adcComparatorA10ns; // Set compare A value
    EPwm7Regs.TBPRD = ibcPfmVariables->adcPeriod10ns; // Set period to 4096 counts
    EPwm7Regs.TBCTL.bit.CTRMODE = TB_FREEZE;      // unfreeze counter
    EDIS;
}

void setupAdcEpwm(struct ADC_VARIABLES adc)
{
    Uint16 acqps = 14;
    Uint32 channel = adc.channel & 0xF;

    //
    //Select the channels to convert and end of conversion flag
    //
    EALLOW;
    if (adc.overSample & SINGLE_SAMPLE_1)
    {
        (*ADC[adc.module]).ADCSOC0CTL.bit.CHSEL = channel;
        (*ADC[adc.module]).ADCSOC0CTL.bit.ACQPS = acqps;
        (*ADC[adc.module]).ADCSOC0CTL.bit.TRIGSEL =
        ADCTRIG_EPWM7_SOCA;
    }
    if (adc.overSample & SINGLE_SAMPLE_2)
    {
        (*ADC[adc.module]).ADCSOC1CTL.bit.CHSEL = channel;
        (*ADC[adc.module]).ADCSOC1CTL.bit.ACQPS = acqps;
        (*ADC[adc.module]).ADCSOC1CTL.bit.TRIGSEL =
        ADCTRIG_EPWM7_SOCA;
    }
    if (adc.overSample & SINGLE_SAMPLE_3)
    {
        (*ADC[adc.module]).ADCSOC2CTL.bit.CHSEL = channel;
        (*ADC[adc.module]).ADCSOC2CTL.bit.ACQPS = acqps;
        (*ADC[adc.module]).ADCSOC2CTL.bit.TRIGSEL =
        ADCTRIG_EPWM7_SOCA;
    }
    if (adc.overSample & SINGLE_SAMPLE_4)
    {
        (*ADC[adc.module]).ADCSOC3CTL.bit.CHSEL = channel;
        (*ADC[adc.module]).ADCSOC3CTL.bit.ACQPS = acqps;
        (*ADC[adc.module]).ADCSOC3CTL.bit.TRIGSEL =
        ADCTRIG_EPWM7_SOCA;
    }
    if (adc.overSample & SINGLE_SAMPLE_5)
    {
        (*ADC[adc.module]).ADCSOC4CTL.bit.CHSEL = channel;
        (*ADC[adc.module]).ADCSOC4CTL.bit.ACQPS = acqps;
        (*ADC[adc.module]).ADCSOC4CTL.bit.TRIGSEL =
        ADCTRIG_EPWM7_SOCA;
    }
    if (adc.overSample & SINGLE_SAMPLE_6)
    {
        (*ADC[adc.module]).ADCSOC5CTL.bit.CHSEL = channel;
        (*ADC[adc.module]).ADCSOC5CTL.bit.ACQPS = acqps;
        (*ADC[adc.module]).ADCSOC5CTL.bit.TRIGSEL =
        ADCTRIG_EPWM7_SOCA;
    }
    if (adc.overSample & SINGLE_SAMPLE_7)
    {
        (*ADC[adc.module]).ADCSOC6CTL.bit.CHSEL = channel;
        (*ADC[adc.module]).ADCSOC6CTL.bit.ACQPS = acqps;
        (*ADC[adc.module]).ADCSOC6CTL.bit.TRIGSEL =
        ADCTRIG_EPWM7_SOCA;
    }
    if (adc.overSample & SINGLE_SAMPLE_8)
    {
        (*ADC[adc.module]).ADCSOC7CTL.bit.CHSEL = channel;
        (*ADC[adc.module]).ADCSOC7CTL.bit.ACQPS = acqps;
        (*ADC[adc.module]).ADCSOC7CTL.bit.TRIGSEL =
        ADCTRIG_EPWM7_SOCA;
    }
    if (adc.overSample & SINGLE_SAMPLE_9)
    {
        (*ADC[adc.module]).ADCSOC8CTL.bit.CHSEL = channel;
        (*ADC[adc.module]).ADCSOC8CTL.bit.ACQPS = acqps;
        (*ADC[adc.module]).ADCSOC8CTL.bit.TRIGSEL =
        ADCTRIG_EPWM7_SOCA;
    }
    if (adc.overSample & SINGLE_SAMPLE_10)
    {
        (*ADC[adc.module]).ADCSOC9CTL.bit.CHSEL = channel;
        (*ADC[adc.module]).ADCSOC9CTL.bit.ACQPS = acqps;
        (*ADC[adc.module]).ADCSOC9CTL.bit.TRIGSEL =
        ADCTRIG_EPWM7_SOCA;
    }
    if (adc.overSample & SINGLE_SAMPLE_11)
    {
        (*ADC[adc.module]).ADCSOC10CTL.bit.CHSEL = channel;
        (*ADC[adc.module]).ADCSOC10CTL.bit.ACQPS = acqps;
        (*ADC[adc.module]).ADCSOC10CTL.bit.TRIGSEL =
        ADCTRIG_EPWM7_SOCA;
    }
    if (adc.overSample & SINGLE_SAMPLE_12)
    {
        (*ADC[adc.module]).ADCSOC11CTL.bit.CHSEL = channel;
        (*ADC[adc.module]).ADCSOC11CTL.bit.ACQPS = acqps;
        (*ADC[adc.module]).ADCSOC11CTL.bit.TRIGSEL =
        ADCTRIG_EPWM7_SOCA;
    }
    if (adc.overSample & SINGLE_SAMPLE_13)
    {
        (*ADC[adc.module]).ADCSOC12CTL.bit.CHSEL = channel;
        (*ADC[adc.module]).ADCSOC12CTL.bit.ACQPS = acqps;
        (*ADC[adc.module]).ADCSOC12CTL.bit.TRIGSEL =
        ADCTRIG_EPWM7_SOCA;
    }
    if (adc.overSample & SINGLE_SAMPLE_14)
    {
        (*ADC[adc.module]).ADCSOC13CTL.bit.CHSEL = channel;
        (*ADC[adc.module]).ADCSOC13CTL.bit.ACQPS = acqps;
        (*ADC[adc.module]).ADCSOC13CTL.bit.TRIGSEL =
        ADCTRIG_EPWM7_SOCA;
    }
    if (adc.overSample & SINGLE_SAMPLE_15)
    {
        (*ADC[adc.module]).ADCSOC14CTL.bit.CHSEL = channel;
        (*ADC[adc.module]).ADCSOC14CTL.bit.ACQPS = acqps;
        (*ADC[adc.module]).ADCSOC14CTL.bit.TRIGSEL =
        ADCTRIG_EPWM7_SOCA;
    }
    if (adc.overSample & SINGLE_SAMPLE_16)
    {
        (*ADC[adc.module]).ADCSOC15CTL.bit.CHSEL = channel;
        (*ADC[adc.module]).ADCSOC15CTL.bit.ACQPS = acqps;
        (*ADC[adc.module]).ADCSOC15CTL.bit.TRIGSEL =
        ADCTRIG_EPWM7_SOCA;
    }

    (*ADC[adc.module]).ADCINTSEL1N2.bit.INT1SEL = 15; //end of SOC15 will set INT7 flag
    (*ADC[adc.module]).ADCINTSEL1N2.bit.INT1E = 1; //enable INT1 flag
    (*ADC[adc.module]).ADCINTFLGCLR.bit.ADCINT1 = 1; //make sure INT1 flag is cleared
    EDIS;
}

void setAdcModule(struct ADC_VARIABLES *adc)
{
    if (adc->channel < 0x10)
    {
        adc->module = ADCA_MODULE;
        AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    }
    else if (adc->channel < 0x20)
    {
        adc->module = ADCB_MODULE;
        AdcSetMode(ADC_ADCB, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    }
    else if (adc->channel < 0x30)
    {
        adc->module = ADCC_MODULE;
        AdcSetMode(ADC_ADCC, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    }
    else if (adc->channel < 0x40)
    {
        adc->module = ADCD_MODULE;
        AdcSetMode(ADC_ADCD, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    }
}

void setAdcSingle(struct ADC_VARIABLES *adc, Uint16 channel, Uint16 soc)
{
    adc->channel = channel;
    adc->overSample = soc;
    adc->enable = ADC_ENABLE;
    setAdcModule(adc);
}

void setAdc1PerModule(struct ADC_VARIABLES *adc, Uint16 channel)
{
    adc->channel = channel;
    adc->overSample = OVERSAMPLE_16;
    adc->enable = ADC_ENABLE;
    setAdcModule(adc);
}

void setAdc2PerModule(struct ADC_VARIABLES *adc1, Uint16 channel1,
                      struct ADC_VARIABLES *adc2, Uint16 channel2)
{
    adc1->channel = channel1;
    adc1->overSample = OVERSAMPLE_1_8;
    adc1->enable = ADC_ENABLE;
    setAdcModule(adc1);

    adc2->channel = channel2;
    adc2->overSample = OVERSAMPLE_2_8;
    adc2->enable = ADC_ENABLE;
    setAdcModule(adc2);
}

void setAdc4PerModule(struct ADC_VARIABLES *adc1, Uint16 channel1,
                      struct ADC_VARIABLES *adc2, Uint16 channel2,
                      struct ADC_VARIABLES *adc3, Uint16 channel3,
                      struct ADC_VARIABLES *adc4, Uint16 channel4)
{
    adc1->channel = channel1;
    adc1->overSample = OVERSAMPLE_1_4;
    adc1->enable = ADC_ENABLE;
    setAdcModule(adc1);

    adc2->channel = channel2;
    adc2->overSample = OVERSAMPLE_2_4;
    adc2->enable = ADC_ENABLE;
    setAdcModule(adc2);

    adc3->channel = channel3;
    adc3->overSample = OVERSAMPLE_3_4;
    adc3->enable = ADC_ENABLE;
    setAdcModule(adc3);

    adc4->channel = channel4;
    adc4->overSample = OVERSAMPLE_4_4;
    adc4->enable = ADC_ENABLE;
    setAdcModule(adc4);
}

interrupt void adcaInterruption(void)
{

    (*adcInterruptionFunction)();

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void adcbInterruption(void)
{

    (*adcInterruptionFunction)();

    AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void adccInterruption(void)
{

    (*adcInterruptionFunction)();

    AdccRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void adcdInterruption(void)
{

    (*adcInterruptionFunction)();

    AdcdRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

