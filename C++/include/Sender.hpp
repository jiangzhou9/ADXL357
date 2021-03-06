#ifndef _SENDER_HPP_
#define _SENDER_HPP_

#include <string>
#include <mqtt/async_client.h>
#include <mqtt/message.h>
#include "mqtt/properties.h"
#include <iostream>

#define MAX_BUFFERED_MSGS 120

using namespace std;

class Sender
{
public:
	Sender(std::string address, std::string client_id, int qos, int mqtt_version = MQTTVERSION_3_1_1, std::string persist_dir = "data-persist");
	~Sender();

	void send(std::string payload, std::string topic);
	bool reConnect();
	bool connected();

private:
	std::string m_address;
	std::string m_persist_dir;
	std::string m_client_id;
	int m_qos;
	bool m_connected;

	mqtt::async_client *m_cli;
	mqtt::connect_options m_conn_opt;
};

#endif
