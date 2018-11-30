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
File        : MOTION_MovingWindow.c
Purpose     : Shows how to use simple motion support
Requirements: WindowManager - (x)
              MemoryDevices - (x)
              AntiAliasing  - ( )
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )
----------------------------------------------------------------------
*/

//
// Recommended memory to run the sample with adequate performance
//
#define RECOMMENDED_MEMORY (1024L * 200)

/*********************************************************************
*
*       Includes
*
**********************************************************************
*/
#include <stdlib.h>
#include "GUI.h"
#include "WM.h"
#include "num_aqi.h"
#include "animation_management.h"
#include "includes.h"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  int Axis;
  int Speed;
} PARA;

static const PARA _aPara[] = {
	  //{ GUI_COORD_X, +1000 },
	  { GUI_COORD_Y, +1 },
	  //{ GUI_COORD_X, -1000 },
	  { GUI_COORD_Y, -480 },
	};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

 //
// Config
//
//#define SHOW_ALPHA       0
#define SHOW_PALETTE     1
//#define SHOW_BMP_GIF_JPG 0
//#define SHOW_HIGH_COLOR  0
//#define SHOW_GRAYSCALE   0

//
// General
//
#if SHOW_ALPHA
  #define MAX_TIME_ALPHA    5000
  #if GUI_SUPPORT_MEMDEV
    #define NUM_STEPS_ALPHA 10
  #endif
#endif

#if SHOW_PALETTE
  #define MAX_TIME_PAL      5000
  #define NUM_STEPS_PAL     10
#endif

#if SHOW_BMP_GIF_JPG
  #define NUM_IMAGES_BGJ    3
#endif

#if SHOW_GRAYSCALE
  #define BITMAP_SIZE_X_4   45
#endif

#if (SHOW_PALETTE || SHOW_HIGH_COLOR || SHOW_BMP_GIF_JPG)
  #define BITMAP_SIZE_X_3   70
#endif

#if (SHOW_PALETTE || SHOW_HIGH_COLOR || SHOW_BMP_GIF_JPG || SHOW_GRAYSCALE)
  #define BITMAP_SIZE_Y     70
  #define X_BORDER          20
#endif

/*********************************************************************
*
*       Structures
*
**********************************************************************
*/
#if 0
  typedef struct {
    const GUI_BITMAP *  pBitmap;
    #if SHOW_PALETTE
      GUI_COLOR        *  pColorsDst;
      const GUI_COLOR  ** ppColorsSrc;
      unsigned            NumColors;
    #endif
  } PAL_BITMAP_CONTEXT;
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
/*********************************************************************
*
*       Alpha specific
*/
#if SHOW_ALPHA
  static const GUI_COLOR _aAlphaColors[] = {
    GUI_WHITE,
    GUI_LIGHTCYAN,
    GUI_GREEN,
    GUI_YELLOW,
    GUI_ORANGE,
    GUI_RED,
  };
#endif

	/******************************************************************************/

	

    /******************************************************************************/

	static const GUI_COLOR _Colorsface_Original[] ={
	
		0x1B1D1D, 0xFFFFFF, 0xF0F0F0, 0xC6C6C6,
		0x545555, 0x292B2B, 0xA9AAAA, 0x454747,
		0x8D8E8E, 0x707171, 0x9B9C9C, 0xE2E2E2,
		0xD4D4D4, 0x373939, 0x7E7F7F, 0xB7B8B8,
		0x626363};
	
	  static GUI_CONST_STORAGE GUI_COLOR _Colorsface_Original1[] = {
		0x1B1D1D, 0xFFFFFF, 0xF0F0F0, 0xD4D4D4,
		0x545555, 0x292B2B, 0xA9AAAA, 0x454747,
		0x8D8E8E, 0x707171, 0x9B9C9C, 0xE2E2E2,
		0xB7B8B8, 0x373939, 0x7E7F7F, 0x626363,
		0xC6C6C6
	  };
	
	
	  static GUI_CONST_STORAGE GUI_COLOR _Colorsface_Original2[] = {
		0x000000, 0x0000FF, 0x00FF00, 0x00FFFF,
		0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF
	  };
	
	  static GUI_CONST_STORAGE GUI_COLOR _Colorsface_Original3[] = {
		0x000000, 0x010101, 0x020202, 0x030303,
		0x040404, 0x050505, 0x060606, 0x070707,
		0x080808, 0x090909, 0x0A0A0A, 0x0B0B0B,
		0x0C0C0C, 0x0D0D0D, 0x0E0E0E, 0x0F0F0F,
		0x101010, 0x111111, 0x121212, 0x131313,
		0x141414, 0x151515, 0x161616, 0x171717,
		0x181818, 0x191919, 0x1A1A1A, 0x1B1B1B,
		0x1C1C1C, 0x1D1D1D, 0x1E1E1E, 0x1F1F1F,
		0x202020, 0x212121, 0x222222, 0x232323,
		0x242424, 0x252525, 0x262626, 0x272727,
		0x282828, 0x292929, 0x2A2A2A, 0x2B2B2B,
		0x2C2C2C, 0x2D2D2D, 0x2E2E2E, 0x2F2F2F,
		0x303030, 0x313131, 0x323232, 0x333333,
		0x343434, 0x353535, 0x363636, 0x373737,
		0x383838, 0x393939, 0x3A3A3A, 0x3B3B3B,
		0x3C3C3C, 0x3D3D3D, 0x3E3E3E, 0x3F3F3F,
		0x404040, 0x414141, 0x424242, 0x434343,
		0x444444, 0x454545, 0x464646, 0x474747,
		0x484848, 0x494949, 0x4A4A4A, 0x4B4B4B,
		0x4C4C4C, 0x4D4D4D, 0x4E4E4E, 0x4F4F4F,
		0x505050, 0x515151, 0x525252, 0x535353,
		0x545454, 0x555555, 0x565656, 0x575757,
		0x585858, 0x595959, 0x5A5A5A, 0x5B5B5B,
		0x5C5C5C, 0x5D5D5D, 0x5E5E5E, 0x5F5F5F,
		0x606060, 0x616161, 0x626262, 0x636363,
		0x646464, 0x656565, 0x666666, 0x676767,
		0x686868, 0x696969, 0x6A6A6A, 0x6B6B6B,
		0x6C6C6C, 0x6D6D6D, 0x6E6E6E, 0x6F6F6F,
		0x707070, 0x717171, 0x727272, 0x737373,
		0x747474, 0x757575, 0x767676, 0x777777,
		0x787878, 0x797979, 0x7A7A7A, 0x7B7B7B,
		0x7C7C7C, 0x7D7D7D, 0x7E7E7E, 0x7F7F7F,
		0x808080, 0x818181, 0x828282, 0x838383,
		0x848484, 0x858585, 0x868686, 0x878787,
		0x888888, 0x898989, 0x8A8A8A, 0x8B8B8B,
		0x8C8C8C, 0x8D8D8D, 0x8E8E8E, 0x8F8F8F,
		0x909090, 0x919191, 0x929292, 0x939393,
		0x949494, 0x959595, 0x969696, 0x979797,
		0x989898, 0x999999, 0x9A9A9A, 0x9B9B9B,
		0x9C9C9C, 0x9D9D9D, 0x9E9E9E, 0x9F9F9F,
		0xA0A0A0, 0xA1A1A1, 0xA2A2A2, 0xA3A3A3,
		0xA4A4A4, 0xA5A5A5, 0xA6A6A6, 0xA7A7A7,
		0xA8A8A8, 0xA9A9A9, 0xAAAAAA, 0xABABAB,
		0xACACAC, 0xADADAD, 0xAEAEAE, 0xAFAFAF,
		0xB0B0B0, 0xB1B1B1, 0xB2B2B2, 0xB3B3B3,
		0xB4B4B4, 0xB5B5B5, 0xB6B6B6, 0xB7B7B7,
		0xB8B8B8, 0xB9B9B9, 0xBABABA, 0xBBBBBB,
		0xBCBCBC, 0xBDBDBD, 0xBEBEBE, 0xBFBFBF,
		0xC0C0C0, 0xC1C1C1, 0xC2C2C2, 0xC3C3C3,
		0xC4C4C4, 0xC5C5C5, 0xC6C6C6, 0xC7C7C7,
		0xC8C8C8, 0xC9C9C9, 0xCACACA, 0xCBCBCB,
		0xCCCCCC, 0xCDCDCD, 0xCECECE, 0xCFCFCF,
		0xD0D0D0, 0xD1D1D1, 0xD2D2D2, 0xD3D3D3,
		0xD4D4D4, 0xD5D5D5, 0xD6D6D6, 0xD7D7D7,
		0xD8D8D8, 0xD9D9D9, 0xDADADA, 0xDBDBDB,
		0xDCDCDC, 0xDDDDDD, 0xDEDEDE, 0xDFDFDF,
		0xE0E0E0, 0xE1E1E1, 0xE2E2E2, 0xE3E3E3,
		0xE4E4E4, 0xE5E5E5, 0xE6E6E6, 0xE7E7E7,
		0xE8E8E8, 0xE9E9E9, 0xEAEAEA, 0xEBEBEB,
		0xECECEC, 0xEDEDED, 0xEEEEEE, 0xEFEFEF,
		0xF0F0F0, 0xF1F1F1, 0xF2F2F2, 0xF3F3F3,
		0xF4F4F4, 0xF5F5F5, 0xF6F6F6, 0xF7F7F7,
		0xF8F8F8, 0xF9F9F9, 0xFAFAFA, 0xFBFBFB,
		0xFCFCFC, 0xFDFDFD, 0xFEFEFE, 0xFFFFFF
	  };
	
	  static GUI_CONST_STORAGE GUI_COLOR _Colorsface_Original4[] = {
		0x000000, 0xFFFFFF, 0xAAAAAA, 0x555555
	  };
	
	  static GUI_CONST_STORAGE GUI_COLOR _Colorsface_Original5[] = {
		0x000000, 0xFFFFFF, 0xAAAAAA, 0x555555
	  };


static const GUI_COLOR * _aPalettesHouse[] = {
  _Colorsface_Original3,
  _Colorsface_Original3,
  _Colorsface_Original5,
  _Colorsface_Original5,
};

static const GUI_COLOR * _aPalettesHouse_Bk[] = {
  _Colorsface_Original5,
  _Colorsface_Original3,
  _Colorsface_Original3
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUIDEMO_MixColors
*/
static GUI_COLOR GUIDEMO_MixColors(GUI_COLOR Color, GUI_COLOR Color0, U8 Intens) {
  U32 r, g, b, a;
  U8  Intens0;

#if (GUI_USE_ARGB)
  if (Color0 & 0xFF000000) {
#else
  if ((Color0 & 0xFF000000) != 0xFF000000) {
#endif
    //
    // Calculate Color separations for FgColor first
    //
    r = ( Color & 0x000000ff)        * Intens;
    g = ( Color & 0x0000ff00)        * Intens;
    b = ( Color & 0x00ff0000)        * Intens;
    a = ((Color & 0xff000000) >> 24) * Intens;
    //
    // Add Color separations for Color0
    //
    Intens0 = 255 - Intens;
    r += ( Color0 & 0x000000ff)        * Intens0;
    g += ( Color0 & 0x0000ff00)        * Intens0;
    b += ( Color0 & 0x00ff0000)        * Intens0;
    a += ((Color0 & 0xff000000) >> 24) * Intens0;
    r =  (r >> 8) & 0x000000ff;
    g =  (g >> 8) & 0x0000ff00;
    b =  (b >> 8) & 0x00ff0000;
    a =  (a << (24 - 8)) & 0xff000000;
    Color = r | g | b | a;
  }
  return Color;
}

  static void _SetPalette(const PAL_BITMAP_CONTEXT * pPalBitmap, unsigned NumPalBitmaps, unsigned PalIndex, U8 Intens) {
  const GUI_COLOR * pColorsSrc0;
  const GUI_COLOR * pColorsSrc1;
  GUI_COLOR       * pColorsDst;
  unsigned          NumColors;
  unsigned          i;
  unsigned          j;

  for (i = 0; i < NumPalBitmaps; i++) {
    pColorsSrc0 = *((pPalBitmap + i)->ppColorsSrc + PalIndex);
    pColorsSrc1 = *((pPalBitmap + i)->ppColorsSrc + PalIndex + 1);
    pColorsDst = (pPalBitmap + i)->pColorsDst;
    NumColors  = (pPalBitmap + i)->NumColors;
	
    for (j = 0; j < NumColors; j++) {
      *(pColorsDst + j) = GUIDEMO_MixColors(*(pColorsSrc0 + j), *(pColorsSrc1 + j), Intens);
    }
	
  }
}

#define HOUSE_FACE_XPOS (295u) 
#define HOUSE_FACE_YPOS (190u) 
#define HOUSE_FACE_CLR_EYE_XSIZE (212u)
#define HOUSE_FACE_CLR_EYE_YSIZE (50u)
#define HOUSE_FACE_CLR_MTH_XSIZE (212u)
#define HOUSE_FACE_CLR_MTH_YSIZE (70u)
#define FACE_OFFSET (8u)


GUI_MEMDEV_Handle hMem0;
GUI_MEMDEV_Handle hMem1;
//#define COLOR_CONF GUICC_8888
//static unsigned int Alpha = 0;
//extern int AnimFlyIn_xpos;
//extern int anim_xpos;
extern GUI_CONST_STORAGE GUI_BITMAP bmhouse;


/*********************************************************************
*
*       _cbWin1
*/
static void _cbWin1(WM_MESSAGE * pMsg) {
  int xSize, ySize,i = 255;

  switch (pMsg->MsgId) {
  case WM_PAINT:
    xSize = WM_GetWindowSizeX(pMsg->hWin);
    ySize = WM_GetWindowSizeY(pMsg->hWin);
    //GUI_DrawGradientV(0, 0, xSize , ySize, GUI_BLACK, GUI_BLACK);
    GUI_SetColor(GUI_GREEN);
    GUI_FillRect(0, 0, HOUSE_FACE_CLR_EYE_XSIZE - 5, HOUSE_FACE_CLR_EYE_YSIZE - 5);
    break;
  }
}

static void _cbWin2(WM_MESSAGE * pMsg) {
  int xSize, ySize;
  unsigned i, j;
  
  switch (pMsg->MsgId) {
  case WM_PAINT:
  	xSize = LCD_GetXSize();
    ySize = LCD_GetYSize();
  	GUI_DrawGradientV(0, 0, xSize, ySize, GUI_BLACK, GUI_BLACK);
		GUI_SetColor(GUI_GREEN);
    GUI_DrawRect(0, 0, HOUSE_FACE_CLR_MTH_XSIZE -5, HOUSE_FACE_CLR_MTH_YSIZE -5);
    break;
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
*       
*/
#if 0
int NormalWorkModeAnimation(void) {
	WM_HWIN  hWin0;
	WM_HWIN  hWin1;
	WM_HWIN  hWin2;

	int      xSize, ySize;
	unsigned i, j;
	unsigned NumPalBitmaps;

	NumPalBitmaps = GUI_COUNTOF(_apPaletteBitmapsSmile);
	xSize = LCD_GetXSize();
	ySize = LCD_GetYSize();

	#if 0
	//hWin0 = WM_CreateWindowAsChild(0, 0, xSize, ySize, WM_HBKWIN, WM_CF_HASTRANS | WM_CF_MEMDEV | WM_CF_SHOW, NULL/*_cbWin0*/ ,0);
	
	//WM_SetCallback(WM_HBKWIN, _cbWin0);
	//WM_InvalidateWindow(hWin0);
	hWin1 = WM_CreateWindowAsChild(296, 183, HOUSE_FACE_CLR_EYE_XSIZE, HOUSE_FACE_CLR_EYE_YSIZE, WM_HBKWIN, WM_CF_MEMDEV | WM_CF_SHOW | WM_CF_MOTION_Y, _cbWin1, 0);
	//hWin2 = WM_CreateWindowAsChild(296, 316, HOUSE_FACE_CLR_MTH_XSIZE, HOUSE_FACE_CLR_MTH_YSIZE, hWin0, WM_CF_MEMDEV | WM_CF_SHOW | WM_CF_MOTION_Y, _cbWin2, 0);
  	#endif
	//
	// œ‘œ÷—€æ¶£¨’ˆ—€
	//
	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse) - 1; i++) {
	for (j = NUM_STEPS_PAL; j > 0; j--) {
	  	_SetPalette(_apPaletteBitmapsSmile, NumPalBitmaps, i, 255 * j / NUM_STEPS_PAL);
		GUI_DrawBitmap(&bmhouse_smile, (xSize  - bmhouse_smile.XSize) / 2 + FACE_OFFSET, HOUSE_FACE_YPOS);
		WM_MoveWindow(hWin1, 0, -2);
		GUI_Delay(90);
	}
	}

	
	//
	// ±’—€°¢’ˆ—€
	//
	for(i = 0;i < 26;i ++)
	{
		WM_MoveWindow(hWin1, 0, 2);
		GUI_DrawBitmap(&bmhouse_smile, (xSize  - bmhouse_smile.XSize) / 2 + FACE_OFFSET, HOUSE_FACE_YPOS);
		
		GUI_Delay(50);
		
	}
	for(i = 0;i < 35;i ++)
	{
		WM_MoveWindow(hWin1, 0, -2);
		GUI_DrawBitmap(&bmhouse_smile, (xSize  - bmhouse_smile.XSize) / 2 + FACE_OFFSET, HOUSE_FACE_YPOS);
		
		GUI_Delay(30);
	}
	#if 0
	//
	// ’≈◊Ï
	//
	for(i = 0;i < 35;i ++)
	{
		GUI_Delay(30);
		GUI_DrawBitmap(&bmhouse_smile, HOUSE_FACE_XPOS, HOUSE_FACE_YPOS);
		WM_MoveWindow(hWin2, 0, -2);
	}
	#endif
	//
	// ±Ì«Èœ˚“˛
	//
	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse_Bk) - 1; i++) {
	for (j = NUM_STEPS_PAL; j > 0; j--) {
	  	_SetPalette(_apPaletteBitmapsSmile_Bk, NumPalBitmaps, i, 255 * j / NUM_STEPS_PAL);
		GUI_DrawBitmap(&bmhouse_smile,  (xSize  - bmhouse_smile.XSize) / 2 + FACE_OFFSET, HOUSE_FACE_YPOS);
	  	GUI_Delay(90);
	}
	}

	#if 0
	WM_BringToBottom(hWin0);
	WM_InvalidateWindow(hWin0);

	WM_BringToBottom(hWin1);
	WM_InvalidateWindow(hWin1);

	WM_BringToBottom(hWin2);
	WM_InvalidateWindow(hWin2);
    
	WM_DeleteWindow(hWin0);
	WM_DeleteWindow(hWin1);
	WM_DeleteWindow(hWin2);
	#endif

	return 1;
	
}

#endif

/*********************************************************************
*
*       
*/

extern void GUI_Enjoy_Out(void);
extern int GUI_Anim_Delay_Fading(int Delay);
extern void GUI_Anim_ClearUp(void);
extern azure_cloud cloud;

static const PAL_BITMAP_CONTEXT _HouseSmilePaletteBitmaps[] = {
   { &bmhouse_smile,     _Colorshouse_smile,     _aPalettesHouse,       GUI_COUNTOF(_Colorshouse_smile)     },
};

static const PAL_BITMAP_CONTEXT _HouseSmilePaletteBitmaps_Bk[] = {
   { &bmhouse_smile,     _Colorshouse_smile,     _aPalettesHouse_Bk,     GUI_COUNTOF(_Colorshouse_smile)     },
};

static const  PAL_BITMAP_CONTEXT _apPaletteBitmapsSmile[] = {
   { &bmhouse_smile,     _Colorshouse_smile,     _aPalettesHouse,     GUI_COUNTOF(_Colorshouse_smile)},
};

static const  PAL_BITMAP_CONTEXT _apPaletteBitmapsSmile_Bk[] = {
   { &bmhouse_smile,     _Colorshouse_smile,     _aPalettesHouse_Bk,   GUI_COUNTOF(_Colorshouse_smile)},
};


static const  PAL_BITMAP_CONTEXT _apPaletteBitmapsLightSmile[] = {
   { &bmlightsmile,     _Colorslightsmile,     _aPalettesHouse,     GUI_COUNTOF(_Colorslightsmile)},
};

static const  PAL_BITMAP_CONTEXT _apPaletteBitmapsLightSmile_Bk[] = {
   { &bmlightsmile,     _Colorslightsmile,     _aPalettesHouse_Bk,   GUI_COUNTOF(_Colorslightsmile)},
};

static const  PAL_BITMAP_CONTEXT _apPaletteBitmapsLightPout[] = {
   { &bmlightpout,     _Colorslightpout,     _aPalettesHouse,     GUI_COUNTOF(_Colorslightpout)},
};

static const  PAL_BITMAP_CONTEXT _apPaletteBitmapsLightPout_Bk[] = {
   { &bmlightpout,     _Colorslightpout,     _aPalettesHouse_Bk,   GUI_COUNTOF(_Colorslightpout)},
};

static const  PAL_BITMAP_CONTEXT _apPaletteBitmapsPout[] = {
   { &bmpout,     _Colorspout,     _aPalettesHouse,     GUI_COUNTOF(_Colorspout)},
};

static const  PAL_BITMAP_CONTEXT _apPaletteBitmapsPout_Bk[] = {
   { &bmpout,     _Colorspout,     _aPalettesHouse_Bk,   GUI_COUNTOF(_Colorspout)},
};


int Face_Appear_Fading_Smile_IAQ(const GUI_BITMAP *pBM, int x_offset, int y_offset)
{
	unsigned i, j;

	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse) - 1; i++) {
	for (j = 10; j > 0; j--) {
		_SetPalette(_apPaletteBitmapsSmile, GUI_COUNTOF(_apPaletteBitmapsSmile), i, 255 * j / 10);
		GUI_DrawBitmap(pBM, (LCD_GetXSize()  - pBM->XSize) / 2 + x_offset, (LCD_GetYSize()  - pBM->YSize) / 2 + y_offset);

		if(GUI_Anim_Delay_Fading(0)){
			GUI_Anim_ClearUp();
			return -1;
		}
	}
	}

	GUI_Enjoy_Out();
		
	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse_Bk) - 1; i++) {
	for (j = 10; j > 0; j--) {
		_SetPalette(_apPaletteBitmapsSmile_Bk, GUI_COUNTOF(_apPaletteBitmapsSmile_Bk), i, 255 * j / 10);
		GUI_DrawBitmap(pBM,  (LCD_GetXSize()  - pBM->XSize) / 2 + x_offset, (LCD_GetYSize()  - pBM->YSize) / 2 + y_offset);
        
		if(GUI_Anim_Delay_Fading(90)){
			GUI_Anim_ClearUp();
			return -1;
		}
	}
	}

	return NULL;
}

int Face_Appear_Fading_Light_Smile_IAQ(const GUI_BITMAP *pBM, int x_offset, int y_offset)
{
	unsigned i, j;

	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse) - 1; i++) {
	for (j = 10; j > 0; j--) {
		_SetPalette(_apPaletteBitmapsLightSmile, GUI_COUNTOF(_apPaletteBitmapsLightSmile), i, 255 * j / 10);
		GUI_DrawBitmap(pBM, (LCD_GetXSize()  - pBM->XSize) / 2 + x_offset, (LCD_GetYSize()  - pBM->YSize) / 2 + y_offset);

		if(GUI_Anim_Delay_Fading(0)){
			GUI_Anim_ClearUp();
			return -1;
		}
	}
	}
    
	if(GUI_Anim_Delay_Fading(1000)){
			GUI_Anim_ClearUp();
			return -1;
	 }
		
	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse_Bk) - 1; i++) {
	for (j = 10; j > 0; j--) {
		_SetPalette(_apPaletteBitmapsLightSmile_Bk, GUI_COUNTOF(_apPaletteBitmapsLightSmile_Bk), i, 255 * j / 10);
		GUI_DrawBitmap(pBM,  (LCD_GetXSize()  - pBM->XSize) / 2 + x_offset, (LCD_GetYSize()  - pBM->YSize) / 2 + y_offset);
        
		if(GUI_Anim_Delay_Fading(90)){
			GUI_Anim_ClearUp();
			return -1;
		}
	}
	}

	return NULL;
}

int Face_Appear_Fading_Light_Pout_IAQ(const GUI_BITMAP *pBM, int x_offset, int y_offset)
{
	unsigned i, j;

	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse) - 1; i++) {
	for (j = 10; j > 0; j--) {
		_SetPalette(_apPaletteBitmapsLightPout, GUI_COUNTOF(_apPaletteBitmapsLightPout), i, 255 * j / 10);
		GUI_DrawBitmap(pBM, (LCD_GetXSize()  - pBM->XSize) / 2 + x_offset, (LCD_GetYSize()  - pBM->YSize) / 2 + y_offset);

		if(GUI_Anim_Delay_Fading(0)){
			GUI_Anim_ClearUp();
			return -1;
		}
	}
	}
    
	if(GUI_Anim_Delay_Fading(1000)){
			GUI_Anim_ClearUp();
			return -1;
	 }
		
	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse_Bk) - 1; i++) {
	for (j = 10; j > 0; j--) {
		_SetPalette(_apPaletteBitmapsLightPout_Bk, GUI_COUNTOF(_apPaletteBitmapsLightPout_Bk), i, 255 * j / 10);
		GUI_DrawBitmap(pBM,  (LCD_GetXSize()  - pBM->XSize) / 2 + x_offset, (LCD_GetYSize()  - pBM->YSize) / 2 + y_offset);
        
		if(GUI_Anim_Delay_Fading(90)){
			GUI_Anim_ClearUp();
			return -1;
		}
	}
	}

	return NULL;
}

int Face_Appear_Fading_Pout_IAQ(const GUI_BITMAP *pBM, int x_offset, int y_offset)
{
	unsigned i, j;

	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse) - 1; i++) {
	for (j = 10; j > 0; j--) {
		_SetPalette(_apPaletteBitmapsPout, GUI_COUNTOF(_apPaletteBitmapsPout), i, 255 * j / 10);
		GUI_DrawBitmap(pBM, (LCD_GetXSize()  - pBM->XSize) / 2 + x_offset, (LCD_GetYSize()  - pBM->YSize) / 2 + y_offset);

		if(GUI_Anim_Delay_Fading(0)){
			GUI_Anim_ClearUp();
			return -1;
		}
	}
	}
    
	if(GUI_Anim_Delay_Fading(1000)){
			GUI_Anim_ClearUp();
			return -1;
	 }
		
	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse_Bk) - 1; i++) {
	for (j = 10; j > 0; j--) {
		_SetPalette(_apPaletteBitmapsPout_Bk, GUI_COUNTOF(_apPaletteBitmapsPout_Bk), i, 255 * j / 10);
		GUI_DrawBitmap(pBM,  (LCD_GetXSize()  - pBM->XSize) / 2 + x_offset, (LCD_GetYSize()  - pBM->YSize) / 2 + y_offset);
        
		if(GUI_Anim_Delay_Fading(90)){
			GUI_Anim_ClearUp();
			return -1;
		}
	}
	}

	return NULL;
}

int Face_Appear_Fading_Smile_EAQ(const GUI_BITMAP *pBM, int x_offset, int y_offset)
{
	unsigned i, j;

	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse) - 1; i++) {
	for (j = 10; j > 0; j--) {
		_SetPalette(_apPaletteBitmapsSmile, GUI_COUNTOF(_apPaletteBitmapsSmile), i, 255 * j / 10);
		GUI_DrawBitmap(pBM, (LCD_GetXSize()  - pBM->XSize) / 2 + x_offset, (LCD_GetYSize()  - pBM->YSize) / 2 + y_offset);

		if(GUI_Anim_Delay_Fading(0)){
			GUI_Anim_ClearUp();
			return -1;
		}
	}
	}
    
	if(GUI_Anim_Delay_Fading(1000)){
			GUI_Anim_ClearUp();
			return -1;
	 }
		
	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse_Bk) - 1; i++) {
	for (j = 10; j > 0; j--) {
		_SetPalette(_apPaletteBitmapsSmile_Bk, GUI_COUNTOF(_apPaletteBitmapsSmile_Bk), i, 255 * j / 10);
		GUI_DrawBitmap(pBM,  (LCD_GetXSize()  - pBM->XSize) / 2 + x_offset, (LCD_GetYSize()  - pBM->YSize) / 2 + y_offset);
        
		if(GUI_Anim_Delay_Fading(90)){
			GUI_Anim_ClearUp();
			return -1;
		}
	}
	}

	return NULL;
}

int Face_Appear_Fading_Light_Smile_EAQ(const GUI_BITMAP *pBM, int x_offset, int y_offset)
{
	unsigned i, j;

	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse) - 1; i++) {
	for (j = 10; j > 0; j--) {
		_SetPalette(_apPaletteBitmapsLightSmile, GUI_COUNTOF(_apPaletteBitmapsLightSmile), i, 255 * j / 10);
		GUI_DrawBitmap(pBM, (LCD_GetXSize()  - pBM->XSize) / 2 + x_offset, (LCD_GetYSize()  - pBM->YSize) / 2 + y_offset);

		if(GUI_Anim_Delay_Fading(0)){
			GUI_Anim_ClearUp();
			return -1;
		}
	}
	}
    
	if(GUI_Anim_Delay_Fading(1000)){
			GUI_Anim_ClearUp();
			return -1;
	 }
		
	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse_Bk) - 1; i++) {
	for (j = 10; j > 0; j--) {
		_SetPalette(_apPaletteBitmapsLightSmile_Bk, GUI_COUNTOF(_apPaletteBitmapsLightSmile_Bk), i, 255 * j / 10);
		GUI_DrawBitmap(pBM,  (LCD_GetXSize()  - pBM->XSize) / 2 + x_offset, (LCD_GetYSize()  - pBM->YSize) / 2 + y_offset);
        
		if(GUI_Anim_Delay_Fading(90)){
			GUI_Anim_ClearUp();
			return -1;
		}
	}
	}

	return NULL;
}

int Face_Appear_Fading_Light_Pout_EAQ(const GUI_BITMAP *pBM, int x_offset, int y_offset)
{
	unsigned i, j;

	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse) - 1; i++) {
	for (j = 10; j > 0; j--) {
		_SetPalette(_apPaletteBitmapsLightPout, GUI_COUNTOF(_apPaletteBitmapsLightPout), i, 255 * j / 10);
		GUI_DrawBitmap(pBM, (LCD_GetXSize()  - pBM->XSize) / 2 + x_offset, (LCD_GetYSize()  - pBM->YSize) / 2 + y_offset);

		if(GUI_Anim_Delay_Fading(0)){
			GUI_Anim_ClearUp();
			return -1;
		}
	}
	}
    
	if(GUI_Anim_Delay_Fading(1000)){
			GUI_Anim_ClearUp();
			return -1;
	 }
		
	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse_Bk) - 1; i++) {
	for (j = 10; j > 0; j--) {
		_SetPalette(_apPaletteBitmapsLightPout_Bk, GUI_COUNTOF(_apPaletteBitmapsLightPout_Bk), i, 255 * j / 10);
		GUI_DrawBitmap(pBM,  (LCD_GetXSize()  - pBM->XSize) / 2 + x_offset, (LCD_GetYSize()  - pBM->YSize) / 2 + y_offset);
        
		if(GUI_Anim_Delay_Fading(90)){
			GUI_Anim_ClearUp();
			return -1;
		}
	}
	}

	return NULL;
}

int Face_Appear_Fading_Pout_EAQ(const GUI_BITMAP *pBM, int x_offset, int y_offset)
{
	unsigned i, j;

	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse) - 1; i++) {
	for (j = 10; j > 0; j--) {
		_SetPalette(_apPaletteBitmapsPout, GUI_COUNTOF(_apPaletteBitmapsPout), i, 255 * j / 10);
		GUI_DrawBitmap(pBM, (LCD_GetXSize()  - pBM->XSize) / 2 + x_offset, (LCD_GetYSize()  - pBM->YSize) / 2 + y_offset);

		if(GUI_Anim_Delay_Fading(0)){
			GUI_Anim_ClearUp();
			return -1;
		}
	}
	}
    
	if(GUI_Anim_Delay_Fading(1000)){
			GUI_Anim_ClearUp();
			return -1;
	 }
		
	for (i = 0; i < GUI_COUNTOF(_aPalettesHouse_Bk) - 1; i++) {
	for (j = 10; j > 0; j--) {
		_SetPalette(_apPaletteBitmapsPout_Bk, GUI_COUNTOF(_apPaletteBitmapsPout_Bk), i, 255 * j / 10);
		GUI_DrawBitmap(pBM,  (LCD_GetXSize()  - pBM->XSize) / 2 + x_offset, (LCD_GetYSize()  - pBM->YSize) / 2 + y_offset);
        
		if(GUI_Anim_Delay_Fading(90)){
			GUI_Anim_ClearUp();
			return -1;
		}
	}
	}

	return NULL;
}


/*************************** End of file ****************************/
