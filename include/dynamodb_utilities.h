#ifndef _DYNAMODB_UTILITIES_H
#define _DYNAMODB_UTILITIES_H

// Functions to upload data to dynamo DB
#include <mhash.h>
//#include <stdio.h> /* printf, sprintf */
//#include <stdlib.h> /* exit */
//#include <unistd.h> /* read, write, close */
//#include <string.h> /* memcpy, memset */
//#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <uuid/uuid.h>
#include <defines.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <json-c/json.h>
#include <glib.h>

void *send_data_to_cloud_wrapper(void *args);
int recv_timeout(int s , int timeout, char *resp);
int send_http_request_to_dynamodb(char *http_request, int http_request_len, char *http_response, int *http_response_len);
int put_dynamodb_http_request(char *payload, int payload_len, char *http_request, int *http_request_len, aws_request_num num);
int get_def_from_cloud(bridge *data);
int get_sensors_def_from_single_core_module(bridge *data, int index);
char *get_alert_msg(data_code_def err);
int send_alert_to_cloud(bridge *data);
int get_core_module_def_from_cloud(bridge *data);
int send_sensor_data_to_cloud(bridge *data);
int free_defs(bridge *data);

#endif
