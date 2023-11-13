#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <chrono>
#include <thread>
#include<unistd.h>

#include "usb_usart.h"

extern "C" {  // jpeglib.h
#include <stdio.h>
#include <setjmp.h>
#include <unistd.h>
#include <stdlib.h>
#include <jconfig.h>
#include <jpeglib.h>
}

class Mat {
public:
    size_t rows;      /* Строки */
    size_t cols;      /* Колонки */

public:
    uint8_t *matrix;   /* Основной массив - он же матрица, т.к. двумерный */

#if 0
private:
    uint16_t rgb565(const uint16_t &R, const uint8_t &G, const uint8_t &B) {
        uint16_t total_color = (uint16_t)(((R & 0b11111000) << 8) | ((G & 0b11111100) << 3) | (B >> 3));
        //((B << 11) & 0xF800) | ((G << 5) & 0x07E0) | (R & 0x001F);
	    total_color = ((total_color << 8) | (total_color >> 8));
        return total_color;
    }
#endif
public:
    Mat(const std::string &filename, const size_t &X, const size_t &Y) {
        struct jpeg_decompress_struct d1;
        struct jpeg_error_mgr m1;
        d1.err = jpeg_std_error(&m1);
        jpeg_create_decompress(&d1);
        FILE *f = fopen(filename.c_str(),"rb");
        jpeg_stdio_src(&d1, f);
        jpeg_read_header(&d1, TRUE);

        rows = Y;
        cols = X;
        const size_t in_rows = d1.image_height;
        const size_t in_cols = d1.image_width;
        matrix = new uint8_t[rows * cols * 3]{};
        //const size_t row_stride = d1.output_width * d1.output_components;
        
        int buffer_height = 1;
        size_t counter = 0;
        jpeg_start_decompress(&d1);
        JSAMPARRAY buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * buffer_height);
        buffer[0] = (JSAMPROW)malloc(sizeof(JSAMPLE) * d1.output_width * d1.output_components);
        const size_t row_stride = d1.output_width * d1.output_components;
        //img.resize(d1.image_height, d1.image_width, 3);
        while (d1.output_scanline < d1.output_height) {
            jpeg_read_scanlines(&d1, buffer, 1);
            std::memcpy(this->matrix + counter, buffer[0], row_stride);
            counter += row_stride;
        }
#if 0
        uint8_t *pBuf = new uint8_t[rows * cols * d1.num_components]{};
        size_t i = 0;
        int buffer_height = 1;
        size_t counter = 0;
        JSAMPARRAY buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * buffer_height);
        buffer[0] = (JSAMPROW)malloc(sizeof(JSAMPLE) * d1.output_width * d1.output_components);
        while (d1.output_scanline < d1.output_height) {
            // Получить экранную строку
            //jpeg_read_scanlines(&d1, buffer, 1);

            i += jpeg_read_scanlines(&d1, (JSAMPARRAY)&(pBuf), 1);
            for (size_t j = 0; j < cols; ++j) {
               // matrix[j + (i - 1) * cols] = (uint8_t)(pBuf[j * d1.num_components]);

            //std::memcpy(&matrix[counter], &buffer[0], row_stride);
            //counter += row_stride;

                //if (j < in_cols) {
                    matrix[j * 3 + (i - 1) * cols + 2] = (uint8_t)(pBuf[j * 3 + 2]);
                    matrix[j * 3 + (i - 1) * cols + 1] = (uint8_t)(pBuf[j * 3 + 1]);
                    matrix[j * 3 + (i - 1) * cols + 0] = (uint8_t)(pBuf[j * 3 + 0]);
                //} else {
                  //  matrix[j + (i - 1) * cols + 2] = (uint8_t)(0x00);
                   // matrix[j + (i - 1) * cols + 1] = (uint8_t)(0x00);
                   // matrix[j + (i - 1) * cols + 0] = (uint8_t)(0x00);
                }
           // }
        }
        //for (; i < rows; ++i) {
          //  for (size_t j = 0; j < cols; ++j) {
            //    matrix[j + i * cols + 2] = (uint8_t)(0x00);
              //  matrix[j + i * cols + 1] = (uint8_t)(0x00);
             //   matrix[j + i * cols + 0] = (uint8_t)(0x00);
           // }
       // }
#endif
        jpeg_finish_decompress(&d1);
        jpeg_destroy_decompress(&d1);
        fclose(f);
        //delete []pBuf;
    }

    ~Mat() {
        delete []matrix;
    }

    void save(const std::string &savename) {
        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
        JSAMPROW row_pointer[1];
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);
        FILE *outfile = fopen(savename.c_str(),"wb");
        if (outfile == NULL) {
            std::cout << "В класс JpegEncoder методу read не удалось открыть файл: " << savename << std::endl;
            return;
        }

        jpeg_stdio_dest(&cinfo, outfile);
        cinfo.image_width = cols;
        cinfo.image_height = rows;
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;
        JSAMPLE* image_buffer = new JSAMPLE[cinfo.image_width * cinfo.image_height * cinfo.input_components]();
        for (size_t i = 0; i < cinfo.image_width * cinfo.image_height * cinfo.input_components; ++i) {
            image_buffer[i] = this->matrix[i];
        }

        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, 100, true);
        jpeg_start_compress(&cinfo, true);

        while (cinfo.next_scanline < cinfo.image_height) {
            row_pointer[0] = (JSAMPROW)&image_buffer[cinfo.next_scanline * cinfo.image_width * cinfo.input_components];
            (void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }

        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);
        fclose(outfile);
    } 
//------------------------------------------------------------------------------------------------------------
    // Basic operations
    uint8_t& operator()(const size_t &i, const size_t &j) const {
        return matrix[j + i * cols]; }

    uint8_t& operator()(const size_t &i, const size_t &j) { return (uint8_t&)matrix[j + i * cols]; }

    uint8_t& operator[](const size_t &k) { return matrix[k]; }
    uint8_t& operator[](const size_t &k) const { return matrix[k]; }
};

#define MY_BDR 9600//4803
#define SIZE 10

void randa8 (uint8_t  *m, const size_t N);
void randa16(uint16_t *m, const size_t N);
void randa32(uint32_t *m, const size_t N);

template<typename T>
void printa(T *m, const size_t &N) {
    for (size_t i = 0; i < N; i++)
        printf("%3d", ((int)m[i]));
	std::cout << std::endl;
}


class ImageDisplay {
    const unsigned int microsecond = 1000000;

    uusart ch;

    static const size_t _size = 1024;

    uint8_t *ptr_tx;
    uint8_t *ptr_rx;

    const uint8_t _cmd_nop = 0x00;
    const uint8_t _cmd_display_info = 0x01;
    const uint8_t _cmd_display_range = 0x02;
    const uint8_t _cmd_print_h_line = 0x03;

    uint16_t X;
    uint16_t Y;

public:
    ImageDisplay() {
        ptr_tx = new uint8_t[_size];
        ptr_rx = new uint8_t[_size];
        ch.setBaudRate(MY_BDR);
        ch.setCharacteristics();
    }
    
    ~ImageDisplay() {
        delete []ptr_tx;
        delete []ptr_rx;

    }
    
    void wait5s() {
        usleep(3 * microsecond);//sleeps for 3 second
    }

    void send(uint8_t cmd) {
        ptr_tx[0] = ptr_tx[1] = cmd;
        ch.writeData8(ptr_tx, 2);
        printf("cmd: 0x%x\n", cmd);
    }

    void readDisplayInfo() {
        ptr_tx[0] = ptr_tx[1] = _cmd_display_info;
        ch.writeData8e(ptr_tx, 2);
        ch.readData8e(ptr_rx, 128);
        char str[128];
        memcpy(str, ptr_rx, 128);
        printf("  DispInfo: %s\n", str);
    }

    void readDisplayRange() {
        ptr_tx[0] = ptr_tx[1] = _cmd_display_range;
        ch.writeData8e(ptr_tx, 2);
        ch.readData8e(ptr_rx, 4);
        X = (uint16_t)(((ptr_rx[1] << 8) | ptr_rx[0])); 
        Y = (uint16_t)(((ptr_rx[3] << 8) | ptr_rx[2]));
        printf("  X: %d\n", X);
        printf("  Y: %d\n", Y);
    }

    void print_image() {
        ptr_tx[0] = ptr_tx[1] = _cmd_print_h_line;
        ch.writeData8e(ptr_tx, 2);
        Mat img("./test.jpg", X, Y);

        for (size_t i = 0; i < img.rows; i++) {
            ch.readData8e(ptr_rx, 1);
            if (ptr_rx[0] == 0xFF)
                ch.writeData8e(&img[(i * img.cols)], (3 * img.cols));
            printf("send %3zu\n", i);
        }
    }
};

int main () {
   Mat img("./test.jpg", 128, 160);
   img.save("./test_test.jpg");
   // ImageDisplay ph;
   // printf("init()\n");
   // ph.readDisplayInfo();
   // ph.readDisplayRange();
   // ph.print_image();
#if 0
    uint8_t *ptr_tx = new uint8_t[SIZE];
	uint8_t *ptr_rx = new uint8_t[SIZE];
	uusart ch1;
    ch1.setBaudRate(MY_BDR);
    ch1.setCharacteristics();
//    ch1.setCharacteristics(FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE);
    //ch1.writeData8(buffer, SIZE);
	//ch1.readData8(buffer, SIZE);

	//ch1.printDeviceInfo();
	for (int i = 0; i < 50; i++) {
	    //ch1.writeData8e(ptr_tx, SIZE);
        //for (int j = 0; j < SIZE; j++) {
          //  ptr_tx[j] += 0x0101; 
        //}
        //printf("Отправка: ");
	    //printa16(ptr_tx, SIZE);
	    ch1.readData8e(ptr_rx, SIZE);
        printf("Прием:    ");
	    printa(ptr_rx, SIZE);
    }
	delete []ptr_rx;
	delete []ptr_tx;
#endif
    return 0;
}

void randa8(uint8_t *m, const size_t N) {
    for (size_t i = 0; i < N; i++)
        m[i] = uint8_t(i + 1);
}

void randa16(uint16_t *m, const size_t N) {
    for (size_t i = 0; i < N; i++)
        m[i] = uint16_t(0x0FFF + i + 1);
}

void randa32(uint32_t *m, const size_t N) {
    for (size_t i = 0; i < N; i++)
        m[i] = uint32_t(0x0FFFFF + i + 1);
}


