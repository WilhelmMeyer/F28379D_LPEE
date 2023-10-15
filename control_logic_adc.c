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

void configureAdcEPWM(struct IBC_PFM_VARIABLES *ibcPfmPointer);
void setupAdcEpwm(struct ADC_VARIABLES adc);

void (*adcInterruptionFunction)(void);


void configureADC(struct IBC_PFM_VARIABLES *ibcPfmPointer)
{
    adcInterruptionFunction = ibcPfmPointer->adcInterruptionFunction;

    EALLOW;
    //
    // Map ISR function at adc[0] and Map adcRegs
    //
    for (int i = 0; i < MAX_ADCS; i++)
    {
        if (ibcPfmPointer->adcs[i].adcChannel < 0x10)
        {
            if (i == 0)
                PieVectTable.ADCA1_INT = &adcaInterruption; //function for ADCA interrupt 1
            ibcPfmPointer->adcs[i].adcModule = ADCA_MODULE;
            AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
        }
        else if (ibcPfmPointer->adcs[i].adcChannel < 0x20)
        {
            if (i == 0)
                PieVectTable.ADCB1_INT = &adcbInterruption; //function for ADCB interrupt 1
            ibcPfmPointer->adcs[i].adcModule = ADCB_MODULE;
            AdcSetMode(ADC_ADCB, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
        }
        else if (ibcPfmPointer->adcs[i].adcChannel < 0x30)
        {
            if (i == 0)
                PieVectTable.ADCC1_INT = &adccInterruption; //function for ADCC interrupt 1
            ibcPfmPointer->adcs[i].adcModule = ADCC_MODULE;
            AdcSetMode(ADC_ADCC, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
        }
        else if (ibcPfmPointer->adcs[i].adcChannel < 0x40)
        {
            if (i == 0)
                PieVectTable.ADCD1_INT = &adcdInterruption; //function for ADCD interrupt 1
            ibcPfmPointer->adcs[i].adcModule = ADCD_MODULE;
            AdcSetMode(ADC_ADCD, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
        }

        //
        //write configurations
        //
        (*ADC[ibcPfmPointer->adcs[i].adcModule]).ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
        (*ADC[ibcPfmPointer->adcs[i].adcModule]).ADCCTL1.bit.INTPULSEPOS = 1;
        (*ADC[ibcPfmPointer->adcs[i].adcModule]).ADCCTL1.bit.ADCPWDNZ = 1;

        //
        //delay for 1ms to allow ADC time to power up
        //
        DELAY_US(1000);
    }

    EDIS;

    configureAdcEPWM(ibcPfmPointer);
    setupAdcEpwm(ibcPfmPointer->adcs[0]);
}


//
// ConfigureEPWM - Configure EPWM SOC and compare values
//
void configureAdcEPWM(struct IBC_PFM_VARIABLES *ibcPfmPointer)
{
    EALLOW;
    // Assumes ePWM clock is already enabled
    EPwm7Regs.ETSEL.bit.SOCAEN = TB_DISABLE;    // Enable SOC on A group
    EPwm7Regs.ETSEL.bit.SOCASEL = 4;   // Select SOC on up-count
    EPwm7Regs.ETPS.bit.SOCAPRD = 1;       // Generate pulse on 1st event
    EPwm7Regs.CMPA.bit.CMPA = ibcPfmPointer->adcComparatorA10ns; // Set compare A value
    EPwm7Regs.TBPRD = ibcPfmPointer->adcPeriod10ns; // Set period to 4096 counts
    EPwm7Regs.TBCTL.bit.CTRMODE = TB_FREEZE;      // unfreeze counter
    EDIS;
}

void setupAdcEpwm(struct ADC_VARIABLES adc)
{
    Uint16 acqps = 14;
    Uint32 channel = adc.adcChannel & 0xF;

    //
    //Select the channels to convert and end of conversion flag
    //
    EALLOW;
    (*ADC[adc.adcModule]).ADCSOC0CTL.bit.CHSEL = channel;
    (*ADC[adc.adcModule]).ADCSOC0CTL.bit.ACQPS = acqps;
    (*ADC[adc.adcModule]).ADCSOC0CTL.bit.TRIGSEL =
    ADCTRIG_EPWM7_SOCA;
    (*ADC[adc.adcModule]).ADCSOC1CTL.bit.CHSEL = channel;
    (*ADC[adc.adcModule]).ADCSOC1CTL.bit.ACQPS = acqps;
    (*ADC[adc.adcModule]).ADCSOC1CTL.bit.TRIGSEL =
    ADCTRIG_EPWM7_SOCA;
    (*ADC[adc.adcModule]).ADCSOC2CTL.bit.CHSEL = channel;
    (*ADC[adc.adcModule]).ADCSOC2CTL.bit.ACQPS = acqps;
    (*ADC[adc.adcModule]).ADCSOC2CTL.bit.TRIGSEL =
    ADCTRIG_EPWM7_SOCA;
    (*ADC[adc.adcModule]).ADCSOC3CTL.bit.CHSEL = channel;
    (*ADC[adc.adcModule]).ADCSOC3CTL.bit.ACQPS = acqps;
    (*ADC[adc.adcModule]).ADCSOC3CTL.bit.TRIGSEL =
    ADCTRIG_EPWM7_SOCA;
    (*ADC[adc.adcModule]).ADCSOC4CTL.bit.CHSEL = channel;
    (*ADC[adc.adcModule]).ADCSOC4CTL.bit.ACQPS = acqps;
    (*ADC[adc.adcModule]).ADCSOC4CTL.bit.TRIGSEL =
    ADCTRIG_EPWM7_SOCA;
    (*ADC[adc.adcModule]).ADCSOC5CTL.bit.CHSEL = channel;
    (*ADC[adc.adcModule]).ADCSOC5CTL.bit.ACQPS = acqps;
    (*ADC[adc.adcModule]).ADCSOC5CTL.bit.TRIGSEL =
    ADCTRIG_EPWM7_SOCA;
    (*ADC[adc.adcModule]).ADCSOC6CTL.bit.CHSEL = channel;
    (*ADC[adc.adcModule]).ADCSOC6CTL.bit.ACQPS = acqps;
    (*ADC[adc.adcModule]).ADCSOC6CTL.bit.TRIGSEL =
    ADCTRIG_EPWM7_SOCA;
    (*ADC[adc.adcModule]).ADCSOC7CTL.bit.CHSEL = channel;
    (*ADC[adc.adcModule]).ADCSOC7CTL.bit.ACQPS = acqps;
    (*ADC[adc.adcModule]).ADCSOC7CTL.bit.TRIGSEL =
    ADCTRIG_EPWM7_SOCA;
    (*ADC[adc.adcModule]).ADCSOC8CTL.bit.CHSEL = channel;
    (*ADC[adc.adcModule]).ADCSOC8CTL.bit.ACQPS = acqps;
    (*ADC[adc.adcModule]).ADCSOC8CTL.bit.TRIGSEL =
    ADCTRIG_EPWM7_SOCA;
    (*ADC[adc.adcModule]).ADCSOC9CTL.bit.CHSEL = channel;
    (*ADC[adc.adcModule]).ADCSOC9CTL.bit.ACQPS = acqps;
    (*ADC[adc.adcModule]).ADCSOC9CTL.bit.TRIGSEL =
    ADCTRIG_EPWM7_SOCA;
    (*ADC[adc.adcModule]).ADCSOC10CTL.bit.CHSEL = channel;
    (*ADC[adc.adcModule]).ADCSOC10CTL.bit.ACQPS = acqps;
    (*ADC[adc.adcModule]).ADCSOC10CTL.bit.TRIGSEL =
    ADCTRIG_EPWM7_SOCA;
    (*ADC[adc.adcModule]).ADCSOC11CTL.bit.CHSEL = channel;
    (*ADC[adc.adcModule]).ADCSOC11CTL.bit.ACQPS = acqps;
    (*ADC[adc.adcModule]).ADCSOC11CTL.bit.TRIGSEL =
    ADCTRIG_EPWM7_SOCA;
    (*ADC[adc.adcModule]).ADCSOC12CTL.bit.CHSEL = channel;
    (*ADC[adc.adcModule]).ADCSOC12CTL.bit.ACQPS = acqps;
    (*ADC[adc.adcModule]).ADCSOC12CTL.bit.TRIGSEL =
    ADCTRIG_EPWM7_SOCA;
    (*ADC[adc.adcModule]).ADCSOC13CTL.bit.CHSEL = channel;
    (*ADC[adc.adcModule]).ADCSOC13CTL.bit.ACQPS = acqps;
    (*ADC[adc.adcModule]).ADCSOC13CTL.bit.TRIGSEL =
    ADCTRIG_EPWM7_SOCA;
    (*ADC[adc.adcModule]).ADCSOC14CTL.bit.CHSEL = channel;
    (*ADC[adc.adcModule]).ADCSOC14CTL.bit.ACQPS = acqps;
    (*ADC[adc.adcModule]).ADCSOC14CTL.bit.TRIGSEL =
    ADCTRIG_EPWM7_SOCA;
    (*ADC[adc.adcModule]).ADCSOC15CTL.bit.CHSEL = channel;
    (*ADC[adc.adcModule]).ADCSOC15CTL.bit.ACQPS = acqps;
    (*ADC[adc.adcModule]).ADCSOC15CTL.bit.TRIGSEL =
    ADCTRIG_EPWM7_SOCA;

    (*ADC[adc.adcModule]).ADCINTSEL1N2.bit.INT1SEL = 0; //end of SOC0 will set INT7 flag
    (*ADC[adc.adcModule]).ADCINTSEL1N2.bit.INT1E = 1; //enable INT1 flag
    (*ADC[adc.adcModule]).ADCINTFLGCLR.bit.ADCINT1 = 1; //make sure INT1 flag is cleared
    EDIS;
}



interrupt void adcaInterruption(void)
{

    (*adcInterruptionFunction)();

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;
}

interrupt void adcbInterruption(void)
{

    (*adcInterruptionFunction)();

    AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;
}

interrupt void adccInterruption(void)
{

    (*adcInterruptionFunction)();

    AdccRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;
}

interrupt void adcdInterruption(void)
{

    (*adcInterruptionFunction)();

    AdcdRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;
}

