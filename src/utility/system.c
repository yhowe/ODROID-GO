#include "system.h"
void taskusleep(int x)
{
	if (x < 1000)
		delayMicroseconds(x);
	else
		vTaskDelay(x / 1000);
}
