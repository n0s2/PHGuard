#include "utils.h"
#include "_driver.h"

NTSTATUS t_antiDebug();
NTSTATUS t_chkProc();

extern BOOLEAN t_antiDebug_ExitCode;
extern KEVENT t_antiDebug_event;