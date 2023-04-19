//**********************************************************************************************************
//*    audioI2S-- I2S audiodecoder for ESP32,                                                              *
//**********************************************************************************************************
//
// first release on 11/2018
// Version 3  , Jul.02/2020
//
//
// THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT.
// FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR
// OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE
//

#include "Arduino.h"
#include "WiFiMulti.h"
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"

#define I2S_DOUT      17
#define I2S_BCLK      19
#define I2S_LRC       18

#define BUTTON_PIN    0

Audio audio;
WiFiMulti wifiMulti;
String ssid =     "CMCC-404";
String password = "zp198612";

int music_id = 1818615845;

// optional
void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(ssid.c_str(), password.c_str());
    wifiMulti.run();
    if(WiFi.status() != WL_CONNECTED){
        WiFi.disconnect(true);
        wifiMulti.run();
    }
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(21); // 0...21

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    String music_url = "https://music.163.com/song/media/outer/url?id=" + String(music_id) + ".mp3";
    audio.connecttohost(music_url.c_str()); //  网易 1818615845
    //   https://music.163.com/song/media/outer/url?id=1960424259.mp3
    //   https://music.163.com/song/media/outer/url?id=1818615845.mp3
    //   https://music.163.com/song/media/outer/url?id=1929356672.mp3
}

void loop()
{
    audio.loop();

    if(Serial.available()){ // put streamURL in serial monitor
        audio.stopSong();
        String r=Serial.readString(); r.trim();
        if(r.length()>5) audio.connecttohost(r.c_str());
        log_i("free heap=%i", ESP.getFreeHeap());
    }

    if(digitalRead(BUTTON_PIN) == LOW)
    {
        delay(5);
        if(digitalRead(BUTTON_PIN) == LOW)
        {
            audio.stopSong();
            music_id = music_id + 1;
            String music_url = "https://music.163.com/song/media/outer/url?id=" + String(music_id) + ".mp3";
            audio.connecttohost(music_url.c_str()); //  网易 1818615845
            Serial.println(music_url);
            while(digitalRead(BUTTON_PIN) == LOW);
        }
    }
}
