/*
*********************************************************************************************************
*	                                  
*	模块名称 : GUI界面主函数
*	文件名称 : MainTask.c
*	版    本 : V1.0
*	说    明 : GUI界面主函数
*		版本号   日期         作者            说明
*		v1.0    2015-08-05  Eric2013  	      首版
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __MainTask_H
#define __MainTask_H

#include "GUI.h"
#include "DIALOG.h"
#include "WM.h"
#include "BUTTON.h"
#include "CHECKBOX.h"
#include "DROPDOWN.h"
#include "EDIT.h"
#include "FRAMEWIN.h"
#include "LISTBOX.h"
#include "MULTIEDIT.h"
#include "RADIO.h"
#include "SLIDER.h"
#include "TEXT.h"
#include "PROGBAR.h"
#include "SCROLLBAR.h"
#include "LISTVIEW.h"
#include "GRAPH.h"
#include "MENU.h"
#include "MULTIPAGE.h"
#include "ICONVIEW.h"
#include "TREEVIEW.h"

#include "ff.h"

/*
************************************************************************
*						  FatFs
************************************************************************
*/
extern FRESULT result;
extern FIL file;
extern DIR DirInf;
extern UINT bw;
extern FATFS fs;

extern void _WriteByte2File(U8 Data, void * p); 
/*
************************************************************************
*						供外部文件调用
************************************************************************
*/
extern void MainTask(void);
extern void TOUCH_Calibration(void);

typedef struct {
  int x;
  int y;
  int Pressed;
  int Duration;
} PID_EVENT;

typedef struct {
  int x;
  int y;
  int xHere;
  int yHere;
  int DirX;
  int DirY;
  int PPM;
  int Dif;
  int PrevTime;
  const GUI_BITMAP* pBitmap;
} NAVIMAP;

typedef struct {
  U8  Sep[4];
} COLOR;

typedef struct {
  int x;
  int y;
  int x0;
  int y0;
  int ArcStart;
  int ArcEnd;
  int ArcRadius;
  int ArcArea1;
  int ArcArea2;
  int NumMarkLines;
  int NumPitchLines;
  int LineLen1;
  int LineLen2;
  int ArcWidth;
  int GradDist;
  int PenSize1;
  int PenSize2;
  int PenSize3;
  int NeedleType;
  int NeedleRadius;
  int NumStep;
  int NumStart;
  int NumExp;
  int LinePos1;
  int LinePos2;
  int ArcPos;
  int AxisRadius;
  int TextDist;
  U16 Flags;
  WM_HWIN hWin;
  GUI_MEMDEV_Handle hMemDev;
  char acText[33];
  const GUI_BITMAP* pBitmap;
  int BitmapY;
  COLOR Color[7];
} SCALE;

typedef struct {
  int NeedlePos;
  int NeedleUPM;
  int NeedleDir;
  int NeedlePrevTime;
} NEEDLE;

extern SCALE  _Scale[4];
extern SCALE  _ScalePrev[4];

extern NEEDLE  _Needle[4];
extern const SCALE _Presets[4][4];
extern GUI_MEMDEV_Handle _hBkMemDev;

extern void _CreateScaleWindow(SCALE* pObj);
extern void _MoveNeedle(NEEDLE* pObj, int Index);
extern void _SetPreset(int Preset, int Scale);

/*
************************************************************************
*						宏定义
************************************************************************
*/
#define   GUI_KEY_NextPage       42


#endif

/*****************************(END OF FILE) *********************************/
