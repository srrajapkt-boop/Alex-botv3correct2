#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <driver/i2s.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "BluetoothA2DPSource.h"

#define W_WIDTH 128
#define W_HEIGHT 64
Adafruit_SSD1306 display(W_WIDTH, W_HEIGHT, &Wire, -1);

WiFiUDP udp;
BluetoothA2DPSource a2dp;

#define I2S_WS  15
#define I2S_SD  32
#define I2S_SCK 14

Servo sL, sR;
int16_t buf[512];

int32_t get_audio_data(Frame *ch, int32_t len) {
    size_t r = 0;
    i2s_read(I2S_NUM_0, &buf, len * 2, &r, 0);
    int32_t smp = r / 2;
    for (int32_t i = 0; i < smp; i++) {
        ch[i].channel1 = buf[i];
        ch[i].channel2 = buf[i];
    }
    return smp;
}

void drawEx(char ex) {
    display.clearDisplay();
    if (ex == 'H') { // HAPPY
        display.fillRoundRect(30, 20, 20, 30, 10, 1);
        display.fillRoundRect(78, 20, 20, 30, 10, 1);
    } else if (ex == 'S') { // SAD
        display.fillTriangle(30, 40, 50, 20, 30, 20, 1);
        display.fillTriangle(98, 40, 78, 20, 98, 20, 1);
    } else { // NEUTRAL
        display.fillCircle(40, 32, 12, 1);
        display.fillCircle(88, 32, 12, 1);
    }
    display.display();
}

void setup() {
    Wire.begin(21, 22);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    drawEx('N');

    WiFi.begin("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD");
    while (WiFi.status() != WL_CONNECTED) { delay(100); }
    udp.begin(4210);

    i2s_config_t mc = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 1,
        .dma_buf_count = 4,
        .dma_buf_len = 512
    };
    i2s_pin_config_t mp = {.bck_io_num = I2S_SCK, .ws_io_num = I2S_WS, .data_out_num = -1, .data_in_num = I2S_SD};
    i2s_driver_install(I2S_NUM_0, &mc, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &mp);

    a2dp.start("YOUR_BT_SPEAKER_NAME", get_audio_data);

    sL.attach(13); sL.write(0);
    sR.attach(12); sR.write(0);
}

void loop() {
    char pkt[32];
    int sz = udp.parsePacket();
    if (sz > 0) {
        int len = udp.read(pkt, 31);
        if (len > 0) {
            pkt[len] = 0;
            if (strncmp(pkt, "EXP:", 4) == 0) drawEx(pkt[4]);
            else if (strncmp(pkt, "SRVL:", 5) == 0) sL.write(atoi(pkt + 5));
            else if (strncmp(pkt, "SRVR:", 5) == 0) sR.write(atoi(pkt + 5));
        }
    }
    delay(10);
}

