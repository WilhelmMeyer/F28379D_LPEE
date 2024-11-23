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

    rms->sine = __divf32(newAcquisition * 0.707106781, rms->value);

    if (rms->sine > 0)
    {
        rms->semiCycle = 1;
    }
    else
    {
        rms->semiCycle = 0;
    }

    return rms->value;
}

void configureRmsCalculation(struct RMS_CALCULATION *rms,
                             Uint32 cutOffFrequencyHz,
                             Uint32 samplingFrequencyHz)
{
    rms->cutOffFrequencyHz = cutOffFrequencyHz;
    rms->coefFilter = exp(-__div2pif32(rms->cutOffFrequencyHz) * frequencyToPeriod(samplingFrequencyHz));
}

void pidConfiguration(struct PID_VARIABLES *pid,
                                Uint16 pidConfiguration, double frequencyHz,
                                double kp, double ti, double td, Uint16 N)
{
    pid->configuration = pidConfiguration;
    pid->period = frequencyToPeriod(frequencyHz);
    pid->proportionalGainKp = kp;

    pid->integrationTimeTi = ti;
    pid->integrationGain = kp * pid->period / (2* ti); // Tustin method KpTs/2Ti

    pid->derivativeTimeTd = td;
    if (pidConfiguration == PID_CONVENTIONAL
            || pidConfiguration == PID_CONVENTIONAL_ANTI_WINDUP
            || pidConfiguration == PID_INCREMENTAL
            || pidConfiguration == PID_INCREMENTAL_ANTI_WINDUP)
    {
        pid->derivativeGain = kp * td / pid->period;
    }
    else
    {
        pid->derivativeGain = kp * N * td
                / (td + (N * pid->period));
        pid->derivativeGainAux = td / (td + (N * pid->period));
    }
}

double updatePidControllerOutput(struct PID_VARIABLES *pid, Uint16 pidOperation,
                                 double openLoopOutput, double setpoint,
                                 double measuredProcessVariable,
                                 double upperSaturationLimit,
                                 double lowerSaturationLimit)
{
    pid->setpoint = setpoint;

    pid->measuredProcessVariable2 = pid->measuredProcessVariable1;
    pid->measuredProcessVariable1 = pid->measuredProcessVariable0;
    pid->measuredProcessVariable0 = measuredProcessVariable;

    pid->err2 = pid->err1;
    pid->err1 = pid->err0;
    pid->err0 = setpoint - measuredProcessVariable;

    if (pidOperation == OPEN_LOOP)
    {
        pid->output0 = openLoopOutput;
    }
    else if (pid->configuration == PI_INCREMENTAL
            || pid->configuration == PI_INCREMENTAL_ANTI_WINDUP)
    {
        pid->outputP0 = pid->proportionalGainKp * (pid->err0 - pid->err1);

        pid->outputI1 = pid->outputI0;
        pid->outputI0 = pid->integrationGain * (pid->err0 + pid->err1);

        pid->outputD1 = pid->outputD0;
        pid->outputD0 = 0;

        pid->output1 = pid->output0;

        //double output = pid->output1 + pid->outputP0 + pid->outputI0
        //        + pid->outputD0;
        double parcialOutput = pid->output1 + pid->outputP0 + pid->outputD0;
        pid->accumulatedIntegral += pid->outputI0;
        double output = parcialOutput + pid->accumulatedIntegral;
        pid->accumulatedIntegral = (parcialOutput - output)
                + pid->accumulatedIntegral;

        if (output > upperSaturationLimit)
        {
            output = upperSaturationLimit;
        }
        else if (output < lowerSaturationLimit)
        {
            output = lowerSaturationLimit;
        }

        pid->output0 = output;
    }

    return pid->output0;
}
