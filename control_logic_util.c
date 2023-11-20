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

    rms->sine = __divf32(newAcquisition * 1.4142135623731, rms->value);

    return rms->value;
}

void configureRmsCalculation(struct RMS_CALCULATION *rms,
                             float cutOffFrequencyHz, Uint16 samplingPeriod10ns)
{
    rms->cutOffFrequencyHz = cutOffFrequencyHz;
    rms->coefFilter = exp(
            -__div2pif32(rms->cutOffFrequencyHz) * samplingPeriod10ns * 1e-8);
}

void pidConfiguration(struct PID_VARIABLES *pid, Uint16 pidConfiguration,
                      Uint16 period10ns, double kp, double ti, double td,
                      Uint16 N)
{
    pid->configuration = pidConfiguration;
    pid->period10ns = period10ns;
    pid->proportionalGainKp = kp;

    pid->integrationTimeTi = ti;
    pid->integrationGain = kp * period10ns * 0.0000000005 / ti; // Tustin method

    pid->derivativeTimeTd = td;
    if (pidConfiguration == PID_CONVENTIONAL
            || pidConfiguration == PID_CONVENTIONAL_ANTI_WINDUP
            || pidConfiguration == PID_INCREMENTAL
            || pidConfiguration == PID_INCREMENTAL_ANTI_WINDUP)
    {
        pid->derivativeGain = kp * td / (period10ns * 0.000000001);
    }
    else
    {
        pid->derivativeGain = kp * N * td
                / (td + (N * period10ns * 0.000000001));
        pid->derivativeGainAux = td / (td + (N * period10ns * 0.000000001));
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

        double output = pid->output1 + pid->outputP0 + pid->outputI0
        + pid->outputD0;
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
