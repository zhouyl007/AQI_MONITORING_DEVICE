#include "vocTask.h"


//调零缓冲区大小
#define ZERO_BUF_NUM          8U
//调零缓冲区的个数
#define SENSOR_AD_BUFF_NUM    32U
#define SENSOR_ADFIT_NUM      4U

#if 1
#define AUTO_ZERO_TIME1    (10*60)		//10MIN
#define AUTO_ZERO_TIME2    (30*60)		//30MIN
#define AUTO_ZERO_TIME3    (60*60)		//60MIN
#define AUTO_ZERO_CYCLE    (8*3600)		//8HOUR
#else
#define AUTO_ZERO_TIME1    (2*60)		//10MIN
#define AUTO_ZERO_TIME2    (10*60)		//30MIN
#define AUTO_ZERO_TIME3    (20*60)		//60MIN
#define AUTO_ZERO_CYCLE    (30*60)		//8HOUR
#endif

//AD采样缓冲区
static uint16_t VOCAD_Buff[ SENSOR_AD_BUFF_NUM ];
//上电缓冲区刷新计数
static uint16_t FristCnt = 0;
//缓冲区刷新计数
static uint16_t VocBufCnt = 0;
//调零缓冲区
static uint16_t Change_zero[8] = {0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF};
//上电等待延时
static uint32_t StartWaitTime = 0;
//自动调零计数
static uint32_t AutoZeroCnt = 0;
//自动调零标志位
static uint8_t  AutoZeroFlag = 0;
//需要调零标志位
static uint8_t  NeedZeroFlag = 0;
//参考零点
static uint32_t VocBdAD[1];
//原始的VOC调零值
static uint32_t OriginalVocR0 = 0;
//VOC电阻
static uint32_t VOC_Resistor = 0;
//电阻比系数
static uint16_t K_VOC = 0;
/*******************************************************
*
*函数：my_memset( void *s, uint32_t ch, size_t n )
*
*内存设置
*
*********************************************************/

static void *my_memset( void *s, uint16_t ch,uint16_t n )
{
    void *start = s;

    while( n-- )
    {
        *( uint16_t * )s = ( uint16_t )ch;
        s = ( uint16_t * )s + 1;
    }
    return start;
}

/****************************************************************************************
*
*函数：FilterTask(unsigned short *Tab, unsigned short Len, unsigned short FliterLen)
*
*排序，去掉FliterLen个最大值和FliterLen个最小值 然后取平均
*
******************************************************************************************/
static uint16_t FilterTask( unsigned short *Tab, unsigned short Len, unsigned short FliterLen )
{
	uint8_t i, j, m;
	uint32_t Sum  = 0;
	uint16_t Temp = 0;
	uint16_t Buffer[ SENSOR_AD_BUFF_NUM ];
	uint16_t TempLen;
	
	for( i = 0; i < Len; i++ )
	{
		Buffer[i] = Tab[i];
	}
	
	//选择排序法进行排序 由小到大的排序
	for( i = 0; i < Len - 1; i++ )//循环的次数
	{
		m = i;
		for( j = i + 1;j < Len; j++ )//找到最大的值
		{
			if( Buffer[j] > Buffer[m] )  m = j;
		}
		if(i != m )
		{
			Temp=Buffer[i];
			Buffer[i] = Buffer[m];
			Buffer[m] = Temp;
		}
	}
	
	//去除一部分最大值和最小值 然后取平均
	TempLen = Len - ( FliterLen * 2 );
	Sum = 0;
	
	for( i = FliterLen; i < Len - FliterLen; i++ )
	{
		 Sum += Buffer[i];
	}
	
	Temp = Sum / TempLen;
	
	return Temp;
}


/*******************************************************
*
*函数：VOC_Resistor_Get( void )
*
*VOC实时电阻数据读取
*
*********************************************************/
uint32_t VOC_Resistor_Get( void ){

	 return VOC_Resistor;//实时电阻
}


/*******************************************************
*
*函数：VOC_Zero_Get( void )
*
*VOC调零数据读取
*
*********************************************************/
uint32_t VOC_Zero_Get( void ){

	 return VocBdAD[0];//零点
}

/*******************************************************
*
*函数：VOC_Zero_Init( uint32_t ZeroDate )
*
*VOC调零数据保存
*
*********************************************************/
void VOC_Zero_Init( uint32_t ZeroDate ){

	VocBdAD[0] = ZeroDate;//零点
}

/*******************************************************
*
*函数：VOC_Switch( uint16_t buf[], uint16_t date )
*
*VOC调零数据筛选
*
*********************************************************/
static void VOC_Switch( uint16_t buf[], uint16_t date ){

	uint8_t  cnt, num = 0;
	uint16_t max = 0;
        
	for( cnt = 0; cnt < 10 ; cnt++ ){
		
		if( max < buf[ cnt ] ){

			max = buf[ cnt ];
			num = cnt;//最大值的下标
		}
	}

	//最大值大于实时值（更新）
	if( max > date) {

		Change_zero[ num ] = date;//VOC数据
	}
}

/*******************************************************
*
*函数：VOC_Date( uint8_t Len )
*
*VOC调零数据计算
*
*********************************************************/
static uint16_t VOC_Date( uint8_t Len ){

	uint8_t  i, j, m;
	uint16_t Temp = 0;
	uint32_t Sum = 0;

	//选择排序法进行排序 由小到大的排序
	for( i = 0; i < Len - 1; i++ )//循环的次数
	{
		m = i;

		for( j = i + 1; j < Len; j++ )//找到最大的值对应的下标
		{
			if( Change_zero[ j ] < Change_zero[ m ] )  m = j;
		}

		if( i != m )
		{
			Temp = Change_zero[ i ];
			Change_zero[ i ] = Change_zero[ m ];//voc
			Change_zero[ m ] = Temp;
		}
	}

	//去极值滤波
	for( i = 0; i < Len - 4; i++ ){
		Sum += Change_zero[ i ];
	}

	return ( (uint16_t)( Sum / ( Len - 4 )) );//获取 Voc平均值

}



/*******************************************************
*
*函数：VOC_ad_Transform_ppb(uint16_t ad_Value, PTRFUN pFun)
*
*VOC ad数据计算浓度（ppb）
*
*********************************************************/
uint16_t VOC_ad_Transform_ppb(uint16_t ad_Value, PTRFUN pFun_Zero_Save)
{
        float  TempFloat = 0;
		int dTempResult = 0;
		
        if(++FristCnt < 10*2)//启动时前20次
        {
            my_memset(VOCAD_Buff, ad_Value, SENSOR_AD_BUFF_NUM);
        }
		else
		{
            VOCAD_Buff[ VocBufCnt ] = ad_Value;//32次
        }

		if(++VocBufCnt >= SENSOR_AD_BUFF_NUM)  
			VocBufCnt = 0;//32次

        //VOC
		TempFloat = FilterTask( VOCAD_Buff, SENSOR_AD_BUFF_NUM, SENSOR_ADFIT_NUM );//去极值平均滤波
        VOC_Resistor = (0xFFF / TempFloat - 1.0) * 100000;
        
        AutoZeroCnt++;

		if(AutoZeroCnt > 60)
		{
/************************************************************************ 调零 ********************************************************************************/        
            VOC_Switch(Change_zero, TempFloat );//数据筛选
            
            if(AutoZeroFlag == 0)//开机没有调过零
            {
                  if((AutoZeroCnt == AUTO_ZERO_TIME1) || ( AutoZeroCnt == AUTO_ZERO_TIME2))
                  {
                            NeedZeroFlag = 1;
                  }
                  else if(AutoZeroCnt == AUTO_ZERO_TIME3)
                  {
                            NeedZeroFlag = 1;
                            AutoZeroFlag = 1;
                  }
            }
            else//8h自动调零
            {
                  if(AutoZeroCnt >= AUTO_ZERO_CYCLE)
                  {
                            AutoZeroCnt = 0;
                            NeedZeroFlag = 2;
                  }
            }
            
            if(NeedZeroFlag > 0)//需要进行调零
            {
                      dTempResult = VOC_Date( ZERO_BUF_NUM );//调零数据计算

                      dTempResult = (0xFFF / (float)dTempResult - 1.0) * 100000;//电阻计算
                      
                      if(NeedZeroFlag == 1)
                      {
                                if(dTempResult > OriginalVocR0)                        
									VocBdAD[0] = dTempResult;     //比当前零点大，直接更新
                                else if(dTempResult < (0.8 * (float)OriginalVocR0))
									VocBdAD[0] = (0.8 * (float)OriginalVocR0);  //比当前零点小，限幅更新
                                else                                                     
									VocBdAD[0] = OriginalVocR0;   //不更新

								//printf("voc recalibration %d----------------\r\n",VocBdAD[0]);//调试指令
                      }
                      else if(NeedZeroFlag == 2)
                      {
                                VocBdAD[0]    = dTempResult;	//零点
                                OriginalVocR0 = VocBdAD[0]; 	//零点
                              
                                pFun_Zero_Save(VocBdAD[0] );	//保存零点

                                my_memset(Change_zero, 0xFFFF, ZERO_BUF_NUM);
                      }
                      
                      NeedZeroFlag = 0;
            }

/************************************************************************ 电阻比值与浓度计算 ********************************************************************************/        

                K_VOC = (int)((VOC_Resistor * 1.0f)/VocBdAD[0] * 1000.0f);//比例系数( 放大10倍 )

				//printf("voc result %d %d %d----------------\r\n",VOC_Resistor,VocBdAD[0],K_VOC);
                
                if(K_VOC >= 1000){          
					K_VOC = 1000;
                }
                else if(K_VOC <= 500){      
					K_VOC = 500;
                }

				//printf("voc result K_VOC %d----------------\r\n",K_VOC);

                dTempResult  = (int)((1000 - K_VOC)*1.2f);//浓度ppb计算
                //printf("voc result %d %d %d %d %d %d %d----------------\r\n",VOC_Resistor,VocBdAD[0],K_VOC,dTempResult,(1000 - K_VOC),((1000 - K_VOC)*6),((1000 - K_VOC)*6) / 5);
         }

		#if 0
        if( V_DebugFlag == 1 )
        {
                uint8_t i;

                printf("VO= %d V1= %d R = %d R0 = %d K = %d %ds ",ad_Value,(uint16_t)TempFloat, VOC_Resistor, VocBdAD[0], K_VOC , AutoZeroCnt);//调试指令

                for( i = 0; i < ZERO_BUF_NUM; i++ )
                        printf(" %d ", Change_zero[i]);

                printf(" %d\n",dTempResult);//调试指令
        }
		#endif

         return dTempResult;

}









