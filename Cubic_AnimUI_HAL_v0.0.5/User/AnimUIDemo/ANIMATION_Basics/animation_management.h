#ifndef _ANIMATION_MANAGEMENT_H
#define _ANIMATION_MANAGEMENT_H

#include "GUI.h"
#include "WM.h"

typedef enum {
	  Mode_Compagnon_AQI_House 		= 0,
	  Mode_Compagnon_AQI_FACE,
	  Mode_Compagnon_AQI_NUM,
	  Mode_Compagnon_AQI_SCALE,
	  Mode_Expert_AQI,
	  Mode_Black_Screen,
	  Mode_Compagnon_AQE_Cloud,
	  Mode_Compagnon_AQE_FACE,
	  Mode_Compagnon_AQE_NUM,
	  Mode_Compagnon_AQE_SCALE,
	  Mode_Expert_AQE,
	  Mode_AQE_Black_Screen,
	  Mode_ALERT_BOAST,
	  Mode_ALERT_NO_WIFI,
	  Mode_Select,
	  Mode_RST_VALD,
	  Mode_CO2_CALB,
	  Mode_Demo_Mode,
	  Mode_Standy,
	  Mode_None,
	  Mode_None_AQE,
	  Mode_Idle,
} INDEX_WORK_MODE;

typedef struct _NORMAL_WORK_MODE{
	U8  state;
	U8  mode;
	U8  startup_state;
	U8  gui_run_state;
	U8  allow_mode_qai;
	U8  allow_mode_qai_expert;
	U8  allow_mode_qae;
	U8  allow_mode_qae_expert;
    U32 run_loop_count;
    U8  demo_mode_en;
	U8  demo_mode_done;
    U8  favor_mode_en;
	U8  favor_mode_out;
    U8  reset_mode_en;
    U8  CO2_recali_en;
	U8  holidayModeEn;
	U8  holidayModeQuitFlag;
	U8  alertModeEN;
	U8  alertModeQuitFlag;
}WORK_MODE_T;

extern WORK_MODE_T run_mode;

typedef struct {
    const GUI_BITMAP *  pBitmap;
    GUI_COLOR        *  pColorsDst;
    const GUI_COLOR  ** ppColorsSrc;
    unsigned            NumColors;
} PAL_BITMAP_CONTEXT;

#endif /* ANIMATION_MANAGEMENT */
