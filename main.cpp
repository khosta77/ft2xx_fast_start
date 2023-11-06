#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <cstdlib>
#include <cmath>

#include "usb_usart.h"

#define MY_BDR 115200
#define SIZE 10

void printa8 (uint8_t  *m, const size_t N);
void printa16(uint16_t *m, const size_t N);
void printa32(uint32_t *m, const size_t N);
void randa8 (uint8_t  *m, const size_t N);
void randa16(uint16_t *m, const size_t N);
void randa32(uint32_t *m, const size_t N);

int main () {
	uint32_t *ptr_tx = new uint32_t[SIZE];
	uint32_t *ptr_rx = new uint32_t[SIZE];
	randa32(ptr_tx, SIZE);

	printa32(ptr_tx, SIZE);
	printa32(ptr_rx, SIZE);

	uusart ch1;
	ch1.printDeviceInfo();
	ch1.setBaudRate(MY_BDR);
	ch1.writeData32(ptr_tx, SIZE);
	ch1.readData32(ptr_rx, SIZE);
	printa32(ptr_tx, SIZE);
	printa32(ptr_rx, SIZE);

	delete []ptr_rx;
	delete []ptr_tx;
    return 0;
}

void randa8(uint8_t *m, const size_t N) {
    for (size_t i = 0; i < N; i++) {
            m[i] = uint8_t(i + 1);
    }
}

void printa16(uint16_t *m, const size_t N) {
    for (size_t i = 0; i < N; i++) {
		std::cout <<  (int)m[i] << " ";
    }
	std::cout << std::endl;
}

void randa16(uint16_t *m, const size_t N) {
    for (size_t i = 0; i < N; i++) {
            m[i] = uint16_t(0x0FFF + i + 1);
    }
}

void printa32(uint32_t *m, const size_t N) {
    for (size_t i = 0; i < N; i++) {
		std::cout <<  (int)m[i] << " ";
    }
	std::cout << std::endl;
}

void randa32(uint32_t *m, const size_t N) {
    for (size_t i = 0; i < N; i++) {
            m[i] = uint32_t(0x0FFFFF + i + 1);
    }
}


