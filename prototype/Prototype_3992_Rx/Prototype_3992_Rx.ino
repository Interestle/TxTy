/*
   This sketch is used for receiving data. Hook up LEDs to this to receive 
   from the other board. I'm using my UNO for this.
*/

/* LED pins */
const uint8_t LED_0 = 4;
const uint8_t LED_1 = 5;
const uint8_t LED_2 = 6;
const uint8_t LED_3 = 7;

/* Button pins */
const uint8_t button = 12;

/* Simple debouncing on the buttons */
volatile uint8_t button_debounce;

/* Parameters needed for sending a message. */
/* Address of the receiver and the transmitter. Chosen arbitrarily */
const String sender_addr = String(20);
const String this_addr = String(10);

/* Count how many times the user pressed the receiver button*/
uint16_t button_count;

/* Simple error state. Flash the LED on the board.*/
void error()
{
  while (1)
  {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(250);
  }
}

/* Basic initialization of the RYLR896 */
void rylr896_init()
{
  Serial.print("AT\r\n"); // One way to send a message

  String message = Serial.readString();
  if (!message.equals("+OK\r\n"))
    error();

  digitalWrite(LED_0, HIGH);

  Serial.println("AT+ADDRESS=" + this_addr); // Another way to send a message without using \r\n (Built in)

  message = Serial.readString();
  if (!message.equals("+OK\r\n"))
    error();

  digitalWrite(LED_1, HIGH);
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(LED_0, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);

  digitalWrite(LED_0, LOW);
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_3, LOW);

  pinMode(button, INPUT);
  button_count = 0;
  button_debounce = 0xFF;

  Serial.begin(115200);
  rylr896_init();

}

void loop()
{
  button_debounce = (button_debounce << 1) | digitalRead(button);

  if(button_debounce == 0x7F)
  {
    String message = String(++button_count);
    message = "AT+SEND=" + sender_addr + "," + String(message.length()) + "," + message;
    Serial.println(message);    
  }

  // Check received messages
  if (Serial.available())
  {
    String message = Serial.readString();

    if (message.startsWith("+ERR"))
      error();

    if (message.startsWith("+RCV"))
    {
      // Find the commas that separate the data.
      int16_t first_comma = message.indexOf(',');
      int16_t second_comma = message.indexOf(',', first_comma + 1);
      int16_t last_comma = message.lastIndexOf(',');
      int16_t second_last_comma = message.lastIndexOf(',', last_comma - 1);

      // Reconstruct the data in a more usable format. This could probably be more optimized for Streams,
      // but this works well enough.
      uint16_t transmitter_address = message.substring(5, first_comma).toInt();
      uint16_t data_length = message.substring(first_comma + 1, second_comma).toInt();
      String data = message.substring(second_comma + 1, second_last_comma);
      int16_t RSSi = message.substring(second_last_comma + 1, last_comma).toInt();
      int16_t SNR = message.substring(last_comma + 1).toInt();

      if (data.equals("Button 0"))
      {
        digitalWrite(LED_0, !digitalRead(LED_0));
      }
      if (data.equals("Button 1"))
      {
        digitalWrite(LED_1, !digitalRead(LED_1));
      }
      if (data.equals("Button 2"))
      {
        digitalWrite(LED_2, !digitalRead(LED_2));
      }
      if (data.equals("Button 3"))
      {
        digitalWrite(LED_3, !digitalRead(LED_3));
      }
    }
  }
}
