#include <stdint.h>
#include <WiFi.h>

#define STASSID "SSID HERE"
#define STAPSK "PASSWORD HERE"



// This program is used to work with STT (Serial over Tcp Tracked). The tracked comes in the form of a second port
// That sends 1 if a threshold is reached. This is used to see if all the data sent to the pico has been sent over 
// the uart.


#define UART_BAUD_RATE 115200
#define TX_THRESHOLD 32             // I.e if there are more then 32 bytes waiting, tell the client the tx threshold is reched
                                    // Signaled by send a 0x01, cleared by sending 0x00

#define STATUS_PORT 4243
#define DATA_PORT 4242

WiFiServer status_server(STATUS_PORT);  // Used to signal if to much data is recived
WiFiServer data_server(DATA_PORT);      // Used to send / recvie data for the uart

// The naming scheme is from the perspective of the UART on the pi pcio
#define RX_BUFFER_SIZE 128 * 1024

uint8_t rx_buffer[RX_BUFFER_SIZE];

int rx_buffer_write_ptr = 0;
int rx_buffer_read_ptr = 0;

uint8_t tx_status = 0;

void setup()      // Core 0
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.setHostname("AWG_UART");
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(100);
  }
  Serial.print("\nConnected! IP address: ");
  Serial.println(WiFi.localIP());

  status_server.begin();
  data_server.begin();
}

void setup1()   // Core 1
{
  Serial1.setTX(0);
  Serial1.setRX(1);
  Serial1.begin(UART_BAUD_RATE);
}

void loop()     // Core 0
{ 
  WiFiClient data_client = data_server.accept();

  if (!data_client) 
  {
    delay(10);
    return;
  }

  delay(200);      // Give some time for the status port to connect (if used)
  WiFiClient status_client = status_server.accept();

  while (data_client.connected())
  {
    int waiting = data_client.available();
    if (waiting)  // Send client data of serial
    {
      Serial1.write(data_client.read());

      uint8_t satus = waiting > TX_THRESHOLD ? 1 : 0;

      digitalWriteFast(LED_BUILTIN, tx_status != satus);

      if (tx_status != satus && status_client && status_client.connected())
      {
        status_client.write(satus);
        status_client.flush();

        tx_status = satus;
      }
    }
    

    if (rx_buffer_read_ptr == rx_buffer_write_ptr) 
      continue;   // We skip the rest of the code if all recived data has been sent over wifi

    if (rx_buffer_read_ptr > rx_buffer_write_ptr)
    {
      int length = RX_BUFFER_SIZE - rx_buffer_read_ptr;
      data_client.write(rx_buffer + rx_buffer_read_ptr, length);

      rx_buffer_read_ptr = 0;
    }
    else
    {
      int length = rx_buffer_write_ptr - rx_buffer_read_ptr;
      data_client.write(rx_buffer + rx_buffer_read_ptr, length);

      rx_buffer_read_ptr += length;
    }

    data_client.flush();
  }
}


void loop1()    // Core 1
{
  // First we handle UART data recive

  if (Serial1.available())
  {
    rx_buffer[rx_buffer_write_ptr++] = Serial1.read();

    if (rx_buffer_write_ptr == RX_BUFFER_SIZE)
      rx_buffer_write_ptr = 0;
  }
}
