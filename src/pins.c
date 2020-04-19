#include <c_types.h>
#include <eagle_soc.h>

// convert gpio number to corresponding address in io mux
uint32_t io_mux_address[] = {
        PERIPHS_IO_MUX_GPIO0_U, // GPIO 0
        PERIPHS_IO_MUX_U0TXD_U, // GPIO 1
        PERIPHS_IO_MUX_GPIO2_U, // GPIO 2
        PERIPHS_IO_MUX_U0RXD_U, // GPIO 3
        PERIPHS_IO_MUX_GPIO4_U, // GPIO 4
        PERIPHS_IO_MUX_GPIO5_U, // GPIO 5
        0, // GPIO 6
        0, // GPIO 7
        0, // GPIO 8
        PERIPHS_IO_MUX_SD_DATA2_U, // GPIO 9
        PERIPHS_IO_MUX_SD_DATA3_U, // GPIO 10
        0, // GPIO 11
        PERIPHS_IO_MUX_MTDI_U, // GPIO 12
        PERIPHS_IO_MUX_MTCK_U, // GPIO 13
        PERIPHS_IO_MUX_MTMS_U, // GPIO 14
        PERIPHS_IO_MUX_MTDO_U, // GPIO 15
};