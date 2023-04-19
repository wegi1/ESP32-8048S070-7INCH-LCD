//
// Created by root on 2022/11/13.
//

#include "HAL.h"

void sdcard_init()
{
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI,SD_CS);
    SPI.setFrequency(1000000);
    SD.begin(SD_CS);

    // pinMode(SD_CS, OUTPUT);
    // digitalWrite(SD_CS, HIGH);

    // spi = SPIClass(FSPI);
    // spi.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SD_CS);
    // if (!SD.begin(SD_CS, spi, 10000000))
    // {
    //     Serial.println("Card Mount Failed");
    //     return;
    // }

    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println("SDHC");
    }
    else
    {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
}
