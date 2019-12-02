//#include <iostream>
#include "ADXL357.hpp"
#include "Logger.hpp"
#include "Sample.hpp"
//#include <iostream>
//#include <fstream>
//#include <ctime>
//#include <thread>
#include "stdio.h"


int main()
{
	udpSocket udpsocket(8080);
	//tcpSocket tcpsocket(8081);
	vector<Sample> samples;
	ADXL357 adxl357;
	//bool writeData = true;
	bool log = false;
	int time = 4;
	string dataPath = "../data/data_test.csv";

	//setup ADXL357 sensor
	adxl357.stop();
	adxl357.setRange(SET_RANGE_10G);
	adxl357.setFilter(SET_HPF_OFF, SET_ODR_4000);
	adxl357.dumpInfo();

	while (1)
	{
		//wait for message from python parent proc to start logging
		char buf[MAXLINE];
		fread(&buf, 1, 3, stdin);
		std::string message(buf);

		if(!message.compare("log"))
		{
			log = true;
			printf("logging");
		}

		if (log)
		{
			Logger logger(&adxl357);
			logger.log(samples, time, true);
		}
	}
}
