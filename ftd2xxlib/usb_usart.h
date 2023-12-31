#ifndef USB_USART_H_
#define USB_USART_H_

#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <cstdlib>
#include <cmath>

extern "C" {
#include "ftd2xx.h"
};

class uusart {
    // http://microsin.net/programming/pc/ftdi-d2xx-functions-api.html - ds
	FT_HANDLE ftHandle;
	FT_STATUS ftStatus;
	DWORD EventDWord;
	DWORD TxBytes;
	DWORD RxBytes;
	DWORD BytesReceived;
    DWORD BytesWritten;

	int bauderate = 9600;

	struct DeviceInfo {
		FT_HANDLE ftHandleTemp;
		DWORD Flags;
		DWORD ID;
		DWORD Type;
		DWORD LocId;
	    	
		char SerialNumber[16];
		char Description[64];
	};

	DeviceInfo di[3];

	bool ftStatusException(const int &module) {
		if (ftStatus == FT_OK)
			return true;
		switch (module) {
			case 0: {  // FT_Open
				printf("---> Ошибка FT_Open()\n");
				break;
			}
			case 1: {  // FT_SetBaudRate
				printf("---> Ошибка FT_SetBaudRate, BaudeRate остался %d\n", bauderate);
				break;
			}
			case 2: {  // FT_Write
				printf("---> Ошибка FT_Write, передача не прошла\n");
				break;
			}
			case 3: {
				printf("---> Ошибка FT_Read, передача не прошла\n");
				break;
			}
			case 4: {
				printf("---> Ошибка FT_GetDeviceInfoDetail, информации о девайсе не получено\n");
				break;
			}
			case 5: {
				printf("---> Ошибка FT_CreateDeviceInfoList, не удалось определить количество подключенных устройств\n");
                break;
			}
            case 6: {
                printf("---> Ошибка FT_GetStatus, возникли траблы\n");
                break;
            }
			default: {
				printf("!!!> Ошибка в неизвестном методе: %d\n", module);
			}
		};
		return false;
	}

public:
	uusart(const int &dev_id = 0) {
		bool buffer = getDeviceInfo(dev_id);
		if (!buffer)
			throw;
		ftStatus = FT_Open(dev_id, &ftHandle);
		buffer = ftStatusException(0);
	}

	~uusart() {
		FT_Close(ftHandle);
	}

	bool setBaudRate(const int &br = 9600) {
	    ftStatus = FT_SetBaudRate(ftHandle, br);
		bool buffer = ftStatusException(1);
		bauderate = (buffer) ? br : bauderate;
		return buffer;
	}
    
    /** @brief setCharacteristics - задание характеристик передачи
     *  @param wordlenght - количество бит на слово (фрейм) - тут должно быть значение FT_BITS_8 или FT_BITS_7
     *  @param stopbit - количество стоп-бит - должно быть FT_STOP_BITS_1 или FT_STOP_BITS_2
     *  @param parity - четность - должно быть FT_PARITY_NONE, FT_PARITY_ODD, FT_PARITY_EVEN, \
     *                  FT_PARITY_MARK или FT_PARITY SPACE
     * */
    bool setCharacteristics(const UCHAR &wordlenght = FT_BITS_8,
                            const UCHAR &stopbit = FT_STOP_BITS_1,
                            const UCHAR &parity = FT_PARITY_NONE) {
        ftStatus = FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
        return ftStatusException(10);
    }

	bool writeData8(uint8_t *df, const size_t &size) {
        ftStatus = FT_Write(ftHandle, df, size, &BytesWritten);
		return ftStatusException(2);
	}

    bool writeData16(uint16_t *df, const size_t &size) {
		uint8_t buffer_array[(sizeof(uint16_t) * size)];
		memcpy(buffer_array, df, (sizeof(uint16_t) * size));
        ftStatus = FT_Write(ftHandle, &buffer_array[0], (sizeof(uint16_t) * size), &BytesWritten);
		return ftStatusException(2);
	}

    bool writeData32(uint32_t *df, const size_t &size) {
		uint8_t buffer_array[(sizeof(uint32_t) * size)];
		memcpy(buffer_array, df, (sizeof(uint32_t) * size));
        ftStatus = FT_Write(ftHandle, &buffer_array[0], (sizeof(uint32_t) * size), &BytesWritten);
		return ftStatusException(2);
	}

#if 0  // с врименной задержкой
    bool writeData8(uint8_t *df, const size_t &size) {
        ftStatus = FT_Write(ftHandle, df, size, &BytesWritten);
		return ftStatusException(2);
	}

    bool writeData16(uint16_t *df, const size_t &size) {
		uint8_t buffer_array[(sizeof(uint16_t) * size)];
		memcpy(buffer_array, df, (sizeof(uint16_t) * size));
        ftStatus = FT_Write(ftHandle, &buffer_array[0], (sizeof(uint16_t) * size), &BytesWritten);
		return ftStatusException(2);
	}

    bool writeData32(uint32_t *df, const size_t &size) {
		uint8_t buffer_array[(sizeof(uint32_t) * size)];
		memcpy(buffer_array, df, (sizeof(uint32_t) * size));
        ftStatus = FT_Write(ftHandle, &buffer_array[0], (sizeof(uint32_t) * size), &BytesWritten);
		return ftStatusException(2);
	}
#endif

    bool writeData8e(uint8_t *df, const size_t &size) {
        do {
            ftStatus = FT_GetStatus(ftHandle, &RxBytes, &TxBytes, &EventDWord);
            if (!ftStatusException(6)) return false;
        } while (TxBytes > 0);
        ftStatus = FT_Write(ftHandle, df, size, &BytesWritten);
		return ftStatusException(2);
	}

    bool writeData16e(uint16_t *df, const size_t &size) {
		uint8_t buffer_array[(sizeof(uint16_t) * size)];
        do {
            ftStatus = FT_GetStatus(ftHandle, &RxBytes, &TxBytes, &EventDWord);
            if (!ftStatusException(6)) return false;
        } while (TxBytes > 0);
		memcpy(buffer_array, df, (sizeof(uint16_t) * size));
        ftStatus = FT_Write(ftHandle, &buffer_array[0], (sizeof(uint16_t) * size), &BytesWritten);
		return ftStatusException(2);
	}

    bool writeData32e(uint32_t *df, const size_t &size) {
		uint8_t buffer_array[(sizeof(uint32_t) * size)];
        do {
            ftStatus = FT_GetStatus(ftHandle, &RxBytes, &TxBytes, &EventDWord);
            if (!ftStatusException(6)) return false;
        } while (TxBytes > 0);
		memcpy(buffer_array, df, (sizeof(uint32_t) * size));
        ftStatus = FT_Write(ftHandle, &buffer_array[0], (sizeof(uint32_t) * size), &BytesWritten);
		return ftStatusException(2);
	}

	/** @brief readData8 - функция читает данные с устройства. В примере использования FT_Read были еще статусы \
	 *					  какие-то и задержки, в общем они не работали
	 *	@param df - массив, который считаем
	 *	@param size - размеры считываемого массива
	 * */
	bool readData8(uint8_t *df, const size_t &size) {		
//		FT_SetTimeouts(ftHandle, 5000, 10);
//		FT_GetStatus(ftHandle, &RxBytes, &TxBytes, &EventDWord);
		ftStatus = FT_Read(ftHandle, df, size, &BytesReceived);
		return ftStatusException(3);
	}

	bool readData16(uint16_t *df, const size_t &size) {
		uint8_t buffer_array[(sizeof(uint16_t) * size)];
		bool buffer = false;
		ftStatus = FT_Read(ftHandle, &buffer_array[0], (sizeof(uint16_t) * size), &BytesReceived);
		buffer = ftStatusException(3);
		if (buffer)
			memcpy(df, buffer_array, (size * sizeof(uint16_t)));
		return buffer;
	}

	bool readData32(uint32_t *df, const size_t &size) {
		uint8_t buffer_array[(sizeof(uint32_t) * size)];
		bool buffer = false;
		ftStatus = FT_Read(ftHandle, &buffer_array[0], (sizeof(uint32_t) * size), &BytesReceived);
		buffer = ftStatusException(3);
		if (buffer)
			memcpy(df, buffer_array, (size * sizeof(uint32_t)));
		return buffer;
	}

    bool readData8t(uint8_t *df, const size_t &size, const int &timewait) {
		ftStatus = FT_SetTimeouts(ftHandle, timewait, 10);
		ftStatus = FT_Read(ftHandle, df, size, &BytesReceived);
		return ftStatusException(3);
	}

	bool readData16t(uint16_t *df, const size_t &size, const int &timewait) {
		uint8_t buffer_array[(sizeof(uint16_t) * size)];
		bool buffer = false;
        ftStatus = FT_SetTimeouts(ftHandle, timewait, 0);
		ftStatus = FT_Read(ftHandle, &buffer_array[0], (sizeof(uint16_t) * size), &BytesReceived);
		buffer = ftStatusException(3);
		if (buffer)
			memcpy(df, buffer_array, (size * sizeof(uint16_t)));
		return buffer;
	}

	bool readData32t(uint32_t *df, const size_t &size, const int &timewait) {
		uint8_t buffer_array[(sizeof(uint32_t) * size)];
		bool buffer = false;
        ftStatus = FT_SetTimeouts(ftHandle, timewait, 0);
		ftStatus = FT_Read(ftHandle, &buffer_array[0], (sizeof(uint32_t) * size), &BytesReceived);
		buffer = ftStatusException(3);
		if (buffer)
			memcpy(df, buffer_array, (size * sizeof(uint32_t)));
		return buffer;
	}

    bool readData8e(uint8_t *df, const size_t &size) {
        do {
            ftStatus = FT_GetStatus(ftHandle, &RxBytes, &TxBytes, &EventDWord);
            if (!ftStatusException(6)) return false;
        } while (RxBytes == 0);
		ftStatus = FT_Read(ftHandle, df, size, &BytesReceived);
		return ftStatusException(3);
	}

	bool readData16e(uint16_t *df, const size_t &size) {
		uint8_t buffer_array[(sizeof(uint16_t) * size)];
		bool buffer = false;
        do {
            ftStatus = FT_GetStatus(ftHandle, &RxBytes, &TxBytes, &EventDWord);
            if (!ftStatusException(6)) return buffer;
        } while (RxBytes == 0);
        ftStatus = FT_Read(ftHandle, &buffer_array[0], (sizeof(uint16_t) * size), &BytesReceived);
		buffer = ftStatusException(3);
		if (buffer)
			memcpy(df, buffer_array, (size * sizeof(uint16_t)));
		return buffer;
	}

	bool readData32e(uint32_t *df, const size_t &size) {
		uint8_t buffer_array[(sizeof(uint32_t) * size)];
		bool buffer = false;
        do {
            ftStatus = FT_GetStatus(ftHandle, &RxBytes, &TxBytes, &EventDWord);
            if (!ftStatusException(6)) return buffer;
        } while (RxBytes == 0);
		ftStatus = FT_Read(ftHandle, &buffer_array[0], (sizeof(uint32_t) * size), &BytesReceived);
		buffer = ftStatusException(3);
		if (buffer)
			memcpy(df, buffer_array, (size * sizeof(uint32_t)));
		return buffer;
	}

	bool getDeviceInfo(const int &dev_id = 0) {
		DWORD numDevs;
		bool buffer = false;

		ftStatus = FT_CreateDeviceInfoList(&numDevs);
		buffer = ftStatusException(5);
		if (!buffer)
			return buffer;
		if ((dev_id < 0) && (dev_id >= numDevs))
			return false;
		ftStatus = FT_GetDeviceInfoDetail(dev_id, &di[dev_id].Flags, &di[dev_id].Type, &di[dev_id].ID,
										  &di[dev_id].LocId, di[dev_id].SerialNumber, di[dev_id].Description, 
										  &di[dev_id].ftHandleTemp);
		return ftStatusException(4);
	}

	void printDeviceInfo(const int &dev_id = 0) {
		printf("Dev %d:\n", dev_id);
		printf(" Flags=0x%x\n", di[dev_id].Flags);
		printf(" Type=0x%x\n", di[dev_id].Type);
		printf(" ID=0x%x\n", di[dev_id].ID);
		printf(" LocId=0x%x\n", di[dev_id].LocId);
		printf(" SerialNumber=%s\n", di[dev_id].SerialNumber);
		printf(" Description=%s\n\n", di[dev_id].Description);
		//printf(" ftHandle=0x%x\n", di[dev_id].ftHandleTemp);
	}
};

#endif  // USB_USART_H_
