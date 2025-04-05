#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Analysis of Tesla CRC calculation

Discussion: https://openinverter.org/forum/viewtopic.php?p=81258#p81258

*/

/* The magicBytes of the message 0x249. This is the value of the CRC byte, while the stick is in idle position. */
uint8_t magicBytes249[16] = { 0x9B, 0xE8, 0x2A, 0xD3, 0xD3, 0x83, 0x4C, 0x5E, 0x3F, 0x5E, 0xE2, 0x28, 0x3A, 0x13, 0xAF, 0xCE };

#define CAN_MESSAGE_LENGTH 4
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


/* Input: Line of the log file, like "28 0B 00 00".
   Output: array canMessage[], filled with the data from the log line. */
void splitLineIntoBytes(void) {
    uint8_t i;
    char hexbuffer[3];
    hexbuffer[2]=0; /* the terminating zero */
    printf("\nCAN message: ");
    for (i=0; i<CAN_MESSAGE_LENGTH; i++) {
        hexbuffer[0] = filelinebuffer[0+3*i];
        hexbuffer[1] = filelinebuffer[1+3*i];
        canMessage[i] = strtol(hexbuffer, NULL, 16);
        printf("%02x ", canMessage[i]);
    }
    //printf("\n");
}

/* Input: array canMessage
   Output: payloadData, crcFromCan, aliveCounter */
void extractPartsOfTheCanMessage(void) {
    int i;
    crcFromCan = canMessage[0]; /* byte 0 is the CRC */
    aliveCounter = canMessage[1] & 0x0F; /* alive counter is in the lower nibble of byte 1 */
    payloadData[0] = canMessage[1] & 0xF0; /* the high nibble of byte 0 is the first payload data */
    for (i=2; i<CAN_MESSAGE_LENGTH; i++) {
        payloadData[i-1] = canMessage[i]; /* copy all remaining bytes of the CAN message into payload buffer. */
    }
    payloadData[i-1] = 0x00; /* a "virtual byte" is appended, which value 0x00. */
    printf("payload ");
    for (i=0; i<CAN_MESSAGE_LENGTH; i++) {
        printf("%02x ", payloadData[i]);
    }
    //printf("\n");
}

/* Input: payloadData ... */
void calculateCrcAndCompare(void) {
    
    crcCalculated = generateCrc8Opensafety(payloadData, CAN_MESSAGE_LENGTH); /* step 1: calculate the CRC over the payload */
    crcCalculated ^= magicBytes249[aliveCounter]; /* step 2: add the magic byte, depending on the alive counter */
    if (crcCalculated == crcFromCan) {
        printf("CRC ok");
        nCounterCrcOk++;
    } else {
        printf("CRC mismatch. CAN: %02x calculated: %02x", crcFromCan, crcCalculated);
        nCounterCrcFail++;
    }
}

void readLogFile(void) {
  FILE* filePointer;
  int nLines=0;

    filePointer = fopen("249_sortedFile.txt", "r"); /* open the log file which contains the original messages */
    while(fgets(filelinebuffer, BUFFER_LENGTH, filePointer)) {
        //printf("%s", filelinebuffer);
        splitLineIntoBytes();
        extractPartsOfTheCanMessage();
        calculateCrcAndCompare();
        nLines++;
    }
    fclose(filePointer);
    printf("\nLooped over %d lines.\n", nLines);
    printf("CRC ok: %d,  CRC fail: %d\n", nCounterCrcOk, nCounterCrcFail);
}

int main() {
  printf("Tesla CRC analysis\n");
  readLogFile();
  return 0;
}
