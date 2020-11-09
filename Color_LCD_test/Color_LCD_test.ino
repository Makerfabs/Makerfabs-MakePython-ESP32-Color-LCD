i #include <Wire.h>
#include "SPI.h"
#include <LovyanGFX.hpp>


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


void setup()
{

    Serial.begin(115200);

    pinMode(SPI_CS, OUTPUT);
    SPI_OFF_TFT;
    SPI.begin(SPI_SCLK, SPI_MISO, SPI_MOSI);

    //TFT(SPI) init


    set_tft();
    tft.begin();
    //tft.init();
    tft.fillScreen(color[6]);


    //tft.drawLine(0,0, 150,150,color[random(5)]);
    //tft.drawLine(30, 40, 200, 40, color[random(3)]);
    //tft.fillRect(0,0,20,200,color[random(3)]);

    tft.setCursor(210, 0);
    tft.setTextColor(0x07e0);
    tft.setTextSize(1);
    tft.println("Hello World!");
}

void loop()
{
    
}


void set_tft()
{
  // パネルクラスに各種設定値を代入していきます。
  // （LCD一体型製品のパネルクラスを選択した場合は、
  //   製品に合った初期値が設定されているので設定は不要です）

  // 通常動作時のSPIクロックを設定します。
  // ESP32のSPIは80MHzを整数で割った値のみ使用可能です。
  // 設定した値に一番近い設定可能な値が使用されます。
  panel.freq_write = 60000000;
  //panel.freq_write = 20000000;

  // 単色の塗り潰し処理時のSPIクロックを設定します。
  // 基本的にはfreq_writeと同じ値を設定しますが、
  // より高い値を設定しても動作する場合があります。
  panel.freq_fill = 60000000;
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
  panel.rotation = 0;

  // setRotationを使用した時の向きを変更したい場合、offset_rotationを設定します。
  // setRotation(0)での向きを 1の時の向きにしたい場合、 1を設定します。
  panel.offset_rotation = 0;

  // 設定を終えたら、LGFXのsetPanel関数でパネルのポインタを渡します。
  tft.setPanel(&panel);
}
