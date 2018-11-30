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

** emWin V5.30 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only  be used  in accordance  with  a license  and should  not be  re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIDEMO_Bitmap.c
Purpose     : Draws bitmaps with and without compression
----------------------------------------------------------------------
*/

#include "GUI.h"
#include "WM.h"
#include "Anim0.h"


/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
//
// Config
//
#define SHOW_ALPHA       0
#define SHOW_PALETTE     0
#define SHOW_BMP_GIF_JPG 1
#define SHOW_HIGH_COLOR  0
#define SHOW_GRAYSCALE   0

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
  #define NUM_STEPS_PAL     20
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
*       Defines
*
**********************************************************************
*/
#define CONTROL_SIZE_X    80
#define CONTROL_SIZE_Y    61
#define INFO_SIZE_Y       65
#define BUTTON_SIZE_X     32
#define BUTTON_SIZE_Y     20
#define PROGBAR_SIZE_X    66
#define PROGBAR_SIZE_Y    12
#define TEXT_SIZE_X       69
#define TEXT_SIZE_Y       7
#define SHOW_PROGBAR_AT   100
#define GUI_ID_HALT       (GUI_ID_USER + 0)
#define GUI_ID_NEXT       (GUI_ID_USER + 1)

#define BK_COLOR_0        0xFF5555
#define BK_COLOR_1        0x880000

#define NUMBYTES_NEEDED   0x200000

#define CIRCLE_RADIUS     100

#define LOGO_DIST_BORDER  5

#define CHAR_READING_TIME 80


#define GUIDEMO_SHOW_CURSOR  (1 << 0)
#define GUIDEMO_SHOW_INFO    (1 << 1)
#define GUIDEMO_SHOW_CONTROL (1 << 2)

#define SHIFT_RIGHT_16(x)    ((x) / 65536)

#define XSIZE_MIN         320
#define YSIZE_MIN         240

/*********************************************************************
*
*       Structures
*
**********************************************************************
*/
#if (SHOW_GRAYSCALE || SHOW_PALETTE || SHOW_HIGH_COLOR)
  typedef struct {
    const GUI_BITMAP *  pBitmap;
    #if SHOW_PALETTE
      GUI_COLOR        *  pColorsDst;
      const GUI_COLOR  ** ppColorsSrc;
      unsigned            NumColors;
    #endif
  } PAL_BITMAP_CONTEXT;
#endif


void _ShowGIF(void) {
  GUI_GIF_IMAGE_INFO GIF_ImageInfo;
  GUI_GIF_INFO       GIF_Info;
  int                ySizeAdd;
  int                xSize;
  int                ySize;
  int                yOff;
  int                i;

  GUI_GIF_GetInfo(_acAnim0, sizeof(_acAnim0), &GIF_Info);
  xSize    = LCD_GetXSize();
  ySize    = LCD_GetYSize();
  ySizeAdd = GUI_GetFontSizeY();
  yOff     = (ySize - CONTROL_SIZE_Y - INFO_SIZE_Y - BITMAP_SIZE_Y - ySizeAdd) / 2;
  for (i = 0; i < GIF_Info.NumImages + 1; i++) {
    if (i == GIF_Info.NumImages) {
      i = 0;
      GUI_GIF_DrawSub(_acAnim0, sizeof(_acAnim0), (xSize - GIF_Info.xSize) / 2, INFO_SIZE_Y + yOff, i);
      break;
    }
    GUI_GIF_DrawSub(_acAnim0, sizeof(_acAnim0), (xSize - GIF_Info.xSize) / 2, INFO_SIZE_Y + yOff, i);
    GUI_GIF_GetImageInfo(_acAnim0, sizeof(_acAnim0), &GIF_ImageInfo, i);
  }
}

/*********************************************************************
*
*       _DrawScreen_BMP_GIF_JPG
*/
void _DrawScreen_BMP_GIF_JPG(int xSize, int ySize) {
  GUI_RECT          Rect;
  int               xOffDisplay;
  int               yOffDisplay;
  int               xOff;
  int               yOff;

  GUI_SetColor(GUI_WHITE);
  #if GUI_WINSUPPORT
    //GUIDEMO_SetInfoText("BMP, GIF and JPEG images");
  #else
    //GUIDEMO_DispTitle("BMP, GIF and JPEG");
  #endif
  if (xSize > XSIZE_MIN) {
    xOffDisplay = (xSize - XSIZE_MIN) / 2;
    xSize       = XSIZE_MIN;
  } else {
    xOffDisplay = 0;
  }
  if (ySize > YSIZE_MIN) {
    yOffDisplay = (ySize - YSIZE_MIN) / 2;
    ySize       = YSIZE_MIN;
  } else {
    yOffDisplay = 0;
  }
//  GUI_SetFont(&GUI_FontRounded16);
  xOff    = (xSize - X_BORDER * 2 - BITMAP_SIZE_X_3 * NUM_IMAGES_BGJ) / (NUM_IMAGES_BGJ + 1);
  Rect.y1 = yOffDisplay + ySize - CONTROL_SIZE_Y - 1;
  yOff    = (ySize - CONTROL_SIZE_Y - INFO_SIZE_Y - BITMAP_SIZE_Y - GUI_GetFontSizeY()) / 2;
  //
  // GIF
  //
  Rect.x0 = xOffDisplay + X_BORDER + xOff * 2 + BITMAP_SIZE_X_3;
  Rect.x1 = Rect.x0 + BITMAP_SIZE_X_3 - 1;
  GUI_DispStringInRect("GIF", &Rect, GUI_TA_HCENTER | GUI_TA_VCENTER);
  _ShowGIF();
  GUI_Delay(2000);
}

/*********************************************************************
*
*       _apFunc
*/
static void (* _apFunc[])(int, int) = {
  #if SHOW_PALETTE
    _DrawScreenPaletteBasedBitmaps,
  #endif
  #if SHOW_BMP_GIF_JPG
    _DrawScreen_BMP_GIF_JPG,
  #endif
  #if SHOW_HIGH_COLOR
    _DrawScreenHighColor,
  #endif
  #if SHOW_ALPHA
    _DrawScreenAlphaBitmaps,
  #endif
  #if SHOW_GRAYSCALE
    _DrawScreenGrayScale,
  #endif
};

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUIDEMO_Bitmap
*/
void GUIDEMO_Bitmap(void) {
  unsigned i;
  int      xSize;
  int      ySize;

  xSize = LCD_GetXSize();
  ySize = LCD_GetYSize();
  if (xSize < XSIZE_MIN || ySize < YSIZE_MIN) {
    //GUIDEMO_ConfigureDemo("Bitmaps", "Showing bitmaps of different\nformats and color depths", GUIDEMO_SHOW_CURSOR | GUIDEMO_SHOW_INFO | GUIDEMO_SHOW_CONTROL);
    return;
  }
  //GUIDEMO_ConfigureDemo("Bitmaps", "Showing bitmaps of different\nformats and color depths", GUIDEMO_SHOW_CURSOR | GUIDEMO_SHOW_INFO | GUIDEMO_SHOW_CONTROL);
//  GUIDEMO_DrawBk();
  //
  // Iterate over subroutines
  //
  for (i = 0; i < GUI_COUNTOF(_apFunc); i++) {
    _apFunc[i](xSize, ySize);
  }
}

/*************************** End of file ****************************/
