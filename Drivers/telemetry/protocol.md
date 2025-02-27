# Message structure
|Sync bytes|Command    |Data Length         |Data   |CRC    |
|----------|-----------|--------------------|-------|-------|
|2 bytes   |1 byte     |1 byte              |N bytes|2 bytes|
|0x7E, 0x7E|0x00 - 0xFF|0x00 - 0xFF(0 - 255)|N Bytes|2 bytes|

## Sync bytes
Sync byte 1 -> 0x7E  
Sync byte 2 -> 0x7E  

## Command
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
|0x0A-0xFF|Reserved        |NA                       |

## Data Length
Data length from 0 bytes to 255 bytes  

## Data
Command payload data  

## CRC
2 Byte CRC CRC-16-ANSI/CRC-16-IBM  

# Commands
## FW_VER
Firmware version  

**Request**
|Sync bytes|Command|Data Length |Data   |CRC    |
|----------|-------|------------|-------|-------|
|0x7E, 0x7E|0x01   |0x00        |NA     |2 bytes|

**Response**
|Sync bytes|Command|Data Length|Data                  |CRC    |
|----------|-------|-----------|----------------------|-------|
|0x7E, 0x7E|0x01   |0x03       |[Major] [Minor] [Rev] |2 bytes|

## HW_VER
Harware version  

**Request**
|Sync bytes|Command|Data Length|Data   |CRC    |
|----------|-------|-----------|-------|-------|
|0x7E, 0x7E|0x02   |0x00       |NA     |2 bytes|

**Response**
|Sync bytes|Command|Data Length|Data                  |CRC    |
|----------|-------|-----------|----------------------|-------|
|0x7E, 0x7E|0x02   |0x03       |[Major] [Minor] [Rev] |2 bytes|

## SET_TIME
Set RTC time    

**Request**
|Sync bytes|Command|Data Length|Data                                          |CRC    |
|----------|-------|-----------|----------------------------------------------|-------|
|0x7E, 0x7E|0x03   |0x06       |[Year] [Month] [Day] [Hour] [Minute] [Second] |2 bytes|

**Response**
|Sync bytes|Command|Data Length|Data         |CRC    |
|----------|-------|-----------|-------------|-------|
|0x7E, 0x7E|0x03   |0x00       |[OK]         |2 bytes|

## GET_TIME
Get RTC time    

**Request**
|Sync bytes|Command|Data Length|Data         |CRC    |
|----------|-------|-----------|-------------|-------|
|0x7E, 0x7E|0x04   |0x00       |NA           |2 bytes|

**Response**
|Sync bytes|Command|Data Length|Data                                          |CRC    |
|----------|-------|-----------|----------------------------------------------|-------|
|0x7E, 0x7E|0x04   |0x06       |[Year] [Month] [Day] [Hour] [Minute] [Second] |2 bytes|

## CPU_TEMP
CPU Temperature   

**Request**
|Sync bytes|Command|Data Length|Data         |CRC    |
|----------|-------|-----------|-------------|-------|
|0x7E, 0x7E|0x05   |0x00       |NA           |2 bytes|

**Response**
|Sync bytes|Command|Data Length|Data      |CRC    |
|----------|-------|-----------|----------|-------|
|0x7E, 0x7E|0x05   |0x01       |[CPU Temp]|2 bytes|

## TEMP
Temperature   

**Request**
|Sync bytes|Command|Data Length|Data         |CRC    |
|----------|-------|-----------|-------------|-------|
|0x7E, 0x7E|0x06   |0x00       |NA           |2 bytes|

**Response**
|Sync bytes|Command|Data Length|Data      |CRC    |
|----------|-------|-----------|----------|-------|
|0x7E, 0x7E|0x06   |0x01       |[Temp]    |2 bytes|

## SET_LOG_INTERVAL
Set Logging interval time   

**Request**
|Sync bytes|Command|Data Length|Data         |CRC    |
|----------|-------|-----------|-------------|-------|
|0x7E, 0x7E|0x07   |0x02       |[Seconds]    |2 bytes|

**Response**
|Sync bytes|Command|Data Length|Data      |CRC    |
|----------|-------|-----------|----------|-------|
|0x7E, 0x7E|0x07   |0x00       |[OK]      |2 bytes|

## GET_LOG_INTERVAL
Get Logging interval time   

**Request**
|Sync bytes|Command|Data Length|Data |CRC    |
|----------|-------|-----------|-----|-------|
|0x7E, 0x7E|0x08   |0x00       |NA   |2 bytes|

**Response**
|Sync bytes|Command|Data Length|Data      |CRC    |
|----------|-------|-----------|----------|-------|
|0x7E, 0x7E|0x08   |0x00       |[Seconds] |2 bytes|

## STREAM_LOGS
Stream log files stored   

**Request**
|Sync bytes|Command|Data Length|Data      |CRC    |
|----------|-------|-----------|----------|-------|
|0x7E, 0x7E|0x09   |0x00       |NA        |2 bytes|

**Response**
|Sync bytes|Command|Data Length|Data                                                        |CRC    |
|----------|-------|-----------|------------------------------------------------------------|-------|
|0x7E, 0x7E|0x09   |0x08       |[Year] [Month] [Day] [Hour] [Minute] [Second] [Type] [Value]|2 bytes|
