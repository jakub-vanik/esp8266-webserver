#include "cpp_main.h"

#include "user_config.h"
#include "wifi_manager.h"
#include "serial_port.h"
#include "server.h"
#include "service_factory.h"
#include "service_file.h"
#include "service_wifi.h"
#include "service_serial.h"
#include "service_gpio.h"
#include "webserver.h"

void cpp_main()
{
  WifiManager *wifi = new WifiManager();
  SerialPort *serial = new SerialPort(UART_NUM_0);
  Server *server = new Server(wifi, serial, 5000);
  ServiceFactory *factory = new ServiceFactory();
  factory->RegisterService(2, "/", new ServiceFile::Builder());
  factory->RegisterService(3, "/wifi/", new ServiceWifi::Builder(wifi));
  factory->RegisterService(4, "/serial/", new ServiceSerial::Builder(serial));
  factory->RegisterService(5, "/gpio/", new ServiceGpio::Builder());
  Webserver *webserver = new Webserver(factory, 80);
}
