#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <chrono>
#include <thread>
#include<unistd.h>
#include <algorithm>
#include <iterator>
#include "./ftd2xxlib/usb_usart.h"

extern "C" {  // jpeglib.h
#include <stdio.h>
#include <setjmp.h>
#include <unistd.h>
#include <stdlib.h>
#include <jconfig.h>
#include <jpeglib.h>
}

inline uint16_t rgb565(const uint8_t &R, const uint8_t &G, const uint8_t &B) {
    return (uint16_t)((R & 0b11111000) << 8) | ((G & 0b11111100) << 3) | (B >> 3);
}

class Mat {
    size_t _rows;
    size_t _cols;
    uint8_t *matrix;

public:
    Mat(const std::string &filename, const size_t &X, const size_t &Y) {
        struct jpeg_decompress_struct d1;
        struct jpeg_error_mgr m1;
        d1.err = jpeg_std_error(&m1);
        jpeg_create_decompress(&d1);
        FILE *f = fopen(filename.c_str(),"rb");
        jpeg_stdio_src(&d1, f);
        jpeg_read_header(&d1, TRUE);

        _rows = Y;
        _cols = X;
        const size_t in_rows = d1.image_height;
        const size_t in_cols = d1.image_width;
        matrix = new uint8_t[_rows * _cols * 3]{}; 
        int buffer_height = 1;
        size_t counter = 0;
        jpeg_start_decompress(&d1);
        JSAMPARRAY buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * buffer_height);
        buffer[0] = (JSAMPROW)malloc(sizeof(JSAMPLE) * d1.output_width * d1.output_components);
        const size_t row_stride = _cols * 3;
        if ((in_rows <= _rows) && (in_cols <= _cols)) {
            for (size_t r = 0; d1.output_scanline < d1.output_height; ++r) {
                if (r >= _rows) {
                    jpeg_read_scanlines(&d1, buffer, 1);
                    continue;
                }
                jpeg_read_scanlines(&d1, buffer, 1);
                for (size_t i = 0; i < row_stride; i++)
                    *(this->matrix + counter + i) = buffer[0][i];
                counter += row_stride;
            }
        } else {
            const double step_rows = (in_rows > _rows) ? (in_rows / _rows) : 1;
            const double step_cols = (in_cols > _cols) ? (in_cols / _cols) : 1;
            for (size_t r = 0; d1.output_scanline < d1.output_height; ++r) {
                if (r >= _rows) {
                    jpeg_read_scanlines(&d1, buffer, 1);
                    continue;
                }
                for (size_t i = 0; i < step_rows; i++)
                    jpeg_read_scanlines(&d1, buffer, 1);
                for (size_t i = 0, j = 0; i < row_stride; i+=3, j+=step_cols) {
                    j = i * step_cols;
                    if (j % 3 == 1)
                        j += 2;
                    else if (j % 3 == 2)
                        j += 1;
                    *(this->matrix + counter + i) = buffer[0][j];
                    *(this->matrix + counter + i + 1) = buffer[0][j + 1];
                    *(this->matrix + counter + i + 2) = buffer[0][j + 2];
                }
                counter += row_stride;
            }
        }
        jpeg_finish_decompress(&d1);
        jpeg_destroy_decompress(&d1);
        fclose(f);
        free(buffer);
        std::reverse(matrix, matrix + (_rows * _cols * 3));
        //printf("end read\n");
    }

    ~Mat() { delete []matrix; }

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
        cinfo.image_width = _cols;
        cinfo.image_height = _rows;
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
    uint8_t& operator()(const size_t &i, const size_t &j) const { return matrix[j + i * _cols]; }
    uint8_t& operator()(const size_t &i, const size_t &j) { return (uint8_t&)matrix[j + i * _cols]; }

    uint8_t& operator[](const size_t &k) { return matrix[k]; }
    uint8_t& operator[](const size_t &k) const { return matrix[k]; }

	inline size_t rows() const noexcept { return _rows; }
	inline size_t cols() const noexcept { return _cols; }
};

#define MY_BDR 1'900'000
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

    uint16_t size;

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

private:
    void mycopy(uint8_t *ptr1, uint8_t *ptr2, const size_t &size) {
        for (size_t i = 0, j = (size - 3); i < size; i += 3, j -= 3) {
            *(ptr1 + i) = *(ptr2 + j);
            *(ptr1 + i + 1) = *(ptr2 + j + 1);
            *(ptr1 + i + 2) = *(ptr2 + j + 2);
        }
    }

    void rgb888torgb565(uint8_t *ptr_old, uint8_t *ptr_new,const size_t &size) {
        uint16_t color = 0;
        for (size_t i = 0, j = 0, RGB888 = (3 * size); i < RGB888; i += 3, j += 2) {
            color = rgb565(*(ptr_old + i), *(ptr_old + i + 1), *(ptr_old + i + 2));
            *(ptr_new + j)     = (uint8_t)((color & 0xFF00) >> 8);
            *(ptr_new + j + 1) = (uint8_t)((color & 0x00FF));
        }
    }
    
    const std::string fn = "./test_img/ILI9488_test_320x480.jpeg";

public:

    void print_image() {
        ptr_tx[0] = ptr_tx[1] = _cmd_print_h_line;
        ch.writeData8e(ptr_tx, 2);
        Mat img(fn, X, Y);
        this->size = (3 * img.cols());
        uint8_t *ptr = new uint8_t[size];
#if 1  // Перевод в RGB666
        for (size_t i = 0; i < img.rows(); i++) {
            ch.readData8e(ptr_rx, 1);
            mycopy(ptr, &img[(i * (img.cols() * 3))], size);
            if (ptr_rx[0] == 0xFF)
                ch.writeData8e(ptr, size);
            printf("send %3zu\n", i);
        }
#else  // Перевод в RGB565
        uint8_t *ptr565 = new uint8_t[(2 * img.cols())];
        rgb888torgb565(ptr, ptr565, img.cols());
        for (size_t i = 0; i < img.rows(); i++) {
            ch.readData8e(ptr_rx, 1);
            mycopy(ptr, &img[(i * (img.cols() * 3))], size); // С этой передачей надо играть
            rgb888torgb565(ptr, ptr565, img.cols());
            if (ptr_rx[0] == 0xFF)
                ch.writeData8e(ptr565, (2 * img.cols()));
            printf("send %3zu\n", i);
        }
#endif
    }
};

int main () {
    ImageDisplay ph;
    printf("Init ImageDisplay\n");
    ph.readDisplayInfo();
    ph.readDisplayRange();
    ph.print_image();
    return 0;
}


