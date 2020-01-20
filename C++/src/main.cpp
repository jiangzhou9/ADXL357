#include <string>
#include <chrono>
#include <unistd.h>
#include <stdio.h>
#include <wiringPi.h>
#include "ADXL357.hpp"
#include "Logger.hpp"
#include "Sample.hpp"
#include "Sender.hpp"

#define MQTT_BROKER_ADDR "tcp://localhost:1883"
#define MQTT_CLIENT_ID	 ""
#define MQTT_QOS				 0
#define MQTT_VER				 MQTTVERSION_3_1_1

void setupGPIO(vector<int> inputs, vector<int> outputs);
bool read_btn(int btnPin);
string buildPayload(vector<Sample> samples, string name, double rate, int range, string timeStamp, double sensitivityFactor);

using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[])
{
	//create vector to save the samples into
	vector<Sample> samples;
	ADXL357 adxl357;
	bool logged = false;
	bool connected = false;
	const double time = 0.005;
	const int btn_pin = 8;

	// Setup the GPIO wiring pi lib, pass btn_pin in as a input
	setupGPIO({btn_pin}, {});

	Sender sender(connected, MQTT_BROKER_ADDR, MQTT_CLIENT_ID, MQTT_QOS, MQTT_VER);
	if(!connected)
	{
		cout << "Warning!! Not connected to the MQTT broker please restart to send data!" << endl;
	}

	//setup ADXL357 sensor
	adxl357.stop();
	adxl357.setRange(SET_RANGE_10G);
	adxl357.setFilter(SET_HPF_OFF, SET_ODR_4000);
	adxl357.dumpInfo();
	double rate = adxl357.getRate();

	while (1)
	{
		Logger logger(&adxl357);
		char tmbuf[32];

		if (!read_btn(btn_pin))
		{
			time_t t = system_clock::to_time_t(system_clock::now());
			strftime(tmbuf, sizeof(tmbuf), "%F %T", localtime(&t));
			//be sure to start the sensor before logging Continuous to avoid starting and stopping the sensor
			adxl357.start();

			//log Continuous, the time parameter determines the polling interval
			while (!digitalRead(btn_pin))
			{
				logger.logContinuous(samples, rate, time, false);
				printf("\rLogging ---> %6d", samples.size());
				fflush(stdout);
			}
			//its now safe to put the sensor back in standby mode
			adxl357.stop();
			logged = true;
		}

		//send the logged samples over MQTT protocol (JSON Format)
		if (logged && connected)
		{
			cout << "\nsending data..." << flush;

			std::string payload = buildPayload(samples, "ADXL357", rate, adxl357.get_range(), tmbuf, adxl357.getSensitivityFactor());
			//Publish to the topic
			sender.send(payload, "ADXL357");
			cout << "OK" << endl;
			samples.clear();
			logged = false;
		}
	}
	return 0;
}



string buildPayload(vector<Sample> samples, string sensorName, double rate, int range, string timeStamp, double sensitivityFactor)
{
			std::string sensor = "\"" + sensorName + "\"";
			std::string sfreq = to_string(rate);
			std::string srange = to_string(range);
			std::string nSamples = to_string(samples.size());
			std::string date = std::string("\"") + timeStamp + std::string("\"");

			std::string xSamples = "[";
			std::string ySamples = "[";
			std::string zSamples = "[";
			for (auto &sample : samples)
			{
				sample.convertSample(sensitivityFactor);
				xSamples += to_string(sample.getX()) + ",";
				ySamples += to_string(sample.getY()) + ",";
				zSamples += to_string(sample.getZ()) + ",";
			}
			xSamples.pop_back();
			ySamples.pop_back();
			zSamples.pop_back();
			xSamples += "]";
			ySamples += "]";
			zSamples += "]";

			return "{ \"Sensor\" : " + sensor + ", \"Frequency\" : " + sfreq + ", \"Range\" : " + srange + ", \"Time_stamp\" : " + date + ", \"NumberSamples\" : " + nSamples + ", \"xSamples\" : " + xSamples + ", \"ySamples\" : " + ySamples + ", \"zSamples\" : " + zSamples + "}";
}


void setupGPIO(vector<int> inputs, vector<int> outputs)
{
	wiringPiSetup();

	for(const auto& input : inputs)
	{
		pinMode(input, INPUT);
		pullUpDnControl(input, PUD_UP);
	}

	for(const auto& output : outputs)
	{
		pinMode(output, OUTPUT);
		pullUpDnControl(output, PUD_UP);
	}
}

bool read_btn(int btnPin)
{
	//debounce pin
	if (!digitalRead(btnPin) || digitalRead(btnPin))
	{
		usleep(1);
		return digitalRead(btnPin);
	}
	//should never reach this part
	return true;
}
