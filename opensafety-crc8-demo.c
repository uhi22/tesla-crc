/* Example CRC calculation
* based on code from https://stackoverflow.com/questions/51752284/how-to-calculate-crc8-in-c
*/

#include <stdio.h>
typedef unsigned char uint8_t;


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

int main()
{
  uint8_t data[4] = {0x00,0x00,0x00,0x00};
  uint8_t crc;

    /* This should lead to
    For 00 00 00 00, the CRC is 00
    For 10 00 00 00, the CRC is 76
    For 20 00 00 00, the CRC is ec
    For 40 00 00 00, the CRC is f7
    For 80 00 00 00, the CRC is c1
    */
    data[0] = 0x00; crc = generateCrc8Opensafety(data, 4); printf("For 00 00 00 00, the CRC is %02x\n", crc);
    data[0] = 0x10; crc = generateCrc8Opensafety(data, 4); printf("For 10 00 00 00, the CRC is %02x\n", crc);
    data[0] = 0x20; crc = generateCrc8Opensafety(data, 4); printf("For 20 00 00 00, the CRC is %02x\n", crc);
    data[0] = 0x40; crc = generateCrc8Opensafety(data, 4); printf("For 40 00 00 00, the CRC is %02x\n", crc);
    data[0] = 0x80; crc = generateCrc8Opensafety(data, 4); printf("For 80 00 00 00, the CRC is %02x\n", crc);
    return 0;
}