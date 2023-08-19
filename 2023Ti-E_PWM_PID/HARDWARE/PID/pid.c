#include "pid.h"

float Kp_x=2,     //2
	  Ki_x=0.15,  //0.15
      Kd_x=2;    //2
float www,zzz;
int pwm_xpid(int xerror)
{
	int pid_ActualPwm;
	static float pid_Integral,pid_Voltage,error_Last;
	pid_Integral+=xerror;
	www=pid_Integral;
	if (pid_Integral<-6000) pid_Integral=-6000;
	if (pid_Integral>6000) pid_Integral=6000;
	pid_Voltage=Kp_x*xerror+Ki_x*pid_Integral+Kd_x*(xerror-error_Last);	
	error_Last=xerror;
	pid_ActualPwm=pid_Voltage*1;
	if (pid_ActualPwm<-1000) pid_ActualPwm=-1000;
	if (pid_ActualPwm>1000) pid_ActualPwm=1000;
	return pid_ActualPwm;
}


float Kp_y=1,     //1
	    Ki_y=0.15,  //0.15
      Kd_y=2;    //2

int pwm_ypid(int yerror)
{
	int pid_ActualPwm;
  static float pid_Integral,pid_Voltage,error_Last;
	pid_Integral+=yerror;
	zzz=pid_Integral;
	if (pid_Integral<-6000) pid_Integral=-6000;
	if (pid_Integral>6000) pid_Integral=6000;
	pid_Voltage=Kp_y*yerror+Ki_y*pid_Integral+Kd_y*(yerror-error_Last);	
	error_Last=yerror;
	pid_ActualPwm=pid_Voltage*1;
	if (pid_ActualPwm<-1000) pid_ActualPwm=-1000;
	if (pid_ActualPwm>1000) pid_ActualPwm=1000;
	return pid_ActualPwm;
}