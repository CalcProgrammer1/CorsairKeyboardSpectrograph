#include <stdio.h>
#include <unistd.h>
#include <usb.h>
#include <math.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include "chuck_fft.h"

#ifdef WIN32
#define USE_WINDOWS_USB
#ifdef USE_WINDOWS_USB
#include <sstream>
#include <wchar.h>
#include <string.h>
#include <Windows.h>
#include <hidsdi.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#endif // USE_WINDOWS_USB
#endif

#define FALSE 0

//#define ENABLE_FANBUS

#ifdef ENABLE_FANBUS
#include "serial_port.h"
#include "fanbus.h"
serial_port ports[2];
fanbus bus1;

static int red_leds[] = { 0x10, 0x13, 0x16, 0x19 };
static int grn_leds[] = { 0x11, 0x14, 0x17, 0x1A };
static int blu_leds[] = { 0x12, 0x15, 0x18, 0x1B };
#endif // ENABLE_FANBUS

#ifdef USE_WINDOWS_USB
HANDLE				dev;
#else
static struct usb_device *dev;
struct usb_dev_handle *handle;
#endif

static int init_keyboard();

static void update_keyboard();

static void set_led( int x, int y, int r, int g, int b );

#ifdef USE_WINDOWS_USB
HANDLE GetDeviceHandle(unsigned int uiVID, unsigned int uiPID, unsigned int uiMI);
#else
static struct usb_device *find_device(uint16_t vendor, uint16_t product);
#endif

char red_val[144];
char grn_val[144];
char blu_val[144];

char data_pkt[5][64] = { 0 };

unsigned char position_map[] = {
  137,  8, 20,255,
        0, 12, 24, 36, 48, 60, 72, 84, 96,108,120,132,  6, 18, 30, 42, 32, 44, 56, 68,255,
        1, 13, 25, 37, 49, 61, 73, 85, 97,109,121,133,  7, 31, 54, 66, 78, 80, 92,104,116,255,
        2, 14, 26, 38, 50, 62, 74, 86, 98,110,122,134, 90,102, 43, 55, 67,  9, 21, 33,128,255,
        3, 15, 27, 39, 51, 63, 75, 87, 99,111,123,135,126, 57, 69, 81,128,255,
        4, 28, 40, 52, 64, 76, 88,100,112,124,136, 79,103, 93,105,117,140,255,
        5, 17, 29, 53, 89,101,113, 91,115,127,139,129,141,140,255,
};

float size_map[] = {
        -15.5, 1, 1, -2.5, 1, -2, 0,
        1, -.5, 1, 1, 1, 1, -.75, 1, 1, 1, 1, -.75, 1, 1, 1, 1, -.5, 1, 1, 1, -.5, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, -.5, 1, 1, 1, -.5, 1, 1, 1, 1, 0,
        1.5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1.5, -.5, 1, 1, 1, -.5, 1, 1, 1, 1, 0,
        1.75, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2.25, -4, 1, 1, 1, 1, 0,
        2.25, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2.75, -1.5, 1, -1.5, 1, 1, 1, 1, 0,
        1.5, 1, 1.25, 6.5, 1.25, 1, 1, 1.5, -.5, 1, 1, 1, -.5, 2, 1, 1, 0,
};

unsigned char led_matrix[7][92];

unsigned char led_waveform[7][92];

float normalizeFFT(float fftin)
{
    if(fftin > 0)
    {
        return 20.0f*log10(fftin);
    }
    else
    {
        return 0;
    }
}

int main(int argc, char *argv[])
{
    float amplitude = 10.0f;
    unsigned char red, grn, blu;
    unsigned char buffer[256];
    float fft[256];
    unsigned char charfft[256];
    float win[256];
    float fftavg;
    int level;
    SDL_Surface* wavs = NULL;
    SDL_Surface* screen = NULL;
    SDL_Event event;

    SDL_Init( SDL_INIT_EVERYTHING );
    screen = SDL_SetVideoMode( 256, 256, 32, SDL_HWSURFACE );
    wavs = SDL_SetVideoMode( 256, 256, 32, SDL_HWSURFACE );
    SDL_WM_SetCaption("FanBus Audio Visualizer", NULL);

    ALCdevice *device = alcCaptureOpenDevice(NULL, 8000, AL_FORMAT_MONO8, 256);
    alcCaptureStart(device);

    hanning(win, 256);

#ifdef ENABLE_FANBUS
#ifdef WIN32
    if( ports[0].serial_open("COM2", 38400) == FALSE )
#else
    if( ports[0].serial_open("/dev/ttyS4", 38400) == FALSE )
#endif
    {
        return 0;
    }

    bus1.fanbus_set_port(&ports[0]);
#endif

    init_keyboard();

    while(1)
    {
            for(int j = 0; j < 92; j++)
            {
            for(int i = 0; i < 256; i++)
            {
                buffer[i] = 0;
                charfft[i] = 0;
                fft[i] = 0;
            }

            alcCaptureSamples(device, (ALCvoid *)buffer, 256);
            level = 0;
            for(int i = 0; i < 256; i++)
            {
                level += abs((int)buffer[i]-128);
                fft[i] = (buffer[i]-128)*amplitude;
            }

            rfft(fft, 256, 1);
            apply_window(fft, win, 256);
            boxRGBA(wavs, 0, 0, 255, 255, 0, 0, 0, 255);

            for(int i = 0; i < 256; i+=2)
            {
                float fftmag = sqrt((fft[i]*fft[i])+(fft[i+1]*fft[i+1]));
                fftavg += fftmag;
                charfft[i] = (unsigned char)fftmag;
                charfft[i+1] = charfft[i];
            }
            fftavg /= 10;
            for(int i = 0; i < 256; i++)
            {
                lineRGBA(wavs, i, 255, i, 255-charfft[i]*4, 0, 255, 0, 255);
                pixelRGBA(wavs, i, 255- (unsigned char)buffer[i], 255, 0, 0, 255);
            }
            lineRGBA(wavs, 247, 255, 247, 255-(unsigned char)fftavg, 255, 255, 255, 128);
            lineRGBA(wavs, 248, 255, 248, 255-(unsigned char)fftavg, 255, 255, 255, 128);
            lineRGBA(wavs, 249, 255, 249, 255-(unsigned char)fftavg, 255, 255, 255, 128);
            lineRGBA(wavs, 250, 255, 250, 255-(unsigned char)fftavg, 255, 255, 255, 128);

            lineRGBA(wavs, 251, 255, 251, 255-(unsigned char)level/5, 255, 255, 255, 128);
            lineRGBA(wavs, 252, 255, 252, 255-(unsigned char)level/5, 255, 255, 255, 128);
            lineRGBA(wavs, 253, 255, 253, 255-(unsigned char)level/5, 255, 255, 255, 128);
            lineRGBA(wavs, 254, 255, 254, 255-(unsigned char)level/5, 255, 255, 255, 128);
            SDL_BlitSurface(wavs, NULL, screen, NULL);
            SDL_Flip(screen);
            SDL_PollEvent(&event);
            if(event.type == SDL_QUIT)
            {
                return 0;
            }
            SDL_Delay(15);

#ifdef ENABLE_FANBUS
            for(int i = 0; i < 4; i++)
            {
                red = 64 * ( sin( ( ( ( j + 23 * i ) / 92.0f ) *6.28 )                ) + 1 );
                grn = 64 * ( sin( ( ( ( j + 23 * i ) / 92.0f ) *6.28 ) + ( 6.28 / 3 ) ) + 1 );
                blu = 64 * ( sin( ( ( ( j + 23 * i ) / 92.0f ) *6.28 ) - ( 6.28 / 3 ) ) + 1 );

                bus1.fanbus_write(red_leds[i], 0x02, red);
                bus1.fanbus_write(grn_leds[i], 0x02, grn);
                bus1.fanbus_write(blu_leds[i], 0x02, blu);

                bus1.fanbus_write(0x0C, 0x02, 0x01);
            }
#endif

            for(int x = 0; x < 91; x++)
            {
                for(int y = 0; y < 7; y++)
                {
                    red = 1.5f * ( sin( ( x / 92.0f ) * 2 * 3.14f ) + 1 );
                    grn = 1.5f * ( sin( ( ( x / 92.0f ) * 2 * 3.14f ) - ( 6.28f / 3 ) ) + 1 );
                    blu = 1.5f * ( sin( ( ( x / 92.0f ) * 2 * 3.14f ) + ( 6.28f / 3 ) ) + 1 );

                    set_led( ( x + j ) % 92, y, red, grn, blu );
                }
            }
            for(int i = 0; i < 91; i++)
            {
                for(int k = 0; k < 7; k++)
                {
                    if( charfft[(int)(i*2.1+1)] > (255/(15 + (i*0.8))) * (7-k) )
                    {
                        set_led( i, k, 0x07, 0x07, 0x07);
                    }
                }
            }
            update_keyboard();
        }
    }
    return 0;
}


static void set_led( int x, int y, int r, int g, int b )
{
    int led = led_matrix[y][x];

    if(led >= 144)
    {
        return;
    }

    if( r > 7 ) r = 7;
    if( g > 7 ) g = 7;
    if( b > 7 ) b = 7;

    r = 7 - r;
    g = 7 - g;
    b = 7 - b;

    red_val[led] = r;
    grn_val[led] = g;
    blu_val[led] = b;
}


static int init_keyboard()
{
    int status = 0;

    printf("Searching for Corsair K70 RGB keyboard...\n");

#ifdef USE_WINDOWS_USB
    dev = GetDeviceHandle(0x1B1C, 0x1B13, 0x3);
#else
    dev = find_device(0x1B1C, 0x1B13);
#endif

    if(dev == NULL)
    {
        printf("Searching for Corsair K95 RGB keyboard...\n");
#ifdef USE_WINDOWS_USB
        dev = GetDeviceHandle(0x1B1C, 0x1B11, 0x3);
#else
        dev = find_device(0x1B1C, 0x1B11);
#endif
    }

    if(dev == NULL)
    {
        printf("Corsair K70 RGB keyboard not detected :(\n");
        return 1;
    }
    else
    {
        printf("Corsair K70 RGB keyboard detected successfully :)\n");
    }

#ifndef USE_WINDOWS_USB
    handle = usb_open(dev);

    if(handle == NULL)
    {
        printf("USB device handle did not open :(\n");
        return 1;
    }
    else
    {
        printf("USB device handle opened successfully :)\n");
    }

#ifndef WIN32
    status = usb_claim_interface(handle, 3);


    status = usb_detach_kernel_driver_np(handle, 3);

    if(status == 0)
    {
        printf("USB interface claimed successfully :)\n");
    }
    else
    {
        printf("USB interface claim failed with error %d :(\n", status);
        return 1;
    }
#endif
#endif

    // Construct XY lookup table
    unsigned char *keys = position_map;
    float *sizes = size_map;
    for (int y = 0; y < 7; y++)
    {
        unsigned char key;
        int size = 0;

        for (int x = 0; x < 92; x++)
        {
            if (size == 0)
            {
                float sizef = *sizes++;
                if (sizef < 0)
                {
                    size = -sizef * 4;
                    key = 255;
                }
                else
                {
                    key = *keys++;
                    size = sizef * 4;
                }
            }

            led_matrix[y][x] = key;
            size--;
        }

        if (*keys++ != 255 || *sizes++ != 0)
        {
            printf("Bad line %d\n", y);
            return 1;
        }
    }

    return 0;
}

#ifdef USE_WINDOWS_USB
static void send_usb_msg(char * data_pkt)
{
    char usb_pkt[65];
    for(int i = 1; i < 65; i++)
    {
        usb_pkt[i] = data_pkt[i-1];
    }
    HidD_SetFeature(dev, usb_pkt, 65);
}
#endif

static void update_keyboard()
{
    // Perform USB control message to keyboard
    //
    // Request Type:  0x21
    // Request:       0x09
    // Value          0x0300
    // Index:         0x03
    // Size:          64

    data_pkt[0][0] = 0x7F;
    data_pkt[0][1] = 0x01;
    data_pkt[0][2] = 0x3C;

    data_pkt[1][0] = 0x7F;
    data_pkt[1][1] = 0x02;
    data_pkt[1][2] = 0x3C;

    data_pkt[2][0] = 0x7F;
    data_pkt[2][1] = 0x03;
    data_pkt[2][2] = 0x3C;

    data_pkt[3][0] = 0x7F;
    data_pkt[3][1] = 0x04;
    data_pkt[3][2] = 0x24;

    data_pkt[4][0] = 0x07;
    data_pkt[4][1] = 0x27;
    data_pkt[4][4] = 0xD8;

    for(int i = 0; i < 60; i++)
    {
        data_pkt[0][i+4] = red_val[i*2+1] << 4 | red_val[i*2];
    }

    for(int i = 0; i < 12; i++)
    {
        data_pkt[1][i+4] = red_val[i*2+121] << 4 | red_val[i*2+120];
    }

    for(int i = 0; i < 48; i++)
    {
        data_pkt[1][i+16] = grn_val[i*2+1] << 4 | grn_val[i*2];
    }

    for(int i = 0; i < 24; i++)
    {
        data_pkt[2][i+4] = grn_val[i*2+97] << 4 | grn_val[i*2+96];
    }

    for(int i = 0; i < 36; i++)
    {
        data_pkt[2][i+28] = blu_val[i*2+1] << 4 | blu_val[i*2];
    }

    for(int i = 0; i < 36; i++)
    {
        data_pkt[3][i+4] = blu_val[i*2+73] << 4 | blu_val[i*2+72];
    }

#ifdef USE_WINDOWS_USB
    send_usb_msg(data_pkt[0]);
    send_usb_msg(data_pkt[1]);
    send_usb_msg(data_pkt[2]);
    send_usb_msg(data_pkt[3]);
    send_usb_msg(data_pkt[4]);
#else
    usb_control_msg(handle, 0x21, 0x09, 0x0300, 0x03, data_pkt[0], 64, 1000);
    usb_control_msg(handle, 0x21, 0x09, 0x0300, 0x03, data_pkt[1], 64, 1000);
    usb_control_msg(handle, 0x21, 0x09, 0x0300, 0x03, data_pkt[2], 64, 1000);
    usb_control_msg(handle, 0x21, 0x09, 0x0300, 0x03, data_pkt[3], 64, 1000);
    usb_control_msg(handle, 0x21, 0x09, 0x0300, 0x03, data_pkt[4], 64, 1000);
#endif
}

#ifndef USE_WINDOWS_USB
static struct usb_device *find_device(uint16_t vendor, uint16_t product)
{
    struct usb_bus *bus;
    struct usb_device *dev;
    struct usb_bus *busses;

    usb_init();
    usb_find_busses();
    usb_find_devices();

    busses = usb_get_busses();

    for(bus = busses; bus; bus = bus->next)
    {
        for(dev = bus->devices; dev; dev = dev->next)
        {
            if((dev->descriptor.idVendor == vendor) && (dev->descriptor.idProduct == product))
            {
                return dev;
            }
        }
    }

    return NULL;
}
#endif

#ifdef USE_WINDOWS_USB
//==================================================================================================
// Code by http://www.reddit.com/user/chrisgzy
//==================================================================================================
bool IsMatchingDevice(wchar_t *pDeviceID, unsigned int uiVID, unsigned int uiPID, unsigned int uiMI)
{
	unsigned int uiLocalVID = 0, uiLocalPID = 0, uiLocalMI = 0;

	LPWSTR pszNextToken = 0;
	LPWSTR pszToken = wcstok(pDeviceID, L"\\#&");
	while (pszToken)
	{
		std::wstring tokenStr(pszToken);
		if (tokenStr.find(L"VID_", 0, 4) != std::wstring::npos)
		{
			std::wistringstream iss(tokenStr.substr(4));
			iss >> std::hex >> uiLocalVID;
		}
		else if (tokenStr.find(L"PID_", 0, 4) != std::wstring::npos)
		{
			std::wistringstream iss(tokenStr.substr(4));
			iss >> std::hex >> uiLocalPID;
		}
		else if (tokenStr.find(L"MI_", 0, 3) != std::wstring::npos)
		{
			std::wistringstream iss(tokenStr.substr(3));
			iss >> std::hex >> uiLocalMI;
		}

		pszToken = wcstok(0, L"\\#&");
	}

	if (uiVID != uiLocalVID || uiPID != uiLocalPID || uiMI != uiLocalMI)
		return false;

	return true;
}


//==================================================================================================
// Code by http://www.reddit.com/user/chrisgzy
//==================================================================================================
HANDLE GetDeviceHandle(unsigned int uiVID, unsigned int uiPID, unsigned int uiMI)
{
	const GUID GUID_DEVINTERFACE_HID = { 0x4D1E55B2L, 0xF16F, 0x11CF, 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 };
	HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_HID, 0, 0, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	if (hDevInfo == INVALID_HANDLE_VALUE)
		return 0;

	HANDLE hReturn = 0;

	SP_DEVINFO_DATA deviceData = { 0 };
	deviceData.cbSize = sizeof(SP_DEVINFO_DATA);

	for (unsigned int i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &deviceData); ++i)
	{
		wchar_t wszDeviceID[MAX_DEVICE_ID_LEN];
		if (CM_Get_Device_IDW(deviceData.DevInst, wszDeviceID, MAX_DEVICE_ID_LEN, 0))
			continue;

		if (!IsMatchingDevice(wszDeviceID, uiVID, uiPID, uiMI))
			continue;

		SP_INTERFACE_DEVICE_DATA interfaceData = { 0 };
		interfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

		if (!SetupDiEnumDeviceInterfaces(hDevInfo, &deviceData, &GUID_DEVINTERFACE_HID, 0, &interfaceData))
			break;

		DWORD dwRequiredSize = 0;
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &interfaceData, 0, 0, &dwRequiredSize, 0);

		SP_INTERFACE_DEVICE_DETAIL_DATA *pData = (SP_INTERFACE_DEVICE_DETAIL_DATA *)new unsigned char[dwRequiredSize];
		pData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

		if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &interfaceData, pData, dwRequiredSize, 0, 0))
		{
			delete pData;
			break;
		}

		HANDLE hDevice = CreateFile(pData->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			delete pData;
			break;
		}

		hReturn = hDevice;
		break;
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);

	return hReturn;
}
#endif
