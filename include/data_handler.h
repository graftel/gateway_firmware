#ifndef _DATA_HANDLER_H
#define _DATA_HANDLER_H

#include <utilities.h>
#include <errno.h>
#include <localdb_utilities.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"

void *db_data_handler(void *args);

#endif
