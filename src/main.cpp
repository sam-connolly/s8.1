#include <mbed.h>
#include <EthernetInterface.h>
#include <rtos.h>
#include <mbed_events.h>

#include <C12832.h>

Thread eventhandler;
EventQueue eventqueue;

C12832 lcd(D11, D13, D12, D7, D10);

/* YOU will have to hardwire the IP address in here */
SocketAddress server("192.168.60.10",65280);
EthernetInterface eth;
UDPSocket udp;
char buffer[512];
char line[80];
int send(char *m, size_t s) {
    nsapi_size_or_error_t r = udp.sendto( server, m, s);
    return r;
}
int receive(char *m, size_t s) {
    SocketAddress reply;
    nsapi_size_or_error_t r = udp.recvfrom(&reply, m,s );
    return r;
}
/* Input from Potentiometers */
AnalogIn  left(A0);
AnalogIn right(A1);

void controller(void) {
    const char text[] = "temperature:?\n";
    char buffer[512];
    float T;
    strcpy(buffer, text);
    send(buffer, strlen(buffer));
    size_t len = receive(buffer, sizeof(buffer));
    buffer[len]='\0';
    lcd.locate(0,0);
    lcd.printf(buffer);
    sscanf(buffer,"temperature:%f", &T);
    lcd.locate(0,15);
    lcd.printf("Setting: %f", right.read()*20+10);

    AnalogIn right(A1);
    if ( T<right.read()*20+10) {
        strcpy(buffer,"heating:on\n");
        send(buffer, strlen(buffer));
    }
    if ( T>right.read()*20+10) {
        strcpy(buffer,"heating:off\n");
        send(buffer, strlen(buffer));
    }
}

int main() {

    printf("conecting \n");
    eth.connect();
    const char *ip = eth.get_ip_address();
    printf("IP address is: %s\n", ip ? ip : "No IP");

    udp.open( &eth);
    SocketAddress source;
        printf("sending messages to %s/%d\n",
                server.get_ip_address(),  server.get_port() );

    eventhandler.start(callback(&eventqueue, &EventQueue::dispatch_forever));

    eventqueue.call_every(1000,controller);

    while(1){}
}
