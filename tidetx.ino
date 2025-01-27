#include "LoRaWan_APP.h"
#include "Arduino.h"

#define RF_FREQUENCY                                915000000 // Hz

#define TX_OUTPUT_POWER                             14        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 128 // Define the payload size here
#define SERIAL1_TIMEOUT 100 //timeout in ms
char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];


static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );
void sleep(void);

int16_t txNumber;
bool sleepMode = false;
int16_t Rssi,rxSize;
uint8_t serialBuffer[64];
int readSize;
int senbr;
int senavenbr;
int startCount;
float sensum;
float senave;
float batv;
uint8_t statID=2;

void setup() {
    Serial.begin(115200);
    Serial1.begin(9600);
    pinMode(Vext, OUTPUT);
    pinMode(GPIO5, OUTPUT);
    digitalWrite(Vext, LOW);
    digitalWrite(GPIO5, LOW); 
    txNumber=0;
    Rssi=0;
    sensum = 0;
    senave = 0;
    senbr = 0;
    senavenbr = 0;
    startCount = 1;
    batv = 0;

    readSize = 0;
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );
}

void loop()
{
	if (txNumber % 60 == 0 && txNumber != 0)
	{
    if (senbr != 0)
    {
      //Serial.printf("Sensor total: %10.2f\r\n", sensum);
      //Serial.printf("Sensor samples: %d\r\n", senbr);
      sensum = sensum/senbr;
      //Serial.printf("Average sensor reading: %10.2f\r\n", sensum);
    }
		uint16_t batteryVoltage = getBatteryVoltage();
    sprintf(txpacket,"S%d,",statID);
		sprintf(txpacket+strlen(txpacket),"V%d,",batteryVoltage);
		sprintf(txpacket+strlen(txpacket),"C%d,",Rssi);
		sprintf(txpacket+strlen(txpacket),"R%5.2f,",sensum);
		Radio.Send( (uint8_t *)txpacket, strlen(txpacket) );
		txNumber = 0;
    Rssi++;
    sensum = 0;
    senbr = 0;
  }
  txNumber++;
  digitalWrite(GPIO5, HIGH);
  delay(100);
  digitalWrite(GPIO5, LOW);
  delay(100);

  if (Serial1.available()) {
    int distance = Serial1.parseInt();
    sensum = sensum + float(distance);
    senbr++;
  }
  delay(800);
}

void OnTxDone( void )
{
	//Serial.print("TX done, entering Radio.Sleep Mode\r\n");
  Radio.Sleep();
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.print("TX Timeout......");
}
