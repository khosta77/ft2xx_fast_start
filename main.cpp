#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <cstdlib>
#include <cmath>

extern "C" {
#include "ftd2xx.h"
};

#define MY_BDR 115200
#define SIZE 10


class uusart {
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

	bool writeData8(uint8_t *df, const size_t &size) {
        ftStatus = FT_Write(ftHandle, df, size, &BytesWritten);
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

	bool writeData16(uint16_t *df, const size_t &size) {
		uint8_t buffer_array[(sizeof(uint16_t) * size)];
		memcpy(buffer_array, df, (sizeof(uint16_t) * size));
        ftStatus = FT_Write(ftHandle, &buffer_array[0], (sizeof(uint16_t) * size), &BytesWritten);
		return ftStatusException(2);
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

	bool writeData32(uint32_t *df, const size_t &size) {
		uint8_t buffer_array[(sizeof(uint32_t) * size)];
		memcpy(buffer_array, df, (sizeof(uint32_t) * size));
        ftStatus = FT_Write(ftHandle, &buffer_array[0], (sizeof(uint32_t) * size), &BytesWritten);
		return ftStatusException(2);
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

void printa8(uint8_t *m, const size_t N) {
    for (size_t i = 0; i < N; i++) {
		std::cout <<  (int)m[i] << " ";
    }
	std::cout << std::endl;
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


int main () {
	uint32_t *ptr_tx = new uint32_t[SIZE];
	uint32_t *ptr_rx = new uint32_t[SIZE];
	randa32(ptr_tx, SIZE);

	printa32(ptr_tx, SIZE);
	printa32(ptr_rx, SIZE);

	uusart ch1;
	ch1.printDeviceInfo();
	ch1.writeData32(ptr_tx, SIZE);
	ch1.readData32(ptr_rx, SIZE);
	printa32(ptr_tx, SIZE);
	printa32(ptr_rx, SIZE);

	delete []ptr_rx;
	delete []ptr_tx;
    return 0;
}

#if 0
int main () {
    FT_HANDLE ftHandle;
    FT_STATUS ftStatus;
    DWORD BytesWritten;
    uint8_t TxBuffer[SIZE]; // содержит данные для записи в устройство

    for (int i = 0; i < SIZE; i++) {
        TxBuffer[i] = (char)i;
    }
    ftStatus = FT_Open(0, &ftHandle);
    if(ftStatus != FT_OK)
    {
        printf("\nError FT_open\n");  // ошибка FT_Open
        return -1;
    }
    ftStatus = FT_SetBaudRate(ftHandle, MY_BDR); // установка скорости 115200 бод
    if (ftStatus != FT_OK) {
        printf("Не удалось выставить BaudeRate: %d\n", MY_BDR);
	    FT_Close(ftHandle);
        return -1;
    }
    for (;;) {
        ftStatus = FT_Write(ftHandle, TxBuffer, sizeof(TxBuffer), &BytesWritten);
        if (ftStatus == FT_OK)
            printf("\nOK\n");    // OK
        else
            printf("\nFUCK\n");  // ошибка
    }
    FT_Close(ftHandle);
    return 0;
}
#endif

#if 0
#define ARRAY_SIZE(x) sizeof((x))/sizeof((x)[0])



/* Test data which is easy to check visually */
static char testPattern[] = "\n"
"0123456789ABCDEF================FEDCBA9876543210\n"
"1DDDDDDDDDDDDDDD================DDDDDDDDDDDDDDD1\n"
"2DDDDDDDDDDDDDDD================DDDDDDDDDDDDDDD2\n"
"3DDDDDDDDDDDDDDD================DDDDDDDDDDDDDDD3\n"
"4DDDDDDDDDDDDDDD================DDDDDDDDDDDDDDD4\n"
"5DDDDDDDDDDDDDDD================DDDDDDDDDDDDDDD5\n"
"6DDDDDDDDDDDDDDD================DDDDDDDDDDDDDDD6\n"
"7DDDDDDDDDDDDDDD================DDDDDDDDDDDDDDD7\n"
"================0DDDDDDDDDDDDDD0================\n"
"================1DDDDDDDDDDDDDD1================\n"
"================2DDDDDDDDDDDDDD2================\n"
"================3DDDDDDDDDDDDDD3================\n"
"================4DDDDDDDDDDDDDD4================\n"
"================5DDDDDDDDDDDDDD5================\n"
"================6DDDDDDDDDDDDDD6================\n"
"================7DDDDDDDDDDDDDD7================\n"
"7DDDDDDDDDDDDDDD================DDDDDDDDDDDDDDD7\n"
"6DDDDDDDDDDDDDDD================DDDDDDDDDDDDDDD6\n"
"5DDDDDDDDDDDDDDD================DDDDDDDDDDDDDDD5\n"
"4DDDDDDDDDDDDDDD================DDDDDDDDDDDDDDD4\n"
"3DDDDDDDDDDDDDDD================DDDDDDDDDDDDDDD3\n"
"2DDDDDDDDDDDDDDD================DDDDDDDDDDDDDDD2\n"
"1DDDDDDDDDDDDDDD================DDDDDDDDDDDDDDD1\n"
"0123456789ABCDEF================FEDCBA9876543210\n"
"\n";



int main(int argc, char *argv[])
{
	int        retCode = -1; // Assume failure
	int        f = 0;
	FT_STATUS  ftStatus = FT_OK;
	FT_HANDLE  ftHandle = NULL;
	int        portNum = -1; // Deliberately invalid
	DWORD      bytesToWrite = 0;
	DWORD      bytesWritten = 0;
	int        inputRate = -1; // Entered on command line
	int        baudRate = -1; // Rate to actually use
	int        rates[] = {50, 75, 110, 134, 150, 200,
	                      300, 600, 1200, 1800, 2400, 4800,
	                      9600, 19200, 38400, 57600, 115200,
	                      230400, 460800, 576000, 921600};

	if (argc > 1)
	{
		sscanf(argv[1], "%d", &portNum);
	}

	if (portNum < 0)
	{
		// Missing, or invalid.  Just use first port.
		portNum = 0;
	}

	if (portNum > 16)
	{
		// User probably specified a baud rate without a port number
		printf("Syntax: %s [port number] [baud rate]\n", argv[0]);
		portNum = 0;
	}

	if (argc > 2)
	{
		sscanf(argv[2], "%d", &inputRate);

		for (f = 0; f < (int)(ARRAY_SIZE(rates)); f++)
		{
			if (inputRate == rates[f])
			{
				// User entered a rate we support, so we'll use it.
				baudRate = inputRate;
				break;
			}
		}
	}

	if (baudRate < 0)
		baudRate = 9600;

	printf("Trying FTDI device %d at %d baud.\n", portNum, baudRate);

	ftStatus = FT_Open(portNum, &ftHandle);
	if (ftStatus != FT_OK)
	{
		printf("FT_Open(%d) failed, with error %d.\n", portNum, (int)ftStatus);
		printf("Use lsmod to check if ftdi_sio (and usbserial) are present.\n");
		printf("If so, unload them using rmmod, as they conflict with ftd2xx.\n");
		goto exit;
	}

	assert(ftHandle != NULL);

	ftStatus = FT_ResetDevice(ftHandle);
	if (ftStatus != FT_OK)
	{
		printf("Failure.  FT_ResetDevice returned %d.\n", (int)ftStatus);
		goto exit;
	}

	ftStatus = FT_SetBaudRate(ftHandle, (ULONG)baudRate);
	if (ftStatus != FT_OK)
	{
		printf("Failure.  FT_SetBaudRate(%d) returned %d.\n",
		       baudRate,
		       (int)ftStatus);
		goto exit;
	}

	ftStatus = FT_SetDataCharacteristics(ftHandle,
	                                     FT_BITS_8,
	                                     FT_STOP_BITS_1,
	                                     FT_PARITY_NONE);
	if (ftStatus != FT_OK)
	{
		printf("Failure.  FT_SetDataCharacteristics returned %d.\n", (int)ftStatus);
		goto exit;
	}

	// Indicate our presence to remote computer
	ftStatus = FT_SetDtr(ftHandle);
	if (ftStatus != FT_OK)
	{
		printf("Failure.  FT_SetDtr returned %d.\n", (int)ftStatus);
		goto exit;
	}

	// Flow control is needed for higher baud rates
	ftStatus = FT_SetFlowControl(ftHandle, FT_FLOW_RTS_CTS, 0, 0);
	if (ftStatus != FT_OK)
	{
		printf("Failure.  FT_SetFlowControl returned %d.\n", (int)ftStatus);
		goto exit;
	}

	// Assert Request-To-Send to prepare remote computer
	ftStatus = FT_SetRts(ftHandle);
	if (ftStatus != FT_OK)
	{
		printf("Failure.  FT_SetRts returned %d.\n", (int)ftStatus);
		goto exit;
	}

	ftStatus = FT_SetTimeouts(ftHandle, 3000, 3000);	// 3 seconds
	if (ftStatus != FT_OK)
	{
		printf("Failure.  FT_SetTimeouts returned %d\n", (int)ftStatus);
		goto exit;
	}

	bytesToWrite = (DWORD)(sizeof(testPattern) - 1); // Don't write string terminator

	ftStatus = FT_Write(ftHandle,
	                    testPattern,
	                    bytesToWrite,
	                    &bytesWritten);
	if (ftStatus != FT_OK)
	{
		printf("Failure.  FT_Write returned %d\n", (int)ftStatus);
		goto exit;
	}

	if (bytesWritten != bytesToWrite)
	{
		printf("Failure.  FT_Write wrote %d bytes instead of %d.\n",
		       (int)bytesWritten,
		       (int)bytesToWrite);
		goto exit;
	}

	// Success
	retCode = 0;
	printf("Successfully wrote %d bytes\n", (int)bytesWritten);

exit:
	if (ftHandle != NULL)
		FT_Close(ftHandle);

	return retCode;
}
#if 0
int main() {
    FT_STATUS ftStatus;
FT_DEVICE_LIST_INFO_NODE *devInfo;         //Объявляем указатель на массив структур
DWORD numDevs;                             //Объявляем переменную, содержащую количество элементов
                                           //(устройств, структур) в массиве

//Это уже знакомо
ftStatus = FT_CreateDeviceInfoList(&numDevs);

if (ftStatus == FT_OK) {
  printf("Number of devices is %d\n",numDevs);
}
//Если подключено хотя бы одно устройство
if (numDevs > 0) {
  //Выделить память для хранения массива в зависимости от количества элементов
  devInfo =
(FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs);
  //Получить информацию о каждом устройстве в массиве
  ftStatus = FT_GetDeviceInfoList(devInfo,&numDevs);
  if (ftStatus == FT_OK) {
    //Если успешно
    for (int i = 0; i < numDevs; i++) {
      printf("Dev %d:\n",i);                                  //Напечатать номер устройства, начиная с 0
      printf("  Flags=0x%x\n",devInfo[i].Flags);              //Напечатать все поля каждой структуры
      printf("  Type=0x%x\n",devInfo[i].Type);
      printf("  ID=0x%x\n",devInfo[i].ID);
      printf("  LocId=0x%x\n",devInfo[i].LocId);
      printf("  SerialNumber=%s\n",devInfo[i].SerialNumber);
      printf("  Description=%s\n",devInfo[i].Description);
      printf("  ftHandle=0x%x\n",devInfo[i].ftHandle);
    }
  }
}
#if 0
    FT_STATUS ftStatus;         //Объявляем переменную типа FT_STATUS. Содержит результат работы функции
    DWORD numDevs;              //Предварительно объявляем переменную, в которой будет храниться число
                            //подключенных D2XX утройств
 
    // Создаем список устройств
    ftStatus = FT_CreateDeviceInfoList(&numDevs); 
    if (ftStatus == FT_OK) { 
        printf("Number of devices is %d\n",numDevs);             //Печатаем количество устройств
    } else { 
        printf("Ничего нет\n");
    }
#endif
    return 0;
}
#endif
#endif
