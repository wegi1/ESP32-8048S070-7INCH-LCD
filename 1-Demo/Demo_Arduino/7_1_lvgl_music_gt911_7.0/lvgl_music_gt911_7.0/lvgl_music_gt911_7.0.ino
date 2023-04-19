
#include "HAL.h"

Ticker ticker;
Audio audio;

struct audioMessage
{
  uint8_t cmd;
  const char *txt;
  uint32_t value;
  uint32_t ret;
} audioTxMessage, audioRxMessage;

enum : uint8_t
{
  SET_VOLUME,
  GET_VOLUME,
  CONNECTTOHOST,
  CONNECTTOSD
};

QueueHandle_t audioSetQueue = NULL;
QueueHandle_t audioGetQueue = NULL;

// #include "lv_demo_widgets.h"
#include <lvgl.h>
#include "lv_demos.h"
/*******************************************************************************
 ******************************************************************************/
#include <Arduino_GFX_Library.h>
#define TFT_BL 2
#define GFX_BL DF_GFX_BL

Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
    GFX_NOT_DEFINED /* CS */, GFX_NOT_DEFINED /* SCK */, GFX_NOT_DEFINED /* SDA */,
    41 /* DE */, 40 /* VSYNC */, 39 /* HSYNC */, 42 /* PCLK */,
    14 /* R0 */, 21 /* R1 */, 47 /* R2 */, 48 /* R3 */, 45 /* R4 */,
    9 /* G0 */, 46 /* G1 */, 3 /* G2 */, 8 /* G3 */, 16 /* G4 */, 1 /* G5 */,
    15 /* B0 */, 7 /* B1 */, 6 /* B2 */, 5 /* B3 */, 4 /* B4 */
);
// option 1:
// 7寸 50PIN 800*480
Arduino_RPi_DPI_RGBPanel *gfx = new Arduino_RPi_DPI_RGBPanel(
  bus,
//  800 /* width */, 0 /* hsync_polarity */, 8/* hsync_front_porch */, 2 /* hsync_pulse_width */, 43/* hsync_back_porch */,
//  480 /* height */, 0 /* vsync_polarity */, 8 /* vsync_front_porch */, 2/* vsync_pulse_width */, 12 /* vsync_back_porch */,
//  1 /* pclk_active_neg */, 16000000 /* prefer_speed */, true /* auto_flush */);

    800 /* width */, 0 /* hsync_polarity */, 210 /* hsync_front_porch */, 30 /* hsync_pulse_width */, 16 /* hsync_back_porch */,
    480 /* height */, 0 /* vsync_polarity */, 22 /* vsync_front_porch */, 13 /* vsync_pulse_width */, 10 /* vsync_back_porch */,
    1 /* pclk_active_neg */, 12000000 /* prefer_speed */, true /* auto_flush */);

/*******************************************************************************
 * Please config the touch panel in touch.h
 ******************************************************************************/
#include "touch.h"


/* Change to your screen resolution */
static uint32_t screenWidth;
static uint32_t screenHeight;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf1;
static lv_color_t *disp_draw_buf2;
static lv_disp_drv_t disp_drv;

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  if (touch_has_signal())
  {
    if (touch_touched())
    {
      data->state = LV_INDEV_STATE_PR;

      /*Set the coordinates*/
      data->point.x = touch_last_x;
      data->point.y = touch_last_y;
    }
    else if (touch_released())
    {
      data->state = LV_INDEV_STATE_REL;
    }
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }
}

void CreateQueues()
{
  audioSetQueue = xQueueCreate(10, sizeof(struct audioMessage));
  audioGetQueue = xQueueCreate(10, sizeof(struct audioMessage));
}

void audioTask(void *parameter)
{
  CreateQueues();
  if (!audioSetQueue || !audioGetQueue)
  {
    log_e("queues are not initialized");
    while (true)
    {
      ; // endless loop
    }
  }

  struct audioMessage audioRxTaskMessage;
  struct audioMessage audioTxTaskMessage;

  while (true)
  {
    if (xQueueReceive(audioSetQueue, &audioRxTaskMessage, 1) == pdPASS)
    {
      if (audioRxTaskMessage.cmd == SET_VOLUME)
      {
        audioTxTaskMessage.cmd = SET_VOLUME;
        audio.setVolume(audioRxTaskMessage.value);
        audioTxTaskMessage.ret = 1;
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == CONNECTTOHOST)
      {
        audioTxTaskMessage.cmd = CONNECTTOHOST;
        audioTxTaskMessage.ret = audio.connecttohost(audioRxTaskMessage.txt);
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == CONNECTTOSD)
      {
        audioTxTaskMessage.cmd = CONNECTTOSD;
        audioTxTaskMessage.ret = audio.connecttoSD(audioRxTaskMessage.txt);
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == GET_VOLUME)
      {
        audioTxTaskMessage.cmd = GET_VOLUME;
        audioTxTaskMessage.ret = audio.getVolume();
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else
      {
        log_i("error");
      }
    }
    audio.loop();
  }
}

void audioInit()
{
  xTaskCreatePinnedToCore(
      audioTask,
      "audioplay",
      5000,
      NULL,
      2 | portPRIVILEGE_BIT,
      NULL,
      0);
}

audioMessage transmitReceive(audioMessage msg)
{
  xQueueSend(audioSetQueue, &msg, portMAX_DELAY);
  if (xQueueReceive(audioGetQueue, &audioRxMessage, portMAX_DELAY) == pdPASS)
  {
    if (msg.cmd != audioRxMessage.cmd)
    {
      log_e("wrong reply from message queue");
    }
  }
  return audioRxMessage;
}
void audioSetVolume(uint8_t vol)
{
  audioTxMessage.cmd = SET_VOLUME;
  audioTxMessage.value = vol;
  audioMessage RX = transmitReceive(audioTxMessage);
}

uint8_t audioGetVolume()
{
  audioTxMessage.cmd = GET_VOLUME;
  audioMessage RX = transmitReceive(audioTxMessage);
  return RX.ret;
}

bool audioConnecttohost(const char *host)
{
  audioTxMessage.cmd = CONNECTTOHOST;
  audioTxMessage.txt = host;
  audioMessage RX = transmitReceive(audioTxMessage);
  return RX.ret;
}

bool audioConnecttoSD(const char *filename)
{
  audioTxMessage.cmd = CONNECTTOSD;
  audioTxMessage.txt = filename;
  audioMessage RX = transmitReceive(audioTxMessage);
  return RX.ret;
}

void tcr1s()
{
  uint32_t act = audio.getAudioCurrentTime();
  uint32_t afd = audio.getAudioFileDuration();
  uint32_t pos = audio.getFilePos();
  Serial.println(act);
  Serial.println(afd);
  // Serial.println("audioTime: %i:%02d - duration: %i:%02d", (act / 60), (act % 60) , (afd / 60), (afd % 60));
}

// lvgl -ui -------------------------------------------------------------------------------

static void event_handler(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED)
  {
    LV_LOG_USER("Clicked");
    Serial.println("test");
  }
  else if (code == LV_EVENT_VALUE_CHANGED)
  {
    LV_LOG_USER("Toggled");
    Serial.println("test2");
  }
}

void lv_example_btn_1(void)
{
  lv_obj_t *label;

  lv_obj_t *btn1 = lv_btn_create(lv_scr_act());
  lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
  lv_obj_align(btn1, LV_ALIGN_CENTER, 0, 0);

  label = lv_label_create(btn1);
  lv_label_set_text(label, "test");
  lv_obj_center(label);

  //        lv_obj_t * btn2 = lv_btn_create(lv_scr_act());
  //        lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
  //        lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
  //        lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
  //        lv_obj_set_height(btn2, LV_SIZE_CONTENT);
  //
  //        label = lv_label_create(btn2);
  //        lv_label_set_text(label, "Toggle");
  //        lv_obj_center(label);
}

// lvgl -ui end-----------------------------------------------------------------------------

void setup()
{

  Serial.begin(115200);

  audioInit();

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(20); // 0...21

  log_i("current volume is: %d", audioGetVolume());

  ticker.attach(1, tcr1s);

  Serial.println("LVGL Widgets Demo");

  // Init touch device

  //
  // Init Display
  gfx->begin();
#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif
  //  gfx->fillScreen(RED);
  //  delay(500);
  //  gfx->fillScreen(GREEN);
  //  delay(500);
  //  gfx->fillScreen(BLUE);
  //  delay(500);
  gfx->fillScreen(BLACK);
  delay(50);
  lv_init();
  delay(10);
  touch_init();

  sdcard_init();
  delay(50);

  screenWidth = gfx->width();
  screenHeight = gfx->height();
#ifdef ESP32
  //disp_draw_buf = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * screenWidth * screenHeight / 4, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  disp_draw_buf1 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * screenWidth * screenHeight / 8, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  disp_draw_buf2 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * screenWidth * screenHeight / 8, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#else
  disp_draw_buf = (lv_color_t *)malloc(sizeof(lv_color_t) * screenWidth * screenHeight / 4);
#endif
  if (!disp_draw_buf1 && !disp_draw_buf2)
  {
    Serial.println("LVGL disp_draw_buf allocate failed!");
  }
  else
  {
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf1, disp_draw_buf2, screenWidth * screenHeight / 8);

    /* Initialize the display */
    lv_disp_drv_init(&disp_drv);
    /* Change the following line to your display resolution */
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /* Initialize the (dummy) input device driver */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

#if 0
      lv_example_btn_1();
#else
    // lv_demo_widgets();
    lv_demo_music();

    Serial.println("Setup done");
#endif
  }
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
}

//*****************************************************************************************
//                                  E V E N T S                                           *
//*****************************************************************************************

// optional
void audio_info(const char *info)
{
  Serial.print("info        ");
  Serial.println(info);
}
void audio_id3data(const char *info)
{ // id3 metadata
  Serial.print("id3data     ");
  Serial.println(info);
}
void audio_eof_mp3(const char *info)
{ // end of file
  Serial.print("eof_mp3     ");
  Serial.println(info);
}
void audio_showstation(const char *info)
{
  Serial.print("station     ");
  Serial.println(info);
}
void audio_showstreamtitle(const char *info)
{
  Serial.print("streamtitle ");
  Serial.println(info);
}
void audio_bitrate(const char *info)
{
  Serial.print("bitrate     ");
  Serial.println(info);
}
void audio_commercial(const char *info)
{ // duration in sec
  Serial.print("commercial  ");
  Serial.println(info);
}
void audio_icyurl(const char *info)
{ // homepage
  Serial.print("icyurl      ");
  Serial.println(info);
}
void audio_lasthost(const char *info)
{ // stream URL played
  Serial.print("lasthost    ");
  Serial.println(info);
}
