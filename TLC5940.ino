/* Arduino RGB LED Color Organ controller firmware.
  A program to run a single Channel of RGB LEDs using a hardware user interface of buttons, switches, and potentiometers.
  For color organ operation a secondary circuit is attached for bass detection using a preamp stage and an active bandpass
  filter. A software threshold cuts audio around 160Hz, bandpass center at 60Hz.

  Andrew Urbanas 2015

 */

 #include "Tlc5940.h"

float red_out, green_out, blue_out; //RGB Brightness output
int mode = 2;
int TOGGLE = 0;
int i = 0;
float intensity = 0;
float t = 0;
int color = 0;  //COLOR array index
int COLOR[15][3] = {
   {255, 0, 0},  //red
   {205, 50, 0}, //orange
   {50, 205, 0},  //yellow
   {0, 255, 0},  //green
   {0, 128, 128},  //cyan
   {0, 0, 255},  //blue
   {205, 0, 50},  //sexy-time pink
};

int buttonState[4]={0};
int lastButtonState[4]={0};
static int lastavg = 0;

void setup()  {
    pinMode(A0, INPUT); //Bass Circuit input
    Tlc.init();
    int direction = 1;
    Tlc.clear();
    Serial.begin(115200);
}

void loop()  {
//Timing setup
      static unsigned int t_comp = 0;
      int t_wait;
      int t_int;

//Output Bound Check
      if(red_out < 0 )
        red_out = 0;
      if(green_out < 0)
        green_out = 0;
      if(blue_out < 0)
        blue_out = 0;

      if(red_out > 4095 )
        red_out = 4095;
      if(green_out > 4095)
        green_out = 4095;
      if(blue_out > 4095)
        blue_out = 4095;

//RGB PWM WRITE
      Tlc.set(0, red_out);
      Tlc.set(1, green_out);
      Tlc.set(2, blue_out);

      Tlc.set(3, blue_out);
      Tlc.set(4, red_out);
      Tlc.set(5, green_out);

      Tlc.set(13, green_out);
      Tlc.set(14, blue_out);
      Tlc.set(15, red_out);

      Tlc.update();

//Bass Detector input smoothing

      int signal;
      int filter = 20;
      static int avg = 127;

      signal = (analogRead(A0));
      signal = signal - 180;
      avg = (filter*avg+signal)/(filter+1);
      if(avg < 0 )
        avg = 0;

      if(mode == 2) {
        intensity = (avg*1.75)+32; //Change intensity to match with glow
        intensity = intensity * 16;
          if (intensity <=32)
            intensity = 32;  //Glow for linear mixing mode
        } else {
          intensity = avg*2;
      }
      t_wait = 1;

//Bounds Check on intensity
      Serial.println(intensity);
      if(intensity >4095)
          intensity = 4095;
      if(intensity <= 0)
          intensity = 0;

//Timing conditions

      if(t_wait <= 1)
         t_wait = 1;
      t_int = millis() - t_comp;


//Color modes
switch (mode) {

/*
Preset Color Jump using COLOR array. In normal mode this jumps between several preset colors at an interval
controlled by the speed potentiometer. If in Color organ mode, the preset color only changes when the intensity
goes from negative to positive
*/
    case(1): {
      if (digitalRead(4) == LOW) {  //Normal mode
        if(t_int >= t_wait*128) {
          t_comp = millis();
          while((color = random(0, 7)) == i); //We don't want the same color twice
          i = color;
        }
      } else {  //Organ mode
        if ((intensity > 0) && (lastavg <= 0)) {
            t_comp = millis();
            while((color = random(0, 7)) == i); //We don't want the same color twice
            i = color;
        }
        lastavg = intensity;
      }

      red_out= COLOR[i][0]*(intensity)/255;
      green_out= COLOR[i][1]*(intensity)/255;
      blue_out= COLOR[i][2]*(intensity)/255;
      break;
    }

/*
Linear Fading with mixing.
*/
    case(2): {
      if(t >= 0 && t < 4095) {
        red_out = intensity*t;
        red_out = red_out/4095;
        green_out = 0;
        blue_out = intensity*(4095-t);
        blue_out = blue_out/4095;
      }
      if(t >= 4096 && t < 8191) {
        red_out = intensity*(8191-t);
        red_out = red_out/4095;
        green_out = intensity*(t-4096);
        green_out = green_out/4095;
        blue_out= 0;
      }
      if (t >= 8192 && t < 12287) {
        red_out= 0;
        green_out = intensity*(12287-t);
        green_out = green_out/4095;
        blue_out = intensity*(t-4096);
        blue_out = blue_out/4095;
      }
      if(t_int >= t_wait/2) { //Timer condition
        t_comp = millis();
        t = t + 1;
        if(t >= 12287)
          t = 0;
      }
      break;
    }

 default: break;
  }
}
