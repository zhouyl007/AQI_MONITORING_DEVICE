#ifndef _TOUCH_KEY_H
#define _TOUCH_KEY_H

typedef enum {

 KEY_STATE_0    = 0,      		
 KEY_STATE_1,             
 KEY_STATE_2,             
 KEY_STATE_3,             
 KEY_STATE_4,
 KEY_STATE_5,
 RST_KEY_STATE_0,         
 RST_KEY_STATE_1,         
 RST_KEY_STATE_2,         
 RST_KEY_STATE_3,         

}KEY_STATE_t;

/* TOUCH KEY */
#define LONG_KEY_TIME       600     		// LONG_KEY_TIME*10MS = 10S
#define SINGLE_KEY_TIME     3        		// SINGLE_KEY_TIME*10MS = 30MS
#define MIDDLE_KEY_TIME     200        		// SINGLE_KEY_TIME*10MS = 2S
#define KEY_INTERVAL 		50 		 		// KEY_INTERVAL*10MS = 500MS 判定双击的时间间隔
#define N_KEY    			0               // no click
#define S_KEY    			1               // single click
#define L_KEY    			10              // long press
#define KEY_DOUBLE 			2     			// double click
#define M_KEY 			    3     			// 3 - 5s hold click

/* RESET KEY */
#define RST_KEY_SHORT_TIME 		200				// 2s 
#define RST_KEY_LONG_TIME 		1000			// 10s
#define RST_S_KEY    			11              // single click
#define RST_L_KEY    			12              // long press

unsigned char key_driver(void);
unsigned char key_read(void);
unsigned char reset_key_driver(void);

#endif
