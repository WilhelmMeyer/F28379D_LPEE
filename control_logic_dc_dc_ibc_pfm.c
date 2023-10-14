//
// Included Files
//
#include "F28x_Project.h"
#include "control_logic.h"

//
// Defines
//

//
// Globals
//
struct IBC_PFM_VARIABLES dcdcIbcPfmVariables;

//
//  Function Prototypes
//
interrupt void aquisitionIsr(void);

//
// Main
//
void main(void)
{

    initializeDsp();

    //
    // Initialize global variables
    //
    dcdcIbcPfmVariables.isrPointer = &aquisitionIsr;
    dcdcIbcPfmVariables.aquisition[0].adcChannel = ADCA1_CHANNEL;
    dcdcIbcPfmVariables.epwmEnable = EPWM1_ENABLE;

    initializeEpwms(&dcdcIbcPfmVariables);

    initializeInterrupts();

    configureDcDcIbcPfm(&dcdcIbcPfmVariables);

//
// IDLE loop. Just sit and loop forever (optional):
//
    for (;;)
    {
        asm ("  NOP");
    }
}

//
// interruption function at every aquisition
//
void aquisitionIsr(void)
{

}
