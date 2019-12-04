//#include <iostream>
#include "ADXL357.hpp"
#include "Logger.hpp"
#include "Sample.hpp"
#include "stdio.h"
//#include <mqtt/client.h>
#include <wiringPi.h>
#include <unistd.h>

#define btn_pin 8

void setupGPIO()
{
		wiringPiSetup();

		pinMode(btn_pin, INPUT);
		pullUpDnControl(btn_pin, PUD_UP);
}

bool read_btn(int btnPin)
{
	//debounce pin
	if(!digitalRead(btn_pin))
	{
		usleep(1);
		return !digitalRead(btn_pin)
	}

	if(digitalRead(btn_pin))
	{
		usleep(1);
		return digitalRead(btn_pin)
	}
}


int main()
{
	setupGPIO();
	vector<Sample> samples;
	ADXL357 adxl357;
	bool logged = false;
	double time = 0.1;

	//setup ADXL357 sensor
	adxl357.stop();
	adxl357.setRange(SET_RANGE_10G);
	adxl357.setFilter(SET_HPF_OFF, SET_ODR_4000);
	adxl357.dumpInfo();

	while (1)
	{
		Logger logger(&adxl357);

		if(!read_btn(btn_pin))
		{
			samples.clear();

			while(!digitalRead(btn_pin))
			{
				logger.log(samples, time, true, true);
				printf("\rLogging ---> %6d", samples.size());
        fflush(stdout);
			}

			logged = true;
		}

		if(logged)
		{
			printf("\nDone! logged %7d samples\n", samples.size());
			logged = false;
		}
	}
}
