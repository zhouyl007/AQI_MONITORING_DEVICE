#include "vocTask.h"


//���㻺������С
#define ZERO_BUF_NUM          8U
//���㻺�����ĸ���
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

//AD����������
static uint16_t VOCAD_Buff[ SENSOR_AD_BUFF_NUM ];
//�ϵ绺����ˢ�¼���
static uint16_t FristCnt = 0;
//������ˢ�¼���
static uint16_t VocBufCnt = 0;
//���㻺����
static uint16_t Change_zero[8] = {0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF};
//�ϵ�ȴ���ʱ
static uint32_t StartWaitTime = 0;
//�Զ��������
static uint32_t AutoZeroCnt = 0;
//�Զ������־λ
static uint8_t  AutoZeroFlag = 0;
//��Ҫ�����־λ
static uint8_t  NeedZeroFlag = 0;
//�ο����
static uint32_t VocBdAD[1];
//ԭʼ��VOC����ֵ
static uint32_t OriginalVocR0 = 0;
//VOC����
static uint32_t VOC_Resistor = 0;
//�����ϵ��
static uint16_t K_VOC = 0;
/*******************************************************
*
*������my_memset( void *s, uint32_t ch, size_t n )
*
*�ڴ�����
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
*������FilterTask(unsigned short *Tab, unsigned short Len, unsigned short FliterLen)
*
*����ȥ��FliterLen�����ֵ��FliterLen����Сֵ Ȼ��ȡƽ��
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
	
	//ѡ�����򷨽������� ��С���������
	for( i = 0; i < Len - 1; i++ )//ѭ���Ĵ���
	{
		m = i;
		for( j = i + 1;j < Len; j++ )//�ҵ�����ֵ
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
	
	//ȥ��һ�������ֵ����Сֵ Ȼ��ȡƽ��
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
*������VOC_Resistor_Get( void )
*
*VOCʵʱ�������ݶ�ȡ
*
*********************************************************/
uint32_t VOC_Resistor_Get( void ){

	 return VOC_Resistor;//ʵʱ����
}


/*******************************************************
*
*������VOC_Zero_Get( void )
*
*VOC�������ݶ�ȡ
*
*********************************************************/
uint32_t VOC_Zero_Get( void ){

	 return VocBdAD[0];//���
}

/*******************************************************
*
*������VOC_Zero_Init( uint32_t ZeroDate )
*
*VOC�������ݱ���
*
*********************************************************/
void VOC_Zero_Init( uint32_t ZeroDate ){

	VocBdAD[0] = ZeroDate;//���
}

/*******************************************************
*
*������VOC_Switch( uint16_t buf[], uint16_t date )
*
*VOC��������ɸѡ
*
*********************************************************/
static void VOC_Switch( uint16_t buf[], uint16_t date ){

	uint8_t  cnt, num = 0;
	uint16_t max = 0;
        
	for( cnt = 0; cnt < 10 ; cnt++ ){
		
		if( max < buf[ cnt ] ){

			max = buf[ cnt ];
			num = cnt;//���ֵ���±�
		}
	}

	//���ֵ����ʵʱֵ�����£�
	if( max > date) {

		Change_zero[ num ] = date;//VOC����
	}
}

/*******************************************************
*
*������VOC_Date( uint8_t Len )
*
*VOC�������ݼ���
*
*********************************************************/
static uint16_t VOC_Date( uint8_t Len ){

	uint8_t  i, j, m;
	uint16_t Temp = 0;
	uint32_t Sum = 0;

	//ѡ�����򷨽������� ��С���������
	for( i = 0; i < Len - 1; i++ )//ѭ���Ĵ���
	{
		m = i;

		for( j = i + 1; j < Len; j++ )//�ҵ�����ֵ��Ӧ���±�
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

	//ȥ��ֵ�˲�
	for( i = 0; i < Len - 4; i++ ){
		Sum += Change_zero[ i ];
	}

	return ( (uint16_t)( Sum / ( Len - 4 )) );//��ȡ Vocƽ��ֵ

}



/*******************************************************
*
*������VOC_ad_Transform_ppb(uint16_t ad_Value, PTRFUN pFun)
*
*VOC ad���ݼ���Ũ�ȣ�ppb��
*
*********************************************************/
uint16_t VOC_ad_Transform_ppb(uint16_t ad_Value, PTRFUN pFun_Zero_Save)
{
        float  TempFloat = 0;
		int dTempResult = 0;
		
        if(++FristCnt < 10*2)//����ʱǰ20��
        {
            my_memset(VOCAD_Buff, ad_Value, SENSOR_AD_BUFF_NUM);
        }
		else
		{
            VOCAD_Buff[ VocBufCnt ] = ad_Value;//32��
        }

		if(++VocBufCnt >= SENSOR_AD_BUFF_NUM)  
			VocBufCnt = 0;//32��

        //VOC
		TempFloat = FilterTask( VOCAD_Buff, SENSOR_AD_BUFF_NUM, SENSOR_ADFIT_NUM );//ȥ��ֵƽ���˲�
        VOC_Resistor = (0xFFF / TempFloat - 1.0) * 100000;
        
        AutoZeroCnt++;

		if(AutoZeroCnt > 60)
		{
/************************************************************************ ���� ********************************************************************************/        
            VOC_Switch(Change_zero, TempFloat );//����ɸѡ
            
            if(AutoZeroFlag == 0)//����û�е�����
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
            else//8h�Զ�����
            {
                  if(AutoZeroCnt >= AUTO_ZERO_CYCLE)
                  {
                            AutoZeroCnt = 0;
                            NeedZeroFlag = 2;
                  }
            }
            
            if(NeedZeroFlag > 0)//��Ҫ���е���
            {
                      dTempResult = VOC_Date( ZERO_BUF_NUM );//�������ݼ���

                      dTempResult = (0xFFF / (float)dTempResult - 1.0) * 100000;//�������
                      
                      if(NeedZeroFlag == 1)
                      {
                                if(dTempResult > OriginalVocR0)                        
									VocBdAD[0] = dTempResult;     //�ȵ�ǰ����ֱ�Ӹ���
                                else if(dTempResult < (0.8 * (float)OriginalVocR0))
									VocBdAD[0] = (0.8 * (float)OriginalVocR0);  //�ȵ�ǰ���С���޷�����
                                else                                                     
									VocBdAD[0] = OriginalVocR0;   //������

								//printf("voc recalibration %d----------------\r\n",VocBdAD[0]);//����ָ��
                      }
                      else if(NeedZeroFlag == 2)
                      {
                                VocBdAD[0]    = dTempResult;	//���
                                OriginalVocR0 = VocBdAD[0]; 	//���
                              
                                pFun_Zero_Save(VocBdAD[0] );	//�������

                                my_memset(Change_zero, 0xFFFF, ZERO_BUF_NUM);
                      }
                      
                      NeedZeroFlag = 0;
            }

/************************************************************************ �����ֵ��Ũ�ȼ��� ********************************************************************************/        

                K_VOC = (int)((VOC_Resistor * 1.0f)/VocBdAD[0] * 1000.0f);//����ϵ��( �Ŵ�10�� )

				//printf("voc result %d %d %d----------------\r\n",VOC_Resistor,VocBdAD[0],K_VOC);
                
                if(K_VOC >= 1000){          
					K_VOC = 1000;
                }
                else if(K_VOC <= 500){      
					K_VOC = 500;
                }

				//printf("voc result K_VOC %d----------------\r\n",K_VOC);

                dTempResult  = (int)((1000 - K_VOC)*1.2f);//Ũ��ppb����
                //printf("voc result %d %d %d %d %d %d %d----------------\r\n",VOC_Resistor,VocBdAD[0],K_VOC,dTempResult,(1000 - K_VOC),((1000 - K_VOC)*6),((1000 - K_VOC)*6) / 5);
         }

		#if 0
        if( V_DebugFlag == 1 )
        {
                uint8_t i;

                printf("VO= %d V1= %d R = %d R0 = %d K = %d %ds ",ad_Value,(uint16_t)TempFloat, VOC_Resistor, VocBdAD[0], K_VOC , AutoZeroCnt);//����ָ��

                for( i = 0; i < ZERO_BUF_NUM; i++ )
                        printf(" %d ", Change_zero[i]);

                printf(" %d\n",dTempResult);//����ָ��
        }
		#endif

         return dTempResult;

}









