//=====================================================================
// 200 : 22 FPS
// 100 : 22 FPS
//  50 : 22 FPS
// MovingCircles
// https://github.com/lovyan03/LovyanGFX/tree/master/examples/Sprite/MovingCircles
//=====================================================================
// MovingCircles with ESP32 2432S028 : 2022.08.01 : macsbug
// https://macsbug.wordpress.com/2022/08/17/esp32-2432s028/
//=====================================================================
// HARD : EESP32_2432S028 : 
/// Dev environment  : Arduino IDE 1.8.19
//  Board Manager    : arduino-esp32 2.0.3-RC1
//  Board            : "ESP32 Dev Module"
//  Flash Size       : "4MB (32Mb)"
//  Partition Scheme : "No OTA (2MB APP/2MB SPIFSS)"
//  Pord             : "dev/cu.wchusbserial14240"
// Library           : lovyanGFX 
//                   : https://github.com/lovyan03/LovyanGFX
//=====================================================================

#pragma GCC optimize ("Ofast")
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
lgfx::Panel_ILI9341      _panel_instance;
lgfx::Bus_SPI            _bus_instance;   // SPIバスのインスタンス
lgfx::Light_PWM          _light_instance;
lgfx::Touch_XPT2046      _touch_instance;

public:
  LGFX(void)
  {
    { // バス制御の設定を行います。
      auto cfg = _bus_instance.config();    // バス設定用の構造体を取得します。
      cfg.spi_host = VSPI_HOST;     // 使用するSPIを選択  (VSPI_HOST or HSPI_HOST)
      cfg.spi_mode = 0;             // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 40000000;    // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
      cfg.freq_read  = 16000000;    // 受信時のSPIクロック
      cfg.spi_3wire  = false;        // 受信をMOSIピンで行う場合はtrueを設定
      cfg.use_lock   = true;        // トランザクションロックを使用する場合はtrueを設定
      cfg.dma_channel = 1;          // Set the DMA channel (1 or 2. 0=disable)   使用するDMAチャンネルを設定 (0=DMA不使用)
      cfg.pin_sclk = 18;            // SPIのSCLKピン番号を設定
      cfg.pin_mosi = 23;            // SPIのMOSIピン番号を設定
      cfg.pin_miso = 19;            // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_dc   = 27;            // SPIのD/Cピン番号を設定  (-1 = disable)
      _bus_instance.config(cfg);    // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance);      // バスをパネルにセットします。
    }

    { // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config();    // 表示パネル設定用の構造体を取得します。
      cfg.pin_cs           =    14;  // CSが接続されているピン番号   (-1 = disable)
      cfg.pin_rst          =    33;  // RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_busy         =    -1;  // BUSYが接続されているピン番号 (-1 = disable)
      cfg.memory_width     =   240;  // ドライバICがサポートしている最大の幅
      cfg.memory_height    =   320;  // ドライバICがサポートしている最大の高さ
      cfg.panel_width      =   240;  // 実際に表示可能な幅
      cfg.panel_height     =   320;  // 実際に表示可能な高さ
      cfg.offset_x         =     0;  // パネルのX方向オフセット量
      cfg.offset_y         =     0;  // パネルのY方向オフセット量
      cfg.offset_rotation  =     0;  // 回転方向の値のオフセット 0~7 (4~7は上下反転)
      cfg.dummy_read_pixel =     8;  // ピクセル読出し前のダミーリードのビット数
      cfg.dummy_read_bits  =     1;  // ピクセル以外のデータ読出し前のダミーリードのビット数
      cfg.readable         =  true;  // データ読出しが可能な場合 trueに設定
      cfg.invert           = false;  // パネルの明暗が反転してしまう場合 trueに設定
      cfg.rgb_order        = false;  // パネルの赤と青が入れ替わってしまう場合 trueに設定
      cfg.dlen_16bit       = false;  // データ長を16bit単位で送信するパネルの場合 trueに設定
      cfg.bus_shared       =  true;  // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

      _panel_instance.config(cfg);
    }
    
    { // バックライト制御の設定を行います。（必要なければ削除）
      auto cfg = _light_instance.config();    // バックライト設定用の構造体を取得します。

      cfg.pin_bl = 32;              // バックライトが接続されているピン番号
      cfg.invert = false;           // バックライトの輝度を反転させる場合 true
      cfg.freq   = 44100;           // バックライトのPWM周波数
      cfg.pwm_channel = 7;          // 使用するPWMのチャンネル番号

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);  // バックライトをパネルにセットします。
    }

    { // タッチスクリーン制御の設定を行います。（必要なければ削除）
      auto cfg = _touch_instance.config();
      cfg.x_min      = 0;    // タッチスクリーンから得られる最小のX値(生の値)
      cfg.x_max      = 239;  // タッチスクリーンから得られる最大のX値(生の値)
      cfg.y_min      = 0;    // タッチスクリーンから得られる最小のY値(生の値)
      cfg.y_max      = 319;  // タッチスクリーンから得られる最大のY値(生の値)
      cfg.pin_int    = -1;   // INTが接続されているピン番号
      cfg.bus_shared = true; // 画面と共通のバスを使用している場合 trueを設定
      cfg.offset_rotation = 0;// 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定
      cfg.spi_host = VSPI_HOST;// 使用するSPIを選択 (HSPI_HOST or VSPI_HOST)
      cfg.freq = 1000000;     // SPIクロックを設定
      cfg.pin_sclk = 18;     // SCLKが接続されているピン番号
      cfg.pin_mosi = 23;     // MOSIが接続されているピン番号
      cfg.pin_miso = 22;     // MISOが接続されているピン番号
      cfg.pin_cs   =  5;     //   CSが接続されているピン番号
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);  // タッチスクリーンをパネルにセットします。
    }
    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};

LGFX tft;

static std::uint32_t sec, psec;
static std::uint32_t fps = 0, frame_count = 0;
static std::int32_t  lcd_width  = 320;
static std::int32_t  lcd_height = 240;

struct obj_info_t {
  std::int32_t x;
  std::int32_t y;
  std::int32_t r;
  std::int32_t dx;
  std::int32_t dy;
  std::uint32_t color;

 void move(){
    x += dx;
    y += dy;
    if (x < 0) {
      x = 0;
      if (dx < 0) dx = - dx;
    } else if (x >= lcd_width) {
      x = lcd_width -1;
      if (dx > 0) dx = - dx;
    }
    if (y < 0) {
      y = 0;
      if (dy < 0) dy = - dy;
    } else if (y >= lcd_height) {
      y = lcd_height - 1;
      if (dy > 0) dy = - dy;
    }
  }
};

static constexpr std::uint32_t obj_count = 100;//200;
static obj_info_t objects[obj_count];
static LGFX_Sprite sprites[2];
static int_fast16_t sprite_height;

void setup(void){
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setColorDepth(24);

  obj_info_t *a;
  for (std::uint32_t i = 0; i < obj_count; ++i) {
    a = &objects[i];
    a->color = (uint32_t)rand() | 0x808080U;
    a->x = rand() % lcd_width;
    a->y = rand() % lcd_height;
    a->dx = ((rand() & 3) + 1) * (i & 1 ? 1 : -1);
    a->dy = ((rand() & 3) + 1) * (i & 2 ? 1 : -1);
    a->r = 8 + (i & 0x07);
  }

  uint32_t div = 2;
  for (;;) {
    sprite_height = (lcd_height + div - 1) / div;
    bool fail = false;
    for (std::uint32_t i = 0; !fail && i < 2; ++i)
    {
      sprites[i].setColorDepth(tft.getColorDepth());
      sprites[i].setFont(&fonts::Font2);
      fail = !sprites[i].createSprite(lcd_width, sprite_height);
    }
    if (!fail) break;
    for (std::uint32_t i = 0; i < 2; ++i)
    {
      sprites[i].deleteSprite();
    }
    ++div;
  }
  tft.startWrite();
}

void loop(void){
  static std::uint_fast8_t flip = 0;
  obj_info_t *a;
  for (std::uint32_t i = 0; i != obj_count; i++) {
    objects[i].move();
  }
  for (std::int32_t y = 0; y < lcd_height; y += sprite_height) {
    flip = flip ? 0 : 1;
    sprites[flip].clear();
    for (std::uint32_t i = 0; i != obj_count; i++) {
      a = &objects[i];
      if (( a->y + a->r >= y ) && ( a->y - a->r <= y + sprite_height ))
        sprites[flip].drawCircle(a->x, a->y - y, a->r, a->color);
    }

    if (y == 0) {
      sprites[flip].setCursor(1,1);
      sprites[flip].setTextColor(TFT_BLACK);
      sprites[flip].printf("obj:%d fps:%d", obj_count, fps);
      sprites[flip].setCursor(0,0);
      sprites[flip].setTextColor(TFT_WHITE);
      sprites[flip].printf("obj:%d fps:%d", obj_count, fps);
    }
    sprites[flip].pushSprite(&tft, 0, y);
  }
  tft.display();

  ++frame_count;
  sec = lgfx::millis() / 1000;
  if (psec != sec) {
    psec = sec;
    fps = frame_count;
    frame_count = 0;
  }
}
