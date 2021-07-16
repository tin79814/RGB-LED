/*
   RGB mix LED
  /* ------------- macros ------------- */
// pin mapping
#define RGB_R_LED 11  // PWM
#define RGB_G_LED 10 // PWM
#define RGB_B_LED 9 // PWM
#define BUTTON 2 //ISR
#define LED_BUTTON 5

// LED operations (common cathode)
#define LED_ON    LOW
#define LED_OFF   HIGH

// operation parameters
#define HUE_DELAY  50       // mili-sec delay for showing a particular color
#define HUE_ANGLE_STEP  1   // changing hue angle n degrees for each delay

// RGB LED max weighting
// for visual balance
#define R_FACTOR 1.0
#define G_FACTOR 0.621
#define B_FACTOR 0.434

// LDR operation
#define LDR_THRESHOLD  250

/* ----------- global variables ---------------- */
long hue_prevTime;
float hue_theta = -1.0; // initial value to turn RGB LED off
volatile byte button_flag = 0;// volatile is for used in an ISR
volatile long button_prevTime;
volatile byte BUTTON_DELAY = 200;


/* ------------------------------------ */
float mod(float a, float b) {
  // calculate the remainder (modulo) of two floats
  // according to https://en.wikipedia.org/wiki/Modulo_operation
  // int() will do the truncation (no rounding)

  return a - (b * int(a / b) );
}

/* ------------------------------------ */
void hsv_to_rgb(float h,  float s, float v, byte *r, byte *g, byte *b) {
  // h, s, v: input parameters
  // r, g, b: results (in 0-255) (using pointer to get the var addresses)
  // conversion formula is based on
  // https://en.wikipedia.org/wiki/HSL_and_HSV

  float c; // chroma
  float h_prime; // H' in wiki
  float x; // intermediate value X for the second largest component of this color
  float r_tmp, g_tmp, b_tmp;  // R_1, G_1, B_1 in wiki
  float m; // offset to match color

  // check the input range
  if (h < 0 || h > 360 || s < 0 || s > 1 || v < 0 || s > 1) {
    // invalid input
    // set the result w/o any calculation
    *r = 0;
    *g = 0;
    *b = 0;
  }
  else {
    // branch for valid input
    c = v * s;
    h_prime = h / 60.0;
    x = c * (1 - abs(mod(h_prime, 2) - 1 )  );

    // calc R_1, G_1, B_1 according to H'
    if (h_prime < 1) {
      // 0 <= H' < 1
      r_tmp = c;
      g_tmp = x;
      b_tmp = 0.0;
    }
    else if (h_prime < 2) {
      // 1 <= H' < 2
      r_tmp = x;
      g_tmp = c;
      b_tmp = 0.0;
    }
    else if (h_prime < 3) {
      // 2 <= H' < 3
      r_tmp = 0.0;
      g_tmp = c;
      b_tmp = x;
    }
    else if (h_prime < 4) {
      // 3 <= H' < 4
      r_tmp = 0.0;
      g_tmp = x;
      b_tmp = c;
    }
    else if (h_prime < 5) {
      // 4 <= H' < 5
      r_tmp = x;
      g_tmp = 0.0;
      b_tmp = c;
    }
    else {
      // 5 <= H' < 6
      r_tmp = c;
      g_tmp = 0.0;
      b_tmp = x;
    }

    m = v - c;
    r_tmp += m;
    g_tmp += m;
    b_tmp += m;

    r_tmp = r_tmp * 255 + 0.5;  // original r_tmp in [0,1], now mapped to [0,255]; + 0.5 is for rounding before truncate
    g_tmp = g_tmp * 255 + 0.5;
    b_tmp = b_tmp * 255 + 0.5;

    *r = constrain(r_tmp, 0, 255);  // make sure it is not out of range
    *g = constrain(g_tmp, 0, 255);
    *b = constrain(b_tmp, 0, 255);
  }
}

/* ------------------------------------ */
void change_hue() {
  // using https://en.wikipedia.org/wiki/HSL_and_HSV
  // for HSV to RGB conversion
  byte r_byte, g_byte, b_byte;
  const float S = 1.0, V = 1.0; // fixed S, V; only H changes

  if (hue_theta > -0.5) {
    // initial value is -1
    // only enter here once

    if (millis() - hue_prevTime > HUE_DELAY ) {
      hue_prevTime = millis();
      hue_theta += HUE_ANGLE_STEP;
      if (hue_theta > 360) {
        // wrap around
        hue_theta -= 360;
      }

      hsv_to_rgb(hue_theta, S, V, &r_byte, &g_byte, &b_byte);

      // scaling down rgb values for more visual balance
      b_byte = (int)(b_byte * B_FACTOR);
      g_byte = (int)(g_byte * G_FACTOR);
      r_byte = (int)(r_byte * R_FACTOR);

      analogWrite(RGB_R_LED, r_byte);
      analogWrite(RGB_G_LED, g_byte);
      analogWrite(RGB_B_LED, b_byte);

    }
  }
  else {
    // for hue_theta < -0.5
    // hue_theta is -1
    // it is the case to turn off rgb LED
    analogWrite(RGB_R_LED, 0);
    analogWrite(RGB_G_LED, 0);
    analogWrite(RGB_B_LED, 0);
  }
}
/* --------------ISR---------------------- */

void change_state() {
  // this is an ISR
  // the body needs short and fast
  // millis() will not be updated in an ISR
  // it will be the same value within each call
  if (millis() - button_prevTime > BUTTON_DELAY) {
    button_flag = !button_flag; // 0 -- > 255; 255 --> 0
    button_prevTime = millis();
  }
}
/* ------------------------------------ */
void setup() {
  // put your setup code here, to run once:


  hue_prevTime = millis();
  hue_theta = 0.0;  // hue will run
  attachInterrupt(digitalPinToInterrupt(BUTTON), change_state, RISING);
  pinMode(LED_BUTTON,OUTPUT);
}

/* ------------------------------------ */
void loop() {
  // put your main code here, to run repeatedly:
  if (button_flag == 0) {
    change_hue();
    digitalWrite(LED_BUTTON, LOW);
  }
  else {
    digitalWrite(LED_BUTTON, HIGH);
  }

}
