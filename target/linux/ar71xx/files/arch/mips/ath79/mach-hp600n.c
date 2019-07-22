/*
 * OPENROUTER 600N board support make by empire.x@qq.com
 */

#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

//#define HP600N_GPIO_LED_USBT	1
//#define HP600N_GPIO_LED_USB		11
//#define HP600N_GPIO_LED_WLAN2G		13
//#define HP600N_GPIO_LED_SYSTEM		14
//#define HP600N_GPIO_LED_QSS		15

#define HP600N_AR9580_LED_WLAN5G	1

//#define HP600N_GPIO_BTN_WPS		16
#define HP600N_GPIO_BTN_RESET		15

//#define HP600N_GPIO_USB_POWER		20

#define HP600N_KEYS_POLL_INTERVAL	20	/* msecs */
#define HP600N_KEYS_DEBOUNCE_INTERVAL	(3 * HP600N_KEYS_POLL_INTERVAL)

#define HP600N_MAC0_OFFSET		0
#define HP600N_MAC1_OFFSET		6
#define HP600N_WMAC_CALDATA_OFFSET	0x1000
#define HP600N_PCIE_CALDATA_OFFSET	0x5000

static const char *hp600n_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data hp600n_flash_data = {
	.part_probes	= hp600n_part_probes,
};

//static struct gpio_led hp600n_leds_gpio[] __initdata = {
//	{
//		.name		= "hp600n:green:qss",
////		.gpio		= HP600N_GPIO_LED_QSS,
////		.active_low	= 1,
//	},
//	{
//		.name		= "hp600n:green:system",
//		.gpio		= HP600N_GPIO_LED_SYSTEM,
//		.active_low	= 1,
//	},
//	{
//		.name		= "hp600n:green:usb",
//		.gpio		= HP600N_GPIO_LED_USB,
//		.active_low	= 1,
//	},
//	{
//		.name		= "hp600n:green:wlan2g",
//		.gpio		= HP600N_GPIO_LED_WLAN2G,
//		.active_low	= 1,
//	},
//	{
//		.name		= "hp600n:green:usbt",
//		.gpio		= HP600N_GPIO_LED_USBT,
//		.active_low	= 1,
//	},
//};

static struct gpio_led ar9580_leds_gpio[] = {
	{
		.name		= "hp600n:green:wlan5g",
		.gpio		= HP600N_AR9580_LED_WLAN5G,
		.active_low	= 1,
	},
};


static struct gpio_keys_button hp600n_gpio_keys[] __initdata = {
//	{
//		.desc		= "QSS button",
//		.type		= EV_KEY,
///		.code		= KEY_WPS_BUTTON,
//		.debounce_interval = HP600N_KEYS_DEBOUNCE_INTERVAL,
//		.gpio		= HP600N_GPIO_BTN_WPS,
//		.active_low	= 1,
//	},
	{
		.desc		= "RESET button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = HP600N_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= HP600N_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg hp600n_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_led_cfg hp600n_ar8327_led_cfg = {
	.led_ctrl0 = 0xc737c737,
	.led_ctrl1 = 0x00000000,
	.led_ctrl2 = 0x00000000,
	.led_ctrl3 = 0x0030c300,
	.open_drain = false,
};

static struct ar8327_platform_data hp600n_ar8327_data = {
	.pad0_cfg = &hp600n_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.led_cfg = &hp600n_ar8327_led_cfg,
};

static struct mdio_board_info hp600n_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &hp600n_ar8327_data,
	},
};

static void __init hp600n_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 tmpmac[ETH_ALEN];

	ath79_register_m25p80(&hp600n_flash_data);
//	ath79_register_leds_gpio(-1, ARRAY_SIZE(hp600n_leds_gpio),
//				 hp600n_leds_gpio);
	ath79_register_gpio_keys_polled(-1, HP600N_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(hp600n_gpio_keys),
					hp600n_gpio_keys);

	ath79_init_mac(tmpmac, mac, -1);
	ath79_register_wmac(art + HP600N_WMAC_CALDATA_OFFSET, tmpmac);

	ath79_init_mac(tmpmac, mac, 1);
	//ap9x_pci_setup_wmac_led_pin(0, 0);


	////////////////////////////////////////////////
	ap9x_pci_setup_wmac_led_pin(0, 5);
	ap9x_pci_setup_wmac_led_pin(1, 5);

	/* 2.4 GHz uses the first fixed antenna group (1, 0, 1, 0) */
	ap9x_pci_setup_wmac_gpio(0, (0xf << 6), (0xa << 6));

	/* 5 GHz uses the second fixed antenna group (0, 1, 1, 0) */
	ap9x_pci_setup_wmac_gpio(1, (0xf << 6), (0x6 << 6));
	//////////////////////////////////////////////////

	ap9x_pci_setup_wmac_leds(0, ar9580_leds_gpio,
                             ARRAY_SIZE(ar9580_leds_gpio));
	ap91_pci_init(art + HP600N_PCIE_CALDATA_OFFSET, tmpmac);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);

	mdiobus_register_board_info(hp600n_mdio0_info,
				    ARRAY_SIZE(hp600n_mdio0_info));

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, -2);

	/* GMAC0 is connected to an AR8327N switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;
	ath79_register_eth(0);

//	gpio_request_one(HP600N_GPIO_USB_POWER,
  //                   GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
  //                   "USB power");
	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_HP600N, "HP600N",	"OpenRouter HP600N",
	     hp600n_setup);
