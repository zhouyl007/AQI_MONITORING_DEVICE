#ifndef _CJSON_APP_H_
#define _CJSON_APP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "includes.h"

int create_post_login_objects(char *DeviceId,char *DeviceType,char *HashKey,char *login_body);
int cJSON_to_str(char *json_string, char *json_string_type,char *str_val);
int create_post_body_objects(char *Mac, unsigned short pm25_val, unsigned short voc_val, unsigned short co2_val, unsigned char h2o_val, char *temp_val,
										  unsigned char bool_alert,	unsigned char err_code, char *sw_ver, char *sn_num, char *type, char *post_body);

int cJSON_to_struct_cloud(char *json_string, azure_cloud *cloud_data);

#endif
       
