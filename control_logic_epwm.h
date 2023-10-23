/*
 * control_logic_epwm.h
 *
 *  Created on: 14 de out de 2023
 *      Author: willj
 */

#ifndef CONTROL_LOGIC_EPWM_H_
#define CONTROL_LOGIC_EPWM_H_

//
// EPWM OPERATION
//
#define EPWM_DISABLE 0
#define EPWM_ENABLE 1

//
// EPWM CONFIGURATION
//
#define EPWM_ALWAYS_OFF 1
#define EPWM_ALWAYS_ON 2
#define EPWM_SC_PFM 3
#define EPWM_SC_PFM_SUPERPOSITION 4
#define EPWM_SC_PFM_DEADBAND 5


//
// EPWM DEADBAND OUT_SWAP
//
#define DB_OUTSWAP_AB 0x0   // out swap off
#define DB_OUTSWAP_BB 0x1   // both outputs as EPWMxB
#define DB_OUTSWAP_AA 0x2   // both outputs as EPWMxA
#define DB_OUTSWAP_BA 0x3   // inverse outputs


//
// EPWM Module
//
#define EPWM1_MODULE 0
#define EPWM2_MODULE 1
#define EPWM3_MODULE 2
#define EPWM4_MODULE 3
#define EPWM5_MODULE 4
#define EPWM6_MODULE 5
#define EPWM7_MODULE 6
#define EPWM8_MODULE 7
#define EPWM9_MODULE 8
#define EPWM10_MODULE 9
#define EPWM11_MODULE 10
#define EPWM12_MODULE 11
#define EPWM13_MODULE 12


//
// EPWMXLINK makes a particular EPWM to follow other EPWM (such as TBPRD, CMPA, CMPB, etc)
//
#define EPWM1_LINK 0x0
#define EPWM2_LINK 0x1
#define EPWM3_LINK 0x2
#define EPWM4_LINK 0x3
#define EPWM5_LINK 0x4
#define EPWM6_LINK 0x5
#define EPWM7_LINK 0x6
#define EPWM8_LINK 0x7
#define EPWM9_LINK 0x8
#define EPWM10_LINK 0x9
#define EPWM11_LINK 0xA
#define EPWM12_LINK 0xB

#endif /* CONTROL_LOGIC_EPWM_H_ */
