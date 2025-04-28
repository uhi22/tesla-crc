#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Analysis of VW CRC calculation

Discussion: https://openinverter.org/forum/viewtopic.php?p=81924#p81924

*/

/* The magicBytes of the message ESP21. This is the value of the CRC byte, while all data bits are zero. */
uint8_t magicBytesESP21[16] = { 0x19^0xBA, 0x13^0xBA, 0xDC^0xBA, 0xA9^0xBA, 0x3F^0xBA, 0xA5^0xBA, 0xB7^0xBA, 0x8D^0xBA,
                                0x4A^0xBA, 0x78^0xBA, 0x34^0xBA, 0x1B^0xBA, 0x82^0xBA, 0x0B^0xBA, 0xF4^0xBA, 0x63^0xBA };

#define CAN_MESSAGE_LENGTH 8 /* the maximum length. Actually may be shorter. */
uint8_t canMessage[CAN_MESSAGE_LENGTH];
#define BUFFER_LENGTH 100
uint8_t filelinebuffer[BUFFER_LENGTH];
uint8_t bytebuffer[BUFFER_LENGTH];
uint8_t payloadData[CAN_MESSAGE_LENGTH]; /* same length as the CAN message, because CRC left out at the beginning, and virtual byte appended. */
uint8_t aliveCounter, crcFromCan, crcCalculated;
uint32_t nCounterCrcOk, nCounterCrcFail;


/*********************************************************************************************/
/* CRC parameters for OPENSAFETY CRC-8, according to
   https://crccalc.com/?crc=00%2000%2001&method=CRC-8/OPENSAFETY&datatype=hex&outtype=hex */
#define POLYNOM 0x2F
uint8_t generateCrc8Opensafety(uint8_t *data, size_t len) {
    uint8_t crc = 0x00; /* initial value: 0x00 */
    size_t i, j;
    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if ((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ POLYNOM);
            else
                crc <<= 1;
        }
    }
    /* no final xor */
    return crc;
}
/*********************************************************************************************/


/* Input: Line of the log file, in the savvyCan format
   Time Stamp,ID,Extended,Dir,Bus,LEN,D1,D2,D3,D4,D5,D6,D7,D8
   for example "4335546,000000FD,false,Rx,0,8,A5,D5,1F,80,00,00,03,00,".
   Output: array canMessage[], filled with the data from the log line. */
int splitLineIntoBytes(uint8_t lenOfMessage) {
    uint8_t i;
    uint32_t nTime;
    char strID[200];
    char strExtended[20];
    char strDir[20];
    uint8_t nBus, nLen;
    uint8_t nConverted;
    char* pch;
    uint8_t elementNumber;

    elementNumber = 0;
    nTime = 0;
    pch = strtok(filelinebuffer,",");
    while (pch != NULL) {
      //printf("%s\n",pch);
      if (elementNumber==0) {
          nTime = atoi(pch);
          //printf("Time %d\n", nTime);
      }
      if (elementNumber==5) { nLen = atoi(pch);}
      if ((elementNumber>=6) && (elementNumber<=13)) { canMessage[elementNumber-6] = strtol(pch, NULL, 16);}
      elementNumber++;
      pch = strtok (NULL, ",");
    }
    if (nTime==0) {
        printf("This line is not a valid CAN message.\n");
        return -1; /* fail */
    } else {
      printf("\nCAN message: ");
      for (i=0; i<lenOfMessage; i++) {
        printf("%02x ", canMessage[i]);
      }
      return 0;
      //printf("\n");
    }
}

/* Input: array canMessage
   Output: payloadData, crcFromCan, aliveCounter */
void extractPartsOfTheCanMessage(uint8_t lenOfMessage) {
    int i;
    crcFromCan = canMessage[0]; /* byte 0 is the CRC */
    aliveCounter = canMessage[1] & 0x0F; /* alive counter is in the lower nibble of byte 1 */
    payloadData[0] = canMessage[1] & 0xF0; /* the high nibble of byte 0 is the first payload data */
    for (i=2; i<lenOfMessage; i++) {
        payloadData[i-1] = canMessage[i]; /* copy all remaining bytes of the CAN message into payload buffer. */
    }
    payloadData[i-1] = 0x00; /* a "virtual byte" is appended, which value 0x00. */
    printf("payload ");
    for (i=0; i<lenOfMessage; i++) {
        printf("%02x ", payloadData[i]);
    }
    //printf("\n");
}

/* Input: payloadData ... */
void calculateCrcAndCompare(uint8_t lenOfMessage, uint8_t *magicBytes) {
    
    crcCalculated = generateCrc8Opensafety(payloadData, lenOfMessage); /* step 1: calculate the CRC over the payload */
    crcCalculated ^= magicBytes[aliveCounter]; /* step 2: add the magic byte, depending on the alive counter */
    if (crcCalculated == crcFromCan) {
        printf("CRC ok");
        nCounterCrcOk++;
    } else {
        printf("CRC mismatch. CAN: %02x calculated: %02x", crcFromCan, crcCalculated);
        nCounterCrcFail++;
    }
}

void readLogFile(char *filename, uint8_t lenOfMessage, uint8_t *magicBytes) {
  FILE* filePointer;
  int nLines=0;

    printf("using log file %s\n", filename);
    filePointer = fopen(filename, "r"); /* open the log file which contains the original messages */
    while(fgets(filelinebuffer, BUFFER_LENGTH, filePointer)) {
        //printf("\n%s", filelinebuffer);
        if (splitLineIntoBytes(lenOfMessage)>=0) {
          extractPartsOfTheCanMessage(lenOfMessage);
          //if (aliveCounter==0) {
            calculateCrcAndCompare(lenOfMessage, magicBytes);
          //}
          nLines++;
          //if (nLines>32) break;
        }
    }
    fclose(filePointer);
    printf("\nLooped over %d lines.\n", nLines);
    printf("CRC ok: %d,  CRC fail: %d\n", nCounterCrcOk, nCounterCrcFail);
}

int main() {
  printf("VAG CRC analysis\n");
  readLogFile("mqb_esp21_0fd_DrivingLog1_7-11-23.csv", 8, magicBytesESP21);
  readLogFile("mqb_esp21_0fd_DrivingLog1_7-17-23.csv", 8, magicBytesESP21);
  return 0;
}
