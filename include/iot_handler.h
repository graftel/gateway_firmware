#ifndef _IOT_HANDLER_H
#define _IOT_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <defines.h>
#include <json-c/json.h>
#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_shadow_interface.h"


int publish_data_to_iot_hub(data_set_def *data_set);
int init_iot(bridge *bridge_data);
void start_device_shadow(bridge *bridge_data);
void *device_shadow_handler(void *args);

#endif
