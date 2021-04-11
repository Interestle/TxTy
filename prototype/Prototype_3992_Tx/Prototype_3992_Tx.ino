/*
   This sketch is used for transmitting data. Hook up buttons to this to
   send to the other board. This sketch currently requires a board with
   multiple serial lines. I'm using my Mega 2560.
*/

#include <LiquidCrystal.h>

/* Button pins */
const uint8_t button_0 = 4;
const uint8_t button_1 = 5;
const uint8_t button_2 = 6;
const uint8_t button_3 = 7;

/* Simple debouncing on the buttons */
volatile uint8_t button_0_debounce;
volatile uint8_t button_1_debounce;
volatile uint8_t button_2_debounce;
volatile uint8_t button_3_debounce;

/* Parameters needed for sending a message. */
/* Address of the receiver and the transmitter. Chosen arbitrarily */
const String target_addr = String(10);
const String this_addr = String(20);

/* LCD Pins */
const uint8_t LCD_RS = 23;
const uint8_t LCD_E  = 22;
const uint8_t LCD_D4 = 17;
const uint8_t LCD_D5 = 16;
const uint8_t LCD_D6 = 15;
const uint8_t LCD_D7 = 14;
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);





/* Simple error state. Flash the LED on the board.*/
void error()
{
  lcd.clear();
  lcd.print("ERROR!");
  
  while (1)
  {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(250);
  }
}

/* Basic initialization of the RYLR896 */
void rylr896_init()
{
  Serial.println("Checking RYLR896:");
  Serial1.print("AT\r\n"); // One way to send a message

  String message = Serial1.readString();
  Serial.print(message);
  if (!message.equals("+OK\r\n"))
    error();

  // Force the sender to have an address of 20. I chose this value arbitrarily.
  Serial.println("Setting ADDRESS to: " + this_addr);
  Serial1.println("AT+ADDRESS=" + this_addr); // Another way to send a message without using \r\n (Built in)

  message = Serial1.readString();
  Serial.print(message);
  if (!message.equals("+OK\r\n"))
    error();
}

void setup()
{
  
  lcd.begin(16,2);
  lcd.print("Setting up...");
  
  Serial.begin(115200); // Serial Monitor
  Serial1.begin(115200); // UART on pins 18 and 19 of Mega.

  /* LED built on the board, good for diagnostics */
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(button_0, INPUT);
  pinMode(button_1, INPUT);
  pinMode(button_2, INPUT);
  pinMode(button_3, INPUT);

  button_0_debounce = 0xFF;
  button_1_debounce = 0xFF;
  button_2_debounce = 0xFF;
  button_3_debounce = 0xFF;

  /* Check that the module is plugged in correctly */
  rylr896_init();

  lcd.setCursor(4,1);
  lcd.print("Ready!");
}

void loop()
{
  button_0_debounce = (button_0_debounce << 1) | digitalRead(button_0);
  button_1_debounce = (button_1_debounce << 1) | digitalRead(button_1);
  button_2_debounce = (button_2_debounce << 1) | digitalRead(button_2);
  button_3_debounce = (button_3_debounce << 1) | digitalRead(button_3);

  if (button_0_debounce == 0x7F)
  {
    Serial.println("Button 0 has been pressed!");
    String message = "Button 0";
    message = "AT+SEND=" + target_addr + "," + String(message.length()) + "," + message;
    Serial1.println(message);
  }

  if (button_1_debounce == 0x7F)
  {
    Serial.println("Button 1 has been pressed!");
    String message = "Button 1";
    message = "AT+SEND=" + target_addr + "," + String(message.length()) + "," + message;
    Serial1.println(message);
  }

  if (button_2_debounce == 0x7F)
  {
    Serial.println("Button 2 has been pressed!");
    String message = "Button 2";
    message = "AT+SEND=" + target_addr + "," + String(message.length()) + "," + message;
    Serial1.println(message);
  }

  if (button_3_debounce == 0x7F)
  {
    Serial.println("Button 3 has been pressed!");
    String message = "Button 3";
    message = "AT+SEND=" + target_addr + "," + String(message.length()) + "," + message;
    Serial1.println(message);
  }

  // read from RYLR896, send to Console
  if (Serial1.available())
  {
    String message = Serial1.readString();
    Serial.print(message);
    
    if (message.startsWith("+RCV"))
    {
      // Find the commas that separate the data.
      int16_t first_comma = message.indexOf(',');
      int16_t second_comma = message.indexOf(',', first_comma + 1);
      int16_t last_comma = message.lastIndexOf(',');
      int16_t second_last_comma = message.lastIndexOf(',', last_comma - 1);

      // Reconstruct the data in a more usable format. This could probably be more optimized,
      // but this works well enough.
      uint16_t transmitter_address = message.substring(5, first_comma).toInt();
      uint16_t data_length = message.substring(first_comma + 1, second_comma).toInt();
      String data = message.substring(second_comma + 1, second_last_comma);
      int16_t RSSi = message.substring(second_last_comma + 1, last_comma).toInt();
      int16_t SNR = message.substring(last_comma + 1).toInt();

      
      Serial.println("Received message, updating LCD...");
      lcd.clear();
      lcd.print("Button pushes:");
      lcd.setCursor(0,1);
      lcd.print(data);
    }
  }


  // read from Console, send to RYLR896
  if (Serial.available())
    Serial1.write(Serial.read());

}
