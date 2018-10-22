# __PeaCalc__
## Overview
PeaCalc is a tiny interpreter for mathematical expressions.  
It runs completely in text-mode and allows full use of the cursor-keys and clipboard.  
Mathematical terms can be edited in the last line.  
With _TAB_ and _SHIFT_ + _TAB_, the previous calculations can be recalled. 

There is a small set of special commands, which can be executed:

* _info_ displays the current version and copyright-notice.  
* _license_ opens the license-file as as published by the Free Software Foundation.  
* _help_ opens this file in a browser.
* _clear_ clears the text-buffer.
* _min_ minimizes the window.
* _exit_ closes PeaCalc.
  
___Note:___

* _No equality-sign is required at the end of the expression!_  
* _The expression in the last line will be calculated, when pressing ENTER!_  

## Supported Operations
PeaCalc currently supports the following arithmetic functions (by priority):  

| Operation    | Description                                                       
|--------------|---------------------------------------------------------------------
|   a + b      | Addition                                                          
|   a - b      | Subtraction                                                       
|   a * b      | Multiplication                                                    
|   a / b      | Division                                                          
|   n √ a      | ^n^th root of a. When n is omitted, 2 is considered as base.      
|              | _Note: Use the back-slash key \\!_                                     
|   a \^ b     | a Power b                                                         
|  n log( b )  | Logarithm of b to n. When n is omitted, e is considered (≈ 2.718).

The following trigonometric functions are supported:

| Operation    | Description                                                       
|--------------|---------------------------------------------------------------------
|    sin(a)    | Sine of a                                                          
|    cos(a)    | Cosine of a                                                        
|    tan(a)    | Tangent of a                                                       
|   asin(a)    | Arc-sine of a                                                      
|   acos(a)    | Arc-cosine of a                                                    
|   atan(a)    | Arc-tangent                                                        

___Note:___

* _The arguments of the trigonometric functions are always defined in radian measure, thus a full circle is defined by an argument of 2 * pi!_

The following bitwise boolean functions are supported (by priority):

| Operation  | Description                                                       
|------------|---------------------------------------------------------------------
|  a & b     | a _AND_ b
|  a \| b    | a _OR_ b
|   ~ a      | _NOT_ a

___Note:___

* _As described in detail below, the parser in PeaCalc is based on double values. Therefore, any binary argument is limited to 2^52^!_
* _Since the \^ sign was defined for a power, support for an XOR could not be given._

## Supported number-formats
PeaCalc internally works only with double-numbers.  
However, user-input can be specified with three additional formats:  

|  Operation  | Description                                                       
|-------------|---------------------------------------------------------------------
|  0x\<NUM\>  | Treat the number as hexadecimal input                              
|  0b\<NUM\>  | Treat the number as binary input                                   
|  \<NUM\>o   | Treat the angle in degree.                                         
|             | It will be multiplied by 180/pi prior to processing.               

The output can be converted by wrapping the expression with a modifier:

|  Operation  | Description                                                       
|-------------|---------------------------------------------------------------------
|   hex(a)    | Calculates a, and then converts its output to hex.
|   bin(a)    | Calculates a, and then converts its output to hex.

___Note:___  
_Since the calculator-engine is completely based on the double data-type, there are three restrictions to be considered:_  

* _The greatest value, which can savely be handled is 10^22^._
* _Any number greater than 2^52^ (=4503599627370496 or approximately 4.5E15) will be chopped in precision and treated as double._
* _The maximum precision, is 16 leading digits._

## Examples

Simple tests of the root-function:

    3 √ 27  
    = 3  
    √2 * √2  
    = 2.00000  
    > |  

Calculate the third side of a right-angled triangle:

    √ (3^2 + 4^2)  
    = 5  
    > |  

Calculate the sine-value at 30°:

    sin(30 *pi/180)
    = 0.50000
    sin(30o)
    = 0.50000
    > |  

Calculate a the voltage at a capacitor as step response from 0 to 20V with a tau of 50 ms after a time of 100 ms:

    20 + (0 - 20) * e^(-100E-3 / 50E-3)  
    = 17.29329  
    > |  

Calculate how many bits are required for an integer of 262144:

    2 log(262144)  
    = 18  
    > |

Calculate with hexadecimal values:

    hex(0xFF * 3)  
    = 0x2FD  
    hex(0x2FD / 3)  
    = 0xFF  
    > |

Calculating with binary values:

    0b11000011  
    = 195  
    hex(0b1010)  
    = 0xA  
    hex(0b1010 | 0b0011)  
    = 0xB  
    bin(0b1010 | 0b0011)  
    = 0b 1011  
    bin(0b101 & ~0b0011)  
    = 0b 0100  
    > |  

## Technical Details
When PeaCalc is closed, it stores its position and a couple other things - including the current window's text - in an initialization file. This file is placed in one of two locations:

* When at start-up of PeaCalc a _PeaCalc.ini_ file is found in its application-directory, or the drive it is run from is removable, than the settings are stored right there.
* When both of these are not the case, PeaCalc places this file in %APPDATA%\\PeaCalc.  
  In windows 10, this expands to something like _C:\\Users\\\<user-name>\\AppData\\Roaming\\PeaCalc_.  
  
This split up is done to enable both a local installation and an installation on a portable USB drive.  
Since there's no way yet of changing these settings out of the application itself, there's at least the possibility to customize PeaCalc via these files. They contain the following settings:

* _Top_, _Left_, _Height_ and _Width_: This is simply the location of the window at shut-down. It is stored to help a user find it, when it is started the next time.
* _Opacity_: Defines, how visible the window is, when it is not active. The value ranges from 0 (invisible) to 255 (solid).
   Note, that in any case, it always stays on top.
* _FontSize_: Sets the size of the font in a range between 1 and 30.
* _Precision_: Defines the precision in digits of the numeric output of float-values. The value ranges from 1 to 16.
* _Lines_: Defines, how many lines are stored in the history.

## Developer Notes
This project is can be built with Mingw-w64 or Visual Studio. Some compiler switches were added, to ensure support for both environments.  
However, the batch-file, which ships with the source-code, relies on MinGw.  
Note, that the 32-bit version of MinGW caused some trouble, so I decided against using it.  
The help-html file is created at build-time from the read-me file using Pandoc.

## License
Copyright (C) 2018 J.D. Schlachter <osw.schlachter@mailbox.org>  
  
This program is free software: you can redistribute it and/or modify  
it under the terms of the GNU General Public License as published by  
the Free Software Foundation, either version 3 of the License, or  
(at your option) any later version.  
  
This program is distributed in the hope that it will be useful,  
but WITHOUT ANY WARRANTY; without even the implied warranty of  
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
GNU General Public License for more details.  
  
You should have received a copy of the GNU General Public License  
along with this program.  If not, see <https://www.gnu.org/licenses/>.

