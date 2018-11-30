/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2015  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.32 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only  be used  in accordance  with  a license  and should  not be  re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : 2DGL_DrawScale.c
Purpose     : Drawing a scale using GUI-functions
Requirements: WindowManager - ( )
              MemoryDevices - ( )
              AntiAliasing  - (x)
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )
----------------------------------------------------------------------
*/

#include "GUI.h"
#include "WM.h"
#include <math.h>
#include <stddef.h>


static WM_HWIN _hWindow1;
static WM_HWIN _hWindow2;
static WM_HWIN _hChild;
static GUI_MEMDEV_Handle _hMem;

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define SPEED 500/*1100*/

//
// Recommended memory to run the sample with adequate performance
//
#define RECOMMENDED_MEMORY (1024L * 5)

/*******************************************************************
*
*       static code
*
********************************************************************
*/
/*******************************************************************
*
*		_cbWindow1
*/
static void _cbWindow1(WM_MESSAGE * pMsg) {
  GUI_RECT Rect;
  int	   x;
  int	   y;
  int tm;
  int tDiff;
  int XSize;
  int XMid; 
  int i;
  int r1;
  int r2;
  int rt;
  int step;
  int r;
  
  r1    = 50;
  r2    = 80;
  rt    = 100;
  y     = 240;
  step  =  15;
  r     = (r1 + r2) / 2;
  XSize = LCD_GetXSize();
  XMid  = XSize / 2;

  switch (pMsg->MsgId) {
  case WM_PRE_PAINT:
    //GUI_MULTIBUF_Begin();
    break;
  case WM_POST_PAINT:
    //GUI_MULTIBUF_End();
    break;
  case WM_PAINT:
  WM_GetInsideRect(&Rect);
  GUI_ClearRectEx(&Rect);
	//LCD_Clear(0x00);
	x = WM_GetWindowSizeX(pMsg->hWin);
	y = WM_GetWindowSizeY(pMsg->hWin);
	#if 1
	GUI_SetPenSize(r2 - r1);	
  	GUI_SetColor(0x0000AA);
  	GUI_AA_DrawArc(XMid / 3, y / 3, r, r, 0, 270);
  	GUI_AA_DrawArc(XMid / 3, y / 3, r, r, 270, 360);
	GUI_SetColor(GUI_BLACK);
  	GUI_AA_DrawArc(XMid / 3, y / 3, r, r, 300, 360);
  	GUI_SetPenSize(2);
  	GUI_SetColor(GUI_WHITE);
  	GUI_AA_DrawArc(XMid / 3, y / 3, r1, r1, 0, 360);
	GUI_AA_DrawArc(XMid / 3, y / 3, r2, r2, 0, 360);
	#endif
	GUI_SetColor(GUI_WHITE);
  	GUI_SetFont(&GUI_Font24_ASCII);
	GUI_DispStringHCenterAt("Child window", x / 3, (y / 3) - 12); 
	break;
  default:
	WM_DefaultProc(pMsg);
  }
}
/*******************************************************************
*
*       _cbChild
*/
static void _cbChild(WM_MESSAGE * pMsg) {
  GUI_RECT Rect;
  int      x;
  int      y;

  switch (pMsg->MsgId) {
  case WM_PAINT:
  	#if 1
    GUI_SetColor(GUI_RED);
    GUI_SetFont(&GUI_Font24_ASCII);
    x = WM_GetWindowSizeX(pMsg->hWin);
    y = WM_GetWindowSizeY(pMsg->hWin);
    GUI_DispStringHCenterAt("Child window", x / 2, (y / 2) - 12);
    //GUI_MEMDEV_CopyToLCDAt(_hMem, x / 4, y / 4);
	#endif
    break;
  default:
    WM_DefaultProc(pMsg);
  }
}

/*******************************************************************
*
*       _DrawScale
*
* Function description
*   Drawing a scale using GUI-functions
*/
static void _DrawScale(void) {
  int tm;
  int tDiff = 0;
  int XSize;
  int XMid; 
  int i;
  int r1;
  int r2;
  int rt;
  int y ;
  int step;
  int r;
  
  XSize = LCD_GetXSize();
  XMid  = XSize / 2;
  r1    = 50;
  r2    = 80;
  rt    = 100;
  y     = 240;
  step  =  15;
  r     = (r1 + r2) / 2;
  
  GUI_SetBkColor(GUI_BLACK);
  GUI_Clear();
  
  #if 0
  GUI_SetColor(GUI_WHITE);
  GUI_SetFont(&GUI_Font24_ASCII);
  GUI_SetTextAlign(GUI_TA_HCENTER);
  GUI_DispStringAt("DrawScale - Sample", 160, 5);

  _hMem = GUI_MEMDEV_Create(0,  0, LCD_GetXSize(), LCD_GetYSize());

  
  GUI_MEMDEV_Select(_hMem);
  GUI_SetPenSize(r2 - r1);	
  GUI_SetColor(0x0000AA);
  GUI_AA_DrawArc(XMid, y, r, r, 0, 270);
  GUI_AA_DrawArc(XMid, y, r, r, 270, 360);
  GUI_SetColor(GUI_WHITE);
  GUI_AA_DrawArc(XMid, y, r, r, 300, 360);
  GUI_SetPenSize(2);
  GUI_SetColor(GUI_WHITE);
  GUI_AA_DrawArc(XMid, y, r1, r1, 0, 360);
  GUI_AA_DrawArc(XMid, y, r2, r2, 0, 360);
  GUI_MEMDEV_Select(0);
  #endif

  _hWindow1 = WM_CreateWindow( 0,  0, 240, 240, WM_CF_SHOW | WM_CF_MEMDEV | WM_CF_MOTION_X, _cbWindow1, 0);
  _hChild = WM_CreateWindowAsChild(0, 0, 200, 200, _hWindow1, WM_CF_SHOW | WM_CF_MEMDEV, _cbChild, 0);
	GUI_Delay(200);
  while(1);
  while (1) {
  	#if 0
    int c = 0;
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(&GUI_Font8x16);
    GUI_SetTextAlign(GUI_TA_LEFT);
    GUI_Delay(SPEED / 2);
    GUI_DispStringAtCEOL("using", 5, 40);
    GUI_DispStringAtCEOL("GUI_AA_DrawArc", 5, 55);
    GUI_Delay(SPEED * 2);
	#endif
	//GUI_MEMDEV_CopyToLCDAt(hMem, -60, 0);
    //GUI_Delay(SPEED);
	//GUI_SetPenSize(r2 - r1);	
    //GUI_SetColor(0x0000AA);
    //GUI_AA_DrawArc(XMid, y, r, r, 0, 270);
	//GUI_AA_DrawArc(XMid, y, r, r, 300, 360);
    //GUI_SetColor(0x00AA00);
    //GUI_AA_DrawArc(XMid, y, r, r, 60, 90);
    //GUI_Delay(SPEED);
    //GUI_SetPenSize(2);
    //GUI_SetColor(GUI_WHITE);
    //GUI_AA_DrawArc(XMid, y, r1, r1, 0, 360);
    //GUI_Delay(SPEED);
    //GUI_AA_DrawArc(XMid, y, r2, r2, 0, 360);
    //GUI_Delay(SPEED);
	#if 0
    GUI_DispStringAtCEOL("", 5, 55);
    GUI_Delay(200);
    GUI_DispStringAtCEOL("using", 5, 40);
    GUI_DispStringAtCEOL("GUI_AA_DrawLine & GUI_DispCharAt", 5, 55);
    GUI_Delay(SPEED * 3);
    for (i = 45; i <= 135; i += step) {
      float co = cos(i * 3.1415926 / 180);
      float si = sin(i * 3.1415926 / 180);
      int   x1 = XMid - (int)(r1 * co);
      int   y1 = y    - (int)(r1 * si);
      int   x2 = XMid - (int)((r2 - 1) * co);
      int   y2 = y    - (int)((r2 - 1) * si);
      int   xt = XMid - (int)(rt * co);
      int   yt = y    - (int)(rt * si);
      GUI_SetColor(GUI_WHITE);
      GUI_SetPenSize(2);
      GUI_AA_DrawLine(x1, y1, x2, y2);
      GUI_SetColor(GUI_GREEN);
      GUI_SetFont(&GUI_Font8x8);
      GUI_DispCharAt('0' + c++, xt - 4, yt - 4);
      GUI_Delay(SPEED / 2);
    }
    GUI_Delay(SPEED * 3);
    GUI_ClearRect(0, 30, 320, 240);
	#endif

	WM_MoveWindow(_hWindow1, 4, 0);
	//for (i = 0; i < 10; i++) {
    //tm = GUI_GetTime();
    //WM_MOTION_SetSpeed(_hWindow1, GUI_COORD_X, 100);
    //WM_MoveTo(_hWindow1,  50 + i,  170);
    //tDiff = GUI_GetTime() - tm;
    
	//GUI_X_Delay(1);
    //GUI_Exec();
  //}
	
  //for (i = 0; i < 10; i++) {
    //WM_MoveWindow(_hWindow1, -5, 0);
    GUI_Delay(10);
    //GUI_Exec();
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       MainTask
*/
void MainTask(void) {

  //WM_SetCreateFlags(WM_CF_MEMDEV);

  GUI_Init();

  //WM_MULTIBUF_Enable(1);
  //
  // Check if recommended memory for the sample is available
  //
  #if 0
  if (GUI_ALLOC_GetNumFreeBytes() < RECOMMENDED_MEMORY) {
    GUI_ErrorOut("Not enough memory available."); 
    return;
  }
  #endif
  _DrawScale();
  while(1); 
}

/*************************** End of file ****************************/

