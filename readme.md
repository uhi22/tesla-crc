# Tesla (and others) CRC Analysis

## Status & News

### 2025-04-28: Added sub-folder for Volkswagen analysis

CRC calculation of MQBs ESP_21 message works.

### 2025-04-06: CRC for 0x229 also fits

The tesla-crc-filereader.c reads the 870 samples of the message 0x249 plus 178 samples of the message 0x229 (both from Ref2), calculates the CRC of each message and compares it with the CRC from the log. No deviation, all CRCs are correctly calculated.

The algorithm for transmitting the end-to-end protected message:

* In the example, we use a 4 byte CAN message. Other length are also possible.
* Use a buffer which is one byte bigger than the CAN message. So 5 bytes in the example. We use index 0 to 4 for them.
* Fill the complete message with 0x00. This is especially needed for byte 0 (later to be filled with the CRC), the byte 1 (later to be filled with the alive counter, and byte 4 (the virtual extra byte).
* Fill the payload data into the message. This will be in the higher nibble of byte 1, and the full bytes 2 and 3.
* Calculate the CRC (using the OPENSAFETY CRC-8, function generateCrc8Opensafety()) starting at byte 1 and length 4. So it sees the virtual extra byte as last input data.
* Calculate the magic byte by using the current alive counter as index in the magic byte table.
* Fill into byte 0 the XOR of the CRC and the magic byte.
* Fill into the lower nibble of byte 1 the alive counter.
* Transmit the bytes 0 to 3 to CAN.
* Increment the alive counter. If it reaches 16, set it to zero.

The table of magic bytes is individual for each CAN identifier. It can be created based on the observed log files. The magic byte is the value of byte 0 of the message, when all payload bits are zero. We have 16 possible values for the alive counter, so the table of magic bytes has 16 entries.

### 2025-04-05: CRC for 0x249 fully verified

The tesla-crc-filereader.c reads the 870 samples from Ref2, calculates the CRC of each message and compares it with the CRC from the log. No deviation, all CRCs are correctly calculated.

```
Looped over 870 lines.
CRC ok: 870,  CRC fail: 0
```

This confirms the approach:
* It is a CRC8 with polynom 2F, like CRC-8/OPENSAFETY.
* Alive counter is not included into CRC. Instead, alive counter is index into a 16-byte-table with magic bytes, which are individual for each ID.

### 2025-04-04: Polynom for 0x249 found

see below. The CRC is using the "12F" polynom of OPENSAFETY (or AUTOSAR).

### 2025-04-04: Calculation for the 0x229 works

We use a table of magic patterns, this is 16 bytes long, and the alive counter is the index in this table.
We simply fill the table with the known CRC values in idle position.
As soon as a button is actuated, the CRC changes by a specific XOR delta. Each button has a specific delta. The delta is the same for all possible 16 message counter values. This behavior is typical for a CRC. So it could be possible, to replace the currently implemented `if button then xorPattern = something` by a real CRC calculation.

Test result: The program gives the same numbers as listed in https://openinverter.org/forum/viewtopic.php?p=81258#p81258

```
uwemi@DESKTOP-OKFJ35V MINGW64 /c/UwesTechnik/tesla-crc
$ gcc tesla-crc.c

uwemi@DESKTOP-OKFJ35V MINGW64 /c/UwesTechnik/tesla-crc
$ ./a.exe
Tesla CRC analysis
Idle position
0x46 0x00 0x00
0x44 0x01 0x00
0x52 0x02 0x00
0x6d 0x03 0x00
0x43 0x04 0x00
0x41 0x05 0x00
0xdd 0x06 0x00
0xf9 0x07 0x00
0x4c 0x08 0x00
0xa5 0x09 0x00
0xf6 0x0a 0x00
0x8c 0x0b 0x00
0x49 0x0c 0x00
0x2f 0x0d 0x00
0x31 0x0e 0x00
0x3b 0x0f 0x00
Full down
0xb7 0x40 0x00
0xb5 0x41 0x00
0xa3 0x42 0x00
0x9c 0x43 0x00
0xb2 0x44 0x00
0xb0 0x45 0x00
0x2c 0x46 0x00
0x08 0x47 0x00
0xbd 0x48 0x00
0x54 0x49 0x00
0x07 0x4a 0x00
0x7d 0x4b 0x00
0xb8 0x4c 0x00
0xde 0x4d 0x00
0xc0 0x4e 0x00
0xca 0x4f 0x00
park button
0xaf 0x00 0x01
0xad 0x01 0x01
0xbb 0x02 0x01
0x84 0x03 0x01
0xaa 0x04 0x01
0xa8 0x05 0x01
0x34 0x06 0x01
0x10 0x07 0x01
0xa5 0x08 0x01
0x4c 0x09 0x01
0x1f 0x0a 0x01
0x65 0x0b 0x01
0xa0 0x0c 0x01
0xc6 0x0d 0x01
0xd8 0x0e 0x01
0xd2 0x0f 0x01
Full up
0xa9 0x20 0x00
0xab 0x21 0x00
0xbd 0x22 0x00
0x82 0x23 0x00
0xac 0x24 0x00
0xae 0x25 0x00
0x32 0x26 0x00
0x16 0x27 0x00
0xa3 0x28 0x00
0x4a 0x29 0x00
0x19 0x2a 0x00
0x63 0x2b 0x00
0xa6 0x2c 0x00
0xc0 0x2d 0x00
0xde 0x2e 0x00
0xd4 0x2f 0x00
Half down
0x49 0x30 0x00
0x4b 0x31 0x00
0x5d 0x32 0x00
0x62 0x33 0x00
0x4c 0x34 0x00
0x4e 0x35 0x00
0xd2 0x36 0x00
0xf6 0x37 0x00
0x43 0x38 0x00
0xaa 0x39 0x00
0xf9 0x3a 0x00
0x83 0x3b 0x00
0x46 0x3c 0x00
0x20 0x3d 0x00
0x3e 0x3e 0x00
0x34 0x3f 0x00

```


## Checking the 0x249

```
Starting with the log data (Ref2), extracting messages which shift a single bit:
9B 00 00 00
ED 10 00 00
77 20 00 00
6C 40 00 00
5A 80 00 00

Calculating the XOR difference between the "zero" and the bit:
data       0    1    2    4    8
crc       9B   ED   77   6C   5A
xor          76
                 EC
                      F7
                          C1

According to Ref4, looking whether this sequence 76 EC F7 C1 fits either to "just shifted" or to "shifted and Xored".

76 << 1 = EC  -> just shifted.
EC << 1 = D8 but we see F7. This will reveal the polynom.
    (EC << 1) XOR F7 = 12F --> 1 0010 1111 --> Polynom is x^8 + x^5 + x^3 + x^2 + x^1 + x^0.
F7 << 1 = EE but we see C1. This will reveal the polynom.
     (F7 << 1) XOR C1 = 12F. As above. Polynom confirmed.


Trying the same procedure with an other part of the data:
9B 00 00 00
87 00 02 00
A3 00 04 00
EB 00 08 00

data    0      2    4   8  
crc     9B     87  A3   EB
xor         1C
                 38
                      70

1C << 1 = 38 --> just shifted
38 << 1 = 70 --> just shifted

```

The polynom 12F (or just 2F if looking to 8 bits) is used in two "official" configurations according to Ref5: CRC-8/OPENSAFETY and CRC-8/AUTOSAR.

Try with CRC-8/OPENSAFETY (Ref5):
```
10 00 00 00 gives 76
20 00 00 00 gives EC
40 00 00 00 gives F7
80 00 00 00 gives C1

00 02 00 00 gives 1C
00 04 00 00 gives 38
00 08 00 00 gives 70
```

This fits perfectly to the above calculated XOR values.

Intermediate calculation algorithm:
* Input: Inputbuffer[4]. Byte0 will later contain the CRC. Byte1 contains the counter in lower nibble. Treat as zero.
* copy inputbuffer[1...3] into temporaryBuffer[0...2]
* set temporaryBuffer[3] = 0
* calculate CRC over temporaryBuffer[0...3], using "CRC-8/OPENSAFETY with poly=2F".
* xor the CRC with 9B (magicByte, depending on alive counter)
* write the result into Inputbuffer[0]
* write the alive counter into lower nibble of Inputbuffer[1]

## Open Todos

(none)

## References

* Ref1: Data of the 0x229 stalk: https://openinverter.org/forum/viewtopic.php?p=81258#p81258
* Ref2: Logs with many combinations, for left and right stalk: https://openinverter.org/forum/viewtopic.php?p=81331#p81331
* Ref3: Simple addition for 0x313: https://openinverter.org/forum/viewtopic.php?p=77694#p77694
* Ref4: How to calculate the CRC polynom out of observed data: https://knx-user-forum.de/forum/%C3%B6ffentlicher-bereich/knx-eib-forum/diy-do-it-yourself/34358-ekey-fingerscanner-per-rs485-auslesen-protokollanalyse?p=626281#post626281
* Ref5: CRC online calculator https://crccalc.com/?crc=00%2000%2001&method=CRC-8/AUTOSAR&datatype=hex&outtype=hex


