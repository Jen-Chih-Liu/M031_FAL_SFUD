/**************************************************************************//**
* @file     main.c
* @version  V1.00
* @brief    Demonstrate how to access SPI flash.
*
* SPDX-License-Identifier: Apache-2.0
* @copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "NuMicro.h"
#include "fal.h"
#include "string.h"
void SYS_Init(void)
{
    /* Enable HIRC clock */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Waiting for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Switch HCLK clock source to HIRC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Enable module clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(SPI0_MODULE);

    /* Set module clock */
    CLK_SetModuleClock(SPI0_MODULE, CLK_CLKSEL2_SPI0SEL_PCLK1, MODULE_NoMsk);
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock and CyclesPerUs automatically. */
    SystemCoreClockUpdate();
}

void UART0_Init()
{
    /* Set GPB multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk);
    SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
}

#define BUF_SIZE 4096
static int fal_test(const char *partiton_name)
{
    int ret;
    int i, j, len;
    uint8_t buf[BUF_SIZE];
    const struct fal_flash_dev *flash_dev = NULL;
    const struct fal_partition *partition = NULL;

    if (!partiton_name)
    {
        printf("Input param partition name is null!");
        return -1;
    }

    partition = fal_partition_find(partiton_name);
    if (partition == NULL)
    {
        printf("Find partition (%s) failed!", partiton_name);
        ret = -1;
        return ret;
    }
		
		printf("\n\r%s\n\r",partition->flash_name);

    flash_dev = fal_flash_device_find(partition->flash_name);
    if (flash_dev == NULL)
    {
        printf("Find flash device (%s) failed!", partition->flash_name);
        ret = -1;
        return ret;
    }

    printf("Flash device : %s   "
          "Flash size : %dK   "
          "Partition : %s   "
          "Partition size: %dK", 
           partition->flash_name, 
           flash_dev->len/1024,
           partition->name,
           partition->len/1024);

    /* ?? `partition` ???????? */
    ret = fal_partition_erase_all(partition);
    if (ret < 0)
    {
        printf("Partition (%s) erase failed!", partition->name);
        ret = -1;
        return ret;
    }
    printf("Erase (%s) partition finish!", partiton_name);

		printf("partition->len:%d\n\r",partition->len);
		
		
    /* ???????????,???????? */
    for (i = 0; i < partition->len;)
    {
			  printf("%d\n\r",i);
        memset(buf, 0x00, BUF_SIZE);
        len = (partition->len - i) > BUF_SIZE ? BUF_SIZE : (partition->len - i);

        /* ? Flash ?? len ?????? buf ??? */
        ret = fal_partition_read(partition, i, buf, len);
        if (ret < 0)
        {
            printf("Partition (%s) read failed!", partition->name);
            ret = -1;
            return ret;
        }
        for(j = 0; j < len; j++)
        {
            /* ????????? 0xFF */
            if (buf[j] != 0xFF)
            {
                printf("The erase operation did not really succeed!");
                ret = -1;
                return ret;
            }
        }
        i += len;
    }

    /* ? 0 ?????? */
    for (i = 0; i < partition->len;)
    {
			 printf("%d\n\r",i);
        /* ??????? 0x00 */
        memset(buf, 0x00, BUF_SIZE);
        len = (partition->len - i) > BUF_SIZE ? BUF_SIZE : (partition->len - i);

        /* ???? */
        ret = fal_partition_write(partition, i, buf, len);
        if (ret < 0)
        {
            printf("Partition (%s) write failed!", partition->name);
            ret = -1;
            return ret;
        }
        i += len;
    }
    printf("Write (%s) partition finish! Write size %d(%dK).", partiton_name, i, i / 1024);

    /* ??????????????? */
    for (i = 0; i < partition->len;)
    {
			 printf("%d\n\r",i);
        /* ??????,? 0xFF ?? */
        memset(buf, 0xFF, BUF_SIZE);
        len = (partition->len - i) > BUF_SIZE ? BUF_SIZE : (partition->len - i);

        /* ????? buf ??? */
        ret = fal_partition_read(partition, i, buf, len);
        if (ret < 0)
        {
            printf("Partition (%s) read failed!", partition->name);
            ret = -1;
            return ret;
        }
        for(j = 0; j < len; j++)
        {
            /* ???????????? 3 ?????? 0x00 */
            if (buf[j] != 0x00)
            {
                printf("The write operation did not really succeed!");
                ret = -1;
                return ret;
            }
        }
        i += len;
    }

    ret = 0;
    return ret;
}
int main(void)
{
   
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, IP clock and multi-function I/O. */
    SYS_Init();

    /* Lock protected registers */
    SYS_LockReg();

    /* Init UART0 for printf */
    UART0_Init();
    int count = 1;
    fal_init();
	  fal_show_part_table();
	#if 0
	 uint8_t buf[BUF_SIZE];
	 const struct fal_partition *partition = NULL;
   partition = fal_partition_find("bank1");
	 fal_partition_read(partition, 0, buf, 64);
	 for(int i=0;i<64;i++)
	 {
	 buf[i]=64-i;
	 }
	 fal_partition_erase(partition,0,2048);
	 fal_partition_write(partition, 0, buf, 64);
	 fal_partition_read(partition, 0, buf, 1024);
	#endif
    if (fal_test("bank0") == 0) {
        printf("Fal partition (%s) test success!", "bank1");
    }
    else {
        printf("Fal partition (%s) test failed!", "bank1");
    }
		
		while(1){};
}
