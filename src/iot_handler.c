#include <iot_handler.h>
#include <defines.h>
#include <utilities.h>

#define MAX_LENGTH_OF_UPDATE_JSON_BUFFER 65535

char pubCertDirectory[PATH_MAX + 1] = "/etc/ssl/certs";
char priCertDirectory[PATH_MAX + 1] = "/etc/ssl/private";

char HostAddress[255] = AWS_IOT_MQTT_HOST;
char rootCA[PATH_MAX + 1];
char clientCRT[PATH_MAX + 1];
char clientKey[PATH_MAX + 1];
uint32_t port = AWS_IOT_MQTT_PORT;
int iot_init_status = 0;

char tmpBuf[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
char cPayload[100];
char cTopic[256];
static AWS_IoT_Client client;
static IoT_Client_Init_Params mqttInitParams;
static IoT_Client_Connect_Params connectParams;
static IoT_Publish_Message_Params paramsQOS1;
char test[100];

bridge *bridge_data;

void *device_shadow_handler(void *args)
{
			bridge_data = (bridge *)args;

			start_device_shadow(bridge_data);

			pthread_exit( NULL );
}

void ShadowUpdateStatusCallback(const char *pThingName, ShadowActions_t action, Shadow_Ack_Status_t status,
								const char *pReceivedJsonDocument, void *pContextData) {
	IOT_UNUSED(pThingName);
	IOT_UNUSED(action);
	IOT_UNUSED(pReceivedJsonDocument);
	IOT_UNUSED(pContextData);

	if(SHADOW_ACK_TIMEOUT == status) {
		IOT_INFO("Update Timeout--");
	} else if(SHADOW_ACK_REJECTED == status) {
		IOT_INFO("Update RejectedXX");
	} else if(SHADOW_ACK_ACCEPTED == status) {
		IOT_INFO("Update Accepted !!");
	}
}

void daq_interval_Callback(const char *pJsonString, uint32_t JsonStringDataLen, jsonStruct_t *pContext) {
	IoT_Error_t rc = FAILURE;

	IOT_INFO("new Data=%.*s", JsonStringDataLen, pJsonString);

	char tmpStr[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
	char tmpStr1[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
	sprintf(tmpStr, "{\"DAQInterval\":%.*s}", JsonStringDataLen, pJsonString);

	IOT_INFO("DAQ_INTERVAL Updated to %d", bridge_data->daq_interval);

	if (update_module(bridge_data, tmpStr, strlen(tmpStr)) != 0)
	{
		IOT_WARN("Update module failed");
	}

 	if (update_config_file(bridge_data) != 0)
	{
		IOT_WARN("update config file failed");
	}

	char tempClientTokenBuffer[MAX_SIZE_CLIENT_TOKEN_CLIENT_SEQUENCE];

	if(aws_iot_fill_with_client_token(tempClientTokenBuffer, MAX_SIZE_CLIENT_TOKEN_CLIENT_SEQUENCE) != SUCCESS){
			IOT_WARN("client token failed");
	}

	sprintf(tmpStr1, "{\"state\":{\"reported\":%.*s}, \"clientToken\":\"%s\"}", strlen(tmpStr), tmpStr, tempClientTokenBuffer);

	rc = aws_iot_shadow_update(&client, AWS_IOT_MY_THING_NAME, tmpStr1, ShadowUpdateStatusCallback, NULL, 4, true);
	if (rc != SUCCESS)
	{
		IOT_WARN("device shadow update failed");
	}

	if(pContext != NULL) {
	//	IOT_INFO("Delta - Window state changed to %s", (char *) (pContext->pData));
			IOT_INFO("Original Data=%s", (char *) (pContext->pData));
	}
}


void coreMoudules_Callback(const char *pJsonString, uint32_t JsonStringDataLen, jsonStruct_t *pContext) {
	IoT_Error_t rc = FAILURE;

	IOT_INFO("new Data=%.*s", JsonStringDataLen, pJsonString);

	char tmpStr[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
	char tmpStr1[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
	sprintf(tmpStr, "{\"CoreModules\":%.*s}", JsonStringDataLen, pJsonString);

	if (update_module(bridge_data, tmpStr, strlen(tmpStr)) != 0)
	{
		IOT_WARN("Update module failed");
	}

 	if (update_config_file(bridge_data) != 0)
	{
		IOT_WARN("update config file failed");
	}

	char tempClientTokenBuffer[MAX_SIZE_CLIENT_TOKEN_CLIENT_SEQUENCE];

	if(aws_iot_fill_with_client_token(tempClientTokenBuffer, MAX_SIZE_CLIENT_TOKEN_CLIENT_SEQUENCE) != SUCCESS){
			IOT_WARN("client token failed");
	}

	sprintf(tmpStr1, "{\"state\":{\"reported\":%.*s}, \"clientToken\":\"%s\"}", strlen(tmpStr), tmpStr, tempClientTokenBuffer);

	rc = aws_iot_shadow_update(&client, AWS_IOT_MY_THING_NAME, tmpStr1, ShadowUpdateStatusCallback, NULL, 4, true);
	if (rc != SUCCESS)
	{
		IOT_WARN("device shadow update failed");
	}

	if(pContext != NULL) {
	//	IOT_INFO("Delta - Window state changed to %s", (char *) (pContext->pData));
			IOT_INFO("Original Data=%s", (char *) (pContext->pData));
	}
}

void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
									IoT_Publish_Message_Params *params, void *pData) {
	IOT_UNUSED(pData);
	IOT_UNUSED(pClient);
	IOT_INFO("Subscribe callback");
	IOT_INFO("%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *)params->payload);
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

int init_iot(bridge *bridge_data)
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

		connectParams.keepAliveIntervalInSec = 120;
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

		strcpy(cTopic, "HXmonitor/RawData");

		ShadowInitParameters_t sp = ShadowInitParametersDefault;
		sp.pHost = AWS_IOT_MQTT_HOST;
		sp.port = AWS_IOT_MQTT_PORT;
		sp.pClientCRT = clientCRT;
		sp.pClientKey = clientKey;
		sp.pRootCA = rootCA;
		sp.enableAutoReconnect = false;
		sp.disconnectHandler = NULL;

		IOT_INFO("Shadow Init");
		rc = aws_iot_shadow_init(&client, &sp);
		if(SUCCESS != rc) {
			IOT_ERROR("Shadow Connection Error");
			return rc;
		}

		ShadowConnectParameters_t scp = ShadowConnectParametersDefault;
		scp.pMyThingName = AWS_IOT_MY_THING_NAME;
		scp.pMqttClientId = AWS_IOT_MQTT_CLIENT_ID;
		scp.mqttClientIdLen = (uint16_t) strlen(AWS_IOT_MQTT_CLIENT_ID);

		IOT_INFO("Shadow Connect");
		rc = aws_iot_shadow_connect(&client, &scp);
		if(SUCCESS != rc) {
			IOT_ERROR("Shadow Connection Error");
			return rc;
		}

		rc = aws_iot_shadow_set_autoreconnect_status(&client, true);
		if(SUCCESS != rc) {
			IOT_ERROR("Unable to set Auto Reconnect to true - %d", rc);
			return rc;
		}

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

  // rc = init_iot();
  // if (rc != SUCCESS)
  // {
      // return rc;
  // }

	while (aws_iot_mqtt_yield(&client, 100) == NETWORK_ATTEMPTING_RECONNECT){};

  sprintf(cPayload, "{\"EpochTimeStamp\" : %d , \"DeviceID\" : \"%s\", \"Value\" : %f , \"DataCode\" : %d}", data_set->timestamp, data_set->id, data_set->data, data_set->data_code);

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

int put_core_module_into_json_string(bridge *bridge_data)
{

	int i,j;

	json_object *jobj_root = json_object_new_object();

	json_object *jarr1 = json_object_new_array();

	for (i = 0; i < bridge_data->size_cm; i++)
	{
			json_object *jcm = json_object_new_object();

			json_object *jcm_value1 = json_object_new_string(bridge_data->cm[i].addr);
			json_object_object_add(jcm,"DeviceID",jcm_value1);

			json_object *jcm_value2 = json_object_new_string(bridge_data->cm[i].ble_addr);
			json_object_object_add(jcm,"Bluetooth_Address",jcm_value2);

			json_object *jcm_value3 = json_object_new_string(bridge_data->cm[i].protocol);
			json_object_object_add(jcm,"Protocol",jcm_value3);

			json_object *jsen_arr = json_object_new_array();

			for (j = 0; j < bridge_data->cm[i].size_sen; j++)
			{
					json_object *jsen_addr = json_object_new_string(bridge_data->cm[i].sen[j].addr);
					json_object_array_add(jsen_arr, jsen_addr);
			}
			json_object_object_add(jcm,"Sensors",jsen_arr);

			json_object_array_add(jarr1, jcm);
	}

	json_object_object_add(jobj_root,"CoreModules",jarr1);

	sprintf(tmpBuf, "%s",json_object_to_json_string(jarr1));

	DEBUG_PRINT("output_data:%s\n",tmpBuf);

	json_object_put(jobj_root);

	return 0;
}

void start_device_shadow(bridge *bridge_data)
{
	int rc = SUCCESS;

	char JsonDocumentBuffer[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
	size_t sizeOfJsonDocumentBuffer = sizeof(JsonDocumentBuffer) / sizeof(JsonDocumentBuffer[0]);


	jsonStruct_t deviceIDHandler;
	deviceIDHandler.cb = NULL;
	deviceIDHandler.pKey = CONFIG_DEVICE_ID;
	deviceIDHandler.pData = bridge_data->addr;
	deviceIDHandler.type = SHADOW_JSON_STRING;

	put_core_module_into_json_string(bridge_data);

	jsonStruct_t coreModulesHandler;
	coreModulesHandler.cb = coreMoudules_Callback;
	coreModulesHandler.pKey = CONFIG_CORE_MODULES;
	coreModulesHandler.pData = tmpBuf;
	coreModulesHandler.type = SHADOW_JSON_OBJECT;

	rc = aws_iot_shadow_register_delta(&client, &coreModulesHandler);

	if(SUCCESS != rc) {
		IOT_ERROR("Shadow Register Delta Error");
	}

	jsonStruct_t daqIntervalHandler;
	daqIntervalHandler.cb = daq_interval_Callback;
	daqIntervalHandler.pKey = CONFIG_DAQ_INTERVAL;
	daqIntervalHandler.pData = &bridge_data->daq_interval;
	daqIntervalHandler.type = SHADOW_JSON_UINT16;

	rc = aws_iot_shadow_register_delta(&client, &daqIntervalHandler);

	if(SUCCESS != rc) {
		IOT_ERROR("Shadow Register Delta Error");
	}

	jsonStruct_t configVersionHandler;
	configVersionHandler.cb = NULL;
	configVersionHandler.pKey = CONFIG_VERSION;
	configVersionHandler.pData = &bridge_data->config_version;
	configVersionHandler.type = SHADOW_JSON_UINT16;

	while (aws_iot_mqtt_yield(&client, 100) == NETWORK_ATTEMPTING_RECONNECT){};

	rc = aws_iot_shadow_init_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
	if(SUCCESS == rc) {
		rc = aws_iot_shadow_add_reported(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, 4, &deviceIDHandler,&daqIntervalHandler,&configVersionHandler,&coreModulesHandler);
		if(SUCCESS == rc) {
			rc = aws_iot_finalize_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
			if(SUCCESS == rc) {
				IOT_INFO("Update Shadow: %s", JsonDocumentBuffer);
				rc = aws_iot_shadow_update(&client, AWS_IOT_MY_THING_NAME, JsonDocumentBuffer,
										   ShadowUpdateStatusCallback, NULL, 4, true);
				printf("rc=%d\n",rc);
			}
		} else {
			printf("rc=%d\n", rc);
		}
	}

	while(1)
	{
		while (aws_iot_mqtt_yield(&client, 200) == NETWORK_ATTEMPTING_RECONNECT){};
		sleep(1);
	}
}
