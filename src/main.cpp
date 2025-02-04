#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_NeoPixel.h>

#define SHT30_I2C_ADDR 0x44
#define SEG7_I2C_ADDR 0x73

#define I2C1_SCL_PIN GPIO_NUM_0
#define I2C1_SDA_PIN GPIO_NUM_1
#define DHT11_PIN GPIO_NUM_4
#define LED_PIN GPIO_NUM_2

DHT dht11 = DHT(GPIO_NUM_4, DHT11);
Adafruit_NeoPixel pixels(1, LED_PIN, NEO_GRB + NEO_KHZ800);

void sht31_softreset()
{
  // ソフトリセット
  Wire.beginTransmission(SHT30_I2C_ADDR);
  Wire.write(0x30);
  Wire.write(0xA2);
  Wire.endTransmission();

  delay(300);

  // ステータスレジスタ消去
  Wire.beginTransmission(SHT30_I2C_ADDR);
  Wire.write(0x30);
  Wire.write(0x41);
  Wire.endTransmission();
}

void get_sht31()
{
  printf("GET sht31\r\n");

  uint8_t read_buf[6] = {0};
  bool success = false;
  uint16_t intHum = 0;

  for (int i = 0; i < 10; i++)
  {
    // 計測データ取得コマンドを送る
    Wire.beginTransmission(SHT30_I2C_ADDR);
    Wire.write(0x24);
    Wire.write(0x00);
    Wire.endTransmission();

    delay(100);

    Wire.requestFrom(SHT30_I2C_ADDR, 6);
    for (int i = 0; i < 6; i++)
    {
      read_buf[i] = Wire.read();
    }
    Wire.endTransmission();

    // if (read_buf[0] != 0 || read_buf[1] != 0 || read_buf[3] != 0 || read_buf[4] != 0)
    // humしか表示しないので、humが表示できなかった場合のみとする
    if (read_buf[3] != 0 || read_buf[4] != 0)
    {
      // 人間が読みやすい値に変換
      int16_t tempRaw, humRaw;
      float_t temp, hum;
      tempRaw = (read_buf[0] << 8) | read_buf[1];   // 上位バイトと下位バイトを結合
      temp = (float_t)(tempRaw) * 175 / 65535 - 45; // ℃に変換
      humRaw = (read_buf[3] << 8) | read_buf[4];    // 上位バイトと下位バイトを結合
      hum = (float_t)(humRaw) / 65535 * 100;        // 0.x%に変換

      printf("temperature: %f, humidity: %f\r\n", temp, hum);

      intHum = hum;

      if (intHum == 0)
      {
        printf("failed intHum: 0\r\n");
        continue;
      }

      // 値が取得できた
      success = true;
      printf("success\r\n");
      break;
    }

    // 取得できなかった
    // 3回トライする
    printf("fail, retry %d\r\n", i = 0);
    delay(3000);
  }

  if (!success)
  {
    printf("fail, reset\r\n");

    sht31_softreset();

    uint8_t msg[2] = {0x03, 0b01111001}; // Eを表示
    Wire.beginTransmission(SEG7_I2C_ADDR);
    Wire.write(msg, 2);
    Wire.endTransmission();

    delay(10000);

    return;
  }

  printf("send hun: %d\r\n", intHum);

  uint8_t msg[4] = {0};
  msg[0] = 0x10;           // 整数入力用のレジスタアドレス
  msg[1] = intHum >> 8;    // 上位8bit
  msg[2] = intHum % 0x100; // 下位8bit
  msg[3] = 0;              // 小数点の位置

  Wire.beginTransmission(SEG7_I2C_ADDR);
  Wire.write(msg, 4);
  Wire.endTransmission();
}

void init_dht11()
{
  dht11.begin();
}

void get_dht11()
{
  float_t temp = dht11.readTemperature();
  float_t hum = dht11.readHumidity();

  printf("temperature: %f, humidity: %f\r\n", temp, hum);
  int_fast32_t intHum = hum;

  printf("send hun: %d\r\n", intHum);

  uint8_t msg[4] = {0};
  msg[0] = 0x10;           // 整数入力用のレジスタアドレス
  msg[1] = intHum >> 8;    // 上位8bit
  msg[2] = intHum % 0x100; // 下位8bit
  msg[3] = 0;              // 小数点の位置

  Wire.beginTransmission(SEG7_I2C_ADDR);
  Wire.write(msg, 4);
  Wire.endTransmission();
}

void setup()
{
  pixels.begin();
  pixels.setPixelColor(0, Adafruit_NeoPixel::Color(0, 32, 0));
  pixels.show();

  Wire.begin(I2C1_SDA_PIN, I2C1_SCL_PIN);

  delay(300);

#ifdef USE_SHT31
  sht31_softreset();
#endif

#ifdef USE_DHT11
  init_dht11();
#endif

  delay(300);

  pixels.setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 32));
  pixels.show();
}

void loop()
{
#ifdef USE_SHT31
  get_sht31();
#endif

#ifdef USE_DHT11
  get_dht11();
#endif

  // delay(10000);
  // delay(5000);
  delay(1000);
}
