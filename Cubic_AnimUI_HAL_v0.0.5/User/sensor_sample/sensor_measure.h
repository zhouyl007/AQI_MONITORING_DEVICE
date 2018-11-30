#ifndef __SENSOR_MEASURE_H
#define	__SENSOR_MEASURE_H

#define	CALIBRATION_EN	    0
#define	PM2008_CTR_EN	    0
#define	PM2005_CTR_EN	    1
#define	CO2_SENSOR_EN	    1
#define FILTER_BUFF_LEN     5
#define	CO2_DEBUG		    0
#define	PM2005_DEBUG	    0
#define RS485_DEBUG   	    0

#define first_raw     ("first raw value")
#define first_cali    ("first cali value")
#define second_raw    ("second raw value")
#define second_cali   ("second cali value")
#define first_coef    ("first coef")
#define second_coef   ("second coef")
#define temp_diff     ("temperature diff")
#define serial_num    ("serial number")

#define RS485_SendBuf       (usb_Transmit)

typedef enum{
	VOC 	    = 0,		
	CO2 	    = 1,		
	PM2005 	    = 2,
	PM2008,
	H2O,
	TEMP,
	IP_NUM_SET,
	SEN_VALUE,
	NUM_ACK,
	SET_SN,
	IDLE,
}DATA_CAL_TYPE;

/* sensor calibration datas */
typedef struct{
	int  voc_cali_data;
    int  temp_cail_data;
	unsigned int co2_cali_data;			
	unsigned int h20_cali_data;	
	unsigned int pm2005_cali_data;	 
    #if 0
	unsigned int pm2008_1_0_data;
	unsigned int pm2008_2_5_data;
	unsigned int pm2008_10_data;
	unsigned int pm2008_0_3_par;
	unsigned int pm2008_0_5_par;
	unsigned int pm2008_1_0_par;
	unsigned int pm2008_2_5_par;
	unsigned int pm2008_5_0_par;
	unsigned int pm2008_10_par;
	unsigned char pm2008_open_flag;
	unsigned char pm2008_read_flag;
    #endif
	unsigned int ip;
	unsigned int num;
    unsigned char sn_wr_flag;
}SENSOR_DATA;

typedef union{
     unsigned char cali_ch[4];
     float cali_f;
}UNION_CALI_COEF;

typedef union{
		unsigned char cali_data_ch[4];
		unsigned long cali_data;
}UNION_CALI_DATA;

typedef struct{
	unsigned int first_raw_value;
    unsigned int first_cali_value;
	unsigned int second_raw_value;
	unsigned int second_cali_value;
}PM25_Cali_Data;

extern SENSOR_DATA  sensor_cali_data;

void sensor_measure(void);
void dev_calibration(void);
extern int CharChangetoHex(char *str, int length);

#endif /* __SENSOR_MEASURE_H */

