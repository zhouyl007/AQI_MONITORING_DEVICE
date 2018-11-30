/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "cJSON_APP.h"

//#define malloc mymalloc
//#define free myfree
//#define realloc myrealloc

/* Used by some code below as an example datatype. */
struct record
{
    const char *precision;
    double lat;
    double lon;
    const char *address;
    const char *city;
    const char *state;
    const char *zip;
    const char *country;
};

typedef struct  
{  
    int id;  
    char firstName[32];  
    char lastName[32];  
    char email[64];  
    int age;  
    float height;  
}people; 


/* Create a bunch of objects as demonstration. */
static int print_preallocated(cJSON *root)
{
    /* declarations */
    char *out = NULL;
    char *buf = NULL;
    char *buf_fail = NULL;
    size_t len = 0;
    size_t len_fail = 0;

    /* formatted print */
    out = cJSON_Print(root);

    /* create buffer to succeed */
    /* the extra 5 bytes are because of inaccuracies when reserving memory */
    len = strlen(out) + 5;
    buf = (char*)malloc(len);
    if (buf == NULL)
    {
        printf("Failed to allocate memory.\n");
		return NULL;
        //exit(1);
    }

    /* create buffer to fail */
    len_fail = strlen(out);
    buf_fail = (char*)malloc(len_fail);
    if (buf_fail == NULL)
    {
        printf("Failed to allocate memory.\n");
		return -1;
        //exit(1);
    }

    /* Print to buffer */
    if (!cJSON_PrintPreallocated(root, buf, (int)len, 1)) {
        printf("cJSON_PrintPreallocated failed!\n");
        if (strcmp(out, buf) != 0) {
            printf("cJSON_PrintPreallocated not the same as cJSON_Print!\n");
            printf("cJSON_Print result:\n%s\n", out);
            printf("cJSON_PrintPreallocated result:\n%s\n", buf);
        }
        free(out);
        free(buf_fail);
        free(buf);
        return -1;
    }

    /* success */
    printf("%s\n", buf);

    /* force it to fail */
    if (cJSON_PrintPreallocated(root, buf_fail, (int)len_fail, 1)) {
        printf("cJSON_PrintPreallocated failed to show error with insufficient memory!\n");
        printf("cJSON_Print result:\n%s\n", out);
        printf("cJSON_PrintPreallocated result:\n%s\n", buf_fail);
        free(out);
        free(buf_fail);
        free(buf);
        return -1;
    }

    free(out);
    free(buf_fail);
    free(buf);
	
    return 0;
}


/* Create a bunch of objects as demonstration. */
static int create_objects(void)
{
    /* declare a few. */
    cJSON *root = NULL;
    cJSON *fmt = NULL;
    cJSON *img = NULL;
    cJSON *thm = NULL;
    cJSON *fld = NULL;
    int i = 0;

    /* Our "days of the week" array: */
    const char *strings[7] =
    {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
    };
    /* Our matrix: */
    int numbers[3][3] =
    {
        {0, -1, 0},
        {1, 0, 0},
        {0 ,0, 1}
    };
    /* Our "gallery" item: */
    int ids[4] = { 116, 943, 234, 38793 };
    /* Our array of "records": */
    struct record fields[2] =
    {
        {
            "zip",
            37.7668,
            -1.223959e+2,
            "",
            "SAN FRANCISCO",
            "CA",
            "94107",
            "US"
        },
        {
            "zip",
            37.371991,
            -1.22026e+2,
            "",
            "SUNNYVALE",
            "CA",
            "94085",
            "US"
        }
    };
    volatile double zero = 0.0;

    /* Here we construct some JSON standards, from the JSON site. */

    /* Our "Video" datatype: */
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
    cJSON_AddItemToObject(root, "format", fmt = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt, "type", "rect");
    cJSON_AddNumberToObject(fmt, "width", 1920);
    cJSON_AddNumberToObject(fmt, "height", 1080);
    cJSON_AddFalseToObject (fmt, "interlace");
    cJSON_AddNumberToObject(fmt, "frame rate", 24);

    /* Print to text */
    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
		return -1;
        //exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);

    /* Our "days of the week" array: */
    root = cJSON_CreateStringArray(strings, 7);

    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
		return -1;
        //exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);

    /* Our matrix: */
    root = cJSON_CreateArray();
    for (i = 0; i < 3; i++)
    {
        cJSON_AddItemToArray(root, cJSON_CreateIntArray(numbers[i], 3));
    }

    /* cJSON_ReplaceItemInArray(root, 1, cJSON_CreateString("Replacement")); */

    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
		return -1;
        //exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);

    /* Our "gallery" item: */
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "Image", img = cJSON_CreateObject());
    cJSON_AddNumberToObject(img, "Width", 800);
    cJSON_AddNumberToObject(img, "Height", 600);
    cJSON_AddStringToObject(img, "Title", "View from 15th Floor");
    cJSON_AddItemToObject(img, "Thumbnail", thm = cJSON_CreateObject());
    cJSON_AddStringToObject(thm, "Url", "http:/*www.example.com/image/481989943");
    cJSON_AddNumberToObject(thm, "Height", 125);
    cJSON_AddStringToObject(thm, "Width", "100");
    cJSON_AddItemToObject(img, "IDs", cJSON_CreateIntArray(ids, 4));

    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
		return -1;
        //exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);

    /* Our array of "records": */
    root = cJSON_CreateArray();
    for (i = 0; i < 2; i++)
    {
        cJSON_AddItemToArray(root, fld = cJSON_CreateObject());
        cJSON_AddStringToObject(fld, "precision", fields[i].precision);
        cJSON_AddNumberToObject(fld, "Latitude", fields[i].lat);
        cJSON_AddNumberToObject(fld, "Longitude", fields[i].lon);
        cJSON_AddStringToObject(fld, "Address", fields[i].address);
        cJSON_AddStringToObject(fld, "City", fields[i].city);
        cJSON_AddStringToObject(fld, "State", fields[i].state);
        cJSON_AddStringToObject(fld, "Zip", fields[i].zip);
        cJSON_AddStringToObject(fld, "Country", fields[i].country);
    }

    /* cJSON_ReplaceItemInObject(cJSON_GetArrayItem(root, 1), "City", cJSON_CreateIntArray(ids, 4)); */

    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
		return -1;
        //exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "number", 1.0 / zero);

    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
		return -1;
        //exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);

	return 0;
}

int create_post_login_objects(char *DeviceId,char *DeviceType,char *HashKey,char *login_body)
{
    cJSON *root = NULL;
	char *cjson_out = NULL;

	root = cJSON_CreateObject();
	
    cJSON_AddStringToObject(root, "DeviceId", DeviceId);
	cJSON_AddStringToObject(root, "DeviceType", DeviceType);
	cJSON_AddStringToObject(root, "HashKey", HashKey);

	/* formatted print */
    cjson_out = cJSON_Print(root);
	
	if(cjson_out != NULL)
		memcpy(login_body,cjson_out,strlen(cjson_out));
	else
		printf("cjson body creat error!\n");
	
    cJSON_Delete(root);
	free(cjson_out);

	return 0;
}

int create_ble_request_objects(char *Mac,char *IdCloud,char *Result,char *Detail,char *Version,char *ble_request)
{
    cJSON *root = NULL;
	char *cjson_out = NULL;

	root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "Mac", Mac);
	cJSON_AddStringToObject(root, "IdCloud", IdCloud);
    cJSON_AddStringToObject(root, "Result", Result);
	cJSON_AddStringToObject(root, "Detail", Detail);
	cJSON_AddStringToObject(root, "SW_Ver", Version);

	/* formatted print */
    cjson_out = cJSON_Print(root);
	
	if(cjson_out != NULL)
		memcpy(ble_request,cjson_out,strlen(cjson_out));
	else
		printf("cjson body creat error!\n");
	
    cJSON_Delete(root);
	free(cjson_out);

	return 0;
}

int create_post_body_objects(char *Mac, unsigned short pm25_val, unsigned short voc_val, unsigned short co2_val, unsigned char h2o_val, char *temp_val,
										  unsigned char bool_alert,	unsigned char err_code, char *sw_ver, char *sn_num, char *type, char *post_body)
{
    cJSON *root = NULL;
	char *cjson_out = NULL;

	root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "id", Mac);
	cJSON_AddNumberToObject(root, "pm25", pm25_val);
    cJSON_AddNumberToObject(root, "voc", voc_val);
	cJSON_AddNumberToObject(root, "co2", co2_val);
	cJSON_AddNumberToObject(root, "h2o", h2o_val);
	cJSON_AddStringToObject(root, "temp", temp_val);
	cJSON_AddNumberToObject(root, "alertmode", bool_alert);
	cJSON_AddNumberToObject(root, "error", err_code);
	cJSON_AddStringToObject(root, "swversion", sw_ver);
	cJSON_AddStringToObject(root, "serialnumber", sn_num);
	cJSON_AddStringToObject(root, "type", type);

	/* formatted print */
    cjson_out = cJSON_Print(root);
	
	if(cjson_out != NULL)
		memcpy(post_body,cjson_out,strlen(cjson_out));
	else
		printf("cjson body creat error!\n");
	
    cJSON_Delete(root);
	free(cjson_out);

	return 0;
}


//parse a key-value pair  
int cJSON_to_str(char *json_string, char *json_string_type,char *str_val)  
{  
    cJSON *root=cJSON_Parse(json_string);  
    if (!root)  {  
        printf("Error before: [%s]\n",cJSON_GetErrorPtr());  
        return -1;  
    }  
    else   {  
        cJSON *item=cJSON_GetObjectItem(root,json_string_type);  
        if(item!=NULL)  {  
            //printf("cJSON_GetObjectItem: type=%d, key is %s, value is %s\n",item->type,item->string,item->valuestring); 
            if(strlen(item->valuestring) <= 64)
                memcpy(str_val,item->valuestring,strlen(item->valuestring));
            else
                printf("cjson body is too long!\n");
        }  
        cJSON_Delete(root);  
    }  
    
    return 0;  
} 

//parse a object to struct  
int cJSON_to_struct(char *json_string, people *person)  
{  
    cJSON *item;  
    cJSON *root=cJSON_Parse(json_string);  
    if (!root)  
    {  
        printf("Error before: [%s]\n",cJSON_GetErrorPtr());  
        return -1;  
    }  
    else  
    {  
        cJSON *object=cJSON_GetObjectItem(root,"person");  
        if(object==NULL)  
        {  
            printf("Error before: [%s]\n",cJSON_GetErrorPtr());  
            cJSON_Delete(root);  
            return -1;  
        }  
        printf("cJSON_GetObjectItem: type=%d, key is %s, value is %s\n",object->type,object->string,object->valuestring);  
  
        if(object!=NULL)  
        {  
            item=cJSON_GetObjectItem(object,"firstName");  
            if(item!=NULL)  
            {  
                printf("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n",item->type,item->string,item->valuestring);  
                memcpy(person->firstName,item->valuestring,strlen(item->valuestring));  
            }  
  
            item=cJSON_GetObjectItem(object,"lastName");  
            if(item!=NULL)  
            {  
                printf("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n",item->type,item->string,item->valuestring);  
                memcpy(person->lastName,item->valuestring,strlen(item->valuestring));  
            }  
  
            item=cJSON_GetObjectItem(object,"email");  
            if(item!=NULL)  
            {  
                printf("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n",item->type,item->string,item->valuestring);  
                memcpy(person->email,item->valuestring,strlen(item->valuestring));  
            }  
  
            item=cJSON_GetObjectItem(object,"age");  
            if(item!=NULL)  
            {  
                printf("cJSON_GetObjectItem: type=%d, string is %s, valueint=%d\n",item->type,item->string,item->valueint);  
                person->age=item->valueint;  
            }  
            else  
            {  
                printf("cJSON_GetObjectItem: get age failed\n");  
            }  
  
            item=cJSON_GetObjectItem(object,"height");  
            if(item!=NULL)  
            {  
                printf("cJSON_GetObjectItem: type=%d, string is %s, valuedouble=%f\n",item->type,item->string,item->valuedouble);  
                person->height=item->valuedouble;  
            }  
        }  
  
        cJSON_Delete(root);  
    }  
    return 0;  
}  
  
//parse a struct array  
int cJSON_to_struct_array(char *text, people worker[])  
{  
    cJSON *json,*arrayItem,*item,*object;  
    int i;  
  
    json=cJSON_Parse(text);  
    if (!json)  
    {  
        printf("Error before: [%s]\n",cJSON_GetErrorPtr());  
    }  
    else  
    {  
        arrayItem=cJSON_GetObjectItem(json,"people");  
        if(arrayItem!=NULL)  
        {  
            int size=cJSON_GetArraySize(arrayItem);  
            printf("cJSON_GetArraySize: size=%d\n",size);  
  
            for(i=0;i<size;i++)  
            {  
                printf("i=%d\n",i);  
                object=cJSON_GetArrayItem(arrayItem,i);  
  
                item=cJSON_GetObjectItem(object,"firstName");  
                if(item!=NULL)  
                {  
                    printf("cJSON_GetObjectItem: type=%d, string is %s\n",item->type,item->string);  
                    memcpy(worker[i].firstName,item->valuestring,strlen(item->valuestring));  
                }  
  
                item=cJSON_GetObjectItem(object,"lastName");  
                if(item!=NULL)  
                {  
                    printf("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n",item->type,item->string,item->valuestring);  
                    memcpy(worker[i].lastName,item->valuestring,strlen(item->valuestring));  
                }  
  
                item=cJSON_GetObjectItem(object,"email");  
                if(item!=NULL)  
                {  
                    printf("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n",item->type,item->string,item->valuestring);  
                    memcpy(worker[i].email,item->valuestring,strlen(item->valuestring));  
                }  
  
                item=cJSON_GetObjectItem(object,"age");  
                if(item!=NULL)  
                {  
                    printf("cJSON_GetObjectItem: type=%d, string is %s, valueint=%d\n",item->type,item->string,item->valueint);  
                    worker[i].age=item->valueint;  
                }  
                else  
                {  
                    printf("cJSON_GetObjectItem: get age failed\n");  
                }  
  
                item=cJSON_GetObjectItem(object,"height");  
                if(item!=NULL)  
                {  
                    printf("cJSON_GetObjectItem: type=%d, string is %s, value=%f\n",item->type,item->string,item->valuedouble);  
                    worker[i].height=item->valuedouble;  
                }  
            }  
        }  
  
        for(i=0;i<3;i++)  
        {  
            printf("i=%d, firstName=%s,lastName=%s,email=%s,age=%d,height=%f\n",  
                    i,  
                    worker[i].firstName,  
                    worker[i].lastName,  
                    worker[i].email,  
                    worker[i].age,  
                    worker[i].height);  
        }  
  
        cJSON_Delete(json);  
    }  
    return 0;  
}  

int cJSON_to_struct_cloud(char *json_string, azure_cloud *cloud_data)  
{  
    cJSON *json,*item;  
  
    json = cJSON_Parse(json_string);
    
    if (!json) {  
        printf("Error before: [%s]\n",cJSON_GetErrorPtr());  
    }  
    else { 
        item = cJSON_GetObjectItem(json,"favoriteDisplay");  
        if(item != NULL) 
            cloud_data->favor = item->valueint;
        
        item = cJSON_GetObjectItem(json,"luminosity");  
        if(item != NULL)  
            cloud_data->lumi = item->valueint;
        
        item = cJSON_GetObjectItem(json,"iaq");  
        if(item != NULL)  
            cloud_data->iaq = (int)item->valuedouble;

        item = cJSON_GetObjectItem(json,"eaq");  
        if(item != NULL)  
            cloud_data->eaq = (int)item->valuedouble;

        item = cJSON_GetObjectItem(json,"pm25Eaq");  
        if(item != NULL)  
            cloud_data->pm25 = (int)item->valuedouble;
        
        item = cJSON_GetObjectItem(json,"pm10");  
        if(item != NULL)  
            cloud_data->pm10 = (int)item->valuedouble;

        item = cJSON_GetObjectItem(json,"no2");  
        if(item != NULL)  
            cloud_data->no2 = (int)item->valuedouble;

        item = cJSON_GetObjectItem(json,"o3");  
        if(item != NULL)  
            cloud_data->o3 = (int)item->valuedouble;
        
        cJSON_Delete(json);  
    }
    
    return 0;  
}  


int cjson_test(void)
{
	uint8_t Receive_byte = 0;  
	people worker[3]={ {0} };
	char Rx_Buffer[512] = {0};
	int Rx_Length = 0;
	
	memset(Rx_Buffer, 0, 512);
	
    /* print the version */
    printf("Version: %s\r\n", cJSON_Version());

	
    /* Now some samplecode for building objects concisely: */
    create_objects();  
      
//  char str_name[40];  
//  int ret = cJSON_to_str(data, str_name);  
  
//  people person;  
//  int ret = cJSON_to_struct(data, &person);  
   
    cJSON_to_struct_array(Rx_Buffer, worker);  
  
    return 0;
}

