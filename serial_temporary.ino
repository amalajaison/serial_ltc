#include <SPI.h>

// Serial LTC program by Amala Jaison

// LTC SPI communications parameters
const int chipSelectPin=10;
uint8_t channel;

// Parameters related to parsing serial input
const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

// variables to hold the parsed data
char messageFromPC[numChars] = {0};
int integerFromPC = 0;
float floatFromPC = 0.0;

boolean newData = false;



void setup()
{
  Serial.begin(9600);
  Serial.println("Enter data in this style <SET channel voltage>");   
  
  SPI.begin();
  pinMode(chipSelectPin, OUTPUT);

// example of changing the output range to +/- 10 Volts
  digitalWrite(chipSelectPin, LOW);

  //LTC2668_write(LTC2668_CS,LTC2668_CMD_WRITE_N_UPDATE_N, selected_dac, dac_code);

  SPI.transfer(byte(0x00));
  SPI.transfer(byte(0xe0));
  SPI.transfer(byte(0x00));
  SPI.transfer(byte(0x03));

  digitalWrite(chipSelectPin, HIGH);


// example of setting the voltage and enabling

  digitalWrite(chipSelectPin, LOW);

 
  channel=0;

  SPI.transfer(byte(0x00));
  SPI.transfer(byte(0x30|channel));
  SPI.transfer(byte(0xff));
  SPI.transfer(byte(0xff));
  
  digitalWrite(chipSelectPin, HIGH);


// example of MUX enable
  digitalWrite(chipSelectPin, LOW);
 
  channel=4;

  SPI.transfer(byte(0x00));
  SPI.transfer(byte(0xb0));
  SPI.transfer(byte(0x00));
  SPI.transfer(byte(0x10|channel));
  
  digitalWrite(chipSelectPin, HIGH);

}


void loop()
{
    recvWithStartEndMarkers();
    if (newData == true) {
        strcpy(tempChars, receivedChars);
            // this temporary copy is necessary to protect the original data
            //   because strtok() used in parseData() replaces the commas with \0
        parseData();
        showParsedData();
        // set voltage and enable

        digitalWrite(chipSelectPin, LOW);


        if(!strncmp(messageFromPC, "SET", 3)) {
          
          Serial.println("Setting");
 
          channel=integerFromPC;


          SPI.transfer(byte(0x00));
          SPI.transfer(byte(0x30|channel));
          //SPI.transfer(byte(0xff));
          //SPI.transfer(byte(0xff));
          SPI.transfer16(dac_value(floatFromPC));

  
          digitalWrite(chipSelectPin, HIGH);
          
        } else if(!strncmp(messageFromPC, "MUX", 3)) {
          
          Serial.println("Changing MUX");
          digitalWrite(chipSelectPin, LOW);
 
          channel=integerFromPC;
 
          SPI.transfer(byte(0x00));
          SPI.transfer(byte(0xb0));
          SPI.transfer(byte(0x00));
          SPI.transfer(byte(0x10|channel));
  
          digitalWrite(chipSelectPin, HIGH);

        }

        newData = false;
    }
}

//============

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

//============

void parseData() {      // split the data into its parts

    char * strtokIndx; // this is used by strtok() as an index
    char delimiter[3]=", ";

    strtokIndx = strtok(tempChars,delimiter);      // get the first part - the string
    strcpy(messageFromPC, strtokIndx); // copy it to messageFromPC
 
    strtokIndx = strtok(NULL,delimiter); // this continues where the previous call left off
    integerFromPC = atoi(strtokIndx);     // convert this part to an integer

    strtokIndx = strtok(NULL,delimiter);
    floatFromPC = atof(strtokIndx);     // convert this part to a float

}

//============

void showParsedData() {
    Serial.print("Message ");
    Serial.println(messageFromPC);
    Serial.print("Integer ");
    Serial.println(integerFromPC);
    Serial.print("Float ");
    Serial.println(floatFromPC);
}

uint16_t dac_value(float volts) {
  float minv=-10.0;
  float maxv=10.0;
  return uint16_t((volts-minv)/(maxv-minv)*65535);  
}
