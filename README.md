# lcd2004tty

## Overview
lcd2004tty is a TTY emulator application.  Data is sent from a host and is displayed on the LCD2004 LCD screen in real-time.  The LCD screen will wrap both horzontally and scroll vertically, as needed.

![lcd2004tty hardware](./doc/lcd2004tty.jpg)

Click [here](https://youtu.be/GMxc_GZaq1w?si=6dG8IiisvAB9vgr) for video demo

## Hardware pinout
This project utilizes the following:
- STM32G474RE Nucleo board
- FTDI LC231X USB-to-UART bridge (PC host connection)
- LCD2004 LCD screen with I2C interface

| Component                |  STM32G4 GPIO  | Nucleo32   | Notes
| ------------------------ |  ------------- | ---------- | ------ 
| LC231X UART Tx           |  PC11          | CN7 pin 2  | UART3 on STM32G4
| LC231X UART Rx           |  PC10          | CN7 pin 1  | UART3 on STM32G4
| LC231X GND               |  N/A           | CN7 pin 8  | GND
| LCD2004 GND              |  N/A           | CN10 pin 9 | GND
| LCD2004 Vcc              |  N/A           | CN10 pin 8 | 5V from STM32G4
| LCD2004 SCL              |  PB8           | CN10 pin 3 | I2C1 on STM32G4
| LCD2004 SDA              |  PB9           | CN10 pin 5 | I2C1 on STM32G4

## LCD2004 LCD Screen
The LCD2004 LCD screen is a LCD screen driven by a Hitachi HD44780 Controller. It contains a TI PCF8574T 8-bit I/O expander for I2C communiation and control.  Despite this LCD being produced by numerous manufacturers, I haven't found good documentation for the I2C protocol.  The device driver used in this application was reversed engineered from various datasheets on the previously mentioned components and python and Arduino code written for this LCD.

### I2C I/O
Based on all the documentation, I2C communication is as follows:
- slave address is 0x27 (this may vary based on the actual I/O expander)
- 7-bit addressing
- 100 KHz clock speed
- There are no registers to write to (at least, not directly).  So the I2C stream starts with an Address frame followed by data frames.

While the HD44780 can operate in 8-bit mode, the PCF8574 doesn't have enough pins to support this.  Therefore, we must operate in 4-bit mode.  As result, sending one data byte must be done in 4 bytes.  Two bytes are for the high nibble and two bytes are for the low nibble.

| I2C byte |    D7–D4    | BL  |  E  | RW  | RS  |           Purpose            
| -------- | ----------- | --- | --- | --- | --- | ----------------------------- 
| 1        | high nibble | 1   | 1   | 0   | x   | E rises, data presented      
| 2        | high nibble | 1   | 0   | 0   | x   | E falls, high nibble latched 
| 3        | low nibble  | 1   | 1   | 0   | x   | E rises, data presented     
| 4        | low nibble  | 1   | 0   | 0   | x   | E falls, low nibble latched  

So to send a Clear Screen command, 0x01, the byte stream to send is, 0x0C 0x08 0x1C 0x18.

The E (enable) bit alternates between high and low where the nibble is latched on the falling edge.  The RS bit is 0 for command writes and 1 for data writes.

Through various sources, I've derived the following initialization sequence:
  | Step |                Purpose                 | I2C transaction (after START, 0x4E) | Wait after STOP 
  |------|----------------------------------------|-------------------------------------|-----------------
  | —    | Power-up                               | (none)                              | ≥ 40 ms         
  | 0    | Set pins to a known state              | 08                                  | —               
  | 1    | Nibble 0x3                             | 3C 38                               | ≥ 4.1 ms        
  | 2    | Nibble 0x3                             | 3C 38                               | ≥ 100 µs        
  | 3    | Nibble 0x3                             | 3C 38                               | ≥ 100 µs        
  | 4    | Nibble 0x2: enter 4-bit mode           | 2C 28                               | ≥ 100 µs        
  | 5    | 0x28 Function set (4-bit, 2-line, 5×8) | 2C 28 8C 88                         | —               
  |      | 0x08 Display off                       | 0C 08 8C 88                         | —               
  |      | 0x01 Clear                             | 0C 08 1C 18                         | ≥ 2 ms          
  | 6    | 0x06 Entry mode (increment, no shift)  | 0C 08 6C 68                         | —               
  |      | 0x0C Display on, cursor off            | 0C 08 CC C8                         | ≥ 40 µs         

The LCD display has a grid of 4 rows and 20 columns.  However, the grid addressing is as follows:
<table>
  <tr>
    <th rowspan="2">Row</th>
    <th colspan="20">Column</th>
  </tr>
  <tr>
    <th>0</th>
    <th>1</th>
    <th>2</th>
    <th>3</th>
    <th>4</th>
    <th>5</th>
    <th>6</th>
    <th>7</th>
    <th>8</th>
    <th>9</th>
    <th>10</th>
    <th>11</th>
    <th>12</th>
    <th>13</th>
    <th>14</th>
    <th>15</th>
    <th>16</th>
    <th>17</th>
    <th>18</th>
    <th>19</th>
  </tr>
  <tr>
    <td>0</td>
    <td>0x00</td>
    <td>0x01</td>
    <td>0x02</td>
    <td>0x03</td>
    <td>0x04</td>
    <td>0x05</td>
    <td>0x06</td>
    <td>0x07</td>
    <td>0x08</td>
    <td>0x09</td>
    <td>0x0A</td>
    <td>0x0B</td>
    <td>0x0C</td>
    <td>0x0D</td>
    <td>0x0E</td>
    <td>0x0F</td>
    <td>0x10</td>
    <td>0x11</td>
    <td>0x12</td>
    <td>0x13</td>
  </tr>
  <tr>
    <td>1</td>
    <td>0x40</td>
    <td>0x41</td>
    <td>0x42</td>
    <td>0x43</td>
    <td>0x44</td>
    <td>0x45</td>
    <td>0x46</td>
    <td>0x47</td>
    <td>0x48</td>
    <td>0x49</td>
    <td>0x4A</td>
    <td>0x4B</td>
    <td>0x4C</td>
    <td>0x4D</td>
    <td>0x4E</td>
    <td>0x4F</td>
    <td>0x50</td>
    <td>0x51</td>
    <td>0x52</td>
    <td>0x53</td>
  </tr>
  <tr>
    <td>2</td>
    <td>0x14</td>
    <td>0x15</td>
    <td>0x16</td>
    <td>0x17</td>
    <td>0x18</td>
    <td>0x19</td>
    <td>0x1A</td>
    <td>0x1B</td>
    <td>0x1C</td>
    <td>0x1D</td>
    <td>0x1E</td>
    <td>0x1F</td>
    <td>0x20</td>
    <td>0x21</td>
    <td>0x22</td>
    <td>0x23</td>
    <td>0x24</td>
    <td>0x25</td>
    <td>0x26</td>
    <td>0x27</td>
  </tr>
  <tr>
    <td>3</td>
    <td>0x54</td>
    <td>0x55</td>
    <td>0x56</td>
    <td>0x57</td>
    <td>0x58</td>
    <td>0x59</td>
    <td>0x5A</td>
    <td>0x5B</td>
    <td>0x5C</td>
    <td>0x5D</td>
    <td>0x5E</td>
    <td>0x5F</td>
    <td>0x60</td>
    <td>0x61</td>
    <td>0x62</td>
    <td>0x63</td>
    <td>0x64</td>
    <td>0x65</td>
    <td>0x66</td>
    <td>0x67</td>
  </tr>
</table>
Keep this addressing in mind when determing screen behavior such as wrapping text.  When data is written to a screen address, the address is automatically incremented or decremented according to initialization.

## References
- STM32G4 Nucleo 64 ([User Manual](https://www.st.com/resource/en/user_manual/um2505-stm32g4-nucleo64-boards-mb1367-stmicroelectronics.pdf)) ([schematic](https://www.st.com/resource/en/schematic_pack/mb1367-g474re-d01-schematic.pdf))
- [FTDI LC231X USB-to-UART bridge](https://ftdichip.com/products/lc231x/)
- [LCD2004 LCD](https://www.handsontec.com/pdf_files/GDM2004DFLYBW.pdf)
- [Hitachi HD44780 LCD controller](https://www.crystalfontz.com/controllers/uploaded/Hitachi%20HD44780%20v0.0.pdf)
- Texas Instruments PCF8574 8-bit I/O Expander  ([TI datasheet](https://www.ti.com/lit/ds/symlink/pcf8574.pdf))  ([HandsOnTech datasheet](https://handsontec.com/dataspecs/module/I2C_LCD_Interface.pdf))
