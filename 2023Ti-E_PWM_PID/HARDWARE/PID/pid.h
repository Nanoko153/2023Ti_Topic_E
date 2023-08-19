#ifndef _PID_H
#define _PID_H
#include "sys.h"
int pwm_xpid(int xerror);
int pwm_ypid(int yerror);
#endif
