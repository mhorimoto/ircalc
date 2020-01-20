#include <IRremote.h>
#include <LiquidCrystal_I2C.h>

#define NUM1  0x2FD807F
#define NUM2  0x2FD40BF
#define NUM3  0x2FDC03F
#define NUM4  0x2FD20DF
#define NUM5  0x2FDA05F
#define NUM6  0x2FD609F
#define NUM7  0x2FDE01F
#define NUM8  0x2FD10EF
#define NUM9  0x2FD906F
#define NUM0  0x2FD50AF
#define CH11  0x2FDD02F
#define CH12  0x2FD30CF
#define CHUP  0x2FDD827
#define CHDN  0x2FDF807

#define OK     0x2FDBC43
#define ENTER  0x2FDBC43

#define BACK   0x2FDDC23

#define BLUE   0x2FDCE31
#define PLUS   0x2FDCE31
#define RED    0x2FD2ED1
#define MINUS  0x2FD2ED1
#define GREEN  0x2FDAE51
#define MULTI  0x2FDAE51
#define YELLOW 0x2FD6E91
#define DIV    0x2FD6E91

#define DISP   0x2FD38C7

#define MAX_STACK_LEVEL 4

int RECV_PIN = 6;

IRrecv irrecv(RECV_PIN);
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

decode_results results;

int lcdx,lcdy,x;
int stack[MAX_STACK_LEVEL];
bool enpr = false;

void setup() {
  int sp;
  Serial.begin(115200);
  irrecv.enableIRIn(); // Start the receiver
  for(sp=0;sp<MAX_STACK_LEVEL;sp++) {
    stack[sp] = 0;
  }
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("CALC V1.00");
  Serial.println("CALC V1.00");
}

void loop() {
  int sp,d,i,j,k;
  char c;
  char textbuf[256];
  if (irrecv.decode(&results)) {
    switch(results.value) {
    case NUM0: d = 0; break;
    case NUM1: d = 1; break;
    case NUM2: d = 2; break;
    case NUM3: d = 3; break;
    case NUM4: d = 4; break;
    case NUM5: d = 5; break;
    case NUM6: d = 6; break;
    case NUM7: d = 7; break;
    case NUM8: d = 8; break;
    case NUM9: d = 9; break;
    case ENTER:
      stack_shift(-1,MAX_STACK_LEVEL,true);
      break;
    case PLUS:
      x += stack[0];
      Serial.println(); Serial.println(x);
      stack_shift(1,MAX_STACK_LEVEL,true);
      break;
    case MINUS:
      stack[0] -= x;
      x = stack[0];
      Serial.println(); Serial.println(x);
      stack_shift(1,MAX_STACK_LEVEL,true);
      break;
    case MULTI:
      x *= stack[0];
      Serial.println(); Serial.println(x);
      stack_shift(1,MAX_STACK_LEVEL,true);
      break;
    case DIV:
      x /= stack[0];
      Serial.println(); Serial.println(x);
      stack_shift(1,MAX_STACK_LEVEL,true);
      break;
    default:
      d = -1;
      break;
    }
    if ((d>=0)&&(d<=9)) {
      x *= 10;
      x += d;
      Serial.print(d,DEC);
    }
    irrecv.resume(); // Receive the next value
  }
}

void stack_shift(int direction,int max_stack,bool enpr) {
  int j;
  char textbuf[256];
  if (direction > 0) {
    for(j=0;j<max_stack-1;j++) stack[j] = stack[(j+1)];
    if (enpr) {
      for(j=0;j<max_stack;j++) {
	sprintf(textbuf,"SP[%d]=%d",j,stack[j]);
	Serial.println(textbuf);
      }
    }
  } else if (direction < 0) {
    for(j=(max_stack-1);j>0;j--) stack[j] = stack[(j-1)];
    stack[0] = x;
    x = 0;
    if (enpr) {
      Serial.println();
      for(j=0;j<max_stack;j++) {
	sprintf(textbuf,"SP[%d]=%d",j,stack[j]);
	Serial.println(textbuf);
      }
    }
  }
}
