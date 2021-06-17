// Hardware antenna for communication
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

// Software cryptography
#include <Crypto.h>

const byte length_msg = 32; // Max payload on nrf24l01
const byte change_array = 96; // char 32 to 127 see ASCII table

Crypto cryp(change_array, length_msg);

byte msg[length_msg];
byte keyC[96] = {0xA7, 0xF1, 0xD9, 0x2A, 0x82, 0xC8, 0xD8, 0xFE, 0x43, 0x4D, 0x98, 0x55, 0x8C, 0xE2, 0xB3, 0x47, 0x17, 0x11, 0x54, 0x2F, 0x2D, 0x5, 0x58, 0xF5, 0x6B, 0xD6, 0x88, 0x7, 0x99, 0x92, 0x48, 0x33, 0x62, 0x41, 0xF3, 0xD, 0x23, 0xE5, 0x5F, 0x30, 0xD1, 0xED, 0x61, 0xC, 0x4B, 0x2, 0x35, 0x39, 0x81, 0x84, 0xB8, 0x14, 0xA2, 0x9C, 0xB4, 0x5A, 0x67, 0xCA, 0xE9, 0xC5, 0xB0, 0xC4, 0x15, 0x8A, 0x9B, 0xF6, 0xF7, 0xE8, 0xA1, 0xD3, 0xA5, 0xD5, 0xE4, 0x5B, 0xFA, 0x6C, 0xC3, 0x51, 0x20, 0xAE, 0xE1, 0x6, 0x6D, 0xFF, 0x34, 0x1E, 0x19, 0xFD, 0x36, 0x50, 0xB7, 0x8F, 0x3A, 0xF, 0xC0, 0x2C};
byte keyM[32] = {0x4, 0x15, 0x1A, 0x8, 0xB, 0x10, 0x9, 0x12, 0x1E, 0x16, 0x2, 0xC, 0xF, 0x1D, 0x1F, 0x7, 0xE, 0x1, 0x0, 0x18, 0x17, 0x14, 0x1B, 0x6, 0x1C, 0xA, 0x13, 0x11, 0x5, 0x19, 0xD, 0x3};

byte state = 0;
bool active = false; // False when the message is finished
byte data = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing antenna");
  
  // Init crypto library
  cryp.init(analogRead(A0));
  
  
  Mirf.cePin = 9;
  Mirf.csnPin = 10;
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();

  Mirf.channel = 1; // 0 to 127
  Mirf.payload = length_msg; // max 32
  Mirf.config(); // Save config

  Mirf.setTADDR((byte *) "nrf02"); // Set TX address (5 bytes)
  Mirf.setRADDR((byte *) "nrf01"); // Set RX address (5 bytes)

  Serial.print("  CE pin: "); Serial.println(Mirf.cePin);
  Serial.print("  CSN pin: "); Serial.println(Mirf.csnPin);
  Serial.print("  Channel: "); Serial.println(Mirf.channel);
  Serial.println("End initializing antenna");
}

void loop() {
  if (Mirf.dataReady()) {
    Mirf.getData(msg); // Receive packet
    if (msg[length_msg - 1] != '\0') { // If crypt so decode
      decode(msg, keyC, keyM);
      for (byte i = 0; i < length_msg - 1; i++) {
        msg[i] += 31;
      }
      msg[length_msg - 1] = '\0'; // Put the last byte into a null char
    }
    Serial.print("Him - ");
    Serial.println((char*) msg); // Display msg
  }

  if (Serial.available()) {
    if (!active) data = Serial.read();
    else if (data != '$') data = Serial.read();
    active = true;
    state = 1;

    // If we want to send a none crypt message write a '$'
    // at the start of message (will not appear in message)
    if (data == '$') state = 0;

    // Store message in array
    while (Serial.available() && state < (length_msg - 1)) {
      msg[state] = Serial.read();
      state++;
    }
    // Fill the rest with space (I had some bugs without)
    for (; state < length_msg - 1; state++) {
      msg[state] = ' ';
    }
    
    Serial.print("You - ");
    msg[state] = '\0';
    if (data != '$') msg[0] = data;
    Serial.println((char*)msg); // Display your message

    // If we add char '$' at the start the message will be not crypt
    if (data != '$') {
      for (byte i = 0; i < length_msg; i++) {
        if (msg[i] >= 32 && msg[i] <= 127) msg[i] -= 31;
        else msg[i] = 0;
      }
      // Encode messsage
      encode(msg, keyC, keyM);
    }

    Mirf.send(msg); // Send message
    while (Mirf.isSending()); // Wait
  } else {
    // When there is no message set to false
    // When we will write a new message it will detect it
    active = false;
  }

  delay(10); // Slow down the code
}

void encode(byte* _msg, byte* _key1, byte* _key2) {
  cryp.encodeC(_msg, _key1);
  cryp.encodeB(_msg);
  cryp.encodeM(_msg, _key2);
  cryp.encodeB(_msg);
}

void decode(byte* _msg, byte* _key1, byte* _key2) {
  cryp.decodeB(_msg);
  cryp.decodeM(_msg, _key2);
  cryp.decodeB(_msg);
  cryp.decodeC(_msg, _key1);
}

void copy_array(byte _size, byte* first, byte* second) {
  for (byte t = 0; t < _size; t++) {
    second[t] = first[t];
  }
}
