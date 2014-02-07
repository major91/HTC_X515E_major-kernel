/* linux/arch/arm/mach-msm/board-rider.h
 *
 * Copyright (C) 2010-2011 HTC Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __ARCH_ARM_MACH_MSM_BOARD_RIDER_H
#define __ARCH_ARM_MACH_MSM_BOARD_RIDER_H

#include <mach/board.h>

#define RIDER_PROJECT_NAME	"rider"

#define MSM_RAM_CONSOLE_BASE	MSM_HTC_RAM_CONSOLE_PHYS
#define MSM_RAM_CONSOLE_SIZE	MSM_HTC_RAM_CONSOLE_SIZE

/* Memory map */

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)
#define QCE_SIZE		0x10000
#define QCE_0_BASE		0x18500000
#endif

#ifdef CONFIG_FB_MSM_LCDC_DSUB
/* VGA = 1440 x 900 x 4(bpp) x 2(pages)
   prim = 1024 x 600 x 4(bpp) x 2(pages)
   This is the difference. */
#define MSM_FB_DSUB_PMEM_ADDER (0x9E3400-0x4B0000)
#else
#define MSM_FB_DSUB_PMEM_ADDER (0)
#endif

#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
/* prim = 960 x 540 x 4(bpp) x 3(pages) */
#define MSM_FB_PRIM_BUF_SIZE (960 * ALIGN(540, 32) * 4 * 3)
#else
/* prim = 960 x 540 x 4(bpp) x 2(pages) */
#define MSM_FB_PRIM_BUF_SIZE (960 * ALIGN(540, 32) * 4 * 2)
#endif


#ifdef CONFIG_FB_MSM_OVERLAY_WRITEBACK
/* width x height x 3 bpp x 2 frame buffer */
#define MSM_FB_WRITEBACK_SIZE roundup(960 * ALIGN(540, 32) * 3 * 2, 4096)
#else
#define MSM_FB_WRITEBACK_SIZE 0
#endif

#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
/* prim = 1024 x 600 x 4(bpp) x 2(pages)
 * hdmi = 1920 x 1080 x 2(bpp) x 1(page)
 * Note: must be multiple of 4096 */
#define MSM_FB_SIZE roundup(MSM_FB_PRIM_BUF_SIZE + 0x3F4800 + MSM_FB_DSUB_PMEM_ADDER, 4096)
#elif defined(CONFIG_FB_MSM_TVOUT)
/* prim = 1024 x 600 x 4(bpp) x 2(pages)
 * tvout = 720 x 576 x 2(bpp) x 2(pages)
 * Note: must be multiple of 4096 */
#define MSM_FB_SIZE roundup(MSM_FB_PRIM_BUF_SIZE + 0x195000 + MSM_FB_DSUB_PMEM_ADDER, 4096)
#else /* CONFIG_FB_MSM_HDMI_MSM_PANEL */
#define MSM_FB_SIZE roundup(MSM_FB_PRIM_BUF_SIZE + MSM_FB_DSUB_PMEM_ADDER, 4096)
#endif /* CONFIG_FB_MSM_HDMI_MSM_PANEL */
#define MSM_PMEM_SF_SIZE 0x4000000 /* 64 Mbytes */
#define MSM_OVERLAY_BLT_SIZE   roundup(960 * ALIGN(540, 32) * 3 * 2, 4096)

#define MSM_PMEM_ADSP_SIZE	0x2F00000
#define MSM_PMEM_AUDIO_SIZE	0x239000

#define MSM_PMEM_SF_BASE		(0x70000000 - MSM_PMEM_SF_SIZE)
#define MSM_PMEM_ADSP_BASE	(MSM_PMEM_SF_BASE - MSM_PMEM_ADSP_SIZE)
#define MSM_OVERLAY_BLT_BASE	(0x40400000)
#define MSM_FB_BASE		(MSM_OVERLAY_BLT_BASE + MSM_OVERLAY_BLT_SIZE)
#define MSM_PMEM_AUDIO_BASE	(MSM_FB_BASE + MSM_FB_SIZE)


#define MSM_SMI_BASE          0x38000000
#define MSM_SMI_SIZE          0x4000000

/* Kernel SMI PMEM Region for video core, used for Firmware */
/* and encoder,decoder scratch buffers */
/* Kernel SMI PMEM Region Should always precede the user space */
/* SMI PMEM Region, as the video core will use offset address */
/* from the Firmware base */
#define KERNEL_SMI_BASE       (MSM_SMI_BASE)
#define KERNEL_SMI_SIZE       0x400000

/* User space SMI PMEM Region for video core*/
/* used for encoder, decoder input & output buffers  */
#define USER_SMI_BASE         (KERNEL_SMI_BASE + KERNEL_SMI_SIZE)
#define USER_SMI_SIZE         (MSM_SMI_SIZE - KERNEL_SMI_SIZE)
#define MSM_PMEM_SMIPOOL_BASE USER_SMI_BASE
#define MSM_PMEM_SMIPOOL_SIZE USER_SMI_SIZE

#define PHY_BASE_ADDR1  0x48000000
#define SIZE_ADDR1      0x21100000

/* GPIO definition */

/* Direct Keys */
#define RIDER_GPIO_KEY_CAM_STEP1   (123)
#define RIDER_GPIO_KEY_VOL_DOWN    (103)
#define RIDER_GPIO_KEY_VOL_UP      (104)
#define RIDER_GPIO_KEY_CAM_STEP2   (115)
#define RIDER_GPIO_KEY_POWER       (125)

#define RIDER_GPIO_CAPTURE_MODE_KEY       (64)
#define RIDER_GPIO_VIDEO_MODE_KEY       (68)

/* Battery */
#define RIDER_GPIO_MBAT_IN		   (61)
#define RIDER_GPIO_CHG_INT		   (126)

/* Wifi */
#define RIDER_GPIO_WIFI_IRQ              (46)
#define RIDER_GPIO_WIFI_SHUTDOWN_N       (96)

/* WiMax */
#define RIDER_GPIO_WIMAX_UART_SIN        (41)
#define RIDER_GPIO_WIMAX_UART_SOUT       (42)
#define RIDER_GPIO_V_WIMAX_1V2_RF_EN     (43)
#define RIDER_GPIO_WIMAX_EXT_RST         (49)
#define RIDER_GPIO_V_WIMAX_DVDD_EN       (94)
#define RIDER_GPIO_V_WIMAX_PVDD_EN       (105)
#define RIDER_GPIO_WIMAX_SDIO_D0         (143)
#define RIDER_GPIO_WIMAX_SDIO_D1         (144)
#define RIDER_GPIO_WIMAX_SDIO_D2         (145)
#define RIDER_GPIO_WIMAX_SDIO_D3         (146)
#define RIDER_GPIO_WIMAX_SDIO_CMD        (151)
#define RIDER_GPIO_WIMAX_SDIO_CLK_CPU    (152)
#define RIDER_GPIO_CPU_WIMAX_SW          (156)
#define RIDER_GPIO_CPU_WIMAX_UART_EN     (157)

/* Sensors */
#define RIDER_SENSOR_I2C_SDA		(72)
#define RIDER_SENSOR_I2C_SCL		(73)
#define RIDER_GYRO_INT               (127)

/* General */
#define RIDER_GENERAL_I2C_SDA		(59)
#define RIDER_GENERAL_I2C_SCL		(60)

/* Microp */

/* TP */
#define RIDER_TP_I2C_SDA           (51)
#define RIDER_TP_I2C_SCL           (52)
#define RIDER_TP_ATT_N             (65)
#define RIDER_TP_ATT_N_XB          (57)

/* LCD */
#define GPIO_LCM_ID	50
#define GPIO_LCM_RST_N	66

/* Audio */
#define RIDER_AUD_CODEC_RST        (67)
#define RIDER_AUD_CDC_LDO_SEL      (116)
/* BT */
#define RIDER_GPIO_BT_HOST_WAKE      (45)
#define RIDER_GPIO_BT_UART1_TX       (53)
#define RIDER_GPIO_BT_UART1_RX       (54)
#define RIDER_GPIO_BT_UART1_CTS      (55)
#define RIDER_GPIO_BT_UART1_RTS      (56)
#define RIDER_GPIO_BT_SHUTDOWN_N     (100)
#define RIDER_GPIO_BT_CHIP_WAKE      (130)
#define RIDER_GPIO_BT_RESET_N        (142)

/* USB */
#define RIDER_GPIO_USB_ID        (63)
#define RIDER_GPIO_MHL_RESET        (70)
#define RIDER_GPIO_MHL_INT		(71)
#define RIDER_GPIO_MHL_USB_EN         (139)
#define RIDER_GPIO_MHL_USB_SW        (99)

/* Camera */
#define RIDER_CAM_I2C_SDA           (47)
#define RIDER_CAM_I2C_SCL           (48)
#define RIDER_CAM_ID           (135)


/* Flashlight */
#define RIDER_FLASH_EN             (29)
#define RIDER_TORCH_EN             (30)

/* Accessory */
#define RIDER_GPIO_AUD_HP_DET        (31)

/* SPI */
#define RIDER_SPI_DO                 (33)
#define RIDER_SPI_DI                 (34)
#define RIDER_SPI_CS                 (35)
#define RIDER_SPI_CLK                (36)

/* PMIC */

/* PMIC GPIO definition */
#define PMGPIO(x) (x-1)
#define RIDER_VOL_UP             (104)
#define RIDER_VOL_DN             (103)
#define RIDER_AUD_QTR_RESET      PMGPIO(21)
#define RIDER_AUD_HANDSET_ENO    PMGPIO(18)
#define RIDER_AUD_SPK_ENO        PMGPIO(19)
#define RIDER_PS_VOUT            PMGPIO(22)
#define RIDER_GREEN_LED          PMGPIO(24)
#define RIDER_AMBER_LED          PMGPIO(25)
#define RIDER_AUD_MIC_SEL        PMGPIO(14)
#define RIDER_SDC3_DET           PMGPIO(34)
#define RIDER_WIFI_BT_SLEEP_CLK  PMGPIO(38)
#define RIDER_WIMAX_HOST_WAKEUP  PMGPIO(17)
#define RIDER_TP_RST             PMGPIO(23)
#define RIDER_TORCH_SET1         PMGPIO(40)
#define RIDER_TORCH_SET2         PMGPIO(31)
#define RIDER_CHG_STAT		 PMGPIO(33)
#define RIDER_AUD_REMO_EN        PMGPIO(15)
#define RIDER_AUD_REMO_PRES      PMGPIO(37)

extern int panel_type;

int __init rider_init_mmc(void);
void __init rider_audio_init(void);
int __init rider_init_keypad(void);
int __init rider_wifi_init(void);
int __init rider_init_panel(struct resource *res, size_t size);

#endif /* __ARCH_ARM_MACH_MSM_BOARD_RIDER_H */
