/*
    Proof of concept code for the cityZEN cash register machine

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <SPI.h>
#include <SD.h>
#include <Keypad.h>
#include <Time.h>
#include <Wire.h>
#include <DS1307RTC.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9340.h>

#define TFT_BL     3   // PE5
#define TFT_CS     10  // PB4
#define TFT_RST    12  // PB6
#define MICROSD_CS 11  // PB5
#define SD_CS      5   // PE3
#define ATMEGA_SS  53
#define KEYPD_1    25  // PA3
#define KEYPD_2    26  // PA4
#define KEYPD_3    28  // PA6
#define KEYPD_4    27  // PA5
#define KEYPD_5    30  // PC7
#define KEYPD_6    29  // PA7
#define KEYPD_7    32  // PC5
#define KEYPD_8    31  // PC6

#define KEYPD_ROWS 5
#define KEYPD_COLS 3
char s_kpdKeys[KEYPD_ROWS][KEYPD_COLS] = {
  {'^','%','$'},
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'c','0','e'}
};
byte s_kpdRowPins[KEYPD_ROWS] = { KEYPD_1, KEYPD_2, KEYPD_3, KEYPD_4, KEYPD_5 };
byte s_kpdColPins[KEYPD_COLS] = { KEYPD_6, KEYPD_7, KEYPD_8 };
Keypad kpd = Keypad(makeKeymap(s_kpdKeys), 
    s_kpdRowPins, s_kpdColPins,
    KEYPD_ROWS, KEYPD_COLS);

SDClass userSD;

Adafruit_ILI9340 tft = Adafruit_ILI9340(TFT_CS, TFT_BL, TFT_RST);

void setup()
{
  Serial.begin(9600);
  pinMode(ATMEGA_SS, OUTPUT); // host slave select disabled
  SD.begin(MICROSD_CS);
  userSD.begin(SD_CS);
  tft.begin();
}

typedef enum { MODE_CASHIER, MODE_REVIEW } Mode;
Mode mode = MODE_CASHIER;
Mode old_mode = MODE_REVIEW;

void loop()
{
  char input = kpd.getKey();
  
  if (input == '^') {
    mode = MODE_CASHIER;
  } else if (input == '$') {
    mode = MODE_REVIEW; 
  }

  switch (mode) {
    case MODE_CASHIER:
      if (old_mode != MODE_CASHIER) {
        renderCashierScreen();
      }
      cashierScreen(input);
      break;
    case MODE_REVIEW:
      if (old_mode != MODE_REVIEW) {
        renderReviewScreen();
      }
      reviewScreen(input);
      break;
    default:
      break;
  }
  
  old_mode = mode;
}

String cashierAmount = "";

void renderCashierScreen()
{
  File itemsListFile;
  
  cashierAmount = "";

  tft.fillScreen(ILI9340_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9340_WHITE);
  tft.println("Cashier");
  
  itemsListFile = userSD.open("items.txt", FILE_READ);
  if (itemsListFile) {
    while (itemsListFile.available()) {
     tft.println(itemsListFile.readStringUntil('\n'));
     // TODO: limit the list to the amount of lines and make it scrollable
    }
   
    itemsListFile.close(); 
  } else {
    tft.println("items.txt could not be opened");
  }
}

void cashierScreen(char input)
{
  // TODO: UI navigation

  if (input >= '0' && input <= '9') {
    // store the typed amounnt
    cashierAmount += input; 
  } else if (input == 'e') {
    storeTransaction();
  } else if (input == 'c') {
    cashierAmount = "";
  }
}

void print2digits(Stream & stream, int number)
{
  if (number >= 0 && number < 10) {
    stream.write('0');
  }
  stream.print(number);
}

void storeTransaction()
{
  tmElements_t tm;
  File transactionsFile = userSD.open("transactions.txt", O_WRITE | O_APPEND);
  
  RTC.read(tm);
  transactionsFile.print(tm.Year);
  print2digits(transactionsFile, tm.Month);
  print2digits(transactionsFile, tm.Day);
  transactionsFile.print('-');
  print2digits(transactionsFile, tm.Hour);
  print2digits(transactionsFile, tm.Minute);
  print2digits(transactionsFile, tm.Second);
  transactionsFile.print(" 0 "); //item code (hardcoded to 0 for demo)
  transactionsFile.print(cashierAmount);
  transactionsFile.print('\n');
  transactionsFile.close();
}

void renderReviewScreen()
{
  File transactionsFile;
  
  tft.fillScreen(ILI9340_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9340_WHITE);
  tft.println("Review");
  
  transactionsFile = userSD.open("transactions.txt", FILE_READ);
  if (transactionsFile) {
    while (transactionsFile.available()) {
     tft.println(transactionsFile.readStringUntil('\n'));
     // TODO: render something better than the raw database file
    }
   
    transactionsFile.close(); 
  } else {
    tft.println("No transactions yet");
  }
}

void reviewScreen(char input)
{
  // TODO: scroll
}

