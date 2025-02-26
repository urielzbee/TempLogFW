### Message structure
|Sync bytes|Command    |Data Length         |Data   |CRC    |
|----------|-----------|--------------------|-------|-------|
|2 bytes   |1 byte     |1 byte              |N bytes|2 bytes|
|0x7E, 0x7E|0x00 - 0xFF|0x00 - 0xFF(0 - 255)|N Bytes|2 bytes|

#### Sync bytes
Sync byte 1 -> 0x7E  
Sync byte 2 -> 0x7E  

#### Command
|ID       |Command         |Description              |
|---------|----------------|-------------------------|
|0x00     |Reserved        |NA                       |
|0x01     |FW_VER          |Firmware version         |
|0x02     |HW_VER          |Hardware version         |
|0x03     |SET_TIME        |Set RTC time             |
|0x04     |GET_TIME        |Get RTC Time             |
|0x05     |CPU_TEMP        |CPU Temperature          |
|0x06     |TEMP            |Temperature              |
|0x07     |SET_LOG_INTERVAL|Set Logging interval time|
|0x08     |GET_LOG_INTERVAL|Get Logging interval time|
|0x09     |STREAM_LOGS     |Stream log files stored  |
|0x09-0xFF|Reserved        |NA                       |