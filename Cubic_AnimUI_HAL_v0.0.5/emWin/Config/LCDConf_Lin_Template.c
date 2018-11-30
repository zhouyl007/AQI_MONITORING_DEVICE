/*
*********************************************************************************************************
*
*	ģ������ : emWin�ĵײ������ļ�
*	�ļ����� : LCDConf_Lin_Template.c
*	��    �� : V1.0 
*	˵    �� : LCD�Ĳ���������emWin�ײ�ӿڶ�������ļ�ʵ�֡�
*              ʹ��˵����
*                1. ����������Ӧ������4.3�磬5���7������
*                2. �û�ʹ��emWinǰ������ʹ��STM32��CRCʱ�ӣ�Ȼ������ļ�bsp_tft_429.c�е�
*                   ����LCD_ConfigLTDC��֮�����GUI_Init()����ʹ��emWin��
*                3. ��ͬ����ʾ�����õĴ���������ͬ���û��Լ�ʹ�õĻ����������Ե��ü��ɡ�
*                   �������ǣ�TOUCH_Scan()��1ms����һ�Ρ�
*                   �������ǣ�GT811_OnePiontScan()����FT5X06_OnePiontScan()��10ms����һ�Ρ�
*                4. ����״̬���߸�����LCD�ĳ���F429���棬��Ļ�ᶶ����������������������뿴
*                   http://bbs.armfly.com/read.php?tid=16892
*              ����˵����
*                1. F429��ͼ�����ɱ����㣬ͼ��1��ͼ��2��ɡ�
*                2. �ܹ���12������ѡ��ǳ���Ҫ���� �����ÿ������ѡ�����ϸ˵����
*              ��ֲ˵����
*                  ���ļ�����ֱ��ʹ�ã���������ѡ������������κ��޸ģ��û�Ҫ�������ṩ����������
*                1. �ṩ����LCD_SetBackLight��ʵ�ֱ���Ŀ���
*                2. �ṩһ������LCD_ConfigLTDC������ʵ�ֲο��ļ�bsp_tft_429.c�д˺�����ʵ�֡�
*                3. �ṩ������������������ʵ��emWin����ֲ����Ȼ���û�Ҳ����ֱ���޸Ĵ��ļ��еĺ���
*                  _LCD_InitController��ʵ��F429/439��˫ͼ�����ü��ɡ�
*
*	�޸ļ�¼ :
*		�汾��    ����         ����      ˵��
*		V1.0    2016-01-05    Eric2013  1. �����໺��ģʽ��bug
*                                       2. ����8λɫ_CM_L8��_CM_AL44�޷�������ʾbug
*                                       3. ���������ȽϺ��ڴ��bug��
*                                       4. ѡ��ʹ�ú���_DMA_MixColors���˺����Լ۽ϵ͡������뿴��
*                                          http://bbs.armfly.com/read.php?tid=16919
*                                       5. ѡ��ʹ��DMA2D�жϡ���Ϊ���жϵ����þ��������߻����õġ�
*                                       6. ע�͵���������ָ���ʱ�ò��ϡ�
*
*		V1.1    2016-01-28    Eric2013  1. ȡ������������uint16_t Width, Height, HSYNC_W, VSYNC_W, HBP, HFP, VBP, VFP
*                                          ǰ���__IO���͡�
��
*       V1.2    2016-07-16    Eric2013  1. ʹ�ܺ���_GetBitsPerPixel��ʹ�ã�Ҫ��565��ʽ��λͼ�޷�������ʾ��
*                                       2. ������������ѡ���ע�͡�
*
*
*	Copyright (C), 2016-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "GUI.h"
#include "GUI_Private.h"
#include "GUIDRV_Lin.h"



/*
**********************************************************************************************************
							�����ⲿ�ļ��ı����ͺ���(bsp_tft_429.c�ļ�)
**********************************************************************************************************
*/
extern  uint16_t Width, Height, HSYNC_W, VSYNC_W, HBP, HFP, VBP, VFP;

/*
**********************************************************************************************************
									�û��������õ�ѡ��
**********************************************************************************************************
*/
/* 0. �ڹٷ�����Ļ����������Ż����ٷ��Ĳ��ֺ���Ч�ʵͣ����ڴ�, 0��ʾ�Ż� */
#define emWin_Optimize   0

/* 
  1. ��ʾ��������ֱ��ʣ������Ѿ�������ʾ������Ӧ��֧��4.3�磬5���7����
     ������д����Ӧ��ʾ���е����ֱ��ʡ�
*/
#define XSIZE_PHYS       800
#define YSIZE_PHYS       480

/* 2. �໺�� / ���������໺�������������ͬʱʹ�ã�emWin��֧�� */
#define NUM_BUFFERS      3 /* ����໺�����������������1,2��3��Ҳ�������֧�������� */
#define NUM_VSCREENS     1 /* �������������� */

/* 3. û��ͼ�㼤��ʱ������ɫ����, ��ʱδ�õ� */
#define BK_COLOR         GUI_DARKBLUE

/* 
   4. �ض���ͼ����������STM32F429/439���û�����ѡ��һ��ͼ���������ͼ�㣬��֧����ͼ�� 
      (1). ����GUI_NUM_LAYERS = 1ʱ������ʹ��ͼ��1ʱ��Ĭ�ϴ���ֵ�Ƿ��͸�ͼ��1�ġ�
	  (2). ����GUI_NUM_LAYERS = 2ʱ����ͼ��1��ͼ��2���Ѿ�ʹ�ܣ���ʱͼ��2�Ƕ��㣬
	       �û���Ҫ�����Լ���ʹ������������������ط���
		   a. ��bsp_touch.c�ļ��еĺ���TOUCH_InitHard�������ò���State.Layer = 1��1�ͱ�ʾ
		      ��ͼ��2���ʹ���ֵ��
		   b. ����GUI_Init�����󣬵��ú���GUI_SelectLayer(1), ���õ�ǰ��������ͼ��2��
*/
#undef  GUI_NUM_LAYERS
#define GUI_NUM_LAYERS    1

/* 
   5. ����ͼ��1��ͼ��2��Ӧ���Դ��ַ
      (1) EXT_SDRAM_ADDR ��SDRAM���׵�ַ��
      (2) LCD_LAYER0_FRAME_BUFFER ��ͼ��1���Դ��ַ��
	  (3) LCD_LAYER1_FRAME_BUFFER ��ͼ��2���Դ��ַ��
	  (4) ÿ��ͼ����Դ��С�ȽϿ�������������¼򵥵�˵����
	      ����û�ѡ�����ɫģʽ = 32λɫARGB8888���Դ�Ĵ�С��
	      XSIZE_PHYS * YSIZE_PHYS * 4 * NUM_VSCREENS * NUM_BUFFERS
		  
	      ��ɫģʽ = 24λɫRGB888���Դ�Ĵ�С��
	      XSIZE_PHYS * YSIZE_PHYS * 3 * NUM_VSCREENS * NUM_BUFFERS
		  
	      ��ɫģʽ = 16λɫRGB566��ARGB1555, ARGB4444��AL88����ô�Դ�Ĵ�С���ǣ�
	      XSIZE_PHYS * YSIZE_PHYS * 2 * NUM_VSCREENS * NUM_BUFFERS

	      ��ɫģʽ = 8λɫL8��AL44����ô�Դ�Ĵ�С���ǣ�
	      XSIZE_PHYS * YSIZE_PHYS * 1 * NUM_VSCREENS * NUM_BUFFERS	
      
      ����Ϊ�˷�������������������׵�16MB��SDRAMǰ8MB�����LCD�Դ�ʹ�ã���8MB����emWin��̬�ڴ档
	  ����24λɫ��16λɫ��8λɫ���û����Զ���ʹ�������壬����ʹ��˫ͼ�㡣����32λɫҲʹ���������˫
	  ͼ��Ļ��ᳬ��8MB�������û������Լ���������Դ��emWin��̬�ڴ�ķ��������
	    ��һ�����ӣ�����800*480�ֱ��ʵ���ʾ����ʹ��32λɫ�������壬��ô����һ��ͼ����Ҫ�Ĵ�С����
      800 * 480 * 4 * 3  = 4.394MB�Ŀռ䣬�����˫ͼ�㣬�Ѿ�����8MB�ķ��䷶Χ��

      (5)Ϊ�˷��������ͼ��2�ĺ궨��LCD_LAYER1_FRAME_BUFFER�еĲ���4�ǰ���32λɫ���õģ�����û���ͼ��1
         ʹ�õ���8λɫ������������1,�����16λɫ��������2�������24λɫ��������3��
*/
#define LCD_LAYER0_FRAME_BUFFER  EXT_SDRAM_ADDR
#define LCD_LAYER1_FRAME_BUFFER  (LCD_LAYER0_FRAME_BUFFER + XSIZE_PHYS * YSIZE_PHYS * 4 * NUM_VSCREENS * NUM_BUFFERS)

/* 
   6. STM32F429/439֧�ֵ���ɫģʽ������ģʽ��֧�֣��û����������á�
      �ر�ע�������������⣺
	  (1) ����û�ѡ����ARGB8888����RGB888ģʽ��LCD��˸�Ƚ������Ļ���
	      �뽵��LTDC��ʱ�Ӵ�С�����ļ�bsp_tft_429.c�ĺ���LCD_ConfigLTDC�������á�
	      a. һ��800*480�ֱ��ʵ���ʾ����ARGB8888����RGB888ģʽLTDCʱ��ѡ��10-20MHz���ɡ�
	      b. 480*272�ֱ��ʵĿ��Ը�Щ��ȡ20MHz���Ҽ��ɡ�
	  (2) 16λɫ����8λɫģʽ��LTDC��ʱ��Ƶ��һ����Ա�24λɫ����32λɫ�ĸ�һ����
*/
#define _CM_ARGB8888      1
#define _CM_RGB888        2
#define _CM_RGB565        3
#define _CM_ARGB1555      4
#define _CM_ARGB4444      5
#define _CM_L8            6
#define _CM_AL44          7
#define _CM_AL88          8

/* 7. ����ͼ��1����ɫģʽ�ͷֱ��ʴ�С */
#define COLOR_MODE_0      _CM_RGB565
#define XSIZE_0           XSIZE_PHYS
#define YSIZE_0           YSIZE_PHYS

/* 8. ����ͼ��2�ĵ���ɫģʽ�ͷֱ��ʴ�С */
#define COLOR_MODE_1      _CM_RGB565
#define XSIZE_1           XSIZE_PHYS
#define YSIZE_1           YSIZE_PHYS

/* 9. ��ͼ������£������û�ѡ�����ɫģʽ���Զ�ѡ��ͼ��1��emWin����������ɫģʽ */
#if   (COLOR_MODE_0 == _CM_ARGB8888)
  #define COLOR_CONVERSION_0 GUICC_M8888I
  #define DISPLAY_DRIVER_0   GUIDRV_LIN_32
#elif (COLOR_MODE_0 == _CM_RGB888)
  #define COLOR_CONVERSION_0 GUICC_M888
  #define DISPLAY_DRIVER_0   GUIDRV_LIN_24
#elif (COLOR_MODE_0 == _CM_RGB565)
  #define COLOR_CONVERSION_0 GUICC_M565
  #define DISPLAY_DRIVER_0   GUIDRV_LIN_16
#elif (COLOR_MODE_0 == _CM_ARGB1555)
  #define COLOR_CONVERSION_0 GUICC_M1555I
  #define DISPLAY_DRIVER_0   GUIDRV_LIN_16
#elif (COLOR_MODE_0 == _CM_ARGB4444)
  #define COLOR_CONVERSION_0 GUICC_M4444I
  #define DISPLAY_DRIVER_0   GUIDRV_LIN_16
#elif (COLOR_MODE_0 == _CM_L8)
  #define COLOR_CONVERSION_0 GUICC_8666
  #define DISPLAY_DRIVER_0   GUIDRV_LIN_8
#elif (COLOR_MODE_0 == _CM_AL44)
  #define COLOR_CONVERSION_0 GUICC_1616I
  #define DISPLAY_DRIVER_0   GUIDRV_LIN_8
#elif (COLOR_MODE_0 == _CM_AL88)
  #define COLOR_CONVERSION_0 GUICC_88666I
  #define DISPLAY_DRIVER_0   GUIDRV_LIN_16
#else
  #error Illegal color mode 0!
#endif

/* 10. ˫ͼ������£������û�ѡ�����ɫģʽ���Զ�ѡ��ͼ��2��emWin����������ɫģʽ */
#if (GUI_NUM_LAYERS > 1)

#if   (COLOR_MODE_1 == _CM_ARGB8888)
  #define COLOR_CONVERSION_1 GUICC_M8888I
  #define DISPLAY_DRIVER_1   GUIDRV_LIN_32
#elif (COLOR_MODE_1 == _CM_RGB888)
  #define COLOR_CONVERSION_1 GUICC_M888
  #define DISPLAY_DRIVER_1   GUIDRV_LIN_24
#elif (COLOR_MODE_1 == _CM_RGB565)
  #define COLOR_CONVERSION_1 GUICC_M565
  #define DISPLAY_DRIVER_1   GUIDRV_LIN_16
#elif (COLOR_MODE_1 == _CM_ARGB1555)
  #define COLOR_CONVERSION_1 GUICC_M1555I
  #define DISPLAY_DRIVER_1   GUIDRV_LIN_16
#elif (COLOR_MODE_1 == _CM_ARGB4444)
  #define COLOR_CONVERSION_1 GUICC_M4444I
  #define DISPLAY_DRIVER_1   GUIDRV_LIN_16
#elif (COLOR_MODE_1 == _CM_L8)
  #define COLOR_CONVERSION_1 GUICC_8666
  #define DISPLAY_DRIVER_1   GUIDRV_LIN_8
#elif (COLOR_MODE_1 == _CM_AL44)
  #define COLOR_CONVERSION_1 GUICC_1616I
  #define DISPLAY_DRIVER_1   GUIDRV_LIN_8
#elif (COLOR_MODE_1 == _CM_AL88)
  #define COLOR_CONVERSION_1 GUICC_88666I
  #define DISPLAY_DRIVER_1   GUIDRV_LIN_16
#else
  #error Illegal color mode 1!
#endif

#else

#undef XSIZE_0
#undef YSIZE_0
#define XSIZE_0       XSIZE_PHYS
#define YSIZE_0       YSIZE_PHYS

#endif

/*11. ����ѡ���⣬��ֹ���ô������ĳЩѡ��û������ */
#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif
#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif
#ifndef   NUM_VSCREENS
  #define NUM_VSCREENS 1
#else
  #if (NUM_VSCREENS <= 0)
    #error At least one screeen needs to be defined!
  #endif
#endif
#if (NUM_VSCREENS > 1) && (NUM_BUFFERS > 1)
  #error Virtual screens and multiple buffers are not allowed!
#endif

/*
**********************************************************************************************************
									ʹ��DMA2D�ض�����ɫ������ת��
**********************************************************************************************************
*/
#define DEFINE_DMA2D_COLORCONVERSION(PFIX, PIXELFORMAT)                                                        \
static void _Color2IndexBulk_##PFIX##_DMA2D(LCD_COLOR * pColor, void * pIndex, U32 NumItems, U8 SizeOfIndex) { \
  _DMA_Color2IndexBulk(pColor, pIndex, NumItems, SizeOfIndex, PIXELFORMAT);                                    \
}                                                                                                              \
static void _Index2ColorBulk_##PFIX##_DMA2D(void * pIndex, LCD_COLOR * pColor, U32 NumItems, U8 SizeOfIndex) { \
  _DMA_Index2ColorBulk(pColor, pIndex, NumItems, SizeOfIndex, PIXELFORMAT);                                    \
}

/* �������� */
static void _DMA_Index2ColorBulk(void * pIndex, LCD_COLOR * pColor, U32 NumItems, U8 SizeOfIndex, U32 PixelFormat);
static void _DMA_Color2IndexBulk(LCD_COLOR * pColor, void * pIndex, U32 NumItems, U8 SizeOfIndex, U32 PixelFormat);

/* ��ɫת�� */
DEFINE_DMA2D_COLORCONVERSION(M8888I, LTDC_Pixelformat_ARGB8888)
DEFINE_DMA2D_COLORCONVERSION(M888,   LTDC_Pixelformat_ARGB8888) /* Internal pixel format of emWin is 32 bit, because of that ARGB8888 */
DEFINE_DMA2D_COLORCONVERSION(M565,   LTDC_Pixelformat_RGB565)
DEFINE_DMA2D_COLORCONVERSION(M1555I, LTDC_Pixelformat_ARGB1555)
DEFINE_DMA2D_COLORCONVERSION(M4444I, LTDC_Pixelformat_ARGB4444)

/*
**********************************************************************************************************
									�ļ���ʹ�õ�ȫ�ֱ���
**********************************************************************************************************
*/
static LTDC_Layer_TypeDef       * _apLayer[]  = { LTDC_Layer1, LTDC_Layer2 };
static const U32                  _aAddr[]    = { LCD_LAYER0_FRAME_BUFFER, LCD_LAYER1_FRAME_BUFFER};
static int                        _aPendingBuffer[GUI_NUM_LAYERS];
static int                        _aBufferIndex[GUI_NUM_LAYERS];
static int                        _axSize[GUI_NUM_LAYERS];
static int                        _aySize[GUI_NUM_LAYERS];
static int                        _aBytesPerPixels[GUI_NUM_LAYERS];
#if 0	 /* �ٷ��˴����������⣬��ʱ��Ϊ���£�����Ҫ����sizeof(U32) */
  static U32 						  _aBuffer_DMA2D[XSIZE_PHYS * sizeof(U32)];
  static U32 						  _aBuffer_FG   [XSIZE_PHYS * sizeof(U32)];
  static U32 						  _aBuffer_BG   [XSIZE_PHYS * sizeof(U32)];
#else
  static U32 						  _aBuffer_DMA2D[XSIZE_PHYS];
  static U32 						  _aBuffer_FG   [XSIZE_PHYS];
  static U32 						  _aBuffer_BG   [XSIZE_PHYS];
#endif

static const LCD_API_COLOR_CONV * _apColorConvAPI[] = {
  COLOR_CONVERSION_0,
#if GUI_NUM_LAYERS > 1
  COLOR_CONVERSION_1,
#endif
};

/*
*********************************************************************************************************
*	�� �� ��: _GetPixelformat
*	����˵��: ��ȡͼ��1����ͼ��2ʹ�õ���ɫ��ʽ
*	��    ��: LayerIndex  ͼ��
*	�� �� ֵ: ��ɫ��ʽ
*********************************************************************************************************
*/
static U32 _GetPixelformat(int LayerIndex) {
  const LCD_API_COLOR_CONV * pColorConvAPI;

  if (LayerIndex >= GUI_COUNTOF(_apColorConvAPI)) {
    return 0;
  }
  pColorConvAPI = _apColorConvAPI[LayerIndex];
  if        (pColorConvAPI == GUICC_M8888I) {
    return LTDC_Pixelformat_ARGB8888;
  } else if (pColorConvAPI == GUICC_M888) {
    return LTDC_Pixelformat_RGB888;
  } else if (pColorConvAPI == GUICC_M565) {
    return LTDC_Pixelformat_RGB565;
  } else if (pColorConvAPI == GUICC_M1555I) {
    return LTDC_Pixelformat_ARGB1555;
  } else if (pColorConvAPI == GUICC_M4444I) {
    return LTDC_Pixelformat_ARGB4444;
  } else if (pColorConvAPI == GUICC_8666) {
    return LTDC_Pixelformat_L8;
  } else if (pColorConvAPI == GUICC_1616I) {
    return LTDC_Pixelformat_AL44;
  } else if (pColorConvAPI == GUICC_88666I) {
    return LTDC_Pixelformat_AL88;
  }
  while (1); // Error
}

/*
*********************************************************************************************************
*	�� �� ��: _GetBytesPerLine
*	����˵��: ����LayerIndexָ����ͼ���xSizeָ���ĳ��Ȼ�ȡ��Ҫ���ֽ���
*	��    ��: LayerIndex  ͼ��
*             xSize       ���ظ���
*	�� �� ֵ: �ֽ���
*********************************************************************************************************
*/
static int _GetBytesPerLine(int LayerIndex, int xSize) {
  int BitsPerPixel, BytesPerLine;

  BitsPerPixel  = LCD_GetBitsPerPixelEx(LayerIndex);
  BytesPerLine = (BitsPerPixel * xSize + 7) / 8;
  return BytesPerLine;
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_LoadLUT
*	����˵��: ����CLUT���Զ�����
*	��    ��: pColor      ǰ����ͼ���CLUT��ַ��ʹ�õ����ݵ�ַ
*             NumItems    ǰ����ͼ�����õ�CLUT�Ĵ�С 
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_LoadLUT(LCD_COLOR * pColor, U32 NumItems) {
  DMA2D->FGCMAR  = (U32)pColor;                     // Foreground CLUT Memory Address Register
  //
  // Foreground PFC Control Register
  //
  DMA2D->FGPFCCR  = LTDC_Pixelformat_RGB888         // Pixel format
                  | ((NumItems - 1) & 0xFF) << 8;   // Number of items to load
  DMA2D->FGPFCCR |= (1 << 5);                       // Start loading
  //
  // Waiting not required here...
  //
}

/*
*********************************************************************************************************
*	�� �� ��: _InvertAlpha_SwapRB
*	����˵��: emWin����ɫ��ʽ��DMA2D����ɫ��ʽ��ͬ��DMA2D����ɫ��ʽҪת��Ϊ
*             emWin����ɫ��ʽ����Ҫ�������㣺
*             1. ����R��B��λ�á�
*             2. ��תalphaͨ����
*             �˺����������������������֮��Ȼ��
*	��    ��: pColorSrc  DMA2D��ʽ��ɫ��ַ����ԭʼ��ɫ
*             pColorDst  emWin��ʽ��ɫ��ַ����ת������ɫ
*             NumItems   Ҫת������ɫ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _InvertAlpha_SwapRB(LCD_COLOR * pColorSrc, LCD_COLOR * pColorDst, U32 NumItems) {
  U32 Color;
  do {
    Color = *pColorSrc++;
    *pColorDst++ = ((Color & 0x000000FF) << 16)         // Swap red <-> blue
                 |  (Color & 0x0000FF00)                // Green
                 | ((Color & 0x00FF0000) >> 16)         // Swap red <-> blue
                 | ((Color & 0xFF000000) ^ 0xFF000000); // Invert alpha
  } while (--NumItems);
}

/*
*********************************************************************************************************
*	�� �� ��: _InvertAlpha
*	����˵��: emWin����ɫ��ʽ��DMA2D����ɫ��ʽ��ͬ��DMA2D����ɫ��ʽҪת��Ϊ
*             emWin����ɫ��ʽ����Ҫ�������㣺
*             1. ����R��B��λ�á�
*             2. ��תalphaͨ����
*             �˺�������ɵڶ���������֮��Ȼ��
*	��    ��: pColorSrc  DMA2D��ʽ��ɫ��ַ����ԭʼ��ɫ
*             pColorDst  emWin��ʽ��ɫ��ַ����ת������ɫ
*             NumItems   Ҫת������ɫ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _InvertAlpha(LCD_COLOR * pColorSrc, LCD_COLOR * pColorDst, U32 NumItems) {
  U32 Color;

  do {
    Color = *pColorSrc++;
    *pColorDst++ = Color ^ 0xFF000000; // Invert alpha
  } while (--NumItems);
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_AlphaBlendingBulk
*	����˵��: ʵ����������ɫ���ת������ȡǰ��ɫ�ͱ���ɫ��ִ��PFC(���ظ�ʽת����)��DMA2D��ʽѡ��洢�����洢����ִ�л�ϡ�
*	��    ��: pColorFG   ǰ��ɫ��ַ
*             pColorBG   ����ɫ��ַ
*             pColorDst  ת������ɫ�洢
*             NumItems   Ҫת������ɫ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_AlphaBlendingBulk(LCD_COLOR * pColorFG, LCD_COLOR * pColorBG, LCD_COLOR * pColorDst, U32 NumItems) {
  //
  // Set up mode
  //
  DMA2D->CR      = 0x00020000UL | (1 << 9);         // Control Register (Memory to memory with blending of FG and BG and TCIE)
  //
  // Set up pointers
  //
  DMA2D->FGMAR   = (U32)pColorFG;                   // Foreground Memory Address Register
  DMA2D->BGMAR   = (U32)pColorBG;                   // Background Memory Address Register
  DMA2D->OMAR    = (U32)pColorDst;                  // Output Memory Address Register (Destination address)
  //
  // Set up offsets
  //
  DMA2D->FGOR    = 0;                               // Foreground Offset Register
  DMA2D->BGOR    = 0;                               // Background Offset Register
  DMA2D->OOR     = 0;                               // Output Offset Register
  //
  // Set up pixel format
  //
  DMA2D->FGPFCCR = LTDC_Pixelformat_ARGB8888;       // Foreground PFC Control Register (Defines the FG pixel format)
  DMA2D->BGPFCCR = LTDC_Pixelformat_ARGB8888;       // Background PFC Control Register (Defines the BG pixel format)
  DMA2D->OPFCCR  = LTDC_Pixelformat_ARGB8888;       // Output     PFC Control Register (Defines the output pixel format)
  //
  // Set up size
  //
  DMA2D->NLR     = (U32)(NumItems << 16) | 1;       // Number of Line Register (Size configuration of area to be transfered)
  //
  // Execute operation
  //
  
  DMA2D->CR     |= 1;  
    
  while (DMA2D->CR & DMA2D_CR_START) {
    //__WFI();                                      // Sleep until next interrupt
  }
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_AlphaBlending
*	����˵��: ʵ����������ɫ���ת��
*	��    ��: pColorFG   ǰ��ɫ��ַ
*             pColorBG   ����ɫ��ַ
*             pColorDst  ת������ɫ�洢
*             NumItems   Ҫת������ɫ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_AlphaBlending(LCD_COLOR * pColorFG, LCD_COLOR * pColorBG, LCD_COLOR * pColorDst, U32 NumItems) {
  //
  // Invert alpha values
  //
  _InvertAlpha(pColorFG, _aBuffer_FG, NumItems);
  _InvertAlpha(pColorBG, _aBuffer_BG, NumItems);
  //
  // Use DMA2D for mixing
  //
  _DMA_AlphaBlendingBulk(_aBuffer_FG, _aBuffer_BG, _aBuffer_DMA2D, NumItems);
  //
  // Invert alpha values
  //
  _InvertAlpha(_aBuffer_DMA2D, pColorDst, NumItems);
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_MixColors
*	����˵��: ͨ��������alphaֵ��ʵ��������ɫ�Ļ��(�о�ִֻ��һ��������ɫ�Ļ��Ҳ��DMA����Щӷ��)
*             �������ɫ��͸���ģ�ֱ�ӷ���ǰ��ɫ��
*	��    ��: Color     ǰ��ɫ��ַ
*             BkColor   ����ɫ��ַ
*             Intens    ��alphaֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
#if emWin_Optimize 
static LCD_COLOR _DMA_MixColors(LCD_COLOR Color, LCD_COLOR BkColor, U8 Intens) {
  U32 ColorFG, ColorBG, ColorDst;

  if ((BkColor & 0xFF000000) == 0xFF000000) {
    return Color;
  }
  ColorFG = Color   ^ 0xFF000000;
  ColorBG = BkColor ^ 0xFF000000;
  //
  // Set up mode
  //
  DMA2D->CR      = 0x00020000UL | (1 << 9);         // Control Register (Memory to memory with blending of FG and BG and TCIE)
  //
  // Set up pointers
  //
  DMA2D->FGMAR   = (U32)&ColorFG;                   // Foreground Memory Address Register
  DMA2D->BGMAR   = (U32)&ColorBG;                   // Background Memory Address Register
  DMA2D->OMAR    = (U32)&ColorDst;                  // Output Memory Address Register (Destination address)
  //
  // Set up pixel format
  //
  DMA2D->FGPFCCR = LTDC_Pixelformat_ARGB8888
                 | (1UL << 16)
                 | ((U32)Intens << 24);
  DMA2D->BGPFCCR = LTDC_Pixelformat_ARGB8888
                 | (0UL << 16)
                 | ((U32)(255 - Intens) << 24);
  DMA2D->OPFCCR  = LTDC_Pixelformat_ARGB8888;
  //
  // Set up size
  //
  DMA2D->NLR     = (U32)(1 << 16) | 1;              // Number of Line Register (Size configuration of area to be transfered)
  //
  // Execute operation
  //

  //_DMA_ExecOperation();
  DMA2D->CR     |= 1;                               // Control Register (Start operation)
  //
  // Wait until transfer is done
  //
  while (DMA2D->CR & DMA2D_CR_START) {
    //__WFI();                                      // Sleep until next interrupt
  }

  return ColorDst ^ 0xFF000000;
}
#endif

/*
*********************************************************************************************************
*	�� �� ��: _DMA_MixColorsBulk
*	����˵��: ͨ��������alphaֵ��ʵ��������ɫ���������
*	��    ��: pColorFG   ǰ��ɫ��ַ
*             pColorBG   ����ɫ��ַ
*             pColorDst  ��Ϻ���ɫ�洢�ĵ�ַ
*             Intens     ��alphaֵ
*             NumItems   ת������ɫ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_MixColorsBulk(LCD_COLOR * pColorFG, LCD_COLOR * pColorBG, LCD_COLOR * pColorDst, U8 Intens, U32 NumItems) {
  //
  // Set up mode
  //
  DMA2D->CR      = 0x00020000UL | (1 << 9);         // Control Register (Memory to memory with blending of FG and BG and TCIE)
  //
  // Set up pointers
  //
  DMA2D->FGMAR   = (U32)pColorFG;                   // Foreground Memory Address Register
  DMA2D->BGMAR   = (U32)pColorBG;                   // Background Memory Address Register
  DMA2D->OMAR    = (U32)pColorDst;                  // Output Memory Address Register (Destination address)
  //
  // Set up pixel format
  //
  DMA2D->FGPFCCR = LTDC_Pixelformat_ARGB8888
                 | (1UL << 16)
                 | ((U32)Intens << 24);
  DMA2D->BGPFCCR = LTDC_Pixelformat_ARGB8888
                 | (0UL << 16)
                 | ((U32)(255 - Intens) << 24);
  DMA2D->OPFCCR  = LTDC_Pixelformat_ARGB8888;
  //
  // Set up size
  //
  DMA2D->NLR     = (U32)(NumItems << 16) | 1;              // Number of Line Register (Size configuration of area to be transfered)
  //
  // Execute operation
  //
  DMA2D->CR     |= 1;  
    
  while (DMA2D->CR & DMA2D_CR_START) {
    //__WFI();                                        // Sleep until next interrupt
  }
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_MixColorsBulk
*	����˵��: ��һ����ʾ����ǰ��ɫ�ͱ���ɫ���л��
*	��    ��: pFG   ǰ��ɫ��ַ
*             pBG   ����ɫ��ַ
*             pDst  ��Ϻ���ɫ�洢�ĵ�ַ
*             OffFG    ǰ��ɫƫ�Ƶ�ַ
*             OffBG    ����ɫƫ�Ƶ�ַ
*             OffDest  ��Ϻ�ƫ�Ƶ�ַ
*             xSize    ��ʾ��x���С
*             ySize    ��ʾ��y���С
*             Intens   ��alphaֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _LCD_MixColorsBulk(U32 * pFG, U32 * pBG, U32 * pDst, unsigned OffFG, unsigned OffBG, unsigned OffDest, unsigned xSize, unsigned ySize, U8 Intens) {
  int y;

  GUI_USE_PARA(OffFG);
  GUI_USE_PARA(OffDest);
  for (y = 0; y < ySize; y++) {
    //
    // Invert alpha values
    //
    _InvertAlpha(pFG, _aBuffer_FG, xSize);
    _InvertAlpha(pBG, _aBuffer_BG, xSize);
    //
    //
    //
    _DMA_MixColorsBulk(_aBuffer_FG, _aBuffer_BG, _aBuffer_DMA2D, Intens, xSize);
    //
    //
    //
    _InvertAlpha(_aBuffer_DMA2D, pDst, xSize);
    pFG  += xSize + OffFG;
    pBG  += xSize + OffBG;
    pDst += xSize + OffDest;
  }
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_ConvertColor
*	����˵��: ��ɫת������һ����ɫ��ʽת������һ����ɫ��ʽ
*	��    ��: pSrc   Դ��ɫ��ַ
*             pDst   ת������ɫ�洢��ַ
*             PixelFormatSrc  Դ��ɫ��ʽ
*             PixelFormatDst  ת������ɫ��ʽ
*             NumItems        Ҫת������ɫ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_ConvertColor(void * pSrc, void * pDst,  U32 PixelFormatSrc, U32 PixelFormatDst, U32 NumItems) {
  //
  // Set up mode
  //
  DMA2D->CR      = 0x00010000UL | (1 << 9);         // Control Register (Memory to memory with pixel format conversion and TCIE)
  //
  // Set up pointers
  //
  DMA2D->FGMAR   = (U32)pSrc;                       // Foreground Memory Address Register (Source address)
  DMA2D->OMAR    = (U32)pDst;                       // Output Memory Address Register (Destination address)
  //
  // Set up offsets
  //
  DMA2D->FGOR    = 0;                               // Foreground Offset Register (Source line offset)
  DMA2D->OOR     = 0;                               // Output Offset Register (Destination line offset)
  //
  // Set up pixel format
  //
  DMA2D->FGPFCCR = PixelFormatSrc;                  // Foreground PFC Control Register (Defines the input pixel format)
  DMA2D->OPFCCR  = PixelFormatDst;                  // Output PFC Control Register (Defines the output pixel format)
  //
  // Set up size
  //
  DMA2D->NLR     = (U32)(NumItems << 16) | 1;       // Number of Line Register (Size configuration of area to be transfered)
  //
  // Execute operation
  //
  DMA2D->CR     |= 1;  
    
  while (DMA2D->CR & DMA2D_CR_START) {
    //__WFI();                                        // Sleep until next interrupt
  }
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_Index2ColorBulk
*	����˵��: ͨ��DMA2D������ǰ��ʾ������ɫ����ת��ΪemWin��32λARGB��ɫ���ݡ�
*	��    ��: pIndex       ��ʾ����ɫ��ַ
*             pColor       ת����������emWin����ɫ��ַ
*             NumItems     ת������ɫ����
*             SizeOfIndex  δʹ��
*             PixelFormat  ��ʾ����ǰʹ�õ���ɫ��ʽ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/*
*********************************************************************************************************
* Purpose:
*   This routine is used by the emWin color conversion routines to use DMA2D for
*   color conversion. It converts the given index values to 32 bit colors.
*   Because emWin uses ABGR internally and 0x00 and 0xFF for opaque and fully
*   transparent the color array needs to be converted after DMA2D has been used.
*********************************************************************************************************
*/
static void _DMA_Index2ColorBulk(void * pIndex, LCD_COLOR * pColor, U32 NumItems, U8 SizeOfIndex, U32 PixelFormat) {
  //
  // Use DMA2D for the conversion
  //
  _DMA_ConvertColor(pIndex, _aBuffer_DMA2D, PixelFormat, LTDC_Pixelformat_ARGB8888, NumItems);
  //
  // Convert colors from ARGB to ABGR and invert alpha values
  //
  _InvertAlpha_SwapRB(_aBuffer_DMA2D, pColor, NumItems);
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_Color2IndexBulk
*	����˵��: ͨ��DMA2D����emWin��32λARGB��ɫ����ת��Ϊ�����ڵ�ǰ��ʾ������ɫ����
*	��    ��: pIndex       ��ʾ����ɫ��ַ
*             pColor       ת����������emWin����ɫ��ַ
*             NumItems     ת������ɫ����
*             SizeOfIndex  δʹ��
*             PixelFormat  ��ʾ����ǰʹ�õ���ɫ��ʽ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/*
*********************************************************************************************************
* Purpose:
*   This routine is used by the emWin color conversion routines to use DMA2D for
*   color conversion. It converts the given 32 bit color array to index values.
*   Because emWin uses ABGR internally and 0x00 and 0xFF for opaque and fully
*   transparent the given color array needs to be converted before DMA2D can be used.
*********************************************************************************************************
*/
#if 1 /* ���˺������룬���Է��ֲ�����˺�����BMP565��ʽ��λͼ�޷�������ʾ */
static int _GetBitsPerPixel(int Pixelformat) 
{
	switch (Pixelformat) 
	{
		case LTDC_Pixelformat_ARGB8888:
			return 32;
		case LTDC_Pixelformat_RGB888:
			return 24;
		case LTDC_Pixelformat_RGB565:
			return 16;
		case LTDC_Pixelformat_ARGB1555:
			return 16;
		case LTDC_Pixelformat_ARGB4444:
			return 16;
		case LTDC_Pixelformat_L8:
			return 8;
		case LTDC_Pixelformat_AL44:
			return 8;
		case LTDC_Pixelformat_AL88:
			return 16;
	}
	return 0;
}
#endif 

static void _DMA_Color2IndexBulk(LCD_COLOR * pColor, void * pIndex, U32 NumItems, U8 SizeOfIndex, U32 PixelFormat) {
  //
  // Convert colors from ABGR to ARGB and invert alpha values
  //
  _InvertAlpha_SwapRB(pColor, _aBuffer_DMA2D, NumItems);
  //
  // Use DMA2D for the conversion
  //
  _DMA_ConvertColor(_aBuffer_DMA2D, pIndex, LTDC_Pixelformat_ARGB8888, PixelFormat, NumItems);

#if 1 /* ���˺������룬���Է��ֲ�����˺�����BMP565��ʽ��λͼ�޷�������ʾ  */
  {
    int BitsPerPixel;
    if (SizeOfIndex == 4) {
      BitsPerPixel = _GetBitsPerPixel(PixelFormat);
      GUI__ExpandPixelIndices(pIndex, NumItems, BitsPerPixel);
    }	
  }
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: _LCD_GetpPalConvTable
*	����˵��: ת����ɫ������Ӧ���������õ���ɫ��ʽ��
*	��    ��: pLogPal   Դ��ɫ���ַ
*             pBitmap   λͼ��ַ
*             LayerIndex  Դ��ɫ��ʽ
*	�� �� ֵ: ת�������ɫ���ַ
*********************************************************************************************************
*/
/*
*********************************************************************************************************
* Purpose:
*   The emWin function LCD_GetpPalConvTable() normally translates the given colors into
*   index values for the display controller. In case of index based bitmaps without
*   transparent pixels we load the palette only to the DMA2D LUT registers to be
*   translated (converted) during the process of drawing via DMA2D.
*********************************************************************************************************
*/
static LCD_PIXELINDEX * _LCD_GetpPalConvTable(const LCD_LOGPALETTE GUI_UNI_PTR * pLogPal, const GUI_BITMAP GUI_UNI_PTR * pBitmap, int LayerIndex) {
  void (* pFunc)(void);
  int DoDefault = 0;

  //
  // Check if we have a non transparent device independent bitmap
  //
  if (pBitmap->BitsPerPixel == 8) {
    pFunc = LCD_GetDevFunc(LayerIndex, LCD_DEVFUNC_DRAWBMP_8BPP);
    if (pFunc) {
      if (pBitmap->pPal) {
        if (pBitmap->pPal->HasTrans) {
          DoDefault = 1;
        }
      } else {
        DoDefault = 1;
      }
    } else {
      DoDefault = 1;
    }
  } else {
    DoDefault = 1;
  }
  //
  // Default palette management for other cases
  //
  if (DoDefault) {
    //
    // Return a pointer to the index values to be used by the controller
    //
    return LCD_GetpPalConvTable(pLogPal);
  }
  //
  // Convert palette colors from ARGB to ABGR
  //
  _InvertAlpha_SwapRB((U32 *)pLogPal->pPalEntries, _aBuffer_DMA2D, pLogPal->NumEntries);
  //
  // Load LUT using DMA2D
  //
  _DMA_LoadLUT(_aBuffer_DMA2D, pLogPal->NumEntries);
  //
  // Return something not NULL
  //
  return _aBuffer_DMA2D;
}

/*
*********************************************************************************************************
*	�� �� ��: _LTDC_LayerEnableColorKeying
*	����˵��: ʹ��ɫ���󣬵�ǰ���أ���ʽת���󡢻��ǰ�����أ�����ɫ�����бȽϡ������ǰ������
*             ��̵� RGB ֵ��ƥ�䣬������ص�����ͨ�� (ARGB) ������Ϊ 0��
*	��    ��: LTDC_Layer_TypeDef   �ṹ��ָ��
*             NewState             DISABLE ��ֹ
*                                  ENABLE  ʹ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _LTDC_LayerEnableColorKeying(LTDC_Layer_TypeDef * LTDC_Layerx, int NewState) {
  if (NewState != DISABLE) {
    LTDC_Layerx->CR |= (U32)LTDC_LxCR_COLKEN;
  } else {
    LTDC_Layerx->CR &= ~(U32)LTDC_LxCR_COLKEN;
  }
  LTDC->SRCR = LTDC_SRCR_VBR; // Reload on next blanking period
}

/*
*********************************************************************************************************
*	�� �� ��: _LTDC_LayerEnableLUT
*	����˵��: ʹ��LUT��ɫ���ұ�
*	��    ��: LTDC_Layer_TypeDef   �ṹ��ָ��
*             NewState             DISABLE ��ֹ
*                                  ENABLE  ʹ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _LTDC_LayerEnableLUT(LTDC_Layer_TypeDef * LTDC_Layerx, int NewState) {
  if (NewState != DISABLE) {
    LTDC_Layerx->CR |= (U32)LTDC_LxCR_CLUTEN;
  } else {
    LTDC_Layerx->CR &= ~(U32)LTDC_LxCR_CLUTEN;
  }
  LTDC->SRCR = LTDC_SRCR_VBR; // Reload on next blanking period
}

/*
*********************************************************************************************************
*	�� �� ��: _LTDC_SetLayerPos
*	����˵��: ����ͼ���λ��
*	��    ��: LayerIndex   �ṹ��ָ��
*             xPos         Xλ��
*             yPos         Yλ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _LTDC_SetLayerPos(int LayerIndex, int xPos, int yPos) {
  int xSize, ySize;
  U32 HorizontalStart, HorizontalStop, VerticalStart, VerticalStop;

  xSize = LCD_GetXSizeEx(LayerIndex);
  ySize = LCD_GetYSizeEx(LayerIndex);
  HorizontalStart = xPos + HBP + 1;
  HorizontalStop  = xPos + HBP + xSize;
  VerticalStart   = yPos + VBP + 1;
  VerticalStop    = yPos + VBP + ySize;
  //
  // Horizontal start and stop position
  //
  _apLayer[LayerIndex]->WHPCR &= ~(LTDC_LxWHPCR_WHSTPOS | LTDC_LxWHPCR_WHSPPOS);
  _apLayer[LayerIndex]->WHPCR = (HorizontalStart | (HorizontalStop << 16));
  //
  // Vertical start and stop position
  //
  _apLayer[LayerIndex]->WVPCR &= ~(LTDC_LxWVPCR_WVSTPOS | LTDC_LxWVPCR_WVSPPOS);
  _apLayer[LayerIndex]->WVPCR  = (VerticalStart | (VerticalStop << 16));
  //
  // Reload configuration
  //
  LTDC_ReloadConfig(LTDC_SRCR_VBR); // Reload on next blanking period
}

/*
*********************************************************************************************************
*	�� �� ��: _LTDC_SetLayerAlpha
*	����˵��: ����ͼ��ĺ㶨Alpha
*	��    ��: LayerIndex   ͼ��
*             Alpha        alpha��ֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _LTDC_SetLayerAlpha(int LayerIndex, int Alpha) {
  //
  // Set constant alpha value
  //
  _apLayer[LayerIndex]->CACR &= ~(LTDC_LxCACR_CONSTA);
  _apLayer[LayerIndex]->CACR  = 255 - Alpha;
  //
  // Reload configuration
  //
  LTDC_ReloadConfig(LTDC_SRCR_IMR/*LTDC_SRCR_VBR*/); // Reload on next blanking period/**/
}

/*
*********************************************************************************************************
*	�� �� ��: _LTDC_SetLUTEntry
*	����˵��: ����LUT��ַ�ʹ˵�ַ��Ӧ��RGBֵ
*	��    ��: LayerIndex   ͼ��
*             Color        RGBֵ
*             Pos          RGB ֵ�� CLUT ��ַ�� CLUT �ڵ���ɫλ�ã�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _LTDC_SetLUTEntry(int LayerIndex, U32 Color, int Pos) {
  U32 r, g, b, a;

  r = ( Color        & 0xff) << 16;
  g = ((Color >>  8) & 0xff) <<  8;
  b = ((Color >> 16) & 0xff);
  a = Pos << 24;
  _apLayer[LayerIndex]->CLUTWR &= ~(LTDC_LxCLUTWR_BLUE | LTDC_LxCLUTWR_GREEN | LTDC_LxCLUTWR_RED | LTDC_LxCLUTWR_CLUTADD);
  _apLayer[LayerIndex]->CLUTWR  = r | g | b | a;
  //
  // Reload configuration
  //
  LTDC_ReloadConfig(LTDC_SRCR_IMR);
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_Copy
*	����˵��: ͨ��DMA2D��ǰ���㸴��ָ���������ɫ���ݵ�Ŀ������
*	��    ��: LayerIndex    ͼ��
*             pSrc          ��ɫ����Դ��ַ
*             pDst          ��ɫ����Ŀ�ĵ�ַ
*             xSize         Ҫ���������X���С����ÿ��������
*             ySize         Ҫ���������Y���С��������
*             OffLineSrc    ǰ����ͼ�����ƫ��
*             OffLineDst    �������ƫ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_Copy(int LayerIndex, void * pSrc, void * pDst, int xSize, int ySize, int OffLineSrc, int OffLineDst) {
  U32 PixelFormat;

  PixelFormat = _GetPixelformat(LayerIndex);
  DMA2D->CR      = 0x00000000UL | (1 << 9);         // Control Register (Memory to memory and TCIE)
  DMA2D->FGMAR   = (U32)pSrc;                       // Foreground Memory Address Register (Source address)
  DMA2D->OMAR    = (U32)pDst;                       // Output Memory Address Register (Destination address)
  DMA2D->FGOR    = OffLineSrc;                      // Foreground Offset Register (Source line offset)
  DMA2D->OOR     = OffLineDst;                      // Output Offset Register (Destination line offset)
  DMA2D->FGPFCCR = PixelFormat;                     // Foreground PFC Control Register (Defines the input pixel format)
  DMA2D->NLR     = (U32)(xSize << 16) | (U16)ySize; // Number of Line Register (Size configuration of area to be transfered)
  DMA2D->CR     |= 1;                               // Start operation
  //
  // Wait until transfer is done
  //
  while (DMA2D->CR & DMA2D_CR_START) {
    //__WFI();                                        // Sleep until next interrupt
  }
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_Fill
*	����˵��: ͨ��DMA2D����ָ�����������ɫ���
*	��    ��: LayerIndex    ͼ��
*             pDst          ��ɫ����Ŀ�ĵ�ַ
*             xSize         Ҫ���������X���С����ÿ��������
*             ySize         Ҫ���������Y���С��������
*             OffLine       ǰ����ͼ�����ƫ��
*             ColorIndex    Ҫ������ɫֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_Fill(int LayerIndex, void * pDst, int xSize, int ySize, int OffLine, U32 ColorIndex) {
  U32 PixelFormat;

  PixelFormat = _GetPixelformat(LayerIndex);
  DMA2D->CR      = 0x00030000UL | (1 << 9);         // Register to memory and TCIE
  DMA2D->OCOLR   = ColorIndex;                      // Color to be used
  DMA2D->OMAR    = (U32)pDst;                       // Destination address
  DMA2D->OOR     = OffLine;                         // Destination line offset
  DMA2D->OPFCCR  = PixelFormat;                     // Defines the number of pixels to be transfered
  DMA2D->NLR     = (U32)(xSize << 16) | (U16)ySize; // Size configuration of area to be transfered
  DMA2D->CR     |= 1;                               // Start operation
  //
  // Wait until transfer is done
  //
  while (DMA2D->CR & DMA2D_CR_START) {
    //__WFI();                                        // Sleep until next interrupt
  }
}

/*
*********************************************************************************************************
*	�� �� ��: _GetBufferSize
*	����˵��: ��ȡָ�����Դ��С
*	��    ��: LayerIndex    ͼ��
*	�� �� ֵ: �Դ��С
*********************************************************************************************************
*/
static U32 _GetBufferSize(int LayerIndex) {
  U32 BufferSize;

  BufferSize = _axSize[LayerIndex] * _aySize[LayerIndex] * _aBytesPerPixels[LayerIndex];
  return BufferSize;
}

/*
*********************************************************************************************************
*	�� �� ��: _LCD_CopyBuffer
*	����˵��: �˺������ڶ໺�壬��һ�������е��������ݸ��Ƶ���һ�����塣
*	��    ��: LayerIndex    ͼ��
*             IndexSrc      Դ�������
*             IndexDst      Ŀ�껺�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _LCD_CopyBuffer(int LayerIndex, int IndexSrc, int IndexDst) 
{
	U32 BufferSize, AddrSrc, AddrDst;

	BufferSize = _GetBufferSize(LayerIndex);
	AddrSrc = _aAddr[LayerIndex] + BufferSize * IndexSrc;
	AddrDst = _aAddr[LayerIndex] + BufferSize * IndexDst;

	_DMA_Copy(LayerIndex, (void *)AddrSrc, (void *)AddrDst, _axSize[LayerIndex], _aySize[LayerIndex], 0, 0);
	_aBufferIndex[LayerIndex] = IndexDst;  // After this function has been called all drawing operations are routed to Buffer[IndexDst]!
}

/*
*********************************************************************************************************
*	�� �� ��: _LCD_CopyRect
*	����˵��: �˺������ڶ໺�壬��һ��������ָ���������ݸ��Ƶ���һ�����塣
*	��    ��: LayerIndex    ͼ��
*             x0            Դ����x��λ��
*             y0            Դ����y��λ��
*             x1            Ŀ���x��λ��
*             y1            Ŀ���y��λ��
*             xSize         Ҫ���Ƶ�x���С
*             ySize         Ҫ���Ƶ�y���С
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _LCD_CopyRect(int LayerIndex, int x0, int y0, int x1, int y1, int xSize, int ySize) 
{
	U32 BufferSize, AddrSrc, AddrDst;
	int OffLine;

	BufferSize = _GetBufferSize(LayerIndex);
	AddrSrc = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y0 * _axSize[LayerIndex] + x0) * _aBytesPerPixels[LayerIndex];
	AddrDst = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y1 * _axSize[LayerIndex] + x1) * _aBytesPerPixels[LayerIndex];
	OffLine = _axSize[LayerIndex] - xSize;
	_DMA_Copy(LayerIndex, (void *)AddrSrc, (void *)AddrDst, xSize, ySize, OffLine, OffLine);
}

/*
*********************************************************************************************************
*	�� �� ��: _LCD_FillRect
*	����˵��: ��ָ�������������ɫ���
*	��    ��: LayerIndex    ͼ��
*             x0            ��ʼx��λ��
*             y0            ��ʼy��λ��
*             x1            ����x��λ��
*             y1            ����y��λ��
*             PixelIndex    Ҫ������ɫֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _LCD_FillRect(int LayerIndex, int x0, int y0, int x1, int y1, U32 PixelIndex) {
  U32 BufferSize, AddrDst;
  int xSize, ySize;

  if (GUI_GetDrawMode() == GUI_DM_XOR) {
    LCD_SetDevFunc(LayerIndex, LCD_DEVFUNC_FILLRECT, NULL);
    LCD_FillRect(x0, y0, x1, y1);
    LCD_SetDevFunc(LayerIndex, LCD_DEVFUNC_FILLRECT, (void(*)(void))_LCD_FillRect);
  } else {
    xSize = x1 - x0 + 1;
    ySize = y1 - y0 + 1;
    BufferSize = _GetBufferSize(LayerIndex);
    AddrDst = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y0 * _axSize[LayerIndex] + x0) * _aBytesPerPixels[LayerIndex];
    _DMA_Fill(LayerIndex, (void *)AddrDst, xSize, ySize, _axSize[LayerIndex] - xSize, PixelIndex);
  }
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_DrawBitmapL8
*	����˵��: ��ָ�������������ɫ���
*	��    ��: LayerIndex    ͼ��
*             x0            ��ʼx��λ��
*             y0            ��ʼy��λ��
*             x1            ����x��λ��
*             y1            ����y��λ��
*             PixelIndex    Ҫ������ɫֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _DMA_DrawBitmapL8(void * pSrc, void * pDst,  U32 OffSrc, U32 OffDst, U32 PixelFormatDst, U32 xSize, U32 ySize) {
  //
  // Set up mode
  //
  DMA2D->CR      = 0x00010000UL | (1 << 9);         // Control Register (Memory to memory with pixel format conversion and TCIE)
  //
  // Set up pointers
  //
  DMA2D->FGMAR   = (U32)pSrc;                       // Foreground Memory Address Register (Source address)
  DMA2D->OMAR    = (U32)pDst;                       // Output Memory Address Register (Destination address)
  //
  // Set up offsets
  //
  DMA2D->FGOR    = OffSrc;                          // Foreground Offset Register (Source line offset)
  DMA2D->OOR     = OffDst;                          // Output Offset Register (Destination line offset)
  //
  // Set up pixel format
  //
  DMA2D->FGPFCCR = LTDC_Pixelformat_L8;             // Foreground PFC Control Register (Defines the input pixel format)
  DMA2D->OPFCCR  = PixelFormatDst;                  // Output PFC Control Register (Defines the output pixel format)
  //
  // Set up size
  //
  DMA2D->NLR     = (U32)(xSize << 16) | ySize;      // Number of Line Register (Size configuration of area to be transfered)
  //
  // Execute operation
  //
  DMA2D->CR     |= 1;                               // Start operation
  //
  // Wait until transfer is done
  //
  while (DMA2D->CR & DMA2D_CR_START) {
    //__WFI();                                        // Sleep until next interrupt
  }
}

/*
*********************************************************************************************************
*	�� �� ��: _LCD_DrawBitmap8bpp
*	����˵��: 8bppλͼ����
*	��    ��: --
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _LCD_DrawBitmap8bpp(int LayerIndex, int x, int y, U8 const * p, int xSize, int ySize, int BytesPerLine) {
  U32 BufferSize, AddrDst;
  int OffLineSrc, OffLineDst;
  U32 PixelFormat;

  BufferSize = _GetBufferSize(LayerIndex);
  AddrDst = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y * _axSize[LayerIndex] + x) * _aBytesPerPixels[LayerIndex];
  OffLineSrc = BytesPerLine - xSize;
  OffLineDst = _axSize[LayerIndex] - xSize;
  PixelFormat = _GetPixelformat(LayerIndex);
  _DMA_DrawBitmapL8((void *)p, (void *)AddrDst, OffLineSrc, OffLineDst, PixelFormat, xSize, ySize);
}

/*
*********************************************************************************************************
*	�� �� ��: _LCD_DrawBitmap16bpp
*	����˵��: 16bppλͼ����
*	��    ��: --
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _LCD_DrawBitmap16bpp(int LayerIndex, int x, int y, U16 const * p, int xSize, int ySize, int BytesPerLine) {
  U32 BufferSize, AddrDst;
  int OffLineSrc, OffLineDst;

  BufferSize = _GetBufferSize(LayerIndex);
  AddrDst = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y * _axSize[LayerIndex] + x) * _aBytesPerPixels[LayerIndex];
  OffLineSrc = (BytesPerLine / 2) - xSize;
  OffLineDst = _axSize[LayerIndex] - xSize;
  _DMA_Copy(LayerIndex, (void *)p, (void *)AddrDst, xSize, ySize, OffLineSrc, OffLineDst);
}

/*
*********************************************************************************************************
*	�� �� ��: _LCD_DrawBitmap32bpp
*	����˵��: 32bppλͼ����
*	��    ��: --
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _LCD_DrawBitmap32bpp(int LayerIndex, int x, int y, U8 const * p, int xSize, int ySize, int BytesPerLine)
{
  U32 BufferSize, AddrDst;
  int OffLineSrc, OffLineDst;

  BufferSize = _GetBufferSize(LayerIndex);
  AddrDst = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y * _axSize[LayerIndex] + x) * _aBytesPerPixels[LayerIndex];
  OffLineSrc = (BytesPerLine / 4) - xSize;
  OffLineDst = _axSize[LayerIndex] - xSize;
  _DMA_Copy(LayerIndex, (void *)p, (void *)AddrDst, xSize, ySize, OffLineSrc, OffLineDst);
}


/*
*********************************************************************************************************
*	�� �� ��: _LCD_DisplayOn
*	����˵��: ��LCD
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _LCD_DisplayOn(void) 
{
	// Enable LCD Backlight
	LCD_SetBackLight(255);
	
	// Display On
	LTDC_Cmd(ENABLE);
}

/*
*********************************************************************************************************
*	�� �� ��: _LCD_DisplayOff
*	����˵��: �ر�LCD
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _LCD_DisplayOff(void) 
{
	// Disable LCD Backlight
	LCD_SetBackLight(0);
	
	// Display Off
	LTDC_Cmd(DISABLE);
}

/*
*********************************************************************************************************
*	�� �� ��: DMA2D_IRQHandler
*	����˵��: DMA2D��������жϷ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DMA2D_IRQHandler(void) 
{
	DMA2D->IFCR = (U32)DMA2D_IFSR_CTCIF;
}

/*
*********************************************************************************************************
*	�� �� ��: LTDC_IRQHandler
*	����˵��: LTDC֡�жϣ����ڹ���໺��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LTDC_IRQHandler(void) 
{
	U32 Addr;
	int i;
	
#if uCOS_EN == 1
	CPU_SR_ALLOC();

	CPU_CRITICAL_ENTER();
	OSIntEnter();                         
	CPU_CRITICAL_EXIT();
#endif

	LTDC->ICR = (U32)LTDC_IER_LIE;
	for (i = 0; i < GUI_NUM_LAYERS; i++) 
	{
		if (_aPendingBuffer[i] >= 0) 
		{
			//
			// Calculate address of buffer to be used as visible frame buffer
			//
			Addr = _aAddr[i] + _axSize[i] * _aySize[i] * _aPendingBuffer[i] * _aBytesPerPixels[i];
			
			//
			// Store address into SFR
			//
			_apLayer[i]->CFBAR &= ~(LTDC_LxCFBAR_CFBADD);   
			_apLayer[i]->CFBAR = Addr;
			
			//
			// Reload configuration
			//
			LTDC_ReloadConfig(LTDC_SRCR_IMR);
			
			//
			// Tell emWin that buffer is used
			//
			GUI_MULTIBUF_ConfirmEx(i, _aPendingBuffer[i]);
			
			//
			// Clear pending buffer flag of layer
			//
			_aPendingBuffer[i] = -1;
		}
	}
	
#if uCOS_EN == 1
	OSIntExit();                           
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: _LCD_InitController
*	����˵��: LCD��ʼ��
*	��    ��: LayerIndex  ѡ��ͼ��0����1
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _LCD_InitController(int LayerIndex) 
{
	LTDC_Layer_InitTypeDef LTDC_Layer_InitStruct = {0};
	int xSize, ySize, BytesPerLine, BitsPerPixel, i;
	U32 Pixelformat, Color;
	static int Done;

	if (LayerIndex >= GUI_COUNTOF(_apLayer)) 
	{
		return;
	}
	
	if (Done == 0) 
	{
		Done = 1;

		//
		// Enable line interrupt
		//
		LTDC_ITConfig(LTDC_IER_LIE, ENABLE);
		NVIC_SetPriority(LTDC_IRQn, 0);
		NVIC_EnableIRQ(LTDC_IRQn);
		
		#if emWin_Optimize 
			//
			// Enable DMA2D transfer complete interrupt
			//
			DMA2D_ITConfig(DMA2D_CR_TCIE, ENABLE);
			NVIC_SetPriority(DMA2D_IRQn, 0);
			NVIC_EnableIRQ(DMA2D_IRQn);
			//
			// Clear transfer complete interrupt flag
			//
			DMA2D->IFCR = (U32)DMA2D_IFSR_CTCIF;
		#endif
	}
	
	//
	// Layer configuration
	//
	xSize = LCD_GetXSizeEx(LayerIndex);
	ySize = LCD_GetYSizeEx(LayerIndex);

	// HorizontalStart = (Offset_X + Hsync + HBP);
    // HorizontalStop  = (Offset_X + Hsync + HBP + Window_Width - 1); 
    // VarticalStart   = (Offset_Y + Vsync + VBP);
    // VerticalStop    = (Offset_Y + Vsync + VBP + Window_Heigh - 1);
	
	LTDC_Layer_InitStruct.LTDC_HorizontalStart = HSYNC_W + HBP + 1;
	LTDC_Layer_InitStruct.LTDC_HorizontalStop = (Width + LTDC_Layer_InitStruct.LTDC_HorizontalStart - 1);
	LTDC_Layer_InitStruct.LTDC_VerticalStart = VSYNC_W + VBP + 1; 
	LTDC_Layer_InitStruct.LTDC_VerticalStop = (Height + LTDC_Layer_InitStruct.LTDC_VerticalStart - 1);

	//
	// Pixel Format configuration
	//
	Pixelformat = _GetPixelformat(LayerIndex);
	LTDC_Layer_InitStruct.LTDC_PixelFormat = Pixelformat;
	
	//
	// Alpha constant (255 totally opaque)
	//
	LTDC_Layer_InitStruct.LTDC_ConstantAlpha = 255;
	
	//
	// Default Color configuration (configure A, R, G, B component values)
	//
	LTDC_Layer_InitStruct.LTDC_DefaultColorBlue  = 0;
	LTDC_Layer_InitStruct.LTDC_DefaultColorGreen = 0;
	LTDC_Layer_InitStruct.LTDC_DefaultColorRed   = 0;
	LTDC_Layer_InitStruct.LTDC_DefaultColorAlpha = 0;
	
	//
	// Configure blending factors
	//
	BytesPerLine = _GetBytesPerLine(LayerIndex, xSize);
	LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_PAxCA;
	LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_PAxCA;
	LTDC_Layer_InitStruct.LTDC_CFBLineLength    = BytesPerLine + 3;
	LTDC_Layer_InitStruct.LTDC_CFBPitch         = BytesPerLine;
	LTDC_Layer_InitStruct.LTDC_CFBLineNumber    = ySize;
	
	//
	// Input Address configuration
	//
	LTDC_Layer_InitStruct.LTDC_CFBStartAdress = _aAddr[LayerIndex];
	LTDC_LayerInit(_apLayer[LayerIndex], &LTDC_Layer_InitStruct);
	
	//
	// Enable LUT on demand
	//
	BitsPerPixel = LCD_GetBitsPerPixelEx(LayerIndex);
	if (BitsPerPixel <= 8) 
	{
		//
		// Enable usage of LUT for all modes with <= 8bpp
		//
		_LTDC_LayerEnableLUT(_apLayer[LayerIndex], ENABLE);
		
		//
		// Optional CLUT initialization for L8 mode (8bpp)
		//
		if (_apColorConvAPI[LayerIndex] == GUICC_1616I) 
		{
			for (i = 0; i < 16; i++) 
			{
				Color = LCD_API_ColorConv_1616I.pfIndex2Color(i);
				_LTDC_SetLUTEntry(LayerIndex, Color, i);
			}			
		}

		//
		// Optional CLUT initialization for AL44 mode (8bpp)
		//
		if (_apColorConvAPI[LayerIndex] == GUICC_8666) 
		{
			for (i = 0; i < 16; i++) 
			{
				Color = LCD_API_ColorConv_8666.pfIndex2Color(i);
				_LTDC_SetLUTEntry(LayerIndex, Color, i);
			}			
		}
	} 
	else 
	{
		//
		// Optional CLUT initialization for AL88 mode (16bpp)
		//
		if (_apColorConvAPI[LayerIndex] == GUICC_88666I) 
		{
			_LTDC_LayerEnableLUT(_apLayer[LayerIndex], ENABLE);
			for (i = 0; i < 256; i++) 
			{
				Color = LCD_API_ColorConv_8666.pfIndex2Color(i);
				_LTDC_SetLUTEntry(LayerIndex, Color, i);
			}
		}
	}
	
	//
	// Enable layer
	//
	LTDC_LayerCmd(_apLayer[LayerIndex], ENABLE);

	//
	// Reload configuration
	//
	LTDC_ReloadConfig(LTDC_SRCR_IMR);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_X_Config
*	����˵��: LCD����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_X_Config(void) 
{
	int i;

	//
	// At first initialize use of multiple buffers on demand
	//
	#if (NUM_BUFFERS > 1)
		for (i = 0; i < GUI_NUM_LAYERS; i++) 
		{
			GUI_MULTIBUF_ConfigEx(i, NUM_BUFFERS);
		}
	#endif
		
	//
	// Set display driver and color conversion for 1st layer
	//
	GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER_0, COLOR_CONVERSION_0, 0, 0);
		
	//
	// Set size of 1st layer
	//
	LCD_SetSizeEx (0, g_LcdWidth, g_LcdHeight);
	LCD_SetVSizeEx(0, g_LcdWidth, g_LcdHeight * NUM_VSCREENS);
	#if (GUI_NUM_LAYERS > 1)
		//
		// Set display driver and color conversion for 2nd layer
		//
		GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER_1, COLOR_CONVERSION_1, 0, 1);
		
		//
		// Set size of 2nd layer
		//
		LCD_SetSizeEx (1, g_LcdWidth, g_LcdHeight);
		LCD_SetVSizeEx(1, g_LcdWidth, g_LcdHeight * NUM_VSCREENS);
	#endif
	
	//
	// Setting up VRam address and custom functions for CopyBuffer-, CopyRect- and FillRect operations
	//
	for (i = 0; i < GUI_NUM_LAYERS; i++) 
	{
		_aPendingBuffer[i] = -1;
		
		//
		// Set VRAM address
		//
		LCD_SetVRAMAddrEx(i, (void *)(_aAddr[i]));
		
		//
		// Remember color depth for further operations
		//
		_aBytesPerPixels[i] = LCD_GetBitsPerPixelEx(i) >> 3;
		
		//
		// Set custom functions for several operations
		//
		LCD_SetDevFunc(i, LCD_DEVFUNC_COPYBUFFER, (void(*)(void))_LCD_CopyBuffer);
		LCD_SetDevFunc(i, LCD_DEVFUNC_COPYRECT,   (void(*)(void))_LCD_CopyRect);
		
		//
		// Filling via DMA2D does only work with 16bpp or more
		//
		if (_GetPixelformat(i) <= LTDC_Pixelformat_ARGB4444) 
		{
			LCD_SetDevFunc(i, LCD_DEVFUNC_FILLRECT, (void(*)(void))_LCD_FillRect);
			LCD_SetDevFunc(i, LCD_DEVFUNC_DRAWBMP_8BPP, (void(*)(void))_LCD_DrawBitmap8bpp); 
		}
		
		//
		// Set up drawing routine for 16bpp bitmap using DMA2D
		//
		if (_GetPixelformat(i) == LTDC_Pixelformat_RGB565) 
		{
			LCD_SetDevFunc(i, LCD_DEVFUNC_DRAWBMP_16BPP, (void(*)(void))_LCD_DrawBitmap16bpp);     // Set up drawing routine for 16bpp bitmap using DMA2D. Makes only sense with RGB565
		}

		//
		// Set up drawing routine for 32bpp bitmap using DMA2D
		//
		if (_GetPixelformat(i) == LTDC_Pixelformat_ARGB8888) 
		{
			LCD_SetDevFunc(i, LCD_DEVFUNC_DRAWBMP_32BPP, (void(*)(void))_LCD_DrawBitmap32bpp);     // Set up drawing routine for 16bpp bitmap using DMA2D. Makes only sense with RGB565
		}

		//
		// Set up custom color conversion using DMA2D, works only for direct color modes because of missing LUT for DMA2D destination
		//
		GUICC_M1555I_SetCustColorConv(_Color2IndexBulk_M1555I_DMA2D, _Index2ColorBulk_M1555I_DMA2D); // Set up custom bulk color conversion using DMA2D for ARGB1555
		GUICC_M565_SetCustColorConv  (_Color2IndexBulk_M565_DMA2D,   _Index2ColorBulk_M565_DMA2D);   // Set up custom bulk color conversion using DMA2D for RGB565
		GUICC_M4444I_SetCustColorConv(_Color2IndexBulk_M4444I_DMA2D, _Index2ColorBulk_M4444I_DMA2D); // Set up custom bulk color conversion using DMA2D for ARGB4444
		GUICC_M888_SetCustColorConv  (_Color2IndexBulk_M888_DMA2D,   _Index2ColorBulk_M888_DMA2D);   // Set up custom bulk color conversion using DMA2D for RGB888
		GUICC_M8888I_SetCustColorConv(_Color2IndexBulk_M8888I_DMA2D, _Index2ColorBulk_M8888I_DMA2D); // Set up custom bulk color conversion using DMA2D for ARGB8888

		//
		// Set up custom alpha blending function using DMA2D
		//
		GUI_SetFuncAlphaBlending(_DMA_AlphaBlending); 
		
		//
		// Set up custom function for translating a bitmap palette into index values.
		// Required to load a bitmap palette into DMA2D CLUT in case of a 8bpp indexed bitmap
		//
		GUI_SetFuncGetpPalConvTable(_LCD_GetpPalConvTable);
		
		//
		// Set up a custom function for mixing up single colors using DMA2D
		//
		#if emWin_Optimize 
			GUI_SetFuncMixColors(_DMA_MixColors);
		#endif
		
		//
		// Set up a custom function for mixing up arrays of colors using DMA2D
		//
		GUI_SetFuncMixColorsBulk(_LCD_MixColorsBulk);
	}
}

/*
*********************************************************************************************************
*       LCD_X_DisplayDriver
*
* Purpose:
*   This function is called by the display driver for several purposes.
*   To support the according task the routine needs to be adapted to
*   the display controller. Please note that the commands marked with
*   'optional' are not cogently required and should only be adapted if
*   the display controller supports these features.
*
* Parameter:
*   LayerIndex - Index of layer to be configured
*   Cmd        - Please refer to the details in the switch statement below
*   pData      - Pointer to a LCD_X_DATA structure
*
* Return Value:
*   < -1 - Error
*     -1 - Command not handled
*      0 - Ok
*********************************************************************************************************
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) 
{
	int r = 0;

	switch (Cmd) 
	{
		case LCD_X_INITCONTROLLER: 
			{
				//
				// Called during the initialization process in order to set up the display controller and put it into operation.
				//
				_LCD_InitController(LayerIndex);
				break;
			}
			
		case LCD_X_SETORG: 
			{
				//
				// Required for setting the display origin which is passed in the 'xPos' and 'yPos' element of p
				//
				LCD_X_SETORG_INFO * p;

				p = (LCD_X_SETORG_INFO *)pData;
				_apLayer[LayerIndex]->CFBAR = _aAddr[LayerIndex] + p->yPos * _axSize[LayerIndex] * _aBytesPerPixels[LayerIndex];
				LTDC_ReloadConfig(LTDC_SRCR_VBR); // Reload on next blanking period
				break;
			}
			
		case LCD_X_SHOWBUFFER: 
			{
				//
				// Required if multiple buffers are used. The 'Index' element of p contains the buffer index.
				//
				LCD_X_SHOWBUFFER_INFO * p;

				p = (LCD_X_SHOWBUFFER_INFO *)pData;
				_aPendingBuffer[LayerIndex] = p->Index;
				break;
			}
			
		case LCD_X_SETLUTENTRY: 
			{
				//
				// Required for setting a lookup table entry which is passed in the 'Pos' and 'Color' element of p
				//
				LCD_X_SETLUTENTRY_INFO * p;

				p = (LCD_X_SETLUTENTRY_INFO *)pData;
				_LTDC_SetLUTEntry(LayerIndex, p->Color, p->Pos);
				break;
			}
		case LCD_X_ON: 
			{
				//
				// Required if the display controller should support switching on and off
				//
				_LCD_DisplayOn();
				break;
			}
			
		case LCD_X_OFF:
			{
				//
				// Required if the display controller should support switching on and off
				//
				_LCD_DisplayOff();
				break;
			}
			
		case LCD_X_SETVIS:
			{
				//
				// Required for setting the layer visibility which is passed in the 'OnOff' element of pData
				//
				LCD_X_SETVIS_INFO * p;

				p = (LCD_X_SETVIS_INFO *)pData;
				LTDC_LayerCmd(_apLayer[LayerIndex], p->OnOff ? ENABLE : DISABLE);

				/* Reload shadow register */
				LTDC_ReloadConfig(LTDC_SRCR_IMR);
				break;
			}
			
		case LCD_X_SETPOS: 
			{
				//
				// Required for setting the layer position which is passed in the 'xPos' and 'yPos' element of pData
				//
				LCD_X_SETPOS_INFO * p;

				p = (LCD_X_SETPOS_INFO *)pData;
				_LTDC_SetLayerPos(LayerIndex, p->xPos, p->yPos);
				break;
			}
			
		case LCD_X_SETSIZE: 
			{
				//
				// Required for setting the layer position which is passed in the 'xPos' and 'yPos' element of pData
				//
				LCD_X_SETSIZE_INFO * p;
				int xPos, yPos;

				GUI_GetLayerPosEx(LayerIndex, &xPos, &yPos);
				p = (LCD_X_SETSIZE_INFO *)pData;
				_axSize[LayerIndex] = p->xSize;
				_aySize[LayerIndex] = p->ySize;
				_LTDC_SetLayerPos(LayerIndex, xPos, yPos);
				break;
			}
			
		case LCD_X_SETALPHA: 
			{
				//
				// Required for setting the alpha value which is passed in the 'Alpha' element of pData
				//
				LCD_X_SETALPHA_INFO * p;

				p = (LCD_X_SETALPHA_INFO *)pData;
				_LTDC_SetLayerAlpha(LayerIndex, p->Alpha);
				break;
			}
			
		case LCD_X_SETCHROMAMODE: 
			{
				//
				// Required for setting the chroma mode which is passed in the 'ChromaMode' element of pData
				//
				LCD_X_SETCHROMAMODE_INFO * p;

				p = (LCD_X_SETCHROMAMODE_INFO *)pData;
				_LTDC_LayerEnableColorKeying(_apLayer[LayerIndex], (p->ChromaMode != 0) ? ENABLE : DISABLE);
				break;
			}
			
		case LCD_X_SETCHROMA: 
			{
				//
				// Required for setting the chroma value which is passed in the 'ChromaMin' and 'ChromaMax' element of pData
				//
				LCD_X_SETCHROMA_INFO * p;
				U32 Color;

				p = (LCD_X_SETCHROMA_INFO *)pData;
				Color = ((p->ChromaMin & 0xFF0000) >> 16) | (p->ChromaMin & 0x00FF00) | ((p->ChromaMin & 0x0000FF) << 16);
				_apLayer[LayerIndex]->CKCR = Color;
				LTDC_ReloadConfig(LTDC_SRCR_VBR); // Reload on next blanking period
				break;
			}
		
		default:
			r = -1;
	}
	
	return r;
}

/*************************** End of file ****************************/
