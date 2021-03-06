/* linux/arch/arm/mach-msm/board-rider-audio.c
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
 *
 */

#include <linux/android_pmem.h>
#include <linux/mfd/pmic8058.h>
#include <linux/mfd/marimba.h>
#include <linux/delay.h>
#include <linux/pmic8058-othc.h>
#include <linux/spi/spi_aic3254.h>

#include <mach/gpio.h>
#include <mach/dal.h>
#include <mach/tpa2051d3.h>
#include <mach/qdsp6v2_1x/snddev_icodec.h>
#include <mach/qdsp6v2_1x/snddev_ecodec.h>
#include <mach/qdsp6v2_1x/snddev_hdmi.h>
#include <mach/qdsp6v2_1x/audio_dev_ctl.h>
#include <mach/htc_acoustic_8x60.h>

#include "board-rider.h"
#include "board-rider-audio-data.h"
#include <mach/qdsp6v2_1x/audio_dev_ctl.h>

static struct mutex bt_sco_lock;
static struct mutex mic_lock;
static int curr_rx_mode;
static atomic_t aic3254_ctl = ATOMIC_INIT(0);

#define BIT_SPEAKER	(1 << 0)
#define BIT_HEADSET	(1 << 1)
#define BIT_RECEIVER	(1 << 2)
#define BIT_FM_SPK	(1 << 3)
#define BIT_FM_HS	(1 << 4)
#define RIDER_AUD_CODEC_RST        (67)
#define RIDER_AUD_HP_EN          PMGPIO(18)
#define RIDER_AUD_MIC_SEL        PMGPIO(14)
#define PM8058_GPIO_BASE			NR_MSM_GPIOS
#define PM8058_GPIO_PM_TO_SYS(pm_gpio)		(pm_gpio + PM8058_GPIO_BASE)
#define PMGPIO(x) (x-1)
void rider_snddev_bmic_pamp_on(int en);


static uint32_t msm_snddev_gpio[] = {
	GPIO_CFG(108, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(109, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(110, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};


static uint32_t msm_aic3254_reset_gpio[] = {
	GPIO_CFG(RIDER_AUD_CODEC_RST, 0,
		GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};

void rider_snddev_poweramp_on(int en)
{
	pr_aud_info("%s %d\n", __func__, en);
	if (en) {
		msleep(50);
		gpio_set_value(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_SPK_ENO), 1);
		if (!atomic_read(&aic3254_ctl))
			curr_rx_mode |= BIT_SPEAKER;
	} else {
		gpio_set_value(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_SPK_ENO), 0);
		if (!atomic_read(&aic3254_ctl))
			curr_rx_mode &= ~BIT_SPEAKER;
	}
}

void rider_snddev_hsed_pamp_on(int en)
{
	pr_aud_info("%s %d\n", __func__, en);
	if (en) {
		msleep(50);
		gpio_request(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_HP_EN),
						"AUD_HP_EN");
		gpio_direction_output(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_HP_EN), 1);
		set_headset_amp(1);
		if (!atomic_read(&aic3254_ctl))
			curr_rx_mode |= BIT_HEADSET;
	} else {
		set_headset_amp(0);
		gpio_request(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_HP_EN),
						"AUD_HP_EN");
		gpio_direction_output(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_HP_EN), 0);
		if (!atomic_read(&aic3254_ctl))
			curr_rx_mode &= ~BIT_HEADSET;
	}
}

void rider_snddev_hs_spk_pamp_on(int en)
{
	pr_aud_info("%s %d\n", __func__, en);
	rider_snddev_poweramp_on(en);
	if (en) {
		/* enable rx route */
		msleep(30);
		gpio_request(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_HP_EN),
						"AUD_HP_EN");
		gpio_direction_output(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_HP_EN), 1);
		set_speaker_headset_amp(1);

		if (!atomic_read(&aic3254_ctl))
			curr_rx_mode |= BIT_HEADSET;
		msleep(5);
	} else {
		/* disable rx route */
		set_speaker_headset_amp(0);
		gpio_request(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_HP_EN),
						"AUD_HP_EN");
		gpio_direction_output(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_HP_EN), 0);
		if (!atomic_read(&aic3254_ctl))
			curr_rx_mode &= ~BIT_HEADSET;
	}
}

void rider_snddev_receiver_pamp_on(int en)
{
	pr_aud_info("%s %d\n", __func__, en);
	if (en) {
		gpio_request(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_HP_EN),
						"AUD_HP_EN");
		gpio_direction_output(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_HP_EN), 1);
		set_handset_amp(1);
		if (!atomic_read(&aic3254_ctl))
			curr_rx_mode |= BIT_RECEIVER;
	} else {
		set_handset_amp(0);
		gpio_request(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_HP_EN),
						"AUD_HP_EN");
		gpio_direction_output(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_HP_EN), 0);
		if (!atomic_read(&aic3254_ctl))
			curr_rx_mode &= ~BIT_RECEIVER;
	}
}

void rider_snddev_bt_sco_pamp_on(int en)
{
	/* to be implemented */
}

/* power on/off externnal mic bias */
void rider_mic_enable(int en, int shift)
{
	pr_aud_info("%s: %d, shift %d\n", __func__, en, shift);

	mutex_lock(&mic_lock);

	if (en)
		pm8058_micbias_enable(OTHC_MICBIAS_2, OTHC_SIGNAL_ALWAYS_ON);
	else
		pm8058_micbias_enable(OTHC_MICBIAS_2, OTHC_SIGNAL_OFF);

	mutex_unlock(&mic_lock);
}

void rider_snddev_imic_pamp_on(int en)
{
	int ret;

	pr_aud_info("%s %d\n", __func__, en);
	rider_snddev_bmic_pamp_on(en);
	if (en) {
		ret = pm8058_micbias_enable(OTHC_MICBIAS_0, OTHC_SIGNAL_ALWAYS_ON);
		if (ret)
			pr_aud_err("%s: Enabling int mic power failed\n", __func__);

	} else {
		ret = pm8058_micbias_enable(OTHC_MICBIAS_0, OTHC_SIGNAL_OFF);
		if (ret)
			pr_aud_err("%s: Enabling int mic power failed\n", __func__);
	}
}

void rider_snddev_bmic_pamp_on(int en)
{
	int ret;

	pr_aud_info("%s %d\n", __func__, en);

	if (en) {
		ret = pm8058_micbias_enable(OTHC_MICBIAS_1, OTHC_SIGNAL_ALWAYS_ON);
		if (ret)
			pr_aud_err("%s: Enabling back mic power failed\n", __func__);

		/* select internal mic path */
		gpio_request(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_MIC_SEL),
						"AUD_MIC_SEL");
		gpio_direction_output(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_MIC_SEL), 0);
	} else {
		ret = pm8058_micbias_enable(OTHC_MICBIAS_1, OTHC_SIGNAL_OFF);
		if (ret)
			pr_aud_err("%s: Enabling back mic power failed\n", __func__);
	}
}

void rider_snddev_emic_pamp_on(int en)
{
	pr_aud_info("%s %d\n", __func__, en);

	if (en) {
		/* select external mic path */

		// Modify for Rider_ICS_35_S #84 - start
		if ((curr_rx_mode & BIT_HEADSET) || (curr_rx_mode & BIT_FM_HS)) {
			// turn on MIC_SEL will introduce a pop sound which occurs in ITS #84, (pop @ pick up phone call)
			// turn off headset amp first and then turn on it after turned on MIC_SEL
			set_headset_amp(0);
			gpio_set_value(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_HP_EN), 0);

			gpio_set_value(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_MIC_SEL), 1); // turn on MIC_SEL

			msleep(50);
			gpio_set_value(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_HP_EN), 1);
			set_headset_amp(1);
		} else {
			gpio_set_value(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_MIC_SEL), 1); // turn on MIC_SEL
		}
		// Modify for Rider_ICS_35_S #84 - end
	}
}

void rider_snddev_stereo_mic_pamp_on(int en)
{
	int ret;

	pr_aud_info("%s %d\n", __func__, en);

	if (en) {
		ret = pm8058_micbias_enable(OTHC_MICBIAS_0, OTHC_SIGNAL_ALWAYS_ON);
		if (ret)
			pr_aud_err("%s: Enabling int mic power failed\n", __func__);

		ret = pm8058_micbias_enable(OTHC_MICBIAS_1, OTHC_SIGNAL_ALWAYS_ON);
		if (ret)
			pr_aud_err("%s: Enabling back mic power failed\n", __func__);

		/* select external mic path */
		gpio_set_value(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_MIC_SEL), 0);
	} else {
		ret = pm8058_micbias_enable(OTHC_MICBIAS_0, OTHC_SIGNAL_OFF);
		if (ret)
			pr_aud_err("%s: Disabling int mic power failed\n", __func__);


		ret = pm8058_micbias_enable(OTHC_MICBIAS_1, OTHC_SIGNAL_OFF);
		if (ret)
			pr_aud_err("%s: Disabling back mic power failed\n", __func__);
	}
}

void rider_snddev_fmspk_pamp_on(int en)
{
	pr_aud_info("%s %d\n", __func__, en);
	if (en) {
		gpio_set_value(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_SPK_ENO), 1);
		set_speaker_amp(1);
		if (!atomic_read(&aic3254_ctl))
			curr_rx_mode |= BIT_FM_SPK;
	} else {
		set_speaker_amp(0);
		gpio_set_value(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_SPK_ENO), 0);
		if (!atomic_read(&aic3254_ctl))
			curr_rx_mode &= ~BIT_FM_SPK;
	}
}

void rider_snddev_fmhs_pamp_on(int en)
{
	pr_aud_info("%s %d\n", __func__, en);
	if (en) {
		// Modify for Rider_ICS_35_S #84 - start (pop @ end of phone call)
		msleep(50);
		// Modify for Rider_ICS_35_S #84 - end
		gpio_set_value(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_HP_EN), 1);
		set_headset_amp(1);
		if (!atomic_read(&aic3254_ctl))
			curr_rx_mode |= BIT_FM_HS;
	} else {
		set_headset_amp(0);
		gpio_set_value(PM8058_GPIO_PM_TO_SYS(RIDER_AUD_HP_EN), 0);
		if (!atomic_read(&aic3254_ctl))
			curr_rx_mode &= ~BIT_FM_HS;
	}
}

void rider_voltage_on(int en)
{
}

int rider_get_rx_vol(uint8_t hw, int network, int level)
{
	int vol = 0;

	/* to be implemented */

	pr_aud_info("%s(%d, %d, %d) => %d\n", __func__, hw, network, level, vol);

	return vol;
}

void rider_rx_amp_enable(int en)
{
	if (curr_rx_mode != 0) {
		atomic_set(&aic3254_ctl, 1);
		pr_aud_info("%s: curr_rx_mode 0x%x, en %d\n",
			__func__, curr_rx_mode, en);
		if (curr_rx_mode & BIT_SPEAKER)
			rider_snddev_poweramp_on(en);
		if (curr_rx_mode & BIT_HEADSET)
			rider_snddev_hsed_pamp_on(en);
		if (curr_rx_mode & BIT_RECEIVER)
			rider_snddev_receiver_pamp_on(en);
		if (curr_rx_mode & BIT_FM_SPK)
			rider_snddev_fmspk_pamp_on(en);
		if (curr_rx_mode & BIT_FM_HS)
			rider_snddev_fmhs_pamp_on(en);
		atomic_set(&aic3254_ctl, 0);;
	}
}

int rider_support_aic3254(void)
{
	return 1;
}

int rider_support_back_mic(void)
{
	return 1;
}

int rider_is_msm_i2s_slave(void)
{
	/* 1 - CPU slave, 0 - CPU master */
	return 1;
}

void rider_spibus_enable(int en)
{
	uint32_t msm_spi_gpio_on[] = {
		GPIO_CFG(RIDER_SPI_DO,  1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
		GPIO_CFG(RIDER_SPI_DI,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
		GPIO_CFG(RIDER_SPI_CS,  1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
		GPIO_CFG(RIDER_SPI_CLK, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	};

	uint32_t msm_spi_gpio_off[] = {
		GPIO_CFG(RIDER_SPI_DO,  1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
		GPIO_CFG(RIDER_SPI_DI,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
		GPIO_CFG(RIDER_SPI_CS,  1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
		GPIO_CFG(RIDER_SPI_CLK, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	};
	pr_aud_info("%s %d\n", __func__, en);
	if (en) {
		gpio_tlmm_config(msm_spi_gpio_on[0], GPIO_CFG_ENABLE);
		gpio_tlmm_config(msm_spi_gpio_on[1], GPIO_CFG_ENABLE);
		gpio_tlmm_config(msm_spi_gpio_on[2], GPIO_CFG_ENABLE);
		gpio_tlmm_config(msm_spi_gpio_on[3], GPIO_CFG_ENABLE);
	} else {
		gpio_tlmm_config(msm_spi_gpio_off[0], GPIO_CFG_DISABLE);
		gpio_tlmm_config(msm_spi_gpio_off[1], GPIO_CFG_DISABLE);
		gpio_tlmm_config(msm_spi_gpio_off[2], GPIO_CFG_DISABLE);
		gpio_tlmm_config(msm_spi_gpio_off[3], GPIO_CFG_DISABLE);
	}
	mdelay(1);
}

void rider_reset_3254(void)
{
	gpio_tlmm_config(msm_aic3254_reset_gpio[0], GPIO_CFG_ENABLE);
	gpio_set_value(RIDER_AUD_CODEC_RST, 0);
	mdelay(1);
	gpio_set_value(RIDER_AUD_CODEC_RST, 1);
}

void rider_get_acoustic_tables(struct acoustic_tables *tb)
{
	strcpy(tb->aic3254,	"IOTable.txt\0");
}

static struct q6v2audio_analog_ops ops = {
	.speaker_enable	        = rider_snddev_poweramp_on,
	.headset_enable	        = rider_snddev_hsed_pamp_on,
	.handset_enable	        = rider_snddev_receiver_pamp_on,
	.headset_speaker_enable	= rider_snddev_hs_spk_pamp_on,
	.bt_sco_enable	        = rider_snddev_bt_sco_pamp_on,
	.int_mic_enable         = rider_snddev_imic_pamp_on,
	.back_mic_enable        = rider_snddev_bmic_pamp_on,
	.ext_mic_enable         = rider_snddev_emic_pamp_on,
	.stereo_mic_enable      = rider_snddev_stereo_mic_pamp_on,
	.fm_headset_enable      = rider_snddev_fmhs_pamp_on,
	.fm_speaker_enable      = rider_snddev_fmspk_pamp_on,
	.voltage_on		= rider_voltage_on,
};

static struct q6v2audio_icodec_ops iops = {
	.support_aic3254 = rider_support_aic3254,
	.is_msm_i2s_slave = rider_is_msm_i2s_slave,
};

static struct q6v2audio_ecodec_ops eops = {
	.bt_sco_enable  = rider_snddev_bt_sco_pamp_on,
};

static struct aic3254_ctl_ops cops = {
	.rx_amp_enable        = rider_rx_amp_enable,
	.reset_3254           = rider_reset_3254,
	.spibus_enable        = rider_spibus_enable,
	.lb_dsp_init          = &LOOPBACK_DSP_INIT_PARAM,
	.lb_receiver_imic     = &LOOPBACK_Receiver_IMIC_PARAM,
	.lb_speaker_imic      = &LOOPBACK_Speaker_IMIC_PARAM,
	.lb_headset_emic      = &LOOPBACK_Headset_EMIC_PARAM,
	.lb_receiver_bmic     = &LOOPBACK_Receiver_BMIC_PARAM,
	.lb_speaker_bmic      = &LOOPBACK_Speaker_BMIC_PARAM,
	.lb_headset_bmic      = &LOOPBACK_Headset_BMIC_PARAM,
};

static struct acoustic_ops acoustic = {
	.enable_mic_bias = rider_mic_enable,
	.support_aic3254 = rider_support_aic3254,
	.support_back_mic = rider_support_back_mic,
	.get_acoustic_tables = rider_get_acoustic_tables
};

void rider_aic3254_set_mode(int config, int mode)
{
	aic3254_set_mode(config, mode);
}

static struct q6v2audio_aic3254_ops aops = {
       .aic3254_set_mode = rider_aic3254_set_mode,
};

void __init rider_audio_init(void)
{
	mutex_init(&bt_sco_lock);
	mutex_init(&mic_lock);

#ifdef CONFIG_MSM8X60_AUDIO
	pr_aud_info("%s\n", __func__);
	htc_8x60_register_analog_ops(&ops);
	htc_8x60_register_icodec_ops(&iops);
	htc_8x60_register_ecodec_ops(&eops);
	acoustic_register_ops(&acoustic);
	htc_8x60_register_aic3254_ops(&aops);
	msm_set_voc_freq(8000, 8000);
#endif

	aic3254_register_ctl_ops(&cops);

	/* PMIC GPIO Init (See board-rider.c) */
/* Reset AIC3254 */
	rider_reset_3254();
	gpio_tlmm_config(
		GPIO_CFG(RIDER_AUD_CDC_LDO_SEL, 0, GPIO_CFG_OUTPUT,
			GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_DISABLE);
	gpio_tlmm_config(msm_snddev_gpio[0], GPIO_CFG_DISABLE);
	gpio_tlmm_config(msm_snddev_gpio[1], GPIO_CFG_DISABLE);
	gpio_tlmm_config(msm_snddev_gpio[2], GPIO_CFG_DISABLE);
}
