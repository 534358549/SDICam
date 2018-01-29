#ifndef __SERIAL_H__
#define __SERIAL_H__

void request_send(int fd);
void clear_send(int fd);
void close_port(int fd);
void set_speed(int fd, int speed);
int set_parity(int fd, int databits, int stopbits, int parity);
int serial_set_speci_baud(int fd,int baud);
int uart_open(const char *dev,unsigned baud);

int serialHandle();


int Serial_test(void);
#endif
