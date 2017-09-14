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
#include <defines.h>
#define NO_INTERNET_CHECK_FREQUENCY 100  // every data points
#define LOCAL_DB_SYNC_TABLE_NAME "SYNC"

int recv_timeout(int s , int timeout, char *resp);
int send_http_request_to_dynamodb(char *http_request, int http_request_len, char *http_response, int *http_response_len);
int put_dynamodb_http_request(char *payload, int payload_len, char *http_request, int *http_request_len, aws_request_num num);


void *db_data_handler(void *args);
void *sync_data_with_cloud_wrapper(void *args);

#endif
