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
#include "control_logic_util.h"

#ifndef CONTROL_LOGIC_H_
#define CONTROL_LOGIC_H_

//
// defines
//

//max total
#define TOTAL_EPWM 13
#define TOTAL_ADC 4
#define TOTAL_ADC_RESULTS 16

//max for the application
#define MAX_ADC 8
#define MAX_EPWM 6

//
// Struct variables definitions
//

extern volatile struct EPWM_REGS *EPWM[TOTAL_EPWM];
extern volatile struct ADC_REGS *ADC[TOTAL_ADC];
extern volatile Uint16 *ADC_RESULTS[TOTAL_ADC][TOTAL_ADC_RESULTS];

struct ADC_VARIABLES
{
    Uint16 channel;
    Uint16 results;
    Uint16 overSample;
    Uint16 module;
    Uint16 enable;
    double coefA;
    double coefB;
    double value;
    Uint32 cutOffFrequencyHz;
    double coefFilter;
    double filteredValue;
};

struct EPWM_VARIABLES
{
    Uint16 period10ns;
    Uint16 comparatorA10ns;
    Uint16 comparatorB10ns;
    Uint16 epwmConfiguration;
    Uint16 risingEdgeDelay10ns;
    Uint16 fallingEdgeDelay10ns;
    Uint16 module;
    Uint16 followUp;
    Uint16 enable;
};

struct IBC_PFM_VARIABLES
{
    void (*adcInterruptionFunction)(void);
    struct ADC_VARIABLES adc[MAX_ADC];
    struct EPWM_VARIABLES epwm[MAX_EPWM];
    Uint16 adcPeriod10ns;
    Uint16 adcComparatorA10ns;
};

//
// control_logic.c global functions
//
extern void initializeDsp(void);

extern void configureIbcPfm(struct IBC_PFM_VARIABLES *ibcPfmVariables);

extern Uint32 frequencyToPeriod10ns(float frequency);

//
// control_logic_adc.c acquisition functions
//
extern void initializeAdcInterrupts(struct IBC_PFM_VARIABLES *ibcPfmVariables);

extern void configureADC(struct ADC_VARIABLES *adc);

extern void configureAdcEPWM(struct IBC_PFM_VARIABLES *ibcPfmVariables);

extern void setAdc1PerModule(struct ADC_VARIABLES *adc, Uint16 channel);

extern void setAdc2PerModule(struct ADC_VARIABLES *adc1, Uint16 channel1,
                             struct ADC_VARIABLES *adc2, Uint16 channel2);

extern void setAdc4PerModule(struct ADC_VARIABLES *adc1, Uint16 channel1,
                             struct ADC_VARIABLES *adc2, Uint16 channel2,
                             struct ADC_VARIABLES *adc3, Uint16 channel3,
                             struct ADC_VARIABLES *adc4, Uint16 channel4);

extern void configureAdcAquisition(struct ADC_VARIABLES *adc, float coefA,
                                   float coefB, float cutOffFrequencyHz,
                                   Uint16 samplingPeriod10ns);

extern void updateSamplingPeriodIbcPfm(
        struct IBC_PFM_VARIABLES *ibcPfmVariables, Uint16 period10ns);

extern Uint16 updateAnalogResultRead(struct ADC_VARIABLES *adc);

extern double updateAnalogValue(struct ADC_VARIABLES *adc);

extern double updateAnalogValueFiltered(struct ADC_VARIABLES *adc);

//
// control_logic_epwm.c epwm functions
//
extern void initializeEpwms(struct IBC_PFM_VARIABLES *ibcPfmVariables);

extern void configureEpwm(struct EPWM_VARIABLES epwm);

extern void updateEpwmPeriodIbcPfm(struct IBC_PFM_VARIABLES *ibcPfmVariables,
                                   Uint16 period10ns);

extern void configureIbcPfmEpwm(struct EPWM_VARIABLES *epwm,
                                Uint16 epwmConfiguration, Uint16 epwmModule,
                                Uint16 followUp, Uint32 period10ns,
                                Uint32 delay10ns);

//
// control_logic_util.c util functions
//
extern double rmsCalculation(struct RMS_CALCULATION *rms,
                             double newAcquisition);

extern void configureRmsCalculation(struct RMS_CALCULATION *rms,
        float cutOffFrequencyHz, Uint16 samplingPeriod10ns);

#endif /* CONTROL_LOGIC_H_ */
