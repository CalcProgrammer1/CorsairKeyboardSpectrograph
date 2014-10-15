#ifndef FANBUS_H
#define FANBUS_H

#include "serial_port.h"

class fanbus
{
	public:
        //FanBus Constructor
        fanbus();
        fanbus(serial_port* fanPort);
        
        //FanBus Register Read and Write
        void fanbus_write(unsigned char int_addr, unsigned char dev_addr, unsigned char val);
        int  fanbus_read (unsigned char int_addr, unsigned char dev_addr);
        
        //FanBus Set Serial Port
        void fanbus_set_port(serial_port* newPort);
        
        //FanBus Device Scan
        unsigned char fanbus_scan_devices();
        
        unsigned char device_list[252];
	    unsigned char device_count;
	    
    private:
	    serial_port* FanBusPort;
};

#endif
