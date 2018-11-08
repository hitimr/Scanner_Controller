#include "../Project.h"

///////////////////////////////////////////////////////////////////////////////
//
//	Settings Handler: if the command handler detected a settings-command it gets forwarded to the Get/Set_Settings_Handler
//	Just like the command_handler its a big switch statement that executes an action depenging on the command
//
///////////////////////////////////////////////////////////////////////////////

void Get_Settings_Handler();
void Set_Settings_Handler();
uint16_t Change(uint8_t, uint16_t);

static uint16_t testSetting;

#define SYSCLKDIV (ClkCfgRegs.SYSCLKDIVSEL.bit.PLLSYSCLKDIV+1)

