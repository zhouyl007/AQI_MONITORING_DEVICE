/*
*********************************************************************************************************
*	                                  
*	模块名称 : GUI界面主函数
*	文件名称 : MainTask.c
*	版    本 : V1.0
*	说    明 : 多功能仪表界面
*              1. 这个例子是由官方的多功能仪表例子修改而来。官方的这个例子设计的较复杂，太偏底层实现，
*                 已经将表盘界面的实现专门整理到了文件App_ScaleWin.c里面。其实官方是打算专门做一个仪表
*                 控件的，但是由于参数设计较麻烦，没有专门作出控件。
*              2. 表盘使用方法
*                 （1）使用前要先初始化表盘结构体变量，本例子是初始化的表盘结构体数组_Scale，初始化了4个。
*                 （2）通过函数_CreateScaleWindow创建表盘窗口，表盘界面效果是在此窗口上面实现的。本例子
*                      最大支持同时创建四个。
*                 （3）表盘的转动是通过函数_MoveNeedle实现的，如果实现其它方式的转动，修改此函数即可。
*              3. 界面上面创建了4个按钮，每个按钮实现不同的表盘效果切换。
*              
*	修改记录 :
*		版本号   日期         作者          说明
*		V1.0    2016-12-23   Eric2013  	    首版   
*                                     
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "MainTask.h"


/*
**********************************************************************************************************
							   变量
**********************************************************************************************************
*/
GUI_MEMDEV_Handle _hBkMemDev;


/*
*********************************************************************************************************
*	函 数 名: _CreateBackGround
*	功能说明: 将梯度色绘制到存储设备里面，用于刷新显示屏背景
*	形    参: 无
*	返 回 值: 无
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
*	函 数 名: _cbBkWindow
*	功能说明: 桌面窗口回调函数
*	形    参: pMsg  消息结构体
*	返 回 值: 无
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
				/* 桌面窗口上所创建四个按钮的回调函数，每个按钮实现的表盘效果不同 */
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
*	函 数 名: _CreateButton
*	功能说明: 创建按钮
*	形    参: pText     按钮上显示的文本
*             x         按钮起始X轴坐标
*             y   		按钮起始Y轴坐标
*             w   		按钮宽度
*             h   		按钮高度
*             hParent   按钮父窗口
*             Id   		按钮ID
*	返 回 值: 无
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
*	函 数 名: MainTask
*	功能说明: GUI主函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MainTask(void) 
{
	BUTTON_SKINFLEX_PROPS BUTTON_Props;
	
	/* 初始化 */
	GUI_Init();
	
	/*
	 关于多缓冲和窗口内存设备的设置说明
	   1. 使能多缓冲是调用的如下函数，用户要在LCDConf_Lin_Template.c文件中配置了多缓冲，调用此函数才有效：
		  WM_MULTIBUF_Enable(1);
	   2. 窗口使能使用内存设备是调用函数：WM_SetCreateFlags(WM_CF_MEMDEV);
	   3. 如果emWin的配置多缓冲和窗口内存设备都支持，二选一即可，且务必优先选择使用多缓冲，实际使用
		  STM32F429BIT6 + 32位SDRAM + RGB565/RGB888平台测试，多缓冲可以有效的降低窗口移动或者滑动时的撕裂
		  感，并有效的提高流畅性，通过使能窗口使用内存设备是做不到的。
	   4. 所有emWin例子默认是开启三缓冲。
	*/
	WM_MULTIBUF_Enable(1);
	
	/*
       触摸校准函数默认是注释掉的，电阻屏需要校准，电容屏无需校准。如果用户需要校准电阻屏的话，执行
	   此函数即可，会将触摸校准参数保存到EEPROM里面，以后系统上电会自动从EEPROM里面加载。
	*/
    //TOUCH_Calibration();
	
	/* 获取按钮启动状态的皮肤 */
	BUTTON_GetSkinFlexProps(&BUTTON_Props, BUTTON_SKINFLEX_PI_ENABLED);
	BUTTON_Props.aColorFrame[0] = 0x000080FF;
	BUTTON_Props.aColorFrame[1] = 0x000080FF;
	BUTTON_Props.aColorFrame[2] = 0x000080FF;

	BUTTON_Props.aColorLower[0] = 0x000080FF;
	BUTTON_Props.aColorLower[1] = 0x000080FF;
	
	BUTTON_Props.aColorUpper[0] = 0x000080FF;
	BUTTON_Props.aColorUpper[1] = 0x000080FF;

	/* 设置按下启动状态的皮肤色 */
	BUTTON_SetSkinFlexProps(&BUTTON_Props, BUTTON_SKINFLEX_PI_ENABLED);
	
	/* 设置聚焦状态的皮肤色 */
	BUTTON_SetSkinFlexProps(&BUTTON_Props, BUTTON_SKINFLEX_PI_FOCUSSED);

	BUTTON_Props.aColorFrame[0] = GUI_GREEN;
	BUTTON_Props.aColorFrame[1] = GUI_GREEN;
	BUTTON_Props.aColorFrame[2] = GUI_GREEN;

	BUTTON_Props.aColorLower[0] = GUI_GREEN;
	BUTTON_Props.aColorLower[1] = GUI_GREEN;
	
	BUTTON_Props.aColorUpper[0] = GUI_GREEN;
	BUTTON_Props.aColorUpper[1] = GUI_GREEN;
	
	/* 设置按下状态的皮肤色 */
	BUTTON_SetSkinFlexProps(&BUTTON_Props, BUTTON_SKINFLEX_PI_PRESSED);
		
	/*  将梯度色绘制到存储设备里面，用于刷新显示屏背景 */
	_CreateBackGround();
	
	/* 设置桌面窗口的回调函数 */
	WM_SetCallback(WM_HBKWIN, &_cbBkWindow);
	
	/* 同类的结构体变量可以直接赋值 */
	_Scale[0] = _Presets[0][0];
	_Scale[1] = _Presets[0][1];
	_Scale[2] = _Presets[0][2];
	_Scale[3] = _Presets[0][3];
	
	/* 创建表盘窗口 */
	_CreateScaleWindow(&_Scale[0]);
	_CreateScaleWindow(&_Scale[1]);
	_CreateScaleWindow(&_Scale[2]);
	_CreateScaleWindow(&_Scale[3]);
	
	/* 创建四个按钮，点击不同按钮，表盘的效果不同 */
	_CreateButton("Style 1", 40,  380, 150, 40, WM_HBKWIN, GUI_ID_BUTTON0);
	_CreateButton("Style 2", 230, 380, 150, 40, WM_HBKWIN, GUI_ID_BUTTON1);
	_CreateButton("Style 3", 420, 380, 150, 40, WM_HBKWIN, GUI_ID_BUTTON2);
	_CreateButton("Style 4", 610, 380, 150, 40, WM_HBKWIN, GUI_ID_BUTTON3);
	
	while(1) 
	{
		/* 转动表盘指针 */
		_MoveNeedle(&_Needle[0], 0);
		_MoveNeedle(&_Needle[1], 1);
		_MoveNeedle(&_Needle[2], 2);
		_MoveNeedle(&_Needle[3], 3);
		
		GUI_Delay(5);
	} 
	
	
	/* 下面的代码是备份，如果表盘窗口不使用了，需要将窗口和用到存储设备都删除掉 */
//	for (i = 0; i < (int)GUI_COUNTOF(_Scale); i++) 
//	{
//		WM_DeleteWindow(_Scale[i].hWin);
//		GUI_MEMDEV_Delete(_Scale[i].hMemDev);
//	}
//	
//	GUI_MEMDEV_Delete(_hBkMemDev);
	
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
