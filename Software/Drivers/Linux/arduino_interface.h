// Copyright 2019-2020, Collabora, Ltd.
// SPDX-License-Identifier: BSL-1.0
/*!
 * @file
 * @brief  Interer face @ref drv_arduino.
 * @author Pete Black <pete.black@collabora.com>
 * @author Jakob Bornecrantz <jakob@collabora.com>
 * @ingroup drv_arduino
 */

#pragma once

#ifdef __cplusplus
extern "C" {
	#endif

	/*!
	 * @defgroup drv_arduino Arduino flexible input device driver
	 * @ingroup drv
	 *
	 * @brief Driver for the Monado Arduino based flexible input device.
	 */

	/*!
	 * Probing function for the Arduino based flexible input device driver.
	 *
	 * @ingroup drv_arduino
	 */
	struct xrt_auto_prober *
	arduino_create_auto_prober(void);

	/*!
	 * Create a arduino device from a serial port path.
	 *
	 * @ingroup drv_arduino
	 */
	struct xrt_device *
	arduino_device_create(const char *port_path);

	/*!
	 * @dir drivers/arduino
	 *
	 * @brief @ref drv_arduino files.
	 */


	#ifdef __cplusplus
}
#endif
