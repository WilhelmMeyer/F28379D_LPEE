/*
 * control_logic_util.h
 *
 *  Created on: 23 de out de 2023
 *      Author: willj
 */

#ifndef CONTROL_LOGIC_UTIL_H_
#define CONTROL_LOGIC_UTIL_H_

//
// Struct variables definitions
//

struct RMS_CALCULATION
{
    double sumSquare;
    double value;
    double sine;
    double coefFilter;
    Uint32 cutOffFrequencyHz;
};

#endif /* CONTROL_LOGIC_UTIL_H_ */
