// Copyright 2019-2020, Collabora, Ltd.
// SPDX-License-Identifier: BSL-1.0
/*!
 * @file
 * @brief  Arduino felxable input device code.
 * @author Pete Black <pete.black@collabora.com>
 * @author Jakob Bornecrantz <jakob@collabora.com>
 * @ingroup drv_arduino
 */

#include "xrt/xrt_device.h"
#include "xrt/xrt_prober.h"
#include "xrt/xrt_tracking.h"

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include "os/os_time.h"
#include "os/os_threading.h"

#include "math/m_api.h"
#include "math/m_imu_pre.h"
#include "math/m_imu_3dof.h"

#include "util/u_var.h"
#include "util/u_time.h"
#include "util/u_misc.h"
#include "util/u_debug.h"
#include "util/u_device.h"
#include "util/u_bitwise.h"
#include "util/u_logging.h"

#include "arduino_interface.h"

#include <stdio.h>
#include <math.h>
#include <assert.h>


DEBUG_GET_ONCE_LOG_OPTION(arduino_log, "ARDUINO_LOG", U_LOGGING_WARN)

/*
 *
 * Structs.
 *
 */

/*!
 * A parsed sample of accel and gyro.
 */
struct arduino_parsed_sample
{
	uint32_t time;
	uint32_t delta;
	struct xrt_vec3_i32 accel;
	struct xrt_vec3_i32 gyro;
};

struct arduino_parsed_input
{
	uint32_t timestamp;
	struct arduino_parsed_sample sample;
};

/*!
 * @implements xrt_device
 */
struct arduino_device
{
	struct xrt_device base;
	int serial_fd;
	struct os_thread_helper oth;



	struct
	{

		//! Device time.
		uint64_t device_time;

		//! Lock for last and fusion.
		struct os_mutex lock;

		uint64_t last_time;

		//! Pre filter for the IMU.
		struct m_imu_pre_filter pre_filter;

		struct m_imu_3dof fusion;
	};


	struct
	{
		bool last;
	} gui;

	enum u_logging_level log_level;
};


/*
 *
 * Smaller helper functions.
 *
 */

#define ARDUINO_TRACE(d, ...) U_LOG_XDEV_IFL_T(&d->base, d->log_level, __VA_ARGS__)
#define ARDUINO_DEBUG(d, ...) U_LOG_XDEV_IFL_D(&d->base, d->log_level, __VA_ARGS__)
#define ARDUINO_INFO(d, ...) U_LOG_XDEV_IFL_I(&d->base, d->log_level, __VA_ARGS__)
#define ARDUINO_WARN(d, ...) U_LOG_XDEV_IFL_W(&d->base, d->log_level, __VA_ARGS__)
#define ARDUINO_ERROR(d, ...) U_LOG_XDEV_IFL_E(&d->base, d->log_level, __VA_ARGS__)

static inline struct arduino_device *
arduino_device(struct xrt_device *xdev)
{
	return (struct arduino_device *)xdev;
}

static uint32_t
calc_delta_and_handle_rollover(uint32_t next, uint32_t last)
{
	uint32_t tick_delta = next - last;

	// The 24-bit tick counter has rolled over,
	// adjust the "negative" value to be positive.
	if (tick_delta > 0xffffff) {
		tick_delta += 0x1000000;
	}

	return tick_delta;
}

static int16_t
read_i16(const uint8_t *buffer, size_t offset)
{
	return (buffer[offset] << 8) | buffer[offset + 1];
}


/*
 *
 * Internal functions.
 *
 */

static void *
arduino_run_thread(void *ptr)
{
	struct arduino_device *ad = (struct arduino_device *)ptr;
	char char_buffer[1];
	char line_buffer[128];
	int line_index = 0;

	while (os_thread_helper_is_running(&ad->oth)) {
		int ret = read(ad->serial_fd, char_buffer, 1);
		if (ret <= 0) {
			os_nanosleep(1000 * 100);
			continue;
		}

		char c = char_buffer[0];


		if (c != '\n' && c != '\r') {
			if (line_index < (int)sizeof(line_buffer) - 1) {
				line_buffer[line_index++] = c;
			}
		}

		else if (line_index > 0) {
			line_buffer[line_index] = '\0';
			line_index = 0;

			float q_w = 1.0f, q_x = 0.0f, q_y = 0.0f, q_z = 0.0f;


			int parsed = sscanf(line_buffer, "%f,%f,%f,%f", &q_w, &q_x, &q_y, &q_z);

			if (parsed == 4) {

				os_mutex_lock(&ad->lock);
				ad->fusion.rot.w = q_w;
				ad->fusion.rot.x = q_x;
				ad->fusion.rot.y = q_y;
				ad->fusion.rot.z = q_z;
				os_mutex_unlock(&ad->lock);
			}
		}
	}

	return NULL;
}



/*
 *
 * Device functions.
 *
 */

static xrt_result_t
arduino_get_fusion_pose(struct arduino_device *ad, enum xrt_input_name name, struct xrt_space_relation *out_relation)
{
	out_relation->pose.orientation = ad->fusion.rot;

	//! @todo assuming that orientation is actually currently tracked.
	out_relation->relation_flags = (enum xrt_space_relation_flags)(XRT_SPACE_RELATION_ORIENTATION_VALID_BIT |
	                                                               XRT_SPACE_RELATION_ORIENTATION_TRACKED_BIT);

	return XRT_SUCCESS;
}

static void
arduino_device_destroy(struct xrt_device *xdev)
{
	struct arduino_device *ad = arduino_device(xdev);

	// Destroy the thread object.
	os_thread_helper_destroy(&ad->oth);

	// Now that the thread is not running we can destroy the lock.
	os_mutex_destroy(&ad->lock);

	// Remove the variable tracking.
	u_var_remove_root(ad);

	// Destroy the fusion.
	m_imu_3dof_close(&ad->fusion);

	// Does null checking and zeros.
	if (ad->serial_fd >= 0) {
		close(ad->serial_fd);
	}

	free(ad);
}

static xrt_result_t
arduino_device_update_inputs(struct xrt_device *xdev)
{
	struct arduino_device *ad = arduino_device(xdev);

	uint64_t now = os_monotonic_get_ns();

	// Lock the data.
	os_mutex_lock(&ad->lock);

	ad->base.inputs[0].timestamp = now;
	ad->base.inputs[1].timestamp = now;
	ad->base.inputs[2].timestamp = now;
	ad->base.inputs[3].timestamp = now;
	ad->base.inputs[4].timestamp = now;
	ad->base.inputs[5].timestamp = now;
	ad->base.inputs[6].timestamp = now;
	ad->base.inputs[7].timestamp = now;

	// Done now.
	os_mutex_unlock(&ad->lock);

	return XRT_SUCCESS;
}

static xrt_result_t
arduino_device_get_tracked_pose(struct xrt_device *xdev,
                                enum xrt_input_name name,
                                int64_t at_timestamp_ns,
                                struct xrt_space_relation *out_relation)
{
	struct arduino_device *ad = arduino_device(xdev);

	(void)at_timestamp_ns;
	return arduino_get_fusion_pose(ad, name, out_relation);
}


/*
 *
 * Bindings
 *
 */

static struct xrt_binding_input_pair simple_inputs[4] = {
    {XRT_INPUT_SIMPLE_SELECT_CLICK, XRT_INPUT_DAYDREAM_BAR_CLICK},
    {XRT_INPUT_SIMPLE_MENU_CLICK, XRT_INPUT_DAYDREAM_CIRCLE_CLICK},
    {XRT_INPUT_SIMPLE_GRIP_POSE, XRT_INPUT_DAYDREAM_POSE},
    {XRT_INPUT_SIMPLE_AIM_POSE, XRT_INPUT_DAYDREAM_POSE},
};

static struct xrt_binding_profile binding_profiles[1] = {
    {
        .name = XRT_DEVICE_SIMPLE_CONTROLLER,
        .inputs = simple_inputs,
        .input_count = ARRAY_SIZE(simple_inputs),
        .outputs = NULL,
        .output_count = 0,
    },
};


/*
 *
 * Prober functions.
 *
 */

struct xrt_device *
arduino_device_create(const char *port_path)
{
	enum u_device_alloc_flags flags = (enum u_device_alloc_flags)(U_DEVICE_ALLOC_TRACKING_NONE);
	struct arduino_device *ad = U_DEVICE_ALLOCATE(struct arduino_device, flags, 8, 0);

	ad->serial_fd = open(port_path, O_RDONLY | O_NOCTTY);
	if (ad->serial_fd < 0) {
		U_LOG_E("Failed to open headset USB port!");
		free(ad);
		return NULL;
	}

	struct termios toptions;
	tcgetattr(ad->serial_fd, &toptions);
	cfsetispeed(&toptions, B921600);
	cfsetospeed(&toptions, B921600);
	toptions.c_cflag &= ~PARENB;
	toptions.c_cflag &= ~CSTOPB;
	toptions.c_cflag &= ~CSIZE;
	toptions.c_cflag |= CS8;
	toptions.c_lflag |= ICANON;
	tcsetattr(ad->serial_fd, TCSANOW, &toptions);

	ad->base.name = XRT_DEVICE_DAYDREAM;
	ad->base.destroy = arduino_device_destroy;
	ad->base.update_inputs = arduino_device_update_inputs;
	ad->base.get_tracked_pose = arduino_device_get_tracked_pose;
	ad->base.inputs[0].name = XRT_INPUT_DAYDREAM_POSE;
	ad->base.inputs[1].name = XRT_INPUT_DAYDREAM_TOUCHPAD_CLICK;
	ad->base.inputs[2].name = XRT_INPUT_DAYDREAM_BAR_CLICK;
	ad->base.inputs[3].name = XRT_INPUT_DAYDREAM_CIRCLE_CLICK;
	ad->base.inputs[4].name = XRT_INPUT_DAYDREAM_VOLDN_CLICK;
	ad->base.inputs[5].name = XRT_INPUT_DAYDREAM_VOLUP_CLICK;
	ad->base.inputs[6].name = XRT_INPUT_DAYDREAM_TOUCHPAD;
	ad->base.binding_profiles = binding_profiles;
	ad->base.binding_profile_count = ARRAY_SIZE(binding_profiles);

	static int controller_num = 0;
	snprintf(ad->base.str, XRT_DEVICE_NAME_LEN, "Arduino");
	snprintf(ad->base.serial, XRT_DEVICE_NAME_LEN, "Arduino %d", controller_num++);

	ad->log_level = debug_get_log_option_arduino_log();

	m_imu_3dof_init(&ad->fusion, M_IMU_3DOF_USE_GRAVITY_DUR_300MS);

	int ret = os_thread_helper_start(&ad->oth, arduino_run_thread, ad);
	if (ret != 0) {
		ARDUINO_ERROR(ad, "Failed to start thread!");
		arduino_device_destroy(&ad->base);
		return NULL;
	}

	u_var_add_root(ad, "Arduino flexible input device", true);
	u_var_add_gui_header(ad, &ad->gui.last, "Last");
	u_var_add_ro_vec3_f32(ad, &ad->fusion.last.accel, "last.accel");
	u_var_add_ro_vec3_f32(ad, &ad->fusion.last.gyro, "last.gyro");

	ad->base.supported.orientation_tracking = true;
	ad->base.supported.position_tracking = false;
	ad->base.device_type = XRT_DEVICE_TYPE_HMD;

	ARDUINO_DEBUG(ad, "Created device!");

	return &ad->base;
}
