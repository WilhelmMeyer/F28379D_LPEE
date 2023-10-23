/*
 * control_logic_util.c
 *
 *  Created on: 23 de out de 2023
 *      Author: willj
 */
#include "F28x_Project.h"
#include "control_logic.h"
#include <math.h>

//
// Defines
//

//
// Globals
//

//
//  Function Prototypes
//
double rmsCalculation(struct RMS_CALCULATION *rms, double newAcquisition)
{
    double sumSquare = newAcquisition * newAcquisition;

    rms->sumSquare = sumSquare + rms->coefFilter * (rms->sumSquare - sumSquare);

    rms->value = __sqrt(rms->sumSquare);

    return rms->value;
}

void configureRmsCalculation(struct RMS_CALCULATION *rms,
                             float cutOffFrequencyHz, Uint16 samplingPeriod10ns)
{
    rms->cutOffFrequencyHz = cutOffFrequencyHz;
    rms->coefFilter = exp(
            -__div2pif32(rms->cutOffFrequencyHz) * samplingPeriod10ns * 1e-8);
}
