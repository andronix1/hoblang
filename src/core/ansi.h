#pragma once

#define ANSI(CMD) "\033[" #CMD "m"

#define ANSI_RESET ANSI(0)
#define ANSI_RED ANSI(31)
#define ANSI_GRAY ANSI(90)
