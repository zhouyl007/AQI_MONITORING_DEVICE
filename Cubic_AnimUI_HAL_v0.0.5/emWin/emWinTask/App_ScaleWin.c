/*
*********************************************************************************************************
*	                                  
*	ģ������ : �๦���Ǳ�������
*	�ļ����� : App_ScaleWin.c
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
#include <stdlib.h>
#include <math.h>
#include <string.h>


/*
**********************************************************************************************************
							                �궨��
**********************************************************************************************************
*/
#define SHOW_RECTS  0

#define AA_FACTOR   3
#define HIRES       1

#if HIRES
  #define FACTOR        AA_FACTOR
  #define HIRES_ON();   GUI_AA_EnableHiRes();
  #define HIRES_OFF();  GUI_AA_DisableHiRes();
#else
  #define FACTOR        1
  #define HIRES_ON();
  #define HIRES_OFF();
#endif

#define FLAG_SHOW_MARK      0
#define FLAG_SHOW_PITCH     1
#define FLAG_SHOW_GRAD      2
#define FLAG_SHOW_ARC       3   // needs five bits (3 - 7)
#define FLAG_SHOW_TEXT      8
#define FLAG_SHOW_SCALE     9
#define FLAG_NEEDLE_FRAME  10
#define FLAG_NEEDLE_LINE   11

#define PI  3.1415926536

#define NEEDLE_GRAD  720

#define ARRAY(aItems) aItems, GUI_COUNTOF(aItems)


static void _DrawScale(SCALE* pObj);


/*
**********************************************************************************************************
							                ��ֵ������
**********************************************************************************************************
*/
SCALE  _Scale[4];
SCALE  _ScalePrev[4];

NEEDLE  _Needle[4] = {
  {0, 5, 1, 0},
  {0, 6, 1, 0},
  {0, 4, 1, 0},
  {0, 2, 1, 0}
};

const int _Pow10[] = {1, 10, 100, 1000};

/*
**********************************************************************************************************
                     �����������Ǳ���ʽ��ÿ������������Դ����ĸ��Ǳ����
**********************************************************************************************************
*/
const SCALE _Presets[4][4] = {
  {
    {
       87, 250, 0, 0,  89, 225,  62,  41, 122,  4, 2, 15,  7, 15, 24,
        2, 2, 2, 1, 100, 1, 0, 0, 0, 0, 0, 16, 25, 0x37B, 0, 0, "Fuel", 0, 0,
      {{{0xFF, 0xFF, 0xFF}}, {{0x00, 0xFF, 0x00}}, {{0xFF, 0x64, 0x00, 0xFF}},
       {{0x98, 0x00, 0x00}}, {{0x90, 0x90, 0x00}}, {{0x00, 0x50, 0x00}}, {{0xE8, 0xE8, 0xFF}}}
    }, {
      144, 136, 0, 0,  72, 225,  70,  93, 184,  3, 1, 15,  7, 15, 24,
        2, 2, 2, 1, 100, 1, 0, 0, 0, 0, 0, 16, 25, 0x37B, 0, 0, "Oil", 0, 0,
      {{{0xFF, 0xFF, 0xFF}}, {{0x00, 0xFF, 0x00}}, {{0xFF, 0x64, 0x00, 0xFF}},
       {{0x98, 0x00, 0x00}}, {{0x90, 0x90, 0x00}}, {{0x00, 0x50, 0x00}}, {{0xE8, 0xE8, 0xFF}}}
    }, {
      335, 185, 0, 0,  58, 234, 149, 258, 335, 15, 4, 25, 10, 25, 29,
        2, 2, 2, 4, 100, 1, 1, 1, 0, 5, 0, 16, 25, 0x77F, 0, 0, "Speed", 0, 0,
      {{{0xFF, 0xFF, 0xFF}}, {{0x00, 0xFF, 0x00}}, {{0x00, 0x8A, 0xFF, 0x88}},
       {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xAA, 0x00, 0x00}}, {{0xE8, 0xE8, 0xFF}}}
    }, {
      522, 208, 0, 0,  59, 225, 108, 257, 311,  8, 4, 25, 10, 25, 34,
        2, 2, 2, 3, 100, 1, 0, 0, 0, 0, 0, 16, 25, 0x77F, 0, 0, "RPM", 0, 0,
      {{{0xFF, 0xFF, 0xFF}}, {{0x00, 0xFF, 0x00}}, {{0xFF, 0xAA, 0x00, 0xB5}},
       {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xAA, 0x00, 0x00}}, {{0xE8, 0xE8, 0xFF}}}
    }
  }, {
    {
       96, 208, 0, 0, 40, 160,  73,  41, 122,  4, 2, 15,  7, 15, 24,
        2, 2, 2, 3, 100, 1, 0, 0, 0, 0, 0, 16, 22, 0x74B, 0, 0, "Fuel", 0, 0,
      {{{0xFF, 0xFF, 0xAC}}, {{0x00, 0xFF, 0x70}}, {{0xFF, 0x64, 0x64, 0x80}},
       {{0x98, 0x00, 0x00}}, {{0x90, 0x90, 0x00}}, {{0x00, 0x50, 0x00}}, {{0xE8, 0xE8, 0xFF}}}
    }, {
      395, 247, 0, 0, 90, 270,  75,  93, 184,  3, 1, 15,  7, 15, 24,
        2, 2, 2, 1, 100, 1, 0, 0, 0, 0, 0, 16, 22, 0x77B, 0, 0, "Oil", 0, 0,
      {{{0xFF, 0xFF, 0xAC}}, {{0x00, 0xFF, 0x70}}, {{0xFF, 0x64, 0x64, 0x80}},
       {{0x98, 0x00, 0x00}}, {{0x90, 0x90, 0x00}}, {{0x00, 0x50, 0x00}}, {{0xE8, 0xE8, 0xFF}}}
    }, {
      272, 184, 0, 0,  58, 254, 156, 261, 327, 12, 4, 19, 10, 17, 42,
        2, 2, 2, 0, 100, 2, 1, 1, 17, 22, 0, 16, 22, 0x7C7, 0, 0, "Speed", 0, 0,
      {{{0xFF, 0xFF, 0xAC}}, {{0x00, 0xFF, 0x70}}, {{0x00, 0xCA, 0xFF, 0x68}},
       {{0x00, 0x8C, 0x00}}, {{0xC4, 0xC4, 0x00}}, {{0xD8, 0x00, 0x00}}, {{0x75, 0xFF, 0xFF}}}
    }, {
      511, 208, 0, 0, 128, 270, 107, 257, 311,  8, 4, 25, 10, 25, 34,
        2, 2, 2, 3, 100, 1, 0, 0, 0, 0, 0, 16, 22, 0x777, 0, 0, "RPM", 0, 0,
      {{{0xFF, 0xFF, 0xAC}}, {{0x00, 0xFF, 0x70}}, {{0xFF, 0xAA, 0x00, 0xB5}},
       {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xAA, 0x00, 0x00}}, {{0xE8, 0xE8, 0xFF}}}
    }
  }, {
    {
      101, 212, 0, 0,  30, 270,  66,  60, 120,  5, 1, 15,  7, 13, 34,
        2, 2, 2, 0, 100, 1, 0, 0, 0, 0, 0, 16, 25, 0xF43, 0, 0, "Fuel", 0, 0,
      {{{0xFF, 0xFF, 0xFF}}, {{0x00, 0xFF, 0x00}}, {{0xA0, 0x7C, 0xFF, 0xA0}},
       {{0x90, 0x00, 0x00}}, {{0x80, 0x80, 0x00}}, {{0x00, 0x70, 0x00}}, {{0xFF, 0xFF, 0xFF}}}
    }, {
      543, 215, 0, 0,  30, 270,  66, 240, 320,  5, 1, 15,  7, 15, 34,
        2, 2, 2, 0, 100, 1, 0, 0, 0, 0, 0, 16, 25, 0xF43, 0, 0, "Oil", 0, 0,
      {{{0xFF, 0xFF, 0xFF}}, {{0x00, 0xFF, 0x00}}, {{0xA0, 0x7C, 0xFF, 0xA0}},
       {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xAA, 0x00, 0x00}}, {{0xFF, 0xFF, 0xFF}}}
    }, {
      325, 155, 0, 0,  30, 270,  95, 269, 317,  5, 1, 16,  8,  9, 34,
        2, 2, 2, 1, 100, 1, 0, 0, 0, 0, 0, 16, 40, 0x773, 0, 0, "Speed", 0, 0,
      {{{0xFF, 0xFF, 0xFF}}, {{0x00, 0xFF, 0x00}}, {{0xFF, 0x4C, 0x38, 0xA0}},
       {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xAA, 0x00, 0x00}}, {{0xFF, 0x64, 0x64}}}
    }, {
      325, 155, 0, 0,  30, 270, 137, 228, 329, 10, 1, 16,  9, 25, 21,
        2, 2, 2, 0, 100, 2, 1, 1, 0, 0, 0, 16, 16, 0x747, 0, 0, "RPM", 0, 0,
      {{{0xFF, 0xFF, 0xFF}}, {{0x00, 0xFF, 0x00}}, {{0x00, 0x8C, 0xFF, 0xA4}},
       {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xAA, 0x00, 0x00}}, {{0xFF, 0x64, 0x64}}}
    }
  }, {
    {
      102, 251, 0, 0,  90, 270,  62,  60, 120,  0, 5, 15,  7, 13, 24,
        2, 2, 2, 4, 100, 1, 0, 0, 0, 0, 0, 20, 25, 0x77F, 0, 0, "Fuel", 0, 0,
      {{{0xFF, 0xFF, 0xC0}}, {{0x00, 0xFF, 0xFF}}, {{0x80, 0xFF, 0x00, 0x80}},
       {{0xAA, 0x00, 0x00}}, {{0x90, 0x90, 0x00}}, {{0x00, 0x70, 0x00}}, {{0xFF, 0xFF, 0xFF}}}
    }, {
      292, 251, 0, 0,  90, 270,  62,  90, 270,  0, 3, 15,  7, 15, 24,
        2, 2, 2, 4, 100, 1, 0, 0, 0, 0, 0, 20, 25, 0x757, 0, 0, "Oil", 0, 0,
      {{{0xFF, 0xFF, 0xC0}}, {{0x00, 0xFF, 0xFF}}, {{0x80, 0xFF, 0x00, 0x80}},
       {{0x90, 0x90, 0x00}}, {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xFF, 0xFF, 0xFF}}}
    }, {
      447, 192, 0, 0, 123, 291, 160, 270, 331, 13, 4, 13,  5,  7, 13,
        2, 2, 2, 2,  80, 2, 1, 1, 35, 44, 50, 16, 25, 0x73F, 0, 0, "Speed", 0, 0,
      {{{0xFF, 0xFF, 0xC0}}, {{0x00, 0xFF, 0xFF}}, {{0x95, 0x64, 0xFF, 0xB0}},
       {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xAA, 0x00, 0x00}}, {{0xFF, 0xFF, 0xFF}}}
    }, {
      197, 144, 0, 0,  81, 255, 124, 257, 311,  8, 1, 13,  5,  7, 10,
        2, 2, 2, 2,  80, 1, 0, 0, 25, 34, 40, 16, 25, 0x73F, 0, 0, "RPM", 0, 0,
      {{{0xFF, 0xFF, 0xC0}}, {{0x00, 0xFF, 0xFF}}, {{0x95, 0x64, 0xFF, 0xB0}},
       {{0x00, 0x70, 0x00}}, {{0x90, 0x90, 0x00}}, {{0xAA, 0x00, 0x00}}, {{0xFF, 0xFF, 0xFF}}}
    }
  }
};

/*
**********************************************************************************************************
                                  �Ǳ������ָ������
**********************************************************************************************************
*/
static GUI_POINT _aNeedle[5];

static const GUI_POINT _aNeedleSrc[5][5] = 
{
	{
		{ FACTOR *  0, FACTOR *  -5},
		{ FACTOR * -5, FACTOR *  25},
		{ FACTOR *  0, FACTOR * 100},
		{ FACTOR *  5, FACTOR *  25}
	},
	
	{
		{ FACTOR * -4, FACTOR *   0},
		{ FACTOR * -3, FACTOR *  60},
		{ FACTOR *  0, FACTOR * 100},
		{ FACTOR *  3, FACTOR *  60},
		{ FACTOR *  4, FACTOR *   0}
	},
	
	{
		{ FACTOR * -3, FACTOR * -13},
		{ FACTOR * -3, FACTOR *  60},
		{ FACTOR *  0, FACTOR * 100},
		{ FACTOR *  3, FACTOR *  60},
		{ FACTOR *  3, FACTOR * -13}
	},
	
	{
		{ FACTOR * -5, FACTOR * -13},
		{ FACTOR * -4, FACTOR *  20},
		{ FACTOR *  0, FACTOR * 100},
		{ FACTOR *  4, FACTOR *  20},
		{ FACTOR *  5, FACTOR * -13}
	},
	
	{
		{ FACTOR * -5, FACTOR * -13},
		{ FACTOR * -4, FACTOR *  65},
		{ FACTOR *  0, FACTOR * 100},
		{ FACTOR *  4, FACTOR *  65},
		{ FACTOR *  5, FACTOR * -13}
	}
};

/*
*********************************************************************************************************
*	�� �� ��: _GetArcLen
*	����˵��: ��ñ��̵ĽǶ�
*	��    ��: pObj   ���̽ṹ��ָ�����   
*	�� �� ֵ: ���̵ĽǶ�
*********************************************************************************************************
*/
static int _GetArcLen(const SCALE* pObj) 
{
	if (pObj->ArcStart > pObj->ArcEnd) 
	{
		return 360 - (pObj->ArcStart - pObj->ArcEnd);
	} 
	else 
	{
		return pObj->ArcEnd - pObj->ArcStart;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: _MagnifyPolygon
*	����˵��: ������������
*	��    ��: pDest     ������������ַ
*             pSrc      ԭʼ�����ַ
*             NumPoints ������
*             Mag       ��������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _MagnifyPolygon(GUI_POINT* pDest, const GUI_POINT* pSrc, int NumPoints, float Mag) 
{
	int i;

	for (i=0; i < NumPoints; i++) 
	{
		(pDest+i)->x = (int)((pSrc+i)->x * Mag);
		(pDest+i)->y = (int)((pSrc+i)->y * Mag);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: _CalcNeedle
*	����˵��: ����ָ����ת�������λ��
*	��    ��: pObj   ���̽ṹ��ָ�����
*             Index  ���������ڿ���ָ������
*	�� �� ֵ: ָ�����ε��������
*********************************************************************************************************
*/
static int _CalcNeedle(const SCALE* pObj, int Index, int Radius) 
{
	int NumPoints, Shape;
	float Angel;
	
	Shape = pObj->NeedleType;
	NumPoints = GUI_COUNTOF(_aNeedleSrc[Shape]);
	
	/* ����ָ��Ҫ��ת�ĽǶȣ������� */
	Angel = -((pObj->ArcStart * PI) + (_GetArcLen(pObj) * _Needle[Index].NeedlePos * PI) / NEEDLE_GRAD) / 180;
	
	/* ���ﲻ֪��Ϊʲô��С��100�� */
	_MagnifyPolygon(_aNeedle, _aNeedleSrc[Shape], NumPoints, Radius / 100.);
	GUI_RotatePolygon(_aNeedle, _aNeedle, NumPoints, Angel);
	
	return NumPoints;
}

/*
*********************************************************************************************************
*	�� �� ��: _CalcPointX
*	����˵��: ���ݰ뾶�ͽǶȻ��X������λ��
*             cos(A-B)=cosAcosB+sinAsinB  ===��sin(Angel)
*	��    ��: r      �뾶
*             Angel  �Ƕ�
*	�� �� ֵ: X������λ��
*********************************************************************************************************
*/
static int _CalcPointX(int r, int Angel) 
{
	return (int)(r * cos((Angel - 90) * PI / 180.0f));
}

/*
*********************************************************************************************************
*	�� �� ��: _CalcPointY
*	����˵��: ���ݰ뾶�ͽǶȻ��Y������λ��
*             sin(A-B)=sinAcosB-sinBcosA ===��-cos(Angel)
*	��    ��: r      �뾶
*             Angel  �Ƕ�
*	�� �� ֵ: Y������λ��
*********************************************************************************************************
*/
static int _CalcPointY(int r, int Angel) 
{
	return (int)(r * sin((Angel - 90) * PI / 180.0f));
}

/*
*********************************************************************************************************
*	�� �� ��: _Max
*	����˵��: ����������ݵ����ֵ
*	��    ��: a   ��ֵ
*             b   ��ֵ
*	�� �� ֵ: ���ֵ
*********************************************************************************************************
*/
static int _Max(int a, int b) 
{
	return((a > b) ? a : b);
}

/*
*********************************************************************************************************
*	�� �� ��: _Max3
*	����˵��: ����������ݵ����ֵ
*	��    ��: a   ��ֵ
*             b   ��ֵ
*             c   ��ֵ
*	�� �� ֵ: ���ֵ
*********************************************************************************************************
*/
static int _Max3(int a, int b, int c) 
{
	int r;
	
	r = (a > b) ? a : b;
	r = (r > c) ? r : c;
	
	return r;
}

/*
*********************************************************************************************************
*	�� �� ��: _Min
*	����˵��: ����������ݵ���Сֵ
*	��    ��: a   ��ֵ
*             b   ��ֵ
*	�� �� ֵ: ��Сֵ
*********************************************************************************************************
*/
static int _Min(int a, int b) 
{
	return((a < b) ? a : b);
}

/*
*********************************************************************************************************
*	�� �� ��: _GetNeedleRect
*	����˵��: ��ȡ����ָ��ռ�õľ�������
*	��    ��: pObj    ���̽ṹ��ָ�����
*             Index   ���������ڿ���ָ������
*             pRect   ��������ṹ��ָ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _GetNeedleRect(const SCALE* pObj, int Index, GUI_RECT* pRect) 
{
    int NumPoints;
	int i;
	int x;
	int y;
	int r;
	int x0;
	int y0;
	int x1;
	int y1;

	x0 =  4096;
	y0 =  4096;
	x1 = -4096;
	y1 = -4096;
	
	r = (pObj->ArcRadius + pObj->PenSize3) * pObj->NeedleRadius / 100;
	
	NumPoints = _CalcNeedle(pObj, Index, r);
	
	for (i = 0; i < NumPoints; i++) 
	{
		x = _aNeedle[i].x / FACTOR;
		y = _aNeedle[i].y / FACTOR;
		x0 = _Min(x0, x);
		y0 = _Min(y0, y);
		x1 = _Max(x1, x);
		y1 = _Max(y1, y);
	}
	
	pRect->x0 = pObj->x0 + x0 - 1;
	pRect->y0 = pObj->y0 + y0 - 1;
	pRect->x1 = pObj->x0 + x1 + 1;
	pRect->y1 = pObj->y0 + y1 + 1;
}

/*
*********************************************************************************************************
*	�� �� ��: _MergeRects
*	����˵��: ������������ϲ���ռ�õ�����������
*	��    ��: pR1   ��������1�ṹ��ָ�����
*             pR2   ��������2�ṹ��ָ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _MergeRects(GUI_RECT* pR1, const GUI_RECT* pR2) 
{
	pR1->x0 = _Min(pR1->x0, pR2->x0);
	pR1->y0 = _Min(pR1->y0, pR2->y0);
	pR1->x1 = _Max(pR1->x1, pR2->x1);
	pR1->y1 = _Max(pR1->y1, pR2->y1);
}

/*
*********************************************************************************************************
*	�� �� ��: _MoveNeedle
*	����˵��: ת������ָ��
*	��    ��: pObj    ���̽ṹ��ָ�����
*             Index   ���������ڿ���ָ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void _MoveNeedle(NEEDLE* pObj, int Index) 
{
	GUI_RECT rOld;
	GUI_RECT rNew;
	int      Dif;
	int      Time;

	if (pObj->NeedleUPM) 
	{
		_GetNeedleRect(&_Scale[Index], Index, &rOld);
		Time = GUI_GetTime();
		Dif = (Time - pObj->NeedlePrevTime) / (60000 / pObj->NeedleUPM / NEEDLE_GRAD);
		if (Dif != 0) 
		{
			pObj->NeedlePos += (Dif * pObj->NeedleDir);
			if (pObj->NeedlePos > NEEDLE_GRAD) 
			{
				pObj->NeedlePos = NEEDLE_GRAD;
				pObj->NeedleDir = -pObj->NeedleDir;
			} 
			else 
			{
				if (pObj->NeedlePos < 0) 
				{
					pObj->NeedlePos = 0;
					pObj->NeedleDir = -pObj->NeedleDir;
				}
			}
			
			_GetNeedleRect(&_Scale[Index], Index, &rNew);
			_MergeRects(&rNew, &rOld);
			WM_InvalidateRect(_Scale[Index].hWin, &rNew);
			pObj->NeedlePrevTime = Time;
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: _InvalidateScale
*	����˵��: ɾ������ʹ�õĴ洢�豸
*	��    ��: pObj    ���̽ṹ��ָ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _InvalidateScale(SCALE* pObj) 
{
	if (pObj->hMemDev) 
	{
		GUI_MEMDEV_Delete(pObj->hMemDev);
		pObj->hMemDev = 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: _ChangeScaleSize
*	����˵��: ���±���
*	��    ��: pObj    ���̽ṹ��ָ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _ChangeScaleSize(SCALE* pObj) 
{
	int x0;
	int y0;
	int x;
	int y;
	int w;
	int h;
	int ArcStart;
	int ArcEnd;
	int rOff;
	int PenSize;
	int r;
	int rNeedle;
	int BitmapW;
	int BitmapY0;
	int BitmapY1;
	int TextW;
	int TextY0;
	int TextY1;
	
	//  
	// Calculate text position
	//
	GUI_SetFont(&GUI_Font8x8);
	
	if (pObj->Flags & (1 << FLAG_SHOW_TEXT)) 
	{
		TextW  = GUI_GetStringDistX(pObj->acText) / 2;
		TextY0 = (pObj->ArcRadius * pObj->TextDist / 100);
		TextY1 = TextY0 + 8;
		if (pObj->TextDist < 0) 
		{
			TextY0 = -TextY0;
			TextY1 = _Max(TextY1, 0);
		} 
		else 
		{
			TextY0 = _Max(-TextY0, 0);
		}
	} 
	else 
	{
		TextW  = 0;
		TextY0 = 0;
		TextY1 = 0;
	}
	
	// 
	// Calculate radius of scale
	//
	PenSize  = _Max3(pObj->PenSize1, pObj->PenSize2, pObj->PenSize3);
	r        = pObj->ArcRadius + PenSize;
	
	// 
	// Calculate radius of needle
	//
	rNeedle  = pObj->ArcRadius * pObj->AxisRadius / 200;
	if (pObj->NeedleType > 1) 
	{
		rNeedle  = _Max(rNeedle, (pObj->NeedleRadius * 22) / 100 + 1);
	}
	
	//  
	// Get arcbow
	//
	ArcStart = pObj->ArcStart;
	ArcEnd   = pObj->ArcEnd;
	
	// 
	// Calculate bitmap position
	//
	if (pObj->pBitmap) 
	{
		BitmapW  = (pObj->pBitmap->XSize / 2) + 1;
		BitmapY0 = -(pObj->pBitmap->YSize / 2) + (pObj->ArcRadius / pObj->BitmapY);
		BitmapY1 = BitmapY0 + pObj->pBitmap->YSize;
		if (pObj->BitmapY < 0) 
		{
			BitmapY0 = -BitmapY0;
			BitmapY1 = _Max(BitmapY1, 0);
		} 
		else 
		{
			BitmapY0 = _Max(-BitmapY0, 0);
		}
	} 
	else 
	{
		BitmapW  = 0;
		BitmapY0 = 0;
		BitmapY1 = 0;
	}
	
	// 
	// Calculate window heigh
	//
	if (ArcStart >= ArcEnd) 
	{
		h = r * 2 + 2;
	} 
	else 
	{
		rOff = _Max3(rNeedle, TextY1, BitmapY1);
		h = _Max(r + rOff, r - _CalcPointY(pObj->ArcRadius, ArcStart) + PenSize + 2);
		h = _Max(h,        r - _CalcPointY(pObj->ArcRadius, ArcEnd)   + PenSize + 2);
	}
	
	// 
	// Calculate window width
	//
	if (ArcStart < 270 && (ArcEnd > 270 || ArcEnd <= ArcStart)) 
	{
		w = r * 2 + 2;
	} 
	else 
	{
		rOff = _Max3(rNeedle, TextW, BitmapW);
		w = _Max(r + rOff, r - _CalcPointX(pObj->ArcRadius, ArcStart) + PenSize + 2);
		w = _Max(w,        r - _CalcPointX(pObj->ArcRadius, ArcEnd)   + PenSize + 2);
	}
	
	// 
	// Calculate y-position of window
	//
	if (((ArcStart < ArcEnd) && ((ArcStart > 180 && ArcEnd > 180) || (ArcStart < 180 && ArcEnd < 180))) ||
	(ArcStart > 180 && ArcEnd < 180)) 
	{
		rOff = _Max3(rNeedle, TextY0, BitmapY0);
		y = _Min(r - rOff, r - _CalcPointY(pObj->ArcRadius, ArcStart) - PenSize - 2);
		y = _Min(y,        r - _CalcPointY(pObj->ArcRadius, ArcEnd)   - PenSize - 2);
		y0        = pObj->y - r + y;
		pObj->y0  = r - y;
		h        -= y;
	} 
	else
	{
		y0        = pObj->y - r - 2;
		pObj->y0  = r + 2;
		h        += 2;
	}
	
	// 
	// Calculate x-position of window
	//
	if (((ArcStart < ArcEnd) && ((ArcStart > 90 && ArcEnd > 90) || (ArcStart < 90 && ArcEnd < 90))) ||
	(ArcStart > 90 && ArcEnd < 90)) 
	{
		rOff = _Max3(rNeedle, TextW, BitmapW);
		x = _Min(r - rOff, r - _CalcPointX(pObj->ArcRadius, ArcStart) - PenSize - 2);
		x = _Min(x,        r - _CalcPointX(pObj->ArcRadius, ArcEnd)   - PenSize - 2);
		x0        = pObj->x - r + x;
		pObj->x0  = r - x;
		w        -= x;
	} 
	else 
	{
		x0        = pObj->x - r - 2;
		pObj->x0  = r + 2;
		w        += 2;
	}
	
	// 
	// Set new window rect
	//
	_InvalidateScale(pObj);
	WM_MoveTo(pObj->hWin, x0, y0);
	WM_SetSize(pObj->hWin, w, h);
	WM_InvalidateWindow(pObj->hWin);
}

/*
*********************************************************************************************************
*	�� �� ��: _SetPreset
*	����˵��: ���ڸı���̵����
*	��    ��: Preset  ֧��4�ֱ�����ۣ���ȡֵ0,1,2,3��
*             Scale   ����ָ��Ҫ�����ı��̴��ڣ���ȡֵ0,1,2,3�����ȡֵΪ-1����ʾ�ı����д������ 
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void _SetPreset(int Preset, int Scale) 
{
	int i;
	int iStart;
	int iEnd;

	if (Preset >= 0 && Preset <= 3) 
	{
		WM_HWIN OldhWin;
		
		iStart = (Scale == -1) ? 0 : Scale;
		iEnd   = (Scale == -1) ? 3 : Scale;
		
		for (i = iStart; i <= iEnd; i++) 
		{
			/* ɾ��֮ǰ�����Ĵ洢�豸 */
			_InvalidateScale(&_Scale[i]);
			
			/* ��ʱ������̴��ھ�� */
			OldhWin = _Scale[i].hWin;

			/* �����̽ṹ�������ֵ�µĲ��� */
			_Scale[i] = _Presets[Preset][i];

			/* ʹ�øոձ���ı��̴��ھ����ֵ */
			_Scale[i].hWin = OldhWin;

			/*���������µı��̴��� */
			_ChangeScaleSize(&_Scale[i]);
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: _CalcColor
*	����˵��: �����ֽ���ʽ���ڵ���ɫֵת����24λ��ɫ
*	��    ��: pColor ��ɫ�ṹ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static GUI_COLOR _CalcColor(const COLOR* pColor) 
{
	GUI_COLOR r;

	r  = ((U32) pColor->Sep[0]);
	r += ((U32) pColor->Sep[1]) << 8;
	r += ((U32) pColor->Sep[2]) << 16;
	
	return r;
}

/*
*********************************************************************************************************
*	�� �� ��: _IntToString
*	����˵��: ��������ת�ַ���
*	��    ��: pStr   ת������ַ���
*             Value  ת��ǰ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _IntToString(char* pStr, int Value) 
{
	char* Ptr;

	Ptr = pStr + 6;
	*(--Ptr) = 0;
	
	Value = _Min(Value, 32767);
	do 
	{
		*(--Ptr) = (Value % 10) + '0';
		Value /= 10;
	} while (Value != 0);
	
	strcpy(pStr, Ptr);
}

/*
*********************************************************************************************************
*	�� �� ��: _DrawLine
*	����˵��: ���Ʊ�������Ŀ̶���
*	��    ��: pObj    ���̽ṹ��ָ�����
*             r1      �̶���ʼ�뾶
*             r2      �̶Ƚ����뾶
*             Angel   �̶��߽Ƕ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DrawLine(const SCALE* pObj, int r1, int r2, float Angel) 
{
	float co = cos(Angel / 180.0f) * FACTOR;
	float si = sin(Angel / 180.0f) * FACTOR;
	int x0 = (int)(pObj->x0 * FACTOR - r1 * co);
	int y0 = (int)(pObj->y0 * FACTOR - r1 * si);
	int x1 = (int)(pObj->x0 * FACTOR - r2 * co);
	int y1 = (int)(pObj->y0 * FACTOR - r2 * si);
	
	GUI_AA_DrawLine(x0, y0, x1, y1);
}

/*
*********************************************************************************************************
*	�� �� ��: _DrawLines
*	����˵��: ���Ʊ�������Ŀ̶���
*	��    ��: pObj    ���̽ṹ��ָ�����
*             iEnd    Ҫ���ƿ̶��ߵĸ���
*             rStart  �̶���ʼ�뾶
*             rEnd    �̶Ƚ����뾶
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DrawLines(const SCALE* pObj, int iEnd, int rStart, int rEnd) 
{
	int i;
	int ArcLen;
	float Angel;

	ArcLen = _GetArcLen(pObj);
	GUI_SetColor(_CalcColor(&pObj->Color[0]));
	HIRES_ON();
	
	for (i = 0; i <= iEnd; i++) 
	{
		Angel = (i * ArcLen * PI) / _Max(iEnd, 1) + (pObj->ArcStart - 90.) * PI;
		_DrawLine(pObj, rStart, rEnd, Angel);
	}
	HIRES_OFF();
}

/*
*********************************************************************************************************
*	�� �� ��: _DrawPitchLines
*	����˵��: ���Ʊ����ϵ�С�̶�
*	��    ��: pObj    ���̽ṹ��ָ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DrawPitchLines(const SCALE* pObj) 
{
	int iEnd;
	int rStart;
	int rEnd;

	if (pObj->NumPitchLines > 0 && (pObj->Flags & (1 << FLAG_SHOW_PITCH))) 
	{
		iEnd = _Max(pObj->NumMarkLines - 1, 1) * (pObj->NumPitchLines + 1);
		rStart = pObj->ArcRadius - pObj->LinePos2;
		rEnd = rStart - pObj->LineLen2;
		GUI_SetPenSize(pObj->PenSize2);
		_DrawLines(pObj, iEnd, rStart, rEnd);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: _DrawMarkLines
*	����˵��: ���Ʊ����ϵĴ�̶�
*	��    ��: pObj    ���̽ṹ��ָ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DrawMarkLines(const SCALE* pObj) 
{
	int iEnd;
	int rStart;
	int rEnd;

	if (pObj->NumMarkLines > 0 && (pObj->Flags & (1 << FLAG_SHOW_MARK))) 
	{
		iEnd = pObj->NumMarkLines - 1;
		rStart = pObj->ArcRadius - pObj->LinePos1;
		rEnd = rStart - pObj->LineLen1;
		GUI_SetPenSize(pObj->PenSize1);
		_DrawLines(pObj, iEnd, rStart, rEnd);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: _DrawGrad
*	����˵��: ���Ʊ����Ͽ̶�ֵ
*	��    ��: pObj    ���̽ṹ��ָ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DrawGrad(const SCALE* pObj) 
{
	int   xt;
	int   yt;
	int   w;
	int   h;
	int   i;
	int   ArcLen;
	int   rStart;
	int   Cnt;
	int   wMax = 0;
	float co;
	float si;
	float Angel;
	char  acText[6];

	if (pObj->NumMarkLines > 0 && (pObj->Flags & (1 << FLAG_SHOW_GRAD))) 
	{
		GUI_SetColor(_CalcColor(&pObj->Color[1]));
		GUI_SetFont(&GUI_Font6x8);
		GUI_SetTextMode(GUI_TEXTMODE_TRANS);
		Cnt = pObj->NumStart * (pObj->NumStep * _Pow10[pObj->NumExp]);
		for (i = 0; i < pObj->NumMarkLines; i++) 
		{
			_IntToString(acText, Cnt);
			wMax = _Max(GUI_GetStringDistX(acText), wMax);
			Cnt += pObj->NumStep * _Pow10[pObj->NumExp];
		}
		
		ArcLen = _GetArcLen(pObj);
		rStart = pObj->ArcRadius - pObj->GradDist - (wMax / 2);
		Cnt = pObj->NumStart * (pObj->NumStep * _Pow10[pObj->NumExp]);
		for (i = 0; i < pObj->NumMarkLines; i++) 
		{
			Angel = (i * ArcLen * PI) / _Max(pObj->NumMarkLines - 1, 1) + (pObj->ArcStart - 90.) * PI;
			co = cos(Angel / 180.0f);
			si = sin(Angel / 180.0f);
			xt = (int)(pObj->x0 - rStart * co);
			yt = (int)(pObj->y0 - rStart * si);
			_IntToString(acText, Cnt);
			w = GUI_GetStringDistX(acText);
			h = (int)(si * _Max(wMax / 2 - 4, 0) + 4);
			GUI_DispStringAt(acText, xt - w / 2 + 1, yt - h + 1);
			Cnt += pObj->NumStep * _Pow10[pObj->NumExp];
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: _DrawText
*	����˵��: ��ʾ�ı��������̵�����
*	��    ��: pObj    ���̽ṹ��ָ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DrawText(const SCALE* pObj) 
{
	int x;
	int y;

	if (pObj->Flags & (1 << FLAG_SHOW_TEXT)) 
	{
		GUI_SetColor(_CalcColor(&pObj->Color[1]));
		GUI_SetFont(&GUI_Font8x8);
		GUI_SetTextMode(GUI_TEXTMODE_TRANS);
		x = pObj->x0;
		y = pObj->y0 + (pObj->ArcRadius * pObj->TextDist / 100);
		GUI_DispStringHCenterAt(pObj->acText, x, y);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: _DrawColorArcs
*	����˵��: ���Ʊ����ڲ����ɫ��
*	��    ��: pObj    ���̽ṹ��ָ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DrawColorArcs(const SCALE* pObj) 
{
	int r;
	int Start;
	int Angel1;
	int Angel2;
	int ArcLen;

	r      = pObj->ArcRadius - (pObj->ArcWidth / 2) - pObj->ArcPos;
	ArcLen = _GetArcLen(pObj);
	Start  = (pObj->ArcStart < pObj->ArcEnd) ? 270 - pObj->ArcStart : 630 - pObj->ArcStart;
	Angel1 = pObj->ArcArea1 * ArcLen / 359;
	Angel2 = pObj->ArcArea2 * ArcLen / 359;
	GUI_SetPenSize(pObj->ArcWidth);
	
	/* ��������ɫ��ʵ�� */
	if (pObj->Flags & (1 << (FLAG_SHOW_ARC+0))) 
	{
		GUI_SetColor(_CalcColor(&pObj->Color[3]));
		GUI_AA_DrawArc(pObj->x0, pObj->y0, r, r, Start - Angel1, Start);
	}
	
	if (pObj->Flags & (1 << (FLAG_SHOW_ARC+1))) 
	{
		GUI_SetColor(_CalcColor(&pObj->Color[4]));
		GUI_AA_DrawArc(pObj->x0, pObj->y0, r, r, Start - Angel2, Start - Angel1);
	}
	
	if (pObj->Flags & (1 << (FLAG_SHOW_ARC+2))) 
	{
		GUI_SetColor(_CalcColor(&pObj->Color[5]));
		GUI_AA_DrawArc(pObj->x0, pObj->y0, r, r, Start - ArcLen, Start - Angel2);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: _DrawArcs
*	����˵��: ���Ʊ���������Բ�� 
*	��    ��: pObj    ���̽ṹ��ָ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DrawArcs(const SCALE* pObj) 
{
	int Start;
	int End;
	int r1;
	int r2;

	Start = (pObj->ArcStart < pObj->ArcEnd) ? 270 - pObj->ArcStart : 630 - pObj->ArcStart;
	End   = 270 - pObj->ArcEnd;
	r1    = pObj->ArcRadius;
	r2    = pObj->ArcRadius - pObj->ArcPos - pObj->ArcWidth;
	GUI_SetColor(_CalcColor(&pObj->Color[0]));
	GUI_SetPenSize(pObj->PenSize3);

	if (pObj->Flags & (1 << (FLAG_SHOW_ARC+3))) 
	{
		GUI_AA_DrawArc(pObj->x0, pObj->y0, r1, r1, End, Start);
	}
	
	/* ���ƻ��ε�Բ�� */
	if (pObj->Flags & (1 << (FLAG_SHOW_ARC+4))) 
	{
		GUI_AA_DrawArc(pObj->x0, pObj->y0, r2, r2, End, Start);
		if (pObj->Flags & (1 << (FLAG_SHOW_ARC+3))) 
		{
			GUI_SetPenSize(pObj->PenSize1);
			HIRES_ON();
			_DrawLine(pObj, r1, r2, (pObj->ArcStart - 90.) * PI);
			_DrawLine(pObj, r1, r2, (pObj->ArcEnd   - 90.) * PI);
			HIRES_OFF();
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: _DrawScale
*	����˵��: ���Ʊ��̽���
*	��    ��: pObj    ���̽ṹ��ָ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DrawScale(SCALE* pObj) 
{
	int x;
	int y;

	#if SHOW_RECTS
		GUI_SetBkColor(GUI_GRAY);
		GUI_Clear();
	#endif
	
	if (pObj->ArcStart != pObj->ArcEnd) 
	{
		/* ��ʾλͼ����ʵ��δ�õ� */
		if (pObj->pBitmap) 
		{
			x = pObj->x0 - (pObj->pBitmap->XSize / 2);
			y = pObj->y0 - (pObj->pBitmap->YSize / 2) + (pObj->ArcRadius / pObj->BitmapY);
			GUI_DrawBitmap(pObj->pBitmap, x, y);
		}
		
		GUI_AA_SetFactor(AA_FACTOR);
		GUI_SetPenShape(GUI_PS_ROUND);
		
		/* ���Ʊ����ڲ����ɫ�� */
		_DrawColorArcs(pObj);
		
		/* ���Ʊ���������Բ�� */
		_DrawArcs(pObj);
		
		/* ���Ʊ����ϵ�С�̶� */
		_DrawPitchLines(pObj);
		
		/* ���Ʊ����ϵĴ�̶� */
		_DrawMarkLines(pObj);
		
		/* ���Ʊ����Ͽ̶�ֵ */
		_DrawGrad(pObj);
		
		/* ��ʾ�ı��������̵����� */
		_DrawText(pObj);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: _UpdateScale
*	����˵��: �����͸��±��̽���
*	��    ��: pObj    ���̽ṹ��ָ�����
*             Index   ���������ڿ���ָ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _UpdateScale(int Index) 
{
	int Mod;

	Mod = 0;
	
	/* �����洢�豸 */
	if (_Scale[Index].hMemDev == 0) 
	{
		GUI_RECT r;
		WM_GetWindowRect(&r);
		_Scale[Index].hMemDev = GUI_MEMDEV_CreateEx(r.x0, r.y0, r.x1 - r.x0 + 1, r.y1 - r.y0 + 1, GUI_MEMDEV_HASTRANS);
		Mod = 1;
	}
	
	/* �״δ����洢�豸��Ҫ�����̻��Ƶ��洢�豸
	   ��������и��£������̻��Ƶ��洢�豸������û�и��£������ظ����� 
	*/
	if (Mod | memcmp(&_ScalePrev[Index], &_Scale[Index], sizeof(SCALE))) 
	{
		GUI_MEMDEV_Handle hPrev = GUI_MEMDEV_Select(_Scale[Index].hMemDev);
		GUI_SetBkColor(GUI_BLACK);
		GUI_MEMDEV_Write(_hBkMemDev);
		GUI_MEMDEV_Clear(_Scale[Index].hMemDev);
		_DrawScale(&_Scale[Index]);
		memcpy(&_ScalePrev[Index], &_Scale[Index], sizeof(SCALE));
		GUI_MEMDEV_Select(hPrev);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: _DrawNeedleThread
*	����˵��: ������˺�����ʲô�õ�
*	��    ��: pObj    ���̽ṹ��ָ�����
*             Index   ���������ڿ���ָ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DrawNeedleThread(const SCALE* pObj, int Index) 
{
	int x0;
	int y0;
	int x1;
	int y1;
	int NumPoints;
	int Radius;

	Radius = (pObj->ArcRadius + pObj->PenSize3) * pObj->NeedleRadius / 100;
	GUI_SetPenSize(3);
	NumPoints = _CalcNeedle(pObj, Index, Radius - 3);
	x0 = pObj->x0 * FACTOR;
	y0 = pObj->y0 * FACTOR;
	x1 = x0 + _aNeedle[NumPoints/2].x;
	y1 = y0 + _aNeedle[NumPoints/2].y;
	
	if ((NumPoints % 2) != 0) 
	{
		x0 += (_aNeedle[0].x + _aNeedle[NumPoints - 1].x) / 2;
		y0 += (_aNeedle[0].y + _aNeedle[NumPoints - 1].y) / 2;
	}
	
	GUI_SetColor(GUI_RED);
	GUI_AA_DrawLine(x0, y0, x1, y1);
}

/*
*********************************************************************************************************
*	�� �� ��: _AA_DrawPolygon
*	����˵��: ���Ʊ�������̶��ߵ�ʵ�ʻ��ƺ���
*	��    ��: pObj    ���̽ṹ��ָ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _AA_DrawPolygon(const GUI_POINT* pSrc, int NumPoints, int x0, int y0) 
{
	int i;
	int x;
	int y;
	int xPrev;
	int yPrev;
	U8  OldPenShape;

	xPrev = x0 + (pSrc+NumPoints-1)->x;
	yPrev = y0 + (pSrc+NumPoints-1)->y;
	OldPenShape = GUI_SetPenShape(GUI_PS_FLAT);
	
	for (i = 0; i < NumPoints; i++) 
	{
		x = x0 + (pSrc+i)->x;
		y = y0 + (pSrc+i)->y;
		GUI_AA_DrawLine(xPrev, yPrev, x, y);
		xPrev = x;
		yPrev = y;
	}
	
	GUI_SetPenShape(OldPenShape);
}

/*
*********************************************************************************************************
*	�� �� ��: _DrawNeedleFrame
*	����˵��: ���Ʊ�������ָ��ı߿�
*	��    ��: pObj    ���̽ṹ��ָ�����
*             Index   ���������ڿ���ָ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DrawNeedleFrame(const SCALE* pObj, int Index) 
{
	int NumPoints;
	int Radius;

	Radius = (pObj->ArcRadius + pObj->PenSize3) * pObj->NeedleRadius / 100;
	GUI_SetPenSize(2);

	NumPoints = _CalcNeedle(pObj, Index, Radius - 2);
	GUI_SetColor(_CalcColor(&pObj->Color[2]));
	_AA_DrawPolygon(_aNeedle, NumPoints, pObj->x0 * FACTOR, pObj->y0 * FACTOR);
}

/*
*********************************************************************************************************
*	�� �� ��: _DrawNeedle
*	����˵��: ���Ʊ��������ָ��
*	��    ��: pObj    ���̽ṹ��ָ�����
*             Index   ���������ڿ���ָ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DrawNeedle(const SCALE* pObj, int Index) 
{
	static GUI_MEMDEV_Handle hMemDev;
	GUI_MEMDEV_Handle        hPrev;
	GUI_RECT                 r;
	int                      NumPoints;
	int                      Radius;

	#if SHOW_RECTS
		_GetNeedleRect(pObj, Index, &r);
		GUI_SetColor(GUI_BLUE);
		GUI_FillRect(r.x0, r.y0, r.x1, r.y1);
	#endif
	
	
	WM_GetWindowRect(&r);
	HIRES_ON();
	
	/* 1. ������ָ����Ƶ��洢�豸�� *******************************************/
	hMemDev = GUI_MEMDEV_CreateEx(r.x0, r.y0, r.x1 - r.x0 + 1, r.y1 - r.y0 + 1, GUI_MEMDEV_HASTRANS);
	
	hPrev = GUI_MEMDEV_Select(hMemDev);
	
	/* ������������������Ҫ���ٷ���������� */
//	GUI_MEMDEV_Write(pObj->hMemDev);
//	GUI_MEMDEV_Clear(hMemDev);

	Radius = (pObj->ArcRadius + pObj->PenSize3) * pObj->NeedleRadius / 100;
	NumPoints = _CalcNeedle(pObj, Index, Radius);
	GUI_SetColor(_CalcColor(&pObj->Color[2]));
	GUI_AA_SetFactor(AA_FACTOR);
	GUI_AA_FillPolygon(_aNeedle, NumPoints, pObj->x0 * FACTOR, pObj->y0 * FACTOR);
	if (pObj->Flags & (1 << FLAG_NEEDLE_LINE)) 
	{
		_DrawNeedleThread(pObj, Index);
	}

	GUI_MEMDEV_Select(hPrev);
	
	
	/* 2. ���ƴ�͸��Ч����ָ�� *******************************************/
	if (pObj->Color[2].Sep[3] != 0xFF) 
	{
		GUI_MEMDEV_WriteAlpha(hMemDev, pObj->Color[2].Sep[3]);
		/* ����ָ��ı߿� */
		if (pObj->Flags & (1 << FLAG_NEEDLE_FRAME)) 
		{
			_DrawNeedleFrame(pObj, Index);
		}
	}
	/* ������ָͨ�� *******************************************/
	else 
	{
		GUI_MEMDEV_Write(hMemDev);
		/* ����ָ��ı߿� */
		if ((pObj->Flags & (1 << FLAG_NEEDLE_FRAME)) && (pObj->Flags & (1 << FLAG_NEEDLE_LINE))) 
		{
			_DrawNeedleFrame(pObj, Index);
		}
	}
	
	/* ������Ϻ󣬽����洢�豸ɾ�� */
	GUI_MEMDEV_Delete(hMemDev);
	HIRES_OFF();

	/* 3. ���Ʊ����м��Բ�� *******************************************/
	GUI_SetColor(_CalcColor(&pObj->Color[6]));
	GUI_AA_FillCircle(pObj->x0, pObj->y0, pObj->ArcRadius * pObj->AxisRadius / 200);
}

/*
*********************************************************************************************************
*	�� �� ��: _cbScaleWin
*	����˵��: �������̴��ڻص�����
*	��    ��: pMsg  ��Ϣ�ṹ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _cbScaleWin(WM_MESSAGE* pMsg) 
{
	int Index;

	switch (pMsg->MsgId) 
	{
		case WM_PAINT:
			/*
				1. ֧��ͬʱ����4�����̴��ڣ��ص��������Ǵ˺�����
		        2. ͨ��forѭ����if�����ж�ѡ��Ҫ���µı��̴���
			*/
			for (Index = 0; Index < 4; Index++) 
			{
				if (_Scale[Index].hWin == pMsg->hWin) 
				{
					if (_Scale[Index].Flags & (1 << FLAG_SHOW_SCALE)) 
					{
						/* ��������и��£������̻��Ƶ��洢�豸������û�и��£������ظ����� */
						_UpdateScale(Index);
						
						/* ���Ʊ��̴洢�豸 */
						GUI_MEMDEV_Write(_Scale[Index].hMemDev);
						
						/* ���Ʊ���ָ�� */
						_DrawNeedle(&_Scale[Index], Index);
					}
					break;
				}
			}
			break;
			
		default:
			WM_DefaultProc(pMsg);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: _CreateScaleWindow
*	����˵��: �������̴���,������ʱ��ע������͸����ʶ
*	��    ��: pObj  ���̽ṹ��ָ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void _CreateScaleWindow(SCALE* pObj) 
{
	pObj->hWin = WM_CreateWindow(0, 0, 10, 10, WM_CF_SHOW | WM_CF_HASTRANS, &_cbScaleWin, 0);
	
	/* �޸ı��̴�С */
	_ChangeScaleSize(pObj);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
