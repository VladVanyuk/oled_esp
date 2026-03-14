#ifndef REGS_OLED_H
#define REGS_OLED_H

#define CHARGEPUMP            0x8D
#define COLUMNADDR            0x21
#define COMSCANDEC            0xC8
#define COMSCANINC            0xC0
#define DISPLAYALLON          0xA5
#define DISPLAYALLON_RESUME   0xA4
#define DISPLAYOFF            0xAE
#define DISPLAYON             0xAF
#define EXTERNALVCC           0x01
#define INVERTDISPLAY         0xA7
#define MEMORYMODE            0x20
#define NORMALDISPLAY         0xA6
#define PAGEADDR              0x22
#define SEGREMAP              0xA0
#define SETCOMPINS            0xDA
#define SETCONTRAST           0x81
#define SETDISPLAYCLOCKDIV    0xD5
#define SETDISPLAYOFFSET      0xD3
#define SETHIGHCOLUMN         0x10
#define SETLOWCOLUMN          0x00
#define SETMULTIPLEX          0xA8
#define SETPRECHARGE          0xD9
#define SETSEGMENTREMAP       0xA1
#define SETSTARTLINE          0x40
#define SETVCOMDETECT         0xDB
#define SWITCHCAPVCC          0x02

// ssd1306_WriteCommand(DISPLAYOFF);
//   ssd1306_WriteCommand(SETDISPLAYCLOCKDIV);
//   ssd1306_WriteCommand(0xF0); // Increase speed of the display max ~96Hz
//   ssd1306_WriteCommand(SETMULTIPLEX);
//   ssd1306_WriteCommand(height() - 1);
//   ssd1306_WriteCommand(SETDISPLAYOFFSET);
//   ssd1306_WriteCommand(0x00);
//   ssd1306_WriteCommand(SETSTARTLINE);
//   ssd1306_WriteCommand(CHARGEPUMP);
//   ssd1306_WriteCommand(0x14);
//   ssd1306_WriteCommand(MEMORYMODE);
//   ssd1306_WriteCommand(0x00);

//   ssd1306_WriteCommand(SEGREMAP | 0x01);
//   ssd1306_WriteCommand(COMSCANDEC);

//   ssd1306_WriteCommand(SETCOMPINS);

//   if (display_geometry == GEOMETRY_128_64)
//   {
//     ssd1306_WriteCommand(0x12);
//   }
//   else if (display_geometry == GEOMETRY_128_32)
//   {
//     ssd1306_WriteCommand(0x02);
//   }

//   ssd1306_WriteCommand(SETCONTRAST);

//   if (display_geometry == GEOMETRY_128_64)
//   {
//     ssd1306_WriteCommand(0xCF);
//   }
//   else if (display_geometry == GEOMETRY_128_32)
//   {
//     ssd1306_WriteCommand(0x8F);
//   }

//   ssd1306_WriteCommand(SETPRECHARGE);
//   ssd1306_WriteCommand(0xF1);
//   ssd1306_WriteCommand(SETVCOMDETECT); //0xDB, (additionally needed to lower the contrast)
//   ssd1306_WriteCommand(0x40);          //0x40 default, to lower the contrast, put 0
//   ssd1306_WriteCommand(DISPLAYALLON_RESUME);
//   ssd1306_WriteCommand(NORMALDISPLAY);
//   ssd1306_WriteCommand(0x2e);            // stop scroll
//   ssd1306_WriteCommand(DISPLAYON);

#endif