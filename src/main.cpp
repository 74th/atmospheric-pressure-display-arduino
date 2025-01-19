#include <Arduino.h>
#include <Wire.h>

#define SHT30_I2C_ADDR 0x44
#define SEG7_I2C_ADDR 0x73

#define I2C1_SCL_PIN GPIO_NUM_0
#define I2C1_SDA_PIN GPIO_NUM_1

void setup()
{
  Wire.begin(I2C1_SDA_PIN, I2C1_SCL_PIN);

  delay(300);

  // ソフトリセット
  Wire.beginTransmission(SHT30_I2C_ADDR);
  Wire.write(0x30);
  Wire.write(0xA2);
  Wire.endTransmission();

  delay(300);
}

void loop()
{
  printf("GET sht31\r\n");

  // 計測データ取得コマンドを送る
  Wire.beginTransmission(SHT30_I2C_ADDR);
  Wire.write(0x24);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(SHT30_I2C_ADDR, 6);
  uint8_t read_buf[6];
  for (int i = 0; i < 6; i++)
  {
    read_buf[i] = Wire.read();
  }
  Wire.endTransmission();

  // 人間が読みやすい値に変換
  int16_t tempRaw, humRaw;
  float_t temp, hum;
  tempRaw = (read_buf[0] << 8) | read_buf[1];   // 上位バイトと下位バイトを結合
  temp = (float_t)(tempRaw) * 175 / 65535 - 45; // ℃に変換
  humRaw = (read_buf[3] << 8) | read_buf[4];    // 上位バイトと下位バイトを結合
  hum = (float_t)(humRaw) / 65535 * 100;        // 0.x%に変換

  printf("temperature: %f, humidity: %f\r\n", temp, hum);

  uint16_t intHum = hum * 10;

  printf("send hun: %d\r\n", intHum);

  uint8_t msg[4] = {0};
  msg[0] = 0x10;           // 整数入力用のレジスタアドレス
  msg[1] = intHum >> 8;    // 上位8bit
  msg[2] = intHum % 0x100; // 下位8bit
  msg[3] = 1;              // 小数点の位置

  Wire.beginTransmission(SEG7_I2C_ADDR);
  Wire.write(msg, 4);
  Wire.endTransmission();

  delay(10000);
}
