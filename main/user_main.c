#include "user_config.h"
#include "esp_system.h"
#include "esp_spiffs.h"
#include "spiffs_nucleus.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "tcpip_adapter.h"
#include "cpp_main.h"

void setup_gpio()
{
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
  for (int i = 0; i < 17; i++)
  {
    if (i != 16)
    {
      gpio_pullup_dis(i);
    }
    else
    {
      gpio_pulldown_dis(i);
    }
    gpio_set_direction(i, GPIO_MODE_DEF_INPUT);
  }
}

void setup_spifs()
{
  struct esp_spiffs_config config;
  config.phys_size = 2048 * 1024;
  config.phys_addr = 1024 * 1024;
  config.phys_erase_block = 4 * 1024;
  config.log_block_size = 4 * 1024;
  config.log_page_size = 128;
  config.fd_buf_size = 2 * sizeof(spiffs_fd);
  config.cache_buf_size = 4 * (sizeof(spiffs_cache_page) + 128);
  esp_spiffs_init(&config);
}

void app_main(void)
{
  nvs_flash_init();
  tcpip_adapter_init();
  setup_gpio();
  setup_spifs();
  cpp_main();
}
