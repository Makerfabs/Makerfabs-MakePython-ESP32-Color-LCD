#include <Wire.h>
#include "SPI.h"
#include <LovyanGFX.hpp>
#include "arduinoFFT.h" 

arduinoFFT FFT = arduinoFFT();

#define SPI_CS      15
#define SPI_DC      22
#define SPI_RST     21
#define SPI_SCLK    14
#define SPI_MOSI    13
#define SPI_MISO    12


//SPI control
#define SPI_ON_TFT digitalWrite(SPI_CS, LOW)
#define SPI_OFF_TFT digitalWrite(SPI_CS, HIGH)

struct LGFX_Config
{
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_sclk = SPI_SCLK;
    static constexpr int spi_mosi = SPI_MOSI;
    static constexpr int spi_miso = SPI_MISO;
};

static lgfx::LGFX_SPI<LGFX_Config> tft;
static LGFX_Sprite sprite(&tft);
static lgfx::Panel_ST7789 panel;

uint16_t color[7] = {0x001F,0xf800,0x07e0,0x07ff,0xf81f,0xffe0,0xffff};

/***************************************************************************/
#define SAMPLES 512              // Must be a power of 2
#define SAMPLING_FREQUENCY 40000 // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define amplitude 200            // Depending on your audio source level, you may need to increase this value
unsigned int sampling_period_us;
unsigned long microseconds;
byte peak[] = {0,0,0,0,0,0,0};
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime, oldTime;

int data_avgs[64];


void setup()
{

    Serial.begin(115200);
    while (!Serial)
        ; // Leonardo: wait for serial monitor
    Serial.println("\n NS2009 test");

    pinMode(SPI_CS, OUTPUT);
    SPI_OFF_TFT;
    SPI.begin(SPI_SCLK, SPI_MISO, SPI_MOSI);

    //TFT(SPI) init
    SPI_ON_TFT;

    set_tft();
    tft.begin();
    //tft.init();
    tft.fillScreen(color[6]);
    SPI_OFF_TFT;
}

void loop()
{
    for (int i = 0; i < SAMPLES; i++) {
        newTime = micros()-oldTime;
        oldTime = newTime;
        vReal[i] = analogRead(34); // A conversion takes about 1uS on an ESP32
        vImag[i] = 0;
        while (micros() < (newTime + sampling_period_us)) { /* do nothing to wait */ }
        }
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    tft.fillScreen(color[6]);
    tft.setCursor(0, 210);
    tft.setTextColor(0xf81f);
    tft.setTextSize(2);
    tft.println("01 02 05 1k 2k 4k 8k ");
    for (int i = 2; i < (SAMPLES/2); i++){ // Don't use sample 0 and only the first SAMPLES/2 are usable.
    // Each array element represents a frequency and its value, is the amplitude. Note the frequencies are not discrete.
    if (vReal[i] > 1500) { // Add a crude noise filter, 10 x amplitude or more
      if (i<=2 )             displayBand(0,(int)vReal[i]); // 125Hz
      if (i >2   && i<=4 )   displayBand(1,(int)vReal[i]); // 250Hz
      if (i >4   && i<=7 )   displayBand(2,(int)vReal[i]); // 500Hz
      if (i >7   && i<=15 )  displayBand(3,(int)vReal[i]); // 1000Hz
      if (i >15  && i<=40 )  displayBand(4,(int)vReal[i]); // 2000Hz
      if (i >40  && i<=70 )  displayBand(5,(int)vReal[i]); // 4000Hz
      if (i >70  && i<=288 ) displayBand(6,(int)vReal[i]); // 8000Hz
      if (i >288           ) displayBand(7,(int)vReal[i]); // 16000Hz
      //Serial.println(i);
    }
    for (byte band = 0; band <= 7; band++) tft.drawLine(36*band,200-peak[band],36*band+28,200-peak[band],color[4]);
  }
  if (millis()%2 == 0) {for (byte band = 0; band <= 7; band++) {if (peak[band] > 4) peak[band] -= 4;if (peak[band] < 4) peak[band] = 0;}} // Decay the peak

}

void displayBand(int band, int dsize){
  int dmax = 200;
  dsize /= amplitude;
  if (dsize > dmax) dsize = dmax;
  if (band == 7) tft.drawLine(36*6,200, 36*6+28,200,TFT_WHITE);
  for (int s = 0; s <= dsize; s=s+8) {tft.fillRect(band*36,200-s, 28, 5,  color[random(3)]);}
  if (dsize > peak[band]) {peak[band] = dsize;}
}


void set_tft()
{
  // パネルクラスに各種設定値を代入していきます。
  // （LCD一体型製品のパネルクラスを選択した場合は、
  //   製品に合った初期値が設定されているので設定は不要です）

  // 通常動作時のSPIクロックを設定します。
  // ESP32のSPIは80MHzを整数で割った値のみ使用可能です。
  // 設定した値に一番近い設定可能な値が使用されます。
  panel.freq_write = 80000000;//60000000;
  //panel.freq_write = 20000000;

  // 単色の塗り潰し処理時のSPIクロックを設定します。
  // 基本的にはfreq_writeと同じ値を設定しますが、
  // より高い値を設定しても動作する場合があります。
  panel.freq_fill = 80000000;//60000000;
  //panel.freq_fill  = 27000000;

  // LCDから画素データを読取る際のSPIクロックを設定します。
  panel.freq_read = 16000000;

  // SPI通信モードを0~3から設定します。
  panel.spi_mode = 0;

  // データ読み取り時のSPI通信モードを0~3から設定します。
  panel.spi_mode_read = 0;

  // 画素読出し時のダミービット数を設定します。
  // 画素読出しでビットずれが起きる場合に調整してください。
  panel.len_dummy_read_pixel = 8;

  // データの読取りが可能なパネルの場合はtrueを、不可の場合はfalseを設定します。
  // 省略時はtrueになります。
  panel.spi_read = true;

  // データの読取りMOSIピンで行うパネルの場合はtrueを設定します。
  // 省略時はfalseになります。
  panel.spi_3wire = false;

  // LCDのCSを接続したピン番号を設定します。
  // 使わない場合は省略するか-1を設定します。
  panel.spi_cs = SPI_CS;

  // LCDのDCを接続したピン番号を設定します。
  panel.spi_dc = SPI_DC;

  // LCDのRSTを接続したピン番号を設定します。
  // 使わない場合は省略するか-1を設定します。
  panel.gpio_rst = SPI_RST;

  // LCDのバックライトを接続したピン番号を設定します。
  // 使わない場合は省略するか-1を設定します。
 // panel.gpio_bl = LCD_BL;

  // バックライト使用時、輝度制御に使用するPWMチャンネル番号を設定します。
  // PWM輝度制御を使わない場合は省略するか-1を設定します。
  panel.pwm_ch_bl = -1;

  // バックライト点灯時の出力レベルがローかハイかを設定します。
  // 省略時は true。true=HIGHで点灯 / false=LOWで点灯になります。
  panel.backlight_level = true;

  // invertDisplayの初期値を設定します。trueを設定すると反転します。
  // 省略時は false。画面の色が反転している場合は設定を変更してください。
  panel.invert = false;

  // パネルの色順がを設定します。  RGB=true / BGR=false
  // 省略時はfalse。赤と青が入れ替わっている場合は設定を変更してください。
  panel.rgb_order = false;

  // パネルのメモリが持っているピクセル数（幅と高さ）を設定します。
  // 設定が合っていない場合、setRotationを使用した際の座標がずれます。
  // （例：ST7735は 132x162 / 128x160 / 132x132 の３通りが存在します）
  panel.memory_width = 240;
  panel.memory_height = 240;

  // パネルの実際のピクセル数（幅と高さ）を設定します。
  // 省略時はパネルクラスのデフォルト値が使用されます。
  panel.panel_width = 240;
  panel.panel_height = 240;

  // パネルのオフセット量を設定します。
  // 省略時はパネルクラスのデフォルト値が使用されます。
  panel.offset_x = 0;
  panel.offset_y = 0;

  // setRotationの初期化直後の値を設定します。
  panel.rotation = 1;

  // setRotationを使用した時の向きを変更したい場合、offset_rotationを設定します。
  // setRotation(0)での向きを 1の時の向きにしたい場合、 1を設定します。
  panel.offset_rotation = 0;

  // 設定を終えたら、LGFXのsetPanel関数でパネルのポインタを渡します。
  tft.setPanel(&panel);
}
