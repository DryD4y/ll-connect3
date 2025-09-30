/*
 * Lian Li SL-Infinity HID Driver
 * 
 * Based on reverse-engineered protocol from Wireshark captures.
 * Uses HID Feature Reports (7-byte packets) instead of USB control transfers.
 * 
 * Usage Examples:
 * 
 * # Step 1: Unload the current module (if loaded)
 * sudo modprobe -r Lian_Li_SL_INFINITY
 * 
 * # Step 2: Load the updated HID driver
 * sudo insmod /home/dev/Documents/GitHub/ll-connect/kernel/Lian_Li_SL_INFINITY.ko
 * 
 * # Step 3: Check the logs for successful initialization
 * sudo dmesg | grep -i "sl-infinity" | tail -10
 * 
 * # Step 4: Test fan profile control
 * echo "0x50" > /proc/Lian_li_SL_INFINITY/fan_profile  # Set to Quiet profile
 * echo "0x20" > /proc/Lian_li_SL_INFINITY/fan_profile  # Set to Standard SP
 * 
 * # Step 5: Test lighting effects
 * echo "0x20 75 100 0" > /proc/Lian_li_SL_INFINITY/lighting_effect  # Rainbow, 75% speed, 100% brightness, right
 * echo "0x21 50 80 1" > /proc/Lian_li_SL_INFINITY/lighting_effect   # Rainbow Morph, 50% speed, 80% brightness, left
 * 
 * # Step 6: Test legacy fan speed control (per port)
 * echo "50" > /proc/Lian_li_SL_INFINITY/Port_1/fan_speed  # Set port 1 to 50% speed
 * echo "75" > /proc/Lian_li_SL_INFINITY/Port_2/fan_speed  # Set port 2 to 75% speed
 * 
 * # Step 7: Check current settings and status
 * cat /proc/Lian_li_SL_INFINITY/status
 * cat /proc/Lian_li_SL_INFINITY/fan_profile
 * cat /proc/Lian_li_SL_INFINITY/lighting_effect
 * 
 * # Step 8: Monitor logs for command execution
 * sudo dmesg | grep -i "sl-infinity" | tail -10
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/jiffies.h>
#include <linux/thermal.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/hid.h>
#include <linux/hidraw.h>
#include <linux/string.h>

#define VENDOR_ID 0x0cf2
#define PRODUCT_ID 0xa102

#define PACKET_SIZE 7   // SL-Infinity device uses 7-byte HID Feature Reports
#define REPORT_ID 0xE0  // HID Report ID for SL-Infinity commands

// SL-Infinity HID Feature Report Layout (7 bytes)
// Byte 0: Report ID (0xE0)
// Byte 1: Mode (Profile/Effect selector)
// Byte 2: Reserved (0x00)
// Byte 3: Speed (0x00-0x64, 0-100%)
// Byte 4: Brightness (0x00-0x64, 0-100%)
// Byte 5: Direction (0x00=Right, 0x01=Left)
// Byte 6: Reserved (0x00)

// Lighting Effect Modes
#define EFFECT_STATIC       0x50
#define EFFECT_RAINBOW      0x20
#define EFFECT_RAINBOW_MORPH 0x21
#define EFFECT_BREATHING    0x22
#define EFFECT_METEOR       0x20
#define EFFECT_RUNWAY       0x22

// Fan Profile Modes
#define PROFILE_QUIET       0x50
#define PROFILE_STANDARD    0x20
#define PROFILE_HIGH        0x21
#define PROFILE_FULL        0x22

// Initialization Commands
#define INIT_HANDSHAKE      0x50
#define INIT_CONFIG         0x10

struct rgb_data {
	int mode, brightness, speed, direction;
	unsigned char colors[4];  // RGB color data (4 bytes for color palette)
};

struct fan_curve {
	int temp;
	int speed;
};

struct port_data {
	int fan_speed, fan_speed_rpm, fan_count;
	struct fan_curve *points;
	int points_used_len;
	int points_total_len;
	struct rgb_data inner_rgb, outer_rgb;
	struct proc_dir_entry *proc_port;
	struct proc_dir_entry *proc_fan_count;
	struct proc_dir_entry *proc_fan_speed;
	struct proc_dir_entry *proc_fan_curve;
	struct proc_dir_entry *proc_inner_and_outer_rgb;
	struct proc_dir_entry *proc_inner_rgb;
	struct proc_dir_entry *proc_outer_rgb;
	struct proc_dir_entry *proc_inner_colors;
	struct proc_dir_entry *proc_outer_colors;
};

static struct hid_device *dev;
static struct port_data ports[4];
static struct proc_dir_entry *proc_root;
static struct proc_dir_entry *proc_status;
static struct proc_dir_entry *proc_mbsync;
static struct proc_dir_entry *proc_fan_profile;
static struct proc_dir_entry *proc_lighting_effect;
static struct timer_list speed_timer;
static struct work_struct speed_wq;
// static int prev_temp = 0;  // Unused for now
static int mb_sync_state;
static int safety_mode = 1;  // Enable safety mode by default
static int current_fan_profile = PROFILE_STANDARD;
static int current_lighting_effect = EFFECT_RAINBOW;
static int current_speed = 0x2E;  // 75%
static int current_brightness = 0x64;  // 100%
static int current_direction = 0x00;  // Right

module_param(safety_mode, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(safety_mode, "Enable safety mode (1=on, 0=off). Limits fan speeds and validates commands.");

// Function declarations
static int sl_infinity_probe(struct hid_device *hdev, const struct hid_device_id *id);
static void sl_infinity_remove(struct hid_device *hdev);
static int send_hid_feature_report(uint8_t mode, uint8_t speed, uint8_t brightness, uint8_t direction);
static int send_fan_profile_command(uint8_t profile);
static int send_lighting_effect_command(uint8_t effect, uint8_t speed, uint8_t brightness, uint8_t direction);
static int send_initialization_sequence(void);
static void set_fan_profile(int profile);
static void set_lighting_effect(int effect, int speed, int brightness, int direction);
static void mb_sync(int enable);
static void speed_work(struct work_struct *work);
static void speed_timer_callback(struct timer_list *t);

// Proc filesystem functions
static ssize_t read_status(struct file *f, char *ubuf, size_t count, loff_t *offs);
static ssize_t read_fan_speed(struct file *f, char *ubuf, size_t count, loff_t *offs);
static ssize_t write_fan_speed(struct file *f, const char *ubuf, size_t count, loff_t *offs);
static ssize_t read_fan_count(struct file *f, char *ubuf, size_t count, loff_t *offs);
static ssize_t write_fan_count(struct file *f, const char *ubuf, size_t count, loff_t *offs);
static ssize_t read_rgb(struct file *f, char *ubuf, size_t count, loff_t *offs);
static ssize_t write_rgb(struct file *f, const char *ubuf, size_t count, loff_t *offs);
static ssize_t read_colors(struct file *f, char *ubuf, size_t count, loff_t *offs);
static ssize_t write_colors(struct file *f, const char *ubuf, size_t count, loff_t *offs);
static inline int check_port(const char *parent_name);
static ssize_t read_mbsync(struct file *f, char *ubuf, size_t count, loff_t *offs);
static ssize_t write_mbsync(struct file *f, const char *ubuf, size_t count, loff_t *offs);
static ssize_t read_fan_profile(struct file *f, char *ubuf, size_t count, loff_t *offs);
static ssize_t write_fan_profile(struct file *f, const char *ubuf, size_t count, loff_t *offs);
static ssize_t read_lighting_effect(struct file *f, char *ubuf, size_t count, loff_t *offs);
static ssize_t write_lighting_effect(struct file *f, const char *ubuf, size_t count, loff_t *offs);

// Proc operations for proc files
static struct proc_ops pops_status = {
	.proc_read = read_status,
};

static struct proc_ops pops_mbsync = {
	.proc_read = read_mbsync,
	.proc_write = write_mbsync,
};

static struct proc_ops pops_fan_speed = {
	.proc_read = read_fan_speed,
	.proc_write = write_fan_speed,
};

static struct proc_ops pops_fan_count = {
	.proc_read = read_fan_count,
	.proc_write = write_fan_count,
};

static struct proc_ops pops_rgb = {
	.proc_read = read_rgb,
	.proc_write = write_rgb,
};

static struct proc_ops pops_colors = {
	.proc_read = read_colors,
	.proc_write = write_colors,
};

static struct proc_ops pops_fan_profile = {
	.proc_read = read_fan_profile,
	.proc_write = write_fan_profile,
};

static struct proc_ops pops_lighting_effect = {
	.proc_read = read_lighting_effect,
	.proc_write = write_lighting_effect,
};

// HID device table - match SL-Infinity device
static const struct hid_device_id sl_infinity_table[] = {
	{ HID_USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
	{ }
};
MODULE_DEVICE_TABLE(hid, sl_infinity_table);

// HID driver structure
static struct hid_driver sl_infinity_driver = {
	.name = "Lian_Li_SL_INFINITY",
	.id_table = sl_infinity_table,
	.probe = sl_infinity_probe,
	.remove = sl_infinity_remove,
};

// Send HID Feature Report (7-byte packet)
static int send_hid_feature_report(uint8_t mode, uint8_t speed, uint8_t brightness, uint8_t direction)
{
	uint8_t report[PACKET_SIZE];
	int ret;
	
	if (!dev) {
		printk(KERN_ERR "Lian li SL-Infinity hub: No HID device available\n");
		return -ENODEV;
	}
	
	// Build 7-byte HID Feature Report
	report[0] = REPORT_ID;     // Report ID (0xE0)
	report[1] = mode;          // Mode (Profile/Effect selector)
	report[2] = 0x00;          // Reserved
	report[3] = speed;         // Speed (0x00-0x64)
	report[4] = brightness;    // Brightness (0x00-0x64)
	report[5] = direction;     // Direction (0x00=Right, 0x01=Left)
	report[6] = 0x00;          // Reserved
	
	printk(KERN_INFO "Lian li SL-Infinity hub: Sending HID report: %02x %02x %02x %02x %02x %02x %02x\n",
	       report[0], report[1], report[2], report[3], report[4], report[5], report[6]);
	
	// Send HID Feature Report
	ret = hid_hw_raw_request(dev, REPORT_ID, report, PACKET_SIZE, HID_FEATURE_REPORT, HID_REQ_SET_REPORT);
	if (ret < 0) {
		printk(KERN_ERR "Lian li SL-Infinity hub: HID feature report failed: %d\n", ret);
		return ret;
	}
	
	printk(KERN_INFO "Lian li SL-Infinity hub: HID feature report sent successfully\n");
	return 0;
}

// Send fan profile command
static int send_fan_profile_command(uint8_t profile)
{
	uint8_t speed = 0x24;  // Default speed (36% = 0x24)
	
	// Map profile to appropriate speed values
	switch (profile) {
		case PROFILE_QUIET:
			speed = 0x00;  // Minimum speed
			break;
		case PROFILE_STANDARD:
		case PROFILE_HIGH:
		case PROFILE_FULL:
			speed = 0x24;  // 36% speed
			break;
		default:
			printk(KERN_WARNING "Lian li SL-Infinity hub: Unknown fan profile 0x%02x\n", profile);
			return -EINVAL;
	}
	
	return send_hid_feature_report(profile, speed, 0x00, 0x00);
}

// Send lighting effect command
static int send_lighting_effect_command(uint8_t effect, uint8_t speed, uint8_t brightness, uint8_t direction)
{
	// Validate parameters
	if (speed > 0x64) speed = 0x64;  // Max 100%
	if (brightness > 0x64) brightness = 0x64;  // Max 100%
	if (direction > 0x01) direction = 0x00;  // Only 0 or 1
	
	return send_hid_feature_report(effect, speed, brightness, direction);
}

// Send initialization sequence
static int send_initialization_sequence(void)
{
	int ret;
	
	printk(KERN_INFO "Lian li SL-Infinity hub: Sending initialization sequence\n");
	
	// Step 1: Initial handshake (e0 50 01 00 00 00 00)
	ret = send_hid_feature_report(INIT_HANDSHAKE, 0x01, 0x00, 0x00);
	if (ret < 0) {
		printk(KERN_ERR "Lian li SL-Infinity hub: Initialization handshake failed\n");
		return ret;
	}
	
	// Step 2: Apply base configuration (e0 10 60 01 03 00 00)
	ret = send_hid_feature_report(INIT_CONFIG, 0x60, 0x01, 0x00);
	if (ret < 0) {
		printk(KERN_ERR "Lian li SL-Infinity hub: Base configuration failed\n");
		return ret;
	}
	
	printk(KERN_INFO "Lian li SL-Infinity hub: Initialization sequence completed\n");
	return 0;
}

// Set fan profile
static void set_fan_profile(int profile)
{
	printk(KERN_INFO "Lian li SL-Infinity hub: Setting fan profile to %d\n", profile);
	send_fan_profile_command(profile);
}

// Set lighting effect
static void set_lighting_effect(int effect, int speed, int brightness, int direction)
{
	printk(KERN_INFO "Lian li SL-Infinity hub: Setting lighting effect %d, speed %d%%, brightness %d%%, direction %d\n", 
	       effect, speed, brightness, direction);
	send_lighting_effect_command(effect, speed, brightness, direction);
}

// Set fan speed (legacy function for compatibility)
static void set_speed(int port, int new_speed)
{
	if (port < 0 || port >= 4) return;
	
	ports[port].fan_speed = new_speed;
	
	// Convert percentage to hex (0-100% -> 0x00-0x64)
	uint8_t speed_hex = (new_speed * 0x64) / 100;
	if (speed_hex > 0x64) speed_hex = 0x64;
	
	// Use QUIET profile (0x50) for fan speed control
	// This is the correct mode for fan control according to protocol
	send_hid_feature_report(PROFILE_QUIET, speed_hex, 0x00, 0x00);
	
	printk(KERN_INFO "Lian li SL-Infinity hub: Set fan speed to %d%% (0x%02x) using QUIET profile\n", 
	       new_speed, speed_hex);
}

// Motherboard sync
static void mb_sync(int enable)
{
	mb_sync_state = enable;
	// TODO: Implement motherboard sync functionality
}

// Speed work function
static void speed_work(struct work_struct *work)
{
	// TODO: Implement temperature-based fan control
}

// Speed timer callback
static void speed_timer_callback(struct timer_list *t)
{
	schedule_work(&speed_wq);
	mod_timer(&speed_timer, jiffies + msecs_to_jiffies(1000));
}

// Proc filesystem read functions
static ssize_t read_status(struct file *f, char *ubuf, size_t count, loff_t *offs)
{
	char buf[512];
	int len = 0;
	int i;
	
	len += sprintf(buf + len, "Lian Li SL-Infinity Hub Status\n");
	len += sprintf(buf + len, "==============================\n");
	len += sprintf(buf + len, "Safety Mode: %s\n", safety_mode ? "ON" : "OFF");
	len += sprintf(buf + len, "Motherboard Sync: %s\n", mb_sync_state ? "ON" : "OFF");
	len += sprintf(buf + len, "Current Fan Profile: 0x%02x\n", current_fan_profile);
	len += sprintf(buf + len, "Current Lighting Effect: 0x%02x\n", current_lighting_effect);
	len += sprintf(buf + len, "\nPort Status:\n");
	
	for (i = 0; i < 4; i++) {
		len += sprintf(buf + len, "Port %d: Fan Speed %d%%, Fan Count %d\n", 
		               i, ports[i].fan_speed, ports[i].fan_count);
	}
	
	return simple_read_from_buffer(ubuf, count, offs, buf, len);
}

static ssize_t read_fan_speed(struct file *f, char *ubuf, size_t count, loff_t *offs)
{
	char buf[32];
	int port = (long)f->private_data;
	int len = sprintf(buf, "%d\n", ports[port].fan_speed);
	return simple_read_from_buffer(ubuf, count, offs, buf, len);
}

static ssize_t write_fan_speed(struct file *f, const char *ubuf, size_t count, loff_t *offs)
{
	char buf[32];
	int port = (long)f->private_data;
	int speed;
	
	if (copy_from_user(buf, ubuf, min(count, sizeof(buf) - 1)))
		return -EFAULT;
	
	buf[count] = '\0';
	if (sscanf(buf, "%d", &speed) == 1) {
		printk(KERN_INFO "Lian li SL-Infinity hub: write_fan_speed called with port=%d, speed=%d\n", port, speed);
		set_speed(port, speed);
	}
	
	return count;
}

static ssize_t read_fan_count(struct file *f, char *ubuf, size_t count, loff_t *offs)
{
	char buf[32];
	int port = (long)f->private_data;
	int len = sprintf(buf, "%d\n", ports[port].fan_count);
	return simple_read_from_buffer(ubuf, count, offs, buf, len);
}

static ssize_t write_fan_count(struct file *f, const char *ubuf, size_t count, loff_t *offs)
{
	char buf[32];
	int port = (long)f->private_data;
	int fan_count;
	
	if (copy_from_user(buf, ubuf, min(count, sizeof(buf) - 1)))
		return -EFAULT;
	
	buf[count] = '\0';
	if (sscanf(buf, "%d", &fan_count) == 1) {
		ports[port].fan_count = fan_count;
	}
	
	return count;
}

static ssize_t read_rgb(struct file *f, char *ubuf, size_t count, loff_t *offs)
{
	char buf[256];
	int port = (long)f->private_data;
	int len = sprintf(buf, "Port %d RGB: Mode %d, Brightness %d, Speed %d, Direction %d\n", 
	                  port, ports[port].inner_rgb.mode, ports[port].inner_rgb.brightness,
	                  ports[port].inner_rgb.speed, ports[port].inner_rgb.direction);
	return simple_read_from_buffer(ubuf, count, offs, buf, len);
}

static ssize_t write_rgb(struct file *f, const char *ubuf, size_t count, loff_t *offs)
{
	char buf[256];
	int port = (long)f->private_data;
	int mode, brightness, speed, direction;
	
	if (copy_from_user(buf, ubuf, min(count, sizeof(buf) - 1)))
		return -EFAULT;
	
	buf[count] = '\0';
	if (sscanf(buf, "%d %d %d %d", &mode, &brightness, &speed, &direction) == 4) {
		ports[port].inner_rgb.mode = mode;
		ports[port].inner_rgb.brightness = brightness;
		ports[port].inner_rgb.speed = speed;
		ports[port].inner_rgb.direction = direction;
		set_lighting_effect(mode, speed, brightness, direction);
	}
	
	return count;
}

static ssize_t read_mbsync(struct file *f, char *ubuf, size_t count, loff_t *offs)
{
	char buf[32];
	int len = sprintf(buf, "%d\n", mb_sync_state);
	return simple_read_from_buffer(ubuf, count, offs, buf, len);
}

static ssize_t write_mbsync(struct file *f, const char *ubuf, size_t count, loff_t *offs)
{
	char buf[32];
	int enable;
	
	if (copy_from_user(buf, ubuf, min(count, sizeof(buf) - 1)))
		return -EFAULT;
	
	buf[count] = '\0';
	if (sscanf(buf, "%d", &enable) == 1) {
		mb_sync(enable);
	}
	
	return count;
}

static ssize_t read_fan_profile(struct file *f, char *ubuf, size_t count, loff_t *offs)
{
	char buf[256];
	int len = 0;
	
	len += sprintf(buf + len, "Current Fan Profile: %d\n", current_fan_profile);
	len += sprintf(buf + len, "Available profiles:\n");
	len += sprintf(buf + len, "  0x50 - Quiet (minimum speed)\n");
	len += sprintf(buf + len, "  0x20 - Standard SP (36%% speed)\n");
	len += sprintf(buf + len, "  0x21 - High SP (36%% speed)\n");
	len += sprintf(buf + len, "  0x22 - Full SP (36%% speed)\n");
	len += sprintf(buf + len, "\nWrite profile number to change (e.g., 0x50 for Quiet)\n");
	
	return simple_read_from_buffer(ubuf, count, offs, buf, len);
}

static ssize_t write_fan_profile(struct file *f, const char *ubuf, size_t count, loff_t *offs)
{
	char buf[32];
	int profile;
	
	if (copy_from_user(buf, ubuf, min(count, sizeof(buf) - 1)))
		return -EFAULT;
	
	buf[count] = '\0';
	if (sscanf(buf, "0x%x", &profile) == 1 || sscanf(buf, "%d", &profile) == 1) {
		current_fan_profile = profile;
		set_fan_profile(profile);
		printk(KERN_INFO "Lian li SL-Infinity hub: Fan profile changed to 0x%02x\n", profile);
	}
	
	return count;
}

static ssize_t read_lighting_effect(struct file *f, char *ubuf, size_t count, loff_t *offs)
{
	char buf[512];
	int len = 0;
	
	len += sprintf(buf + len, "Current Lighting Effect: 0x%02x\n", current_lighting_effect);
	len += sprintf(buf + len, "Speed: %d%% (0x%02x)\n", (current_speed * 100) / 0x64, current_speed);
	len += sprintf(buf + len, "Brightness: %d%% (0x%02x)\n", (current_brightness * 100) / 0x64, current_brightness);
	len += sprintf(buf + len, "Direction: %s (0x%02x)\n", current_direction ? "Left" : "Right", current_direction);
	len += sprintf(buf + len, "\nAvailable effects:\n");
	len += sprintf(buf + len, "  0x50 - Static Color\n");
	len += sprintf(buf + len, "  0x20 - Rainbow\n");
	len += sprintf(buf + len, "  0x21 - Rainbow Morph\n");
	len += sprintf(buf + len, "  0x22 - Breathing\n");
	len += sprintf(buf + len, "  0x20 - Meteor\n");
	len += sprintf(buf + len, "  0x22 - Runway\n");
	len += sprintf(buf + len, "\nWrite: effect speed brightness direction\n");
	len += sprintf(buf + len, "Example: 0x20 75 100 0 (Rainbow, 75%% speed, 100%% brightness, right)\n");
	
	return simple_read_from_buffer(ubuf, count, offs, buf, len);
}

static ssize_t write_lighting_effect(struct file *f, const char *ubuf, size_t count, loff_t *offs)
{
	char buf[256];
	int effect, speed, brightness, direction;
	
	if (copy_from_user(buf, ubuf, min(count, sizeof(buf) - 1)))
		return -EFAULT;
	
	buf[count] = '\0';
	if (sscanf(buf, "0x%x %d %d %d", &effect, &speed, &brightness, &direction) == 4 ||
	    sscanf(buf, "%d %d %d %d", &effect, &speed, &brightness, &direction) == 4) {
		current_lighting_effect = effect;
		current_speed = (speed * 0x64) / 100;  // Convert percentage to hex
		current_brightness = (brightness * 0x64) / 100;
		current_direction = direction;
		set_lighting_effect(effect, current_speed, current_brightness, current_direction);
		printk(KERN_INFO "Lian li SL-Infinity hub: Lighting effect changed to 0x%02x\n", effect);
	}
	
	return count;
}

// HID probe function
static int sl_infinity_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
	int i, ret;
	
	dev = hdev;
	
	printk(KERN_INFO "Lian li SL-Infinity hub: HID device connected\n");
	
	// Check if we can access the device
	if (!hdev) {
		printk(KERN_ERR "Lian li SL-Infinity hub: No HID device found\n");
		return -ENODEV;
	}
	
	printk(KERN_INFO "Lian li SL-Infinity hub: HID device found: VID=0x%04x, PID=0x%04x\n", 
	       hdev->vendor, hdev->product);
	
	// Initialize HID device
	ret = hid_parse(hdev);
	if (ret) {
		printk(KERN_ERR "Lian li SL-Infinity hub: HID parse failed: %d\n", ret);
		return ret;
	}
	
	ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (ret) {
		printk(KERN_ERR "Lian li SL-Infinity hub: HID hardware start failed: %d\n", ret);
		return ret;
	}
	
	printk(KERN_INFO "Lian li SL-Infinity hub: HID device initialized successfully\n");
	
	// Send initialization sequence
	ret = send_initialization_sequence();
	if (ret < 0) {
		printk(KERN_WARNING "Lian li SL-Infinity hub: Initialization sequence failed: %d\n", ret);
		// Continue anyway, device might still work
	}
	
	// Initialize with default rainbow effect
	printk(KERN_INFO "Lian li SL-Infinity hub: Setting default rainbow effect\n");
	send_lighting_effect_command(EFFECT_RAINBOW, 0x2E, 0x64, 0x00);  // 75% speed, 100% brightness, right direction
	
	// Initialize ports
	for (i = 0; i < 4; i++) {
		ports[i].fan_speed = 0;
		ports[i].fan_count = 0;
		ports[i].fan_speed_rpm = 0;
		ports[i].points = NULL;
		ports[i].points_used_len = 0;
		ports[i].points_total_len = 0;
		memset(&ports[i].inner_rgb, 0, sizeof(ports[i].inner_rgb));
		memset(&ports[i].outer_rgb, 0, sizeof(ports[i].outer_rgb));
	}
	
	// Create proc filesystem entries
	proc_root = proc_mkdir("Lian_li_SL_INFINITY", NULL);
	if (!proc_root) {
		printk(KERN_ERR "Lian li SL-Infinity hub: Failed to create proc directory\n");
		return -ENOMEM;
	}
	
	proc_status = proc_create("status", 0444, proc_root, &pops_status);
	proc_mbsync = proc_create("mbsync", 0666, proc_root, &pops_mbsync);
	proc_fan_profile = proc_create("fan_profile", 0666, proc_root, &pops_fan_profile);
	proc_lighting_effect = proc_create("lighting_effect", 0666, proc_root, &pops_lighting_effect);
	
	// Create port directories
	for (i = 0; i < 4; i++) {
		char port_name[32];
		sprintf(port_name, "Port_%d", i + 1);
		ports[i].proc_port = proc_mkdir(port_name, proc_root);
		
		if (ports[i].proc_port) {
		// Fan speed - pass port number as private data
		ports[i].proc_fan_speed = proc_create_data("fan_speed", 0666, ports[i].proc_port, &pops_fan_speed, (void *)(long)i);
		if (!ports[i].proc_fan_speed) {
			printk(KERN_ERR "Lian li SL-Infinity hub: Failed to create fan_speed proc file for port %d\n", i);
		}
		
		// Fan count - pass port number as private data
		ports[i].proc_fan_count = proc_create_data("fan_count", 0666, ports[i].proc_port, &pops_fan_count, (void *)(long)i);
		if (!ports[i].proc_fan_count) {
			printk(KERN_ERR "Lian li SL-Infinity hub: Failed to create fan_count proc file for port %d\n", i);
		}
		
		// RGB - pass port number as private data
		ports[i].proc_inner_rgb = proc_create_data("inner_rgb", 0666, ports[i].proc_port, &pops_rgb, (void *)(long)i);
		ports[i].proc_outer_rgb = proc_create_data("outer_rgb", 0666, ports[i].proc_port, &pops_rgb, (void *)(long)i);
		
		// Colors - pass port number as private data
		ports[i].proc_inner_colors = proc_create_data("inner_colors", 0666, ports[i].proc_port, &pops_colors, (void *)(long)i);
		ports[i].proc_outer_colors = proc_create_data("outer_colors", 0666, ports[i].proc_port, &pops_colors, (void *)(long)i);
		}
	}
	
	// Initialize work queue and timer
	INIT_WORK(&speed_wq, speed_work);
	timer_setup(&speed_timer, speed_timer_callback, 0);
	mod_timer(&speed_timer, jiffies + msecs_to_jiffies(1000));
	
	return 0;
}

// HID remove function
static void sl_infinity_remove(struct hid_device *hdev)
{
	int i;
	
	printk(KERN_INFO "Lian li SL-Infinity hub: HID device disconnected\n");
	
	// Stop HID hardware
	hid_hw_stop(hdev);
	
	// Clean up timer and work queue
	del_timer_sync(&speed_timer);
	cancel_work_sync(&speed_wq);
	
	// Remove proc filesystem entries
	for (i = 0; i < 4; i++) {
		if (ports[i].proc_fan_speed) proc_remove(ports[i].proc_fan_speed);
		if (ports[i].proc_fan_count) proc_remove(ports[i].proc_fan_count);
		if (ports[i].proc_inner_rgb) proc_remove(ports[i].proc_inner_rgb);
		if (ports[i].proc_port) proc_remove(ports[i].proc_port);
	}
	
	if (proc_lighting_effect) proc_remove(proc_lighting_effect);
	if (proc_fan_profile) proc_remove(proc_fan_profile);
	if (proc_mbsync) proc_remove(proc_mbsync);
	if (proc_status) proc_remove(proc_status);
	if (proc_root) proc_remove(proc_root);
	
	dev = NULL;
}

// Module initialization
static int __init sl_infinity_init(void)
{
	int ret;
	
	printk(KERN_INFO "Lian li SL-Infinity hub: HID driver loaded\n");
	
	ret = hid_register_driver(&sl_infinity_driver);
	if (ret) {
		printk(KERN_ERR "Lian li SL-Infinity hub: hid_register_driver failed. Error number %d\n", ret);
		return ret;
	}
	
	return 0;
}

// Helper function to map port names to port numbers (unused - now using private_data)
static inline int check_port(const char *parent_name) 
{
	if (strcmp(parent_name, "Port_1") == 0) {
		return 0;
	} else if (strcmp(parent_name, "Port_2") == 0) {
		return 1;
	} else if (strcmp(parent_name, "Port_3") == 0) {
		return 2;
	} else {
		return 3;
	}
}

// Colors functions
static ssize_t read_colors(struct file *f, char *ubuf, size_t count, loff_t *offs)
{
	size_t textsize = 8;  // 4 bytes * 2 chars per byte
	char *text = kcalloc(textsize, sizeof(*text), GFP_KERNEL);
	int to_copy, not_copied, delta;
	const char *name = f->f_path.dentry->d_name.name;
	unsigned char *colors;

	to_copy = min(count, textsize);

	int port = (long)f->private_data;

	if      (strcmp(name, "inner_colors") == 0) colors = ports[port].inner_rgb.colors;
	else if (strcmp(name, "outer_colors") == 0) colors = ports[port].outer_rgb.colors;

	int text_i = 0;
	for (int i = 0; i < 4; i++) {
		sprintf(&text[text_i], "%02x", colors[i]);
		text_i += 2;
	}

	not_copied = copy_to_user(ubuf, text, to_copy);
	delta = to_copy - not_copied;

	kfree(text);

	if (*offs) return 0;
	else *offs = textsize;

	return delta;
}

static ssize_t write_colors(struct file *f, const char *ubuf, size_t count, loff_t *offs)
{
	size_t text_size = 4;
	char *text = kcalloc(text_size*2, sizeof(*text), GFP_KERNEL);
	int to_copy, not_copied, copied;
	const char *name = f->f_path.dentry->d_name.name;
	unsigned char *colors;

	int port = (long)f->private_data;
	if      (strcmp(name, "inner_colors") == 0) colors = ports[port].inner_rgb.colors;
	else if (strcmp(name, "outer_colors") == 0) colors = ports[port].outer_rgb.colors;

	to_copy = min(count, text_size*2);

	not_copied = copy_from_user(text, ubuf, to_copy);

	copied = to_copy - not_copied;

	int text_i = 0;
	for (int i = 0; i < text_size; i++) {
		unsigned int tmpc;
		sscanf(&text[text_i], "%02x", &tmpc); 
		colors[i] = tmpc;
		text_i += 2;
	}

	kfree(text);
	return copied;
}

// Module cleanup
static void __exit sl_infinity_exit(void)
{
	hid_unregister_driver(&sl_infinity_driver);
	printk(KERN_INFO "Lian li SL-Infinity hub: HID driver unloaded\n");
}

module_init(sl_infinity_init);
module_exit(sl_infinity_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("L-Connect Team");
MODULE_DESCRIPTION("Lian Li SL-Infinity USB Driver");
MODULE_VERSION("0.0.1");
