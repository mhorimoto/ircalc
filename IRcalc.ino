#include <IRremote.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <RtcDS3231.h>

#define POWER 0x2FD48B7
#define HOME  0x27D2CD3

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
#define VOLUP 0x2FD58A7
#define VOLDN 0x2FD7887
#define MUTE  0x2FD08F7
#define PRG   0x2FD7689
#define DBUT  0xC23D28D7


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
RtcDS3231<TwoWire> Rtc(Wire);

decode_results results;

int lcdx,lcdy,x;
int stack[MAX_STACK_LEVEL];
bool enpr = false;
bool bklight = true;
bool inMenu = false;

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
  lcd.print("CALC V1.38");
  Serial.println("CALC V1.38");
  lcdx=0;lcdy=0;
  Rtc.Begin();
}

void loop() {
  int sp,d,i,j,k;
  char c;
  char textbuf[256];
  char lcdbuf[17];
  if (irrecv.decode(&results)) {
    if (inMenu) {
      switch(results.value) {
      case POWER:
	if (bklight) {
	  bklight = false;
	  lcd.noBacklight();
	} else {
	  bklight = true;
	  lcd.backlight();
	}
	break;
      case MUTE:
	inMenu = false;
	break;
      case PRG:
	RtcDateTime now = Rtc.GetDateTime();
	printDateTime(now);
	Serial.println();
	lcd.setCursor(0,0);
	lcd.print("CLOCK");
	lcd.setCursor(0,1);
	lcd.print(now);
	break;
      default:
	if (results.value!=0xffffffff) {
	  lcd.setCursor(0,1);
	  sprintf(lcdbuf,"0x%02X%02X%02X%02X",(byte)((results.value>>24)&(0xff)),
		  (byte)((results.value>>16)&(0xff)),
		  (byte)((results.value>>8)&(0xff)),(byte)results.value&(0xff));
	  Serial.println(lcdbuf);
	  break;
	}
      }
    } else {
      switch(results.value) {
      case MUTE:
	inMenu = true;
	lcd.setCursor(0,0);
	lcd.print("MENU MODE       ");
	break;
      case POWER:
	if (bklight) {
	  bklight = false;
	  lcd.noBacklight();
	} else {
	  bklight = true;
	  lcd.backlight();
	}
	break;
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
	lcdx = 0;
	lcd.setCursor(0,1);
	lcd.print("                ");
	lcd.setCursor(0,1);
	lcd.print(0,DEC);
	stack_shift(-1,MAX_STACK_LEVEL,true);
	break;
      case PLUS:
	lcdx = 0;
	x += stack[0];
	lcd.setCursor(0,1);
	lcd.print("                ");
	lcd.setCursor(0,1);
	lcd.print(x,DEC);
	Serial.println(); Serial.println(x);
	stack_shift(1,MAX_STACK_LEVEL,true);
	break;
      case MINUS:
	lcdx = 0;
	stack[0] -= x;
	x = stack[0];
	Serial.println(); Serial.println(x);
	stack_shift(1,MAX_STACK_LEVEL,true);
	break;
      case MULTI:
	lcdx = 0;
	x *= stack[0];
	Serial.println(); Serial.println(x);
	stack_shift(1,MAX_STACK_LEVEL,true);
	break;
      case DIV:
	lcdx = 0;
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
	lcd.setCursor(lcdx,1);
	lcd.print(d);
	lcdx++;
      }
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
	if (j==0) {
	  lcd.setCursor(0,0);
	  lcd.print("                ");
	  lcd.setCursor(0,0);
	  lcd.print(textbuf);
	}
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
	if (j==0) {
	  lcd.setCursor(0,0);
	  lcd.print("                ");
	  lcd.setCursor(0,0);
	  lcd.print(textbuf);
	}
	Serial.println(textbuf);
      }
    }
  }
}
void printDateTime(const RtcDateTime& dt) {
  char datestring[20];

  sprintf(datestring, "%02u/%02u/%04u %02u:%02u:%02u",
	     dt.Month(),
	     dt.Day(),
	     dt.Year(),
	     dt.Hour(),
	     dt.Minute(),
	     dt.Second() );
  Serial.print(datestring);
}
