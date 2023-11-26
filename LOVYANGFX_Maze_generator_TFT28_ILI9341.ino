/*
Maze generator on 320x240 ili9341 display (Wemos/esp8266 sketch)
  Walls are 3 pixels wide; corridors are 6 pixels wide
  'Growing Tree' algorithm. See
  http://www.jamisbuck.org/presentations/rubyconf2011/index.html#growing-tree
  (c) 2017 paulF
*/
// http://www.dellascolto.com/bitwise/2017/11/02/maze-generator/

//=====================================================================
// Maze generator ESP32_2432S028 : 2022.08.01 : KLYCKIT
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
// Maze generator with 16bit MRB3511 S3 Minikit : 2022.07.22 : KLYCKIT
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
      cfg.pin_miso = 19;     // MISOが接続されているピン番号
      cfg.pin_cs   =  5;     //   CSが接続されているピン番号
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);  // タッチスクリーンをパネルにセットします。
    }
    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};

LGFX tft;

#define TFT_BLACK  0x0000
#define TFT_WHITE  0xFFFF
#define TFT_RED    0xF800
#define YELLOW     0xFFE0 
#define GREEN      0x07E0
#define CYAN       0x07FF
#define BLUE       0x001F

//******************************************************
int HW = 320;  // 480;  // 800
int VW = 240;  // 320;  // 480
int HB = 35;   // 53;   // 88   // HW / 9;
int VB = 26;   // 35;   // 53   // VW / 9;
int NB = 900 ; // 1855; // 4664 // VB * HB;
//******************************************************

// active set: [input_order][col_index/row_index]
  int active[ 900][2]; // 320x240 **********************
//int active[1855][2]; // 480x320 **********************
//int active[4664][2]; // 800x480 **********************
int n_active;          // # of cells in active set 
int k,r,start_col,exit_col,exit_row; 
boolean found_exit;    // for solver 
int step_delay = 5;
 
// byte array 'cell_status' is used for keeping track of two different things:
// a. visit status (first bit); 0: unvisited; 1: visited
// b. status of connections with neighbours (last four bits); BxxxxWSEN 
  byte cell_status[35][26]; // 320x240 *****************
//byte cell_status[53][35]; // 480x320 *****************
//byte cell_status[88][53]; // 800x480 *****************
// holds visit status (first bit) and connection status (last four bits); 
//  [col_index][row_index]
// neighbours are zero-index ordeTFT_RED as N,E,S,W
int dx[4] = { 0, 1, 0,-1};
int dy[4] = {-1, 0, 1, 0};

//=====================================================================
void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
}

//=====================================================================
boolean select_next(int act){
  // parameter is current number of cells in active set
  // pick random neighbour index and random search direction (CW or CCW)
  int b = random(4);
  int cw = 2*random(2)-1;   // -1 (CCW) or +1 (CW) 
  boolean found = false; 
  // unvisited neighbour found for newest cell in active set?
  int act_x = active[act-1][0];
  int act_y = active[act-1][1];
  for (int bc=0;bc<4;bc++) { // bc is just a neighbour counter
    if (act_x+dx[b]>=0 && act_x+dx[b]<HB && act_y+dy[b]>=0 && act_y+dy[b]<VB){
       // valid neighbour?
      if (cell_status[act_x+dx[b]][act_y+dy[b]] < 128) {  
      // found an unvisited neighbour:add to active set,
      //  mark visited and remove wall
        active[act][0] = act_x+dx[b];
        active[act][1] = act_y+dy[b];
        n_active++;
        cell_status[act_x+dx[b]][act_y+dy[b]] += 128; //B10000000
        found = true;
        // erase wall between processed cell and found neighbour 
        // (on display and in cell_status array):
        if (b==0) {         // break out N
          tft.fillRect(3+9*act_x,9*act_y,6,3,TFT_WHITE);
          cell_status[act_x][act_y] += 1;     // B00000001
          cell_status[act_x][act_y-1] += 4;   // B00000100  
        } else if (b==1) {   // break out E
          tft.fillRect(9*(act_x+1),3+9*act_y,3,6,TFT_WHITE);
          cell_status[act_x][act_y] += 2;     // B00000010  
          cell_status[act_x+1][act_y] += 8;   // B00001000 
        } else if (b==2) {     // break out S
          tft.fillRect(3+9*act_x,9*(act_y+1),6,3,TFT_WHITE);   
          cell_status[act_x][act_y] += 4;     // B00000100 
          cell_status[act_x][act_y+1] += 1;   // B00000001
        } else if (b==3) {      // break out W
          tft.fillRect(9*act_x,3+9*act_y,3,6,TFT_WHITE);
          cell_status[act_x][act_y] += 8;     // B00001000 
          cell_status[act_x-1][act_y] += 2;   // B00000010       
        }
        break; 
      }
    }
    // index of next neighbour (cyclic)
    b+=cw; // cw can be -1 or +1
    if (b>3) b=0;
    if (b<0) b=3; 
  }
  return found;
}

//=====================================================================
boolean search_next(int act) {
  // pick random neighbour index and random search direction (CW or CCW)
  int b = random(4);
  int cw = 2*random(2)-1;// -1 (CCW) or +1 (CW) 
  boolean found = false; // unvisited and reachable neighbour found 
                         //  for newest cell in path?
  int act_x = active[act-1][0];
  int act_y = active[act-1][1];
  for (int bc=0;bc<4;bc++) {        // bc is just a neighbour counter
   if (act_x+dx[b]>=0 && act_x+dx[b]<HB && act_y+dy[b]>=0 && act_y+dy[b]<VB){
    // valid neighbour?
    if (cell_status[act_x+dx[b]][act_y+dy[b]] >= 128) {//unvisited neighbor
     // now check if there's wall between the active cell and this neighbor
     if (b==0 && bitRead(cell_status[act_x][act_y],0) == 1){//N,reachable
      tft.fillRect(4+9*act_x,4+9*(act_y-1),4,4,TFT_RED);
      found = true;
     } else if (b==1 && bitRead(cell_status[act_x][act_y],1) == 1){//E,reachable
       tft.fillRect(4+9*(act_x+1),4+9*act_y,4,4,TFT_RED); 
       found = true;
     } else if (b==2 && bitRead(cell_status[act_x][act_y],2) == 1){//S,reachable
       tft.fillRect(4+9*act_x,4+9*(act_y+1),4,4,TFT_RED);
       found = true;
     } else if (b==3 && bitRead(cell_status[act_x][act_y],3) == 1){//W,reachable
       tft.fillRect(4+9*(act_x-1),4+9*act_y,4,4,TFT_RED);
       found = true;
     }
    }
    if (found == true) {
      delay(step_delay);
      active[act][0] = act_x+dx[b];
      active[act][1] = act_y+dy[b];
      n_active++;
      cell_status[act_x+dx[b]][act_y+dy[b]] -= 128; 
      //B10000000 (reset first bit (MSB) to 0 which now means 'visited'!)
      if ((act_x+dx[b] == exit_col || act_x+dx[b] == exit_col+1) && 
          (act_y+dy[b] == exit_row || act_y+dy[b] == exit_row+1)) 
           found_exit = true;
        return true;
      } 
    }     
    // index of next neighbour (cyclic)
    b+=cw; // cw can be -1 or +1
    if (b>3) b=0;
    if (b<0) b=3;
  }
  return found;
}

//=====================================================================
void loop(void) {
  // create new maze
  for (k=0;k<NB;k++){active[k][0] = 0; active[k][1] = 0;}
  for (k=0;k<HB;k++){for (r=0;r<VB;r++){ cell_status[k][r] = 0;}}
  tft.fillScreen(TFT_WHITE);  // TFT_WHITE cells
  //tft.setBrightness(100);
  // draw walls TFT_BLACK
  for (k=0;k<(HB+1);k++)     { tft.fillRect(k*9,0,3,VW,TFT_BLACK);}
  for (int r=0;r<(VB+1);r++) { tft.fillRect(0,r*9,HW,3,TFT_BLACK);}
  // color remaining border space TFT_BLACK
  tft.fillRect(HW-2,0,2,VW,TFT_BLACK);
  tft.fillRect(0,VW-3,HW,3,TFT_BLACK);
  
  //pick random start cell for the generating process and put it in active set
  n_active = 1;
  active[0][0] = random(HB);
  active[0][1] = random(VB);
  cell_status[active[0][0]][active[0][1]] = 128; // B10000000;
  // build the maze
  while (n_active>0) {  // active set not empty
    if (select_next(n_active) == false) n_active--; // 'back tracking'
  }
  // draw entrance (random at long side border) 
  start_col = random(HB);
  tft.fillRect(3+9*start_col,VW-6,6,6,TFT_WHITE);
  // draw random target (4 cells in center area)
  exit_col = 10+random(15);
  exit_row = 10+random(7);
  tft.fillRect(3+9*exit_col,3+9*exit_row,15,15,TFT_CYAN);
  tft.fillCircle(10+9*exit_col,10+9*exit_row,5,TFT_RED); 
  // process start cell (start_col,0): 
  n_active = 1;
  active[0][0] = start_col;
  active[0][1] = VB-1;
  cell_status[start_col][VB-1] -= 128; //B10000000 (mark start cell visited)
  tft.fillRect(4+9*start_col,VW-11,4,11,TFT_RED);
  // Turn on the background LED to actually show the maze
  //digitalWrite(TFT_LED, HIGH);
  delay(1500);
  // build a path to exit
  found_exit = false;
  while (found_exit == false) {
   if (search_next(n_active) == false) {
    //remove last entry from path (and color it TFT_WHITE)
    n_active--;  // 'back tracking'
    tft.fillRect(4+9*active[n_active][0],4+9*active[n_active][1],4,4,TFT_WHITE);
    delay(step_delay);
   }
  }
  // path completed, now we will color the open walls between 
  //  consecutive cells in the path as well 
  for (int s=n_active;s>1;s--){
    // note: n_active is a counter,not a zero_offset index
    int cx = active[s-1][0];   // c: current; p: previous
    int cy = active[s-1][1];
    int px = active[s-2][0];
    int py = active[s-2][1];
    if (cx == px) {   // N/S neighbors
      if (cy>py) {    // color N wall of current cell
        tft.fillRect(4+9*cx,9*cy-1,4,5,TFT_RED);
      } else {        // color N wall of current cell
        tft.fillRect(4+9*cx,9*(cy+1)-1,4,5,TFT_RED);
      }
    } else {          // E/W neighbors
      if (cx>px) {    // color W wall of current cell
        tft.fillRect(9*cx-1,4+9*cy,5,4,TFT_RED);
      } else {        // color E wall of current cell
        tft.fillRect(9*(cx+1)-1,4+9*cy,5,4,TFT_RED);
      }
    }
  }  
  tft.fillRect(4+9*start_col,VW-7,4,7,TFT_RED); 
  // final cosmetics: color entrance and exit
  delay(4000);
  //tft.setBrightness(0);  // Turn off the background LED 
}
//=====================================================================
