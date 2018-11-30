#ifndef _I2C_SIMULATION_H
#define _I2C_SIMULATION_H

#include  "bsp.h"

#ifndef uchar
	#define uchar unsigned char
#endif
#ifndef uint
	#define uint  unsigned int
#endif
           
	
/****************** API Interface ************************/
extern void I2CSendByte(unsigned char SendDat);
extern void I2CSendByte_S(unsigned char SlaAdd, unsigned char * SendDat, unsigned char Send_Num);
extern unsigned char I2CReceiveByte(void);
extern void I2CReceiveByte_S(unsigned char SlaAdd, unsigned char * RcvDat, unsigned char Rcv_Num);

/****************** API Interface ************************/


#endif
