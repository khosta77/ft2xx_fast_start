#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <cstdlib>
#include <cmath>

#include "usb_usart.h"

#define MY_BDR 489
#define SIZE 20

void printa8 (uint8_t  *m, const size_t N);
void printa16(uint16_t *m, const size_t N);
void printa32(uint32_t *m, const size_t N);
void randa8 (uint8_t  *m, const size_t N);
void randa16(uint16_t *m, const size_t N);
void randa32(uint32_t *m, const size_t N);

int main () {
 	uint8_t *ptr_tx = new uint8_t[SIZE];
	uint8_t *ptr_rx = new uint8_t[SIZE];
	//randa8(ptr_tx, SIZE);

	//printa8(ptr_tx, SIZE);
	//printa32(ptr_rx, SIZE);

	uusart ch1;
#if 0
    for (int i = 464; i < 500; i++) {
        if (!ch1.setBaudRate(i))
            continue;
        const size_t sz = 1;
        bool mrk = false;
        for (size_t j = 0; j < sz; j++) {
            ch1.writeData8(ptr_tx, SIZE);
            printa8(ptr_tx, SIZE);
            for (int i = 0; i < 100000; i++);
            ch1.readData8(ptr_rx, SIZE);
            printa8(ptr_rx, SIZE);
            for (int k = 0; k < SIZE; k++) {
                if ((ptr_tx[k] + 1) != (ptr_rx[k])) {
                    printf("BDR: %4d ---> ERROR\n", i);
                    mrk = true;
                    break;
                }
            }
            for (int k = 0; k < SIZE; k++)  ptr_tx[k] += 0x01;
            if (mrk)
                break;
        }
        if (!mrk)
            printf("BDR: %4d ---> OK\n", i);
    }
    const UCHAR &wordlenght = FT_BITS_8,
                            const UCHAR &stopbit = FT_STOP_BITS_1,
                            const UCHAR &parity = FT_PARITY_NONE) {
#endif
    ch1.setBaudRate(MY_BDR);
    ch1.setCharacteristics();
//    ch1.setCharacteristics(FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE);
    //ch1.writeData8(buffer, SIZE);
	//ch1.readData8(buffer, SIZE);

	//ch1.printDeviceInfo();
	for (int i = 0; i < 50; i++) {
	    ch1.writeData8e(ptr_tx, SIZE);
        for (int j = 0; j < SIZE; j++) {
            ++ptr_tx[j]; 
        }
        printf("Отправка: ");
	    printa8(ptr_tx, SIZE);
	    ch1.readData8e(ptr_rx, SIZE);
        printf("Прием:    ");
	    printa8(ptr_rx, SIZE);
    }
	delete []ptr_rx;
	delete []ptr_tx;
    return 0;
}

void printa8(uint8_t *m, const size_t N) {
    for (size_t i = 0; i < N; i++)
        printf("%3d", ((int)m[i]));
	std::cout << std::endl;
}

void randa8(uint8_t *m, const size_t N) {
    for (size_t i = 0; i < N; i++)
        m[i] = uint8_t(i + 1);
}

void printa16(uint16_t *m, const size_t N) {
    for (size_t i = 0; i < N; i++)
        printf("%3d", ((int)m[i]));
	std::cout << std::endl;
}

void randa16(uint16_t *m, const size_t N) {
    for (size_t i = 0; i < N; i++)
        m[i] = uint16_t(0x0FFF + i + 1);
}

void printa32(uint32_t *m, const size_t N) {
    for (size_t i = 0; i < N; i++)
        printf("%3d", ((int)m[i]));
	std::cout << std::endl;
}

void randa32(uint32_t *m, const size_t N) {
    for (size_t i = 0; i < N; i++)
        m[i] = uint32_t(0x0FFFFF + i + 1);
}


