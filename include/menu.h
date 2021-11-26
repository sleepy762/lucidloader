#include "debug.h" 
#include "config.h" 
#include "chainloader.h"

void MainMenu();
void SuccssesMenu(boot_entry_s* head);
void FailMenu();
void Logger();
void ResetComputer();
void ShutDown();
boot_entry_s * GetCurrOS(uint8_t numOfPartition, boot_entry_s * head);
