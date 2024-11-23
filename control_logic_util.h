/*
 * control_logic_util.h
 *
 *  Created on: 23 de out de 2023
 *      Author: willj
 */

#ifndef CONTROL_LOGIC_UTIL_H_
#define CONTROL_LOGIC_UTIL_H_

//
// PID configurations
//
#define PID_CONVENTIONAL 0
#define PID_CONVENTIONAL_ANTI_WINDUP 1
#define PID_CONVENTIONAL_ANTI_WINDUP_DERIVATIVE_FILTER 2
#define PID_CONVENTIONAL_ANTI_WINDUP_DERIVATIVE_FILTER_OUTPUT 3
#define PI_CONVENTIONAL 4
#define PI_CONVENTIONAL_ANTI_WINDUP 5
#define PID_INCREMENTAL 6
#define PID_INCREMENTAL_ANTI_WINDUP 7
#define PID_INCREMENTAL_ANTI_WINDUP_DERIVATIVE_FILTER 8
#define PID_INCREMENTAL_ANTI_WINDUP_DERIVATIVE_FILTER_OUTPUT 9
#define PI_INCREMENTAL 10
#define PI_INCREMENTAL_ANTI_WINDUP 11

//
// Controller operations
//
#define OPEN_LOOP 0
#define CLOSED_LOOP 1


//
// Struct variables definitions
//

struct RMS_CALCULATION
{
    double sumSquare;
    double value;
    double sine;
    double coefFilter;
    Uint16 semiCycle;
    Uint32 cutOffFrequencyHz;
};


struct ZERO_CROSSING_LEAD
{
    double sine;
    double sineDerivativeApproximation;
    double sineWithLead;
    Uint16 quadrant;
};


struct PID_VARIABLES
{
    Uint16 configuration;
    double period;
    double setpoint;

    double proportionalGainKp;

    double integrationTimeTi;
    double integrationGain;

    double derivativeTimeTd;
    double derivativeGain;
    double derivativeGainAux;

    double err0;
    double err1;
    double err2;

    double measuredProcessVariable0;
    double measuredProcessVariable1;
    double measuredProcessVariable2;

    double output0;
    double output1;
    double output2;

    double outputP0;

    double accumulatedIntegral;
    double outputI0;
    double outputI1;

    double outputD0;
    double outputD1;
};

#endif /* CONTROL_LOGIC_UTIL_H_ */
