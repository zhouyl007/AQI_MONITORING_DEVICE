/*
*********************************************************************************************************
*	                                  
*	ģ������ : GUI����������
*	�ļ����� : MainTask.c
*	��    �� : V1.0
*	˵    �� : �๦���Ǳ����
*              1. ����������ɹٷ��Ķ๦���Ǳ������޸Ķ������ٷ������������ƵĽϸ��ӣ�̫ƫ�ײ�ʵ�֣�
*                 �Ѿ������̽����ʵ��ר���������ļ�App_ScaleWin.c���档��ʵ�ٷ��Ǵ���ר����һ���Ǳ�
*                 �ؼ��ģ��������ڲ�����ƽ��鷳��û��ר�������ؼ���
*              2. ����ʹ�÷���
*                 ��1��ʹ��ǰҪ�ȳ�ʼ�����̽ṹ��������������ǳ�ʼ���ı��̽ṹ������_Scale����ʼ����4����
*                 ��2��ͨ������_CreateScaleWindow�������̴��ڣ����̽���Ч�����ڴ˴�������ʵ�ֵġ�������
*                      ���֧��ͬʱ�����ĸ���
*                 ��3�����̵�ת����ͨ������_MoveNeedleʵ�ֵģ����ʵ��������ʽ��ת�����޸Ĵ˺������ɡ�
*              3. �������洴����4����ť��ÿ����ťʵ�ֲ�ͬ�ı���Ч���л���
*              
*	�޸ļ�¼ :
*		�汾��   ����         ����          ˵��
*		V1.0    2016-12-23   Eric2013  	    �װ�   
*                                     
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "MainTask.h"


/*
**********************************************************************************************************
							   ����
**********************************************************************************************************
*/
GUI_MEMDEV_Handle _hBkMemDev;


/*
*********************************************************************************************************
*	�� �� ��: _CreateBackGround
*	����˵��: ���ݶ�ɫ���Ƶ��洢�豸���棬����ˢ����ʾ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _CreateBackGround(void) 
{
	GUI_MEMDEV_Handle hMemPrev;
	GUI_RECT          r;
	int               xSize;
	int               ySize;

	xSize = LCD_GetXSize();
	ySize = LCD_GetYSize();
	r.x0  = 0;
	r.x1  = xSize - 1;
	r.y0  = 0;
	r.y1  = ySize - 1;
	
	_hBkMemDev = GUI_MEMDEV_CreateEx(r.x0, r.y0, r.x1 + 1, r.y1 + 1, GUI_MEMDEV_NOTRANS);
	hMemPrev = GUI_MEMDEV_Select(_hBkMemDev);
	GUI_DrawGradientV(r.x0, r.y0, r.x1 + 1, r.y1 + 1,GUI_BLUE, GUI_BLACK);
	GUI_MEMDEV_Select(hMemPrev);
}

/*
*********************************************************************************************************
*	�� �� ��: _cbBkWindow
*	����˵��: ���洰�ڻص�����
*	��    ��: pMsg  ��Ϣ�ṹ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _cbBkWindow(WM_MESSAGE* pMsg) 
{
	int Id;
	
	switch (pMsg->MsgId) 
	{
		
		case WM_PAINT:
			GUI_MEMDEV_Write(_hBkMemDev);
			break;
		
		case WM_NOTIFY_PARENT:

			Id = WM_GetId(pMsg->hWinSrc);
			
			switch (pMsg->Data.v) 
			{
				/* ���洰�����������ĸ���ť�Ļص�������ÿ����ťʵ�ֵı���Ч����ͬ */
				case WM_NOTIFICATION_RELEASED:
					if (Id >= GUI_ID_BUTTON0 && Id <= GUI_ID_BUTTON3) 
					{
						_SetPreset(Id - GUI_ID_BUTTON0, -1);
					} 
					break;
			}
			break;
		
		default:
			WM_DefaultProc(pMsg);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: _CreateButton
*	����˵��: ������ť
*	��    ��: pText     ��ť����ʾ���ı�
*             x         ��ť��ʼX������
*             y   		��ť��ʼY������
*             w   		��ť���
*             h   		��ť�߶�
*             hParent   ��ť������
*             Id   		��ťID
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _CreateButton(const char* pText, int x, int y, int w, int h, WM_HWIN hParent, int Id) 
{
	WM_HWIN hBut;
	hBut = BUTTON_CreateAsChild(x, y, w, h, hParent, Id, WM_CF_SHOW);
	BUTTON_SetText(hBut, pText);
	BUTTON_SetFont(hBut, &GUI_Font16_1);
}

/*
*********************************************************************************************************
*	�� �� ��: MainTask
*	����˵��: GUI������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MainTask(void) 
{
	BUTTON_SKINFLEX_PROPS BUTTON_Props;
	
	/* ��ʼ�� */
	GUI_Init();
	
	/*
	 ���ڶ໺��ʹ����ڴ��豸������˵��
	   1. ʹ�ܶ໺���ǵ��õ����º������û�Ҫ��LCDConf_Lin_Template.c�ļ��������˶໺�壬���ô˺�������Ч��
		  WM_MULTIBUF_Enable(1);
	   2. ����ʹ��ʹ���ڴ��豸�ǵ��ú�����WM_SetCreateFlags(WM_CF_MEMDEV);
	   3. ���emWin�����ö໺��ʹ����ڴ��豸��֧�֣���ѡһ���ɣ����������ѡ��ʹ�ö໺�壬ʵ��ʹ��
		  STM32F429BIT6 + 32λSDRAM + RGB565/RGB888ƽ̨���ԣ��໺�������Ч�Ľ��ʹ����ƶ����߻���ʱ��˺��
		  �У�����Ч����������ԣ�ͨ��ʹ�ܴ���ʹ���ڴ��豸���������ġ�
	   4. ����emWin����Ĭ���ǿ��������塣
	*/
	WM_MULTIBUF_Enable(1);
	
	/*
       ����У׼����Ĭ����ע�͵��ģ���������ҪУ׼������������У׼������û���ҪУ׼�������Ļ���ִ��
	   �˺������ɣ��Ὣ����У׼�������浽EEPROM���棬�Ժ�ϵͳ�ϵ���Զ���EEPROM������ء�
	*/
    //TOUCH_Calibration();
	
	/* ��ȡ��ť����״̬��Ƥ�� */
	BUTTON_GetSkinFlexProps(&BUTTON_Props, BUTTON_SKINFLEX_PI_ENABLED);
	BUTTON_Props.aColorFrame[0] = 0x000080FF;
	BUTTON_Props.aColorFrame[1] = 0x000080FF;
	BUTTON_Props.aColorFrame[2] = 0x000080FF;

	BUTTON_Props.aColorLower[0] = 0x000080FF;
	BUTTON_Props.aColorLower[1] = 0x000080FF;
	
	BUTTON_Props.aColorUpper[0] = 0x000080FF;
	BUTTON_Props.aColorUpper[1] = 0x000080FF;

	/* ���ð�������״̬��Ƥ��ɫ */
	BUTTON_SetSkinFlexProps(&BUTTON_Props, BUTTON_SKINFLEX_PI_ENABLED);
	
	/* ���þ۽�״̬��Ƥ��ɫ */
	BUTTON_SetSkinFlexProps(&BUTTON_Props, BUTTON_SKINFLEX_PI_FOCUSSED);

	BUTTON_Props.aColorFrame[0] = GUI_GREEN;
	BUTTON_Props.aColorFrame[1] = GUI_GREEN;
	BUTTON_Props.aColorFrame[2] = GUI_GREEN;

	BUTTON_Props.aColorLower[0] = GUI_GREEN;
	BUTTON_Props.aColorLower[1] = GUI_GREEN;
	
	BUTTON_Props.aColorUpper[0] = GUI_GREEN;
	BUTTON_Props.aColorUpper[1] = GUI_GREEN;
	
	/* ���ð���״̬��Ƥ��ɫ */
	BUTTON_SetSkinFlexProps(&BUTTON_Props, BUTTON_SKINFLEX_PI_PRESSED);
		
	/*  ���ݶ�ɫ���Ƶ��洢�豸���棬����ˢ����ʾ������ */
	_CreateBackGround();
	
	/* �������洰�ڵĻص����� */
	WM_SetCallback(WM_HBKWIN, &_cbBkWindow);
	
	/* ͬ��Ľṹ���������ֱ�Ӹ�ֵ */
	_Scale[0] = _Presets[0][0];
	_Scale[1] = _Presets[0][1];
	_Scale[2] = _Presets[0][2];
	_Scale[3] = _Presets[0][3];
	
	/* �������̴��� */
	_CreateScaleWindow(&_Scale[0]);
	_CreateScaleWindow(&_Scale[1]);
	_CreateScaleWindow(&_Scale[2]);
	_CreateScaleWindow(&_Scale[3]);
	
	/* �����ĸ���ť�������ͬ��ť�����̵�Ч����ͬ */
	_CreateButton("Style 1", 40,  380, 150, 40, WM_HBKWIN, GUI_ID_BUTTON0);
	_CreateButton("Style 2", 230, 380, 150, 40, WM_HBKWIN, GUI_ID_BUTTON1);
	_CreateButton("Style 3", 420, 380, 150, 40, WM_HBKWIN, GUI_ID_BUTTON2);
	_CreateButton("Style 4", 610, 380, 150, 40, WM_HBKWIN, GUI_ID_BUTTON3);
	
	while(1) 
	{
		/* ת������ָ�� */
		_MoveNeedle(&_Needle[0], 0);
		_MoveNeedle(&_Needle[1], 1);
		_MoveNeedle(&_Needle[2], 2);
		_MoveNeedle(&_Needle[3], 3);
		
		GUI_Delay(5);
	} 
	
	
	/* ����Ĵ����Ǳ��ݣ�������̴��ڲ�ʹ���ˣ���Ҫ�����ں��õ��洢�豸��ɾ���� */
//	for (i = 0; i < (int)GUI_COUNTOF(_Scale); i++) 
//	{
//		WM_DeleteWindow(_Scale[i].hWin);
//		GUI_MEMDEV_Delete(_Scale[i].hMemDev);
//	}
//	
//	GUI_MEMDEV_Delete(_hBkMemDev);
	
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
