/*
 * control_logic_adc.c
 *
 *  Created on: 13 de out de 2023
 *      Author: willj
 */
#include "F28x_Project.h"
#include "control_logic.h"
#include "control_logic_adc.h"


//
// Defines
//

//
// Globals
//
struct IBC_PFM_VARIABLES *dcdcIbcPfmGlobal;

//
//  Function Prototypes
//
interrupt void adcaInterruption(void);
interrupt void adcbInterruption(void);
interrupt void adccInterruption(void);
interrupt void adcdInterruption(void);

void (*adcaInterruptionFunction)(void);
void (*adcbInterruptionFunction)(void);
void (*adccInterruptionFunction)(void);
void (*adcdInterruptionFunction)(void);



//
// ConfigureEPWM - Configure EPWM SOC and compare values
//
void configureAdcEPWM(void)
{
    EALLOW;
    // Assumes ePWM clock is already enabled
    EPwm7Regs.ETSEL.bit.SOCAEN = TB_DISABLE;    // Enable SOC on A group
    EPwm7Regs.ETSEL.bit.SOCASEL = 4;   // Select SOC on up-count
    EPwm7Regs.ETPS.bit.SOCAPRD = 1;       // Generate pulse on 1st event
    EPwm7Regs.CMPA.bit.CMPA = 500;     // Set compare A value
    EPwm7Regs.TBPRD = 2000;             // Set period to 4096 counts
    EPwm7Regs.TBCTL.bit.CTRMODE = TB_FREEZE;      // unfreeze counter
    EDIS;
}

void setupAdcEpwm(struct IBC_PFM_VARIABLES *dcdcIbcPfmPointer)
{
    Uint16 acqps;
    Uint32 channel = dcdcIbcPfmPointer->aquisition[0].adcChannel & 0xF;
    dcdcIbcPfmGlobal = dcdcIbcPfmPointer;
    //
    //determine minimum acquisition window (in SYSCLKS) based on resolution
    //
    if (ADC_RESOLUTION_12BIT == AdcaRegs.ADCCTL2.bit.RESOLUTION)
    {
        acqps = 14; //75ns
    }
    else //resolution is 16-bit
    {
        acqps = 63; //320ns
    }

    //
    //Select the channels to convert and end of conversion flag
    //
    EALLOW;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC0CTL.bit.CHSEL = channel;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC0CTL.bit.ACQPS = acqps;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC0CTL.bit.TRIGSEL = ADCTRIG_EPWM7_SOCA;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC1CTL.bit.CHSEL = channel;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC1CTL.bit.ACQPS = acqps;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC1CTL.bit.TRIGSEL = ADCTRIG_EPWM7_SOCA;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC2CTL.bit.CHSEL = channel;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC2CTL.bit.ACQPS = acqps;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC2CTL.bit.TRIGSEL = ADCTRIG_EPWM7_SOCA;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC3CTL.bit.CHSEL = channel;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC3CTL.bit.ACQPS = acqps;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC3CTL.bit.TRIGSEL = ADCTRIG_EPWM7_SOCA;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC4CTL.bit.CHSEL = channel;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC4CTL.bit.ACQPS = acqps;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC4CTL.bit.TRIGSEL = ADCTRIG_EPWM7_SOCA;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC5CTL.bit.CHSEL = channel;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC5CTL.bit.ACQPS = acqps;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC5CTL.bit.TRIGSEL = ADCTRIG_EPWM7_SOCA;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC6CTL.bit.CHSEL = channel;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC6CTL.bit.ACQPS = acqps;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC6CTL.bit.TRIGSEL = ADCTRIG_EPWM7_SOCA;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC7CTL.bit.CHSEL = channel;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC7CTL.bit.ACQPS = acqps;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC7CTL.bit.TRIGSEL = ADCTRIG_EPWM7_SOCA;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC8CTL.bit.CHSEL = channel;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC8CTL.bit.ACQPS = acqps;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC8CTL.bit.TRIGSEL = ADCTRIG_EPWM7_SOCA;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC9CTL.bit.CHSEL = channel;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC9CTL.bit.ACQPS = acqps;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC9CTL.bit.TRIGSEL = ADCTRIG_EPWM7_SOCA;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC10CTL.bit.CHSEL = channel;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC10CTL.bit.ACQPS = acqps;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC10CTL.bit.TRIGSEL = ADCTRIG_EPWM7_SOCA;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC11CTL.bit.CHSEL = channel;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC11CTL.bit.ACQPS = acqps;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC11CTL.bit.TRIGSEL = ADCTRIG_EPWM7_SOCA;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC12CTL.bit.CHSEL = channel;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC12CTL.bit.ACQPS = acqps;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC12CTL.bit.TRIGSEL = ADCTRIG_EPWM7_SOCA;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC13CTL.bit.CHSEL = channel;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC13CTL.bit.ACQPS = acqps;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC13CTL.bit.TRIGSEL = ADCTRIG_EPWM7_SOCA;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC14CTL.bit.CHSEL = channel;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC14CTL.bit.ACQPS = acqps;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC14CTL.bit.TRIGSEL = ADCTRIG_EPWM7_SOCA;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC15CTL.bit.CHSEL = channel;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC15CTL.bit.ACQPS = acqps;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCSOC15CTL.bit.TRIGSEL = ADCTRIG_EPWM7_SOCA;

    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCINTSEL1N2.bit.INT1SEL = 0; //end of SOC0 will set INT7 flag
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCINTSEL1N2.bit.INT1E = 1;   //enable INT1 flag
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCINTFLGCLR.bit.ADCINT1 = 1; //make sure INT1 flag is cleared
    EDIS;
}

void configureADC(struct IBC_PFM_VARIABLES *dcdcIbcPfmPointer)
{
    //
    // Map ISR functions
    //
    EALLOW;
    if (dcdcIbcPfmPointer->aquisition[0].adcChannel < 0x10)
    {
        PieVectTable.ADCA1_INT = dcdcIbcPfmPointer->isrPointer; //function for ADCA interrupt 1
        dcdcIbcPfmPointer->aquisition[0].adcRegs = &AdcaRegs;
    }
    else if (dcdcIbcPfmPointer->aquisition[0].adcChannel < 0x20)
    {
        PieVectTable.ADCB1_INT = dcdcIbcPfmPointer->isrPointer; //function for ADCA interrupt 1
        dcdcIbcPfmPointer->aquisition[0].adcRegs = &AdcbRegs;
    }
    else if (dcdcIbcPfmPointer->aquisition[0].adcChannel < 0x30)
    {
        PieVectTable.ADCC1_INT = dcdcIbcPfmPointer->isrPointer; //function for ADCA interrupt 1
        dcdcIbcPfmPointer->aquisition[0].adcRegs = &AdccRegs;
    }
    else if (dcdcIbcPfmPointer->aquisition[0].adcChannel < 0x40)
    {
        PieVectTable.ADCD1_INT = dcdcIbcPfmPointer->isrPointer; //function for ADCA interrupt 1
        dcdcIbcPfmPointer->aquisition[0].adcRegs = &AdcdRegs;
    }

    //
    //write configurations
    //
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCCTL1.bit.INTPULSEPOS = 1;
    dcdcIbcPfmPointer->aquisition[0].adcRegs->ADCCTL1.bit.ADCPWDNZ = 1;

    //
    //delay for 1ms to allow ADC time to power up
    //
    DELAY_US(1000);

    EDIS;
}

interrupt void adcaInterruption(void)
{

    (*adcaInterruptionFunction)();

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;
}

interrupt void adcbInterruption(void)
{

    (*adcbInterruptionFunction)();

    AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;
}

interrupt void adccInterruption(void)
{

    (*adccInterruptionFunction)();

    AdccRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;
}

interrupt void adcdInterruption(void)
{

    (*adcdInterruptionFunction)();

    AdcdRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;
}

