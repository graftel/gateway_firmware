#include <iot_handler.h>

char pubCertDirectory[PATH_MAX + 1] = "/etc/ssl/certs";
char priCertDirectory[PATH_MAX + 1] = "/etc/ssl/private";

char HostAddress[255] = AWS_IOT_MQTT_HOST;
char rootCA[PATH_MAX + 1];
char clientCRT[PATH_MAX + 1];
char clientKey[PATH_MAX + 1];
uint32_t port = AWS_IOT_MQTT_PORT;
int iot_init_status = 0;

char cPayload[100];
char cTopic[256];
static AWS_IoT_Client client;
static IoT_Client_Init_Params mqttInitParams;
static IoT_Client_Connect_Params connectParams;
static IoT_Publish_Message_Params paramsQOS1;



void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
									IoT_Publish_Message_Params *params, void *pData) {
	IOT_UNUSED(pData);
	IOT_UNUSED(pClient);
	IOT_INFO("Subscribe callback");
	IOT_INFO("%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, params->payload);
}

void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data) {
	IOT_WARN("MQTT Disconnect");
	IoT_Error_t rc = FAILURE;

	if(NULL == pClient) {
		return;
	}

	IOT_UNUSED(data);

	if(aws_iot_is_autoreconnect_enabled(pClient)) {
		IOT_INFO("Auto Reconnect is enabled, Reconnecting attempt will start now");
	} else {
		IOT_WARN("Auto Reconnect not enabled. Starting manual reconnect...");
		rc = aws_iot_mqtt_attempt_reconnect(pClient);
		if(NETWORK_RECONNECTED == rc) {
			IOT_WARN("Manual Reconnect Successful");
		} else {
			IOT_WARN("Manual Reconnect Failed - %d", rc);
		}
	}
}

int init_iot()
{
    if (iot_init_status == 0)
    {

      IoT_Error_t rc = FAILURE;

      mqttInitParams = iotClientInitParamsDefault;
      connectParams = iotClientConnectParamsDefault;

      snprintf(rootCA, PATH_MAX + 1, "%s/%s", pubCertDirectory, AWS_IOT_ROOT_CA_FILENAME);
      snprintf(clientCRT, PATH_MAX + 1, "%s/%s", pubCertDirectory, AWS_IOT_CERTIFICATE_FILENAME);
      snprintf(clientKey, PATH_MAX + 1, "%s/%s", priCertDirectory, AWS_IOT_PRIVATE_KEY_FILENAME);

      IOT_DEBUG("rootCA %s", rootCA);
      IOT_DEBUG("clientCRT %s", clientCRT);
      IOT_DEBUG("clientKey %s", clientKey);

      mqttInitParams.enableAutoReconnect = false; // We enable this later below
      mqttInitParams.pHostURL = HostAddress;
      mqttInitParams.port = port;
      mqttInitParams.pRootCALocation = rootCA;
      mqttInitParams.pDeviceCertLocation = clientCRT;
      mqttInitParams.pDevicePrivateKeyLocation = clientKey;
      mqttInitParams.mqttCommandTimeout_ms = 20000;
      mqttInitParams.tlsHandshakeTimeout_ms = 5000;
      mqttInitParams.isSSLHostnameVerify = true;
      mqttInitParams.disconnectHandler = disconnectCallbackHandler;
      mqttInitParams.disconnectHandlerData = NULL;

      rc = aws_iot_mqtt_init(&client, &mqttInitParams);
      if(SUCCESS != rc) {
        IOT_ERROR("aws_iot_mqtt_init returned error : %d ", rc);
        return rc;
      }

      connectParams.keepAliveIntervalInSec = 10;
      connectParams.isCleanSession = true;
      connectParams.MQTTVersion = MQTT_3_1_1;
      connectParams.pClientID = AWS_IOT_MQTT_CLIENT_ID;
      connectParams.clientIDLen = (uint16_t) strlen(AWS_IOT_MQTT_CLIENT_ID);
      connectParams.isWillMsgPresent = false;

      IOT_INFO("Connecting...");
      rc = aws_iot_mqtt_connect(&client, &connectParams);
      if(SUCCESS != rc) {
        IOT_ERROR("Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
        return rc;
      }

      rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
    	if(SUCCESS != rc) {
    		IOT_ERROR("Unable to set Auto Reconnect to true - %d", rc);
    		return rc;
    	}

      paramsQOS1.qos = QOS1;
      paramsQOS1.payload = (void *) cPayload;
      paramsQOS1.isRetained = 0;

      sprintf(cTopic, "%s/RawData", BRIDGE_SERIAL_NUM);

      iot_init_status = 1;
      return rc;
    }
    else{
      return 0;
    }
}

int publish_data_to_iot_hub(data_set_def *data_set)
{
  IoT_Error_t rc = FAILURE;

  rc = init_iot();
  if (rc != SUCCESS)
  {
      return rc;
  }

	while (aws_iot_mqtt_yield(&client, 100) == NETWORK_ATTEMPTING_RECONNECT){};

  sprintf(cPayload, "{\"Time_Stamp\" : %d , \"Device_ID\" : \"%s\", \"Data\" : %f , \"Data_Code\" : %d}", data_set->timestamp, data_set->id, data_set->data, data_set->data_code);

  printf("cPayload=%s\n", cPayload);


  paramsQOS1.payloadLen = strlen(cPayload);

  rc = aws_iot_mqtt_publish(&client, cTopic, strlen(cTopic), &paramsQOS1);
  if (rc == MQTT_REQUEST_TIMEOUT_ERROR) {
    IOT_WARN("QOS1 publish ack not received.\n");
	}
	else{
		rc = SUCCESS;
	}


  if(SUCCESS != rc) {
    IOT_ERROR("An error occurred in the loop.\n");
  } else {
    IOT_INFO("Publish done\n");
  }

  return rc;

}
