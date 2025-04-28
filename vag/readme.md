# CRC Analysis for Volkswagen End-to-End message

## Status & News

### 2025-04-28: ESP_21 understood

The vag-crc-filereader.c reads log files (csv from SavvyCAN), calculates the CRC and compares it with the transmitted CRC.
CRC of all 91622 messages is correct.

- started with the same same CRC polynom as described in the tesla analysis.
- also assumed the same message structure as in the tesla.
- how to find the magic bytes?
    - set all bytes in the magic table to 0 for the beginning.
    - run the program, it will show discrepancy between the calculated and the transmitted CRC.
    - the magic byte is the XOR between the calculated and the transmitted CRC.
    - fill the table of magic bytes accordingly.

Conclusions: ESP_21 uses the same CRC algorithm as described in the tesla analysis. Magic bytes are found.

## Open Todos

- [ ] improve documentation

## References

* Ref1: Log of the ESP_21 messsage: https://openinverter.org/forum/viewtopic.php?p=81924#p81924
* Ref2: How to calculate the CRC polynom out of observed data: https://knx-user-forum.de/forum/%C3%B6ffentlicher-bereich/knx-eib-forum/diy-do-it-yourself/34358-ekey-fingerscanner-per-rs485-auslesen-protokollanalyse?p=626281#post626281
* Ref3: CRC online calculator https://crccalc.com/?crc=00%2000%2001&method=CRC-8/AUTOSAR&datatype=hex&outtype=hex


