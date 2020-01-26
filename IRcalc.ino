#include <IRremote.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <RtcDS3231.h>

#define POWER  0x02FD48B7
#define HOME   0x027D2CD3

#define NUM1   0x02FD807F
#define NUM2   0x02FD40BF
#define NUM3   0x02FDC03F
#define NUM4   0x02FD20DF
#define NUM5   0x02FDA05F
#define NUM6   0x02FD609F
#define NUM7   0x02FDE01F
#define NUM8   0x02FD10EF
#define NUM9   0x02FD906F
#define NUM0   0x02FD50AF
#define CH11   0x02FDD02F
#define CH12   0x02FD30CF
#define CHUP   0x02FDD827
#define CHDN   0x02FDF807
#define VOLUP  0x02FD58A7
#define VOLDN  0x02FD7887
#define MUTE   0x02FD08F7
#define PRG    0x02FD7689
#define DBUT   0xC23D28D7


#define OK     0x02FDBC43
#define ENTER  0x02FDBC43

#define BACK   0x02FDDC23

#define BLUE   0x02FDCE31
#define PLUS   0x02FDCE31
#define RED    0x02FD2ED1
#define MINUS  0x02FD2ED1
#define GREEN  0x02FDAE51
#define MULTI  0x02FDAE51
#define YELLOW 0x02FD6E91
#define DIV    0x02FD6E91

#define DISP   0x02FD38C7

#define LARROW 0x02FDFA05
#define RARROW 0x02FDDA25
#define UARROW 0x02FD7C83
#define DARROW 0x02FDFC03

#define CHDIGI 0x02FD5EA1
#define BSDIGI 0x02FD3EC1
#define CSDIGI 0x02FDBE41

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
  lcd.print("CALC V1.47");
  Serial.println("CALC V1.47");
  lcdx=0;lcdy=0;
  Rtc.Begin();
}

void loop() {
  int sp,d,i,j,k;
  char c;
  char textbuf[256];
  char lcdbuf[17];
  RtcDateTime nowtime;
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
      case BACK:
	inMenu = false;
	lcd.setCursor(0,0);
	lcd.print("CALC MODE       ");
	break;
	//      case DISP:
      default:
	lcd.setCursor(0,0);
	lcd.print("UNKNOWN CODE    ");
	Serial.println("UNKNOWN CODE");
	if (results.value!=0xffffffff) {
	  lcd.setCursor(0,1);
	  sprintf(lcdbuf,"0x%02X%02X%02X%02X",(byte)((results.value>>24)&(0xff)),
		  (byte)((results.value>>16)&(0xff)),
		  (byte)((results.value>>8)&(0xff)),(byte)results.value&(0xff));
	  Serial.println(lcdbuf);
	}
	break;
      case PRG:
	nowtime = Rtc.GetDateTime();
	printDateTime(nowtime);
	Serial.println();
	lcd.setCursor(0,0);
	lcd.print("CLOCK");
	lcd.setCursor(0,1);
	sprintf(lcdbuf,"%4u%02u%02u %02u%02u%02u ",
		nowtime.Year(),
		nowtime.Month(),
		nowtime.Day(),
		nowtime.Hour(),
		nowtime.Minute(),
		nowtime.Second() );
	lcd.print(lcdbuf);
	break;
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
