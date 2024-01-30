#pragma once

#include <iostream>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <sys/ioctl.h>
#include <vector>


class SerialPort {
private:
    int fd;  // File descriptor for the serial port
    uint8_t buffByte[8];
    int bytesAvailable;
    int bytesAvailableLast;
    bool availableBuffer;

public:
    SerialPort(const char*);
    ~SerialPort();

    void configure();
    void sendBytes(const uint8_t*, size_t);
    void send_byte(const uint8_t, bool& success);
    uint8_t receive_byte(bool& success);
    void makeBufferNotAvailable();
    void receive8Bytes();

    void flush() const;

    const uint8_t* getReadBuffer() const { return buffByte; }
    const bool getBufferAvailability() const {return availableBuffer;}

    int startTimeout();
};
