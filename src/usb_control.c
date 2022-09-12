#include "usb_control.h"
#include "pinout.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "tcpm.h"

static const char *TAG = "usb_control";

#define FUSB302_I2C_ADDR 0b01000100
#define I2C_MASTER_SCL_IO I2C_SCL_GPIO
#define I2C_MASTER_SDA_IO I2C_SDA_GPIO
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_MASTER_TIMEOUT_MS 1000

#define WRITE_BIT I2C_MASTER_WRITE  /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ    /*!< I2C master read */
#define ACK_CHECK_EN 0x1            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0           /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                 /*!< I2C ack value */
#define NACK_VAL 0x1                /*!< I2C nack value */

void usb_control_init(void) {
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    i2c_param_config(i2c_master_port, &conf);

    // debugging
    uint8_t address;
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
    for (int i = 0; i < 128; i += 16) {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j++) {
            fflush(stdout);
            address = i + j;
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (address << 1) | WRITE_BIT, ACK_CHECK_EN);
            i2c_master_stop(cmd);
            esp_err_t ret = i2c_master_cmd_begin(i2c_master_port, cmd, 50 / portTICK_PERIOD_MS);
            i2c_cmd_link_delete(cmd);
            if (ret == ESP_OK) {
                printf("%02x ", address);
            } else if (ret == ESP_ERR_TIMEOUT) {
                printf("UU ");
            } else {
                printf("-- ");
            }
        }
        printf("\r\n");
    }
}

void fusb302_register_read(uint8_t reg_addr, uint8_t *data, size_t len) {
    esp_err_t err = i2c_master_write_read_device(I2C_MASTER_NUM, FUSB302_I2C_ADDR, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);
}

void fusb302_register_read_byte(uint8_t reg_addr, uint8_t *data) {
    esp_err_t err = i2c_master_write_read_device(I2C_MASTER_NUM, FUSB302_I2C_ADDR, &reg_addr, 1, data, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);
}

void fusb302_register_write_byte(uint8_t reg_addr, uint8_t data) {
    int ret;
    uint8_t write_buf[2] = {reg_addr, data};
    ret = i2c_master_write_to_device(I2C_MASTER_NUM, FUSB302_I2C_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK_WITHOUT_ABORT(ret);
}

void usb_enable_power_delivery(void) {

}