//FanBus Protocol Files
#include "fanbus.h"
fanbus::fanbus()
{
    FanBusPort = (serial_port*)NULL;
}

fanbus::fanbus(serial_port* fanPort)
{
    FanBusPort = fanPort;
}

void fanbus::fanbus_write(unsigned char int_addr, unsigned char dev_addr, unsigned char val)
{
    char fan_cmd[] = {0x00, int_addr, dev_addr, val, 0xFF};
    FanBusPort->serial_write(fan_cmd, 5);
    //Flush the serial read buffer
    FanBusPort->serial_flush_rx();
}

int fanbus::fanbus_read(unsigned char int_addr, unsigned char dev_addr)
{
    char fan_cmd[6] = {0x01, int_addr, dev_addr, 0x00, 0xFF};
    FanBusPort->serial_write(fan_cmd, 5);
    if(FanBusPort->serial_read(fan_cmd, 6) > 5)
    {
        return fan_cmd[5];
    }
    else
    {
        return -1;
    }
}

void fanbus::fanbus_set_port(serial_port* newPort)
{
    FanBusPort = newPort;
}

unsigned char fanbus::fanbus_scan_devices()
{
    device_count = 0;
    for(unsigned char i = 0x03; i < 0xFF; i++)
    {
        printf("FanBusScan: Looking for FanBus device at Address %02X.\n", i);
        if(fanbus_read(0x00, i) >= 0)
        {
            printf("FanBusScan: Found device at Address %02X\n", i);
            device_list[device_count] = i;
            device_count++;
        }
    }
    //printf("Val: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",read_buf[0],read_buf[1],read_buf[2],read_buf[3],read_buf[4],read_buf[5]);
    printf("FanBusScan: Complete, found %d devices.\n", device_count);
    return device_count;
}
