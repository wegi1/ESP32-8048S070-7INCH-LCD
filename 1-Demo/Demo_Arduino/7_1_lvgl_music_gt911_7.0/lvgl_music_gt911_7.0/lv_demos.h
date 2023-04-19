/**
 * @file lv_demos.h
 *
 */

#ifndef LV_DEMOS_H
#define LV_DEMOS_H

#define LV_USE_DEMO_MUSIC       1
#if LV_USE_DEMO_MUSIC
# define LV_DEMO_MUSIC_SQUARE       0
# define LV_DEMO_MUSIC_LANDSCAPE    0
# define LV_DEMO_MUSIC_ROUND        0
# define LV_DEMO_MUSIC_LARGE        0
# define LV_DEMO_MUSIC_AUTO_PLAY    0
#endif



#ifdef __cplusplus
//extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl.h>



#if LV_USE_DEMO_MUSIC
#include "lv_demo_music.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/


/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
//} /* extern "C" */
#endif

#endif /*LV_DEMO_H*/
