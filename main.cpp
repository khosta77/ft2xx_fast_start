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
    uint16_t *matrix;   /* Основной массив - он же матрица, т.к. двумерный */

private:
    uint16_t rgb565(const uint16_t &R, const uint8_t &G, const uint8_t &B) {
        uint16_t total_color = (uint16_t)(((R & 0b11111000) << 8) | ((G & 0b11111100) << 3) | (B >> 3));
        //((B << 11) & 0xF800) | ((G << 5) & 0x07E0) | (R & 0x001F);
	    total_color = ((total_color << 8) | (total_color >> 8));
        return total_color;
    }

public:
    Mat(const std::string &filename) {
        struct jpeg_decompress_struct d1;
        struct jpeg_error_mgr m1;
        d1.err = jpeg_std_error(&m1);
        jpeg_create_decompress(&d1);
        FILE *f = fopen(filename.c_str(),"rb");
        jpeg_stdio_src(&d1, f);
        jpeg_read_header(&d1, TRUE);

        rows = d1.image_height;
        cols = d1.image_width;
        matrix = new uint16_t[rows * cols]{};

        jpeg_start_decompress(&d1);
        uint8_t *pBuf = new uint8_t[rows * cols * d1.num_components]{};
        for (size_t i = 0; d1.output_scanline < d1.output_height;) {
            // Получить экранную строку
            i += jpeg_read_scanlines(&d1, (JSAMPARRAY)&(pBuf), 1);
            for (size_t j = 0; j < cols; ++j) {
                matrix[j + (i - 1) * cols] = rgb565(pBuf[j * d1.num_components + 2], 
                                                    pBuf[j * d1.num_components + 1],
                                                    pBuf[j * d1.num_components + 0]);
            }
        }

        jpeg_finish_decompress(&d1);
        jpeg_destroy_decompress(&d1);
        fclose(f);
        delete []pBuf;
    }

    ~Mat() {
        delete []matrix;
    } 
//------------------------------------------------------------------------------------------------------------
    // Basic operations
    uint16_t& operator()(const size_t &i, const size_t &j) const {
        return matrix[j + i * cols]; }

    uint16_t& operator()(const size_t &i, const size_t &j) { return (uint16_t&)matrix[j + i * cols]; }

    uint16_t& operator[](const size_t &k) { return matrix[k]; }
    uint16_t& operator[](const size_t &k) const { return matrix[k]; }
};

#define MY_BDR 489
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
        Mat img("./test.jpg");

        for (size_t i = 0; i < img.rows; i++) {
            ch.readData8e(ptr_rx, 1);
            if (ptr_rx[0] == 0xFF)
                ch.writeData16e(&img[(i * img.cols)], img.cols);
            printf("send %3zu\n", i);
        }
    }
};

int main () {
    ImageDisplay ph;
    //ph.readDisplayInfo();
    //ph.readDisplayRange();
    ph.print_image();
#if 0
    uint16_t *ptr_tx = new uint16_t[SIZE];
	uint16_t *ptr_rx = new uint16_t[SIZE];
	uusart ch1;
    ch1.setBaudRate(MY_BDR);
    ch1.setCharacteristics();
//    ch1.setCharacteristics(FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE);
    //ch1.writeData8(buffer, SIZE);
	//ch1.readData8(buffer, SIZE);

	//ch1.printDeviceInfo();
	for (int i = 0; i < 50; i++) {
	    ch1.writeData16e(ptr_tx, SIZE);
        for (int j = 0; j < SIZE; j++) {
            ptr_tx[j] += 0x0101; 
        }
        printf("Отправка: ");
	    printa16(ptr_tx, SIZE);
	    ch1.readData16e(ptr_rx, SIZE);
        printf("Прием:    ");
	    printa16(ptr_rx, SIZE);
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


