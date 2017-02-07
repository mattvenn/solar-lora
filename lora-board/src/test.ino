#include <SPI.h>
#include <RH_RF95.h>

#define GPIO7 8
#define GPIO8 9
#define GPIO9 10

#define batt_adc_on 11
#define batt_adc A0
#define panel_adc A1
/* radio setup */
#define RFM95_CS A2
#define RFM95_RST A3
#define RFM95_INT 2 // oh shit?

// panel adc is 12k and 10k, 
// batt adc is 5.6k and 12k
// vcc = 3300, adc count = 1024
// v = ((r1+r2)/r2) * 3.22mv * ADC count
#define ADC_PAN_CONST 7.0898
#define ADC_BAT_CONST 4.7266

/*
WDT code from 
http://donalmorrissey.blogspot.com.es/2010/04/sleeping-arduino-part-5-wake-up-via.html
 */
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 434.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);


volatile int f_wdt=1;

void setup()
{
    Serial.begin(115200);
    Serial.println("started");

    pinMode(GPIO7, OUTPUT);
    pinMode(GPIO8, OUTPUT);
    pinMode(GPIO9, OUTPUT);

    pinMode(batt_adc_on, OUTPUT);
    digitalWrite(batt_adc_on, false);

    // this pin is tied high externally to stop radio responding when programming icsp, so will draw 0.17mA when set low
    //digitalWrite(radio_cs, true);
    setupWDT();
    setupRadio();
}

int panel_mv = 0;
int batt_mv = 0;

void measure_sensors()
{
    Serial.print("pan:");
    panel_mv = ADC_PAN_CONST * analogRead(panel_adc);
    Serial.println(panel_mv);
    digitalWrite(batt_adc_on, true);
    delay(1);
    Serial.print("bat:");
    batt_mv = ADC_BAT_CONST * analogRead(batt_adc);
    Serial.println(batt_mv);
    digitalWrite(batt_adc_on, false);

}
void loop()
{

  if(f_wdt == 1)
  {
    //digitalWrite(GPIO7, !digitalRead(GPIO7));
    measure_sensors();
    send_pack();

    // saves 1.8mA
    rf95.sleep();
    
    f_wdt = 0;
    //todo put rfm to sleep here
    enterSleep();
  }
  else
  {
  }
}

int16_t packetnum = 0;  // packet counter, we increment per xmission

void send_pack()
{
  Serial.println("Sending to rf95_server");
  // Send a message to rf95_server
  
  char buffer [50];

  int sendLen = sprintf(buffer, "BAT:%d PAN:%d UP:%lu", batt_mv, panel_mv, millis());
  sendLen += 1;  // add one for the null terminator
  Serial.println(sendLen);

  rf95.send((uint8_t *)buffer, sendLen);
  //rf95.send((uint8_t *)radiopacket, 20);

  Serial.println("Waiting for packet to complete..."); delay(10);
  rf95.waitPacketSent();

/*
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  Serial.println("Waiting for reply..."); delay(10);
  if (rf95.waitAvailableTimeout(1000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);    
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
  else
  {
    Serial.println("No reply, is there a listener around?");
  }
  delay(1000);
*/
}
    



ISR(WDT_vect)
{
  if(f_wdt == 0)
  {
    f_wdt=1;
  }
  else
  {
    Serial.println("WDT Overrun!!!");
  }
}


void enterSleep(void)
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   /* EDIT: could also use SLEEP_MODE_PWR_DOWN for lowest power consumption. */
  sleep_enable();
  
  /* Now enter sleep mode. */
  sleep_mode();
  
  /* The program will continue from here after the WDT timeout*/
  sleep_disable(); /* First thing to do is disable sleep. */
  
  /* Re-enable the peripherals. */
  power_all_enable();
}


void setupWDT()
{
  /*** Setup the WDT ***/
  /* Clear the reset flag. */
  MCUSR &= ~(1<<WDRF);
  
  /* In order to change WDE or the prescaler, we need to
   * set WDCE (This will allow updates for 4 clock cycles).
   */
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  /* set new watchdog timeout prescaler value */
  WDTCSR = 1<<WDP0 | 1<<WDP3; /* 8.0 seconds */
  
  /* Enable the WD interrupt (note no reset). */
  WDTCSR |= _BV(WDIE);
}
  



void setupRadio() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.println("Feather LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}

