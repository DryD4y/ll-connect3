// SPDX-License-Identifier: GPL-2.0
/*
 * ENE / Lian Li SL-Infinity Hub – Minimal Fan Control HID Driver (segmented E0 over Control SET_REPORT)
 *
 * Tested target: VID:PID 0x0CF2:0xA102 (ENE LianLi-SL-infinity-v1.4)
 *
 * Features:
 *  - Exposes per-port fan speed under /proc/Lian_li_SL_INFINITY/Port_{1..4}/fan_speed (0..100)
 *  - Sends per-port commands using "Windows-style" segmented 0xE0 reports:
 *      * Build the full 0xE0 packet (12 bytes for Port Select, 17 bytes for Set Duty)
 *      * Split into 6-byte chunks AFTER the ReportID (0xE0), prefix each with 0xE0
 *      * Send each chunk via HID Control SET_REPORT (reportnum=0xE0), with ONLY the 6 bytes as payload
 *        (Linux raw_request expects the buffer WITHOUT the ReportID when reportnum is passed)
 *  - Falls back to interrupt OUT if available, but works when it is not (common on this hub).
 *
 * Notes:
 *  - RPM readout is stubbed (returns 0). This driver focuses on reliable per-port duty control.
 *  - Port selectors use INDEX (01 00, 02 00, 03 00, 04 00), not bitmasks.
 *
 * Build:
 *   make
 * Load:
 *   sudo insmod Lian_Li_SL_INFINITY.ko
 * Use:
 *   echo 60 | sudo tee /proc/Lian_li_SL_INFINITY/Port_1/fan_speed
 *   echo 80 | sudo tee /proc/Lian_li_SL_INFINITY/Port_2/fan_speed
 *
 * If hid-generic binds first, you can unbind and bind this driver:
 *   echo 0003:0CF2:A102.0001 | sudo tee /sys/bus/hid/drivers/hid-generic/unbind
 *   echo 0003:0CF2:A102.0001 | sudo tee /sys/bus/hid/drivers/Lian_Li_SL_INFINITY/bind
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/hid.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/seq_file.h>

#define DRV_NAME        "Lian_Li_SL_INFINITY"
#define PROC_ROOT_NAME  "Lian_li_SL_INFINITY"
#define MAX_PORTS       4

#define ENE_VID 0x0CF2
#define ENE_PID 0xA102

struct sli_port {
	int                     port_id;   /* 1..4 */
	struct proc_dir_entry  *dir;
	struct proc_dir_entry  *pde_speed;
	struct proc_dir_entry  *pde_rpm;
	struct sli_dev         *parent;
	int                     last_duty; /* cached duty (0..100), -1 unknown */
};

struct sli_dev {
	struct hid_device      *hdev;
	struct proc_dir_entry  *proc_root;
	struct sli_port         ports[MAX_PORTS];
};

/* -------------------- Robust segmented E0 sender -------------------- */

static int sli_send_e0_segment_ctrl(struct hid_device *hdev, const u8 *data6, size_t data_len6)
/* Send one 7-byte report over CONTROL: reportID=0xE0 + 6 data bytes.
 * hid_hw_raw_request() expects the data buffer WITHOUT the ReportID, and 'reportnum' carries the ReportID.
 */
{
	u8 tmp[6] = {0,0,0,0,0,0};
	size_t copy = min((size_t)6, data_len6);
	int rc;

	if (!data6 || data_len6 == 0)
		return 0;

	memcpy(tmp, data6, copy);

	rc = hid_hw_raw_request(hdev,
				0xE0,                /* reportnum (ReportID) */
				tmp,                 /* data (NO ReportID)   */
				sizeof(tmp),         /* must be 6            */
				HID_OUTPUT_REPORT,   /* type                 */
				HID_REQ_SET_REPORT); /* request              */
	return rc;
}

static int sli_send_e0_segment_try_paths(struct hid_device *hdev, const u8 seg7[7])
/* Try interrupt OUT with a 7-byte buffer (with ReportID). If missing, fall back to control SET_REPORT. */
{
	int rc;

	rc = hid_hw_output_report(hdev, (u8 *)seg7, 7);
	if (rc >= 0) return 0;

	/* Log once for visibility; rc often -ENOSYS/-EINVAL on devices lacking int-out */
	hid_info(hdev, "OUTPUT (int-out) failed rc=%d, trying control SET_REPORT (len=7, id=0xe0)\n", rc);
	return sli_send_e0_segment_ctrl(hdev, &seg7[1], 6);
}

static int sli_send_e0_full_segmented(struct hid_device *hdev, const u8 *full, size_t flen)
/* full[0] must be 0xE0, flen = 12 (select) or 17 (duty). Split AFTER 0xE0 into 6-byte chunks, each framed as 0xE0 + 6 bytes. */
{
	size_t idx = 1; /* start right AFTER 0xE0 */
	u8 seg[7];
	int rc;

	if (flen < 2 || !full || full[0] != 0xE0)
		return -EINVAL;

	while (idx < flen) {
		size_t left = flen - idx;
		size_t take = min((size_t)6, left);

		memset(seg, 0x00, sizeof(seg));
		seg[0] = 0xE0;
		memcpy(&seg[1], &full[idx], take);

		hid_info(hdev, "E0 seg frame: %*ph\n", (int)sizeof(seg), seg);

		rc = sli_send_e0_segment_try_paths(hdev, seg);
		if (rc < 0) return rc;

		msleep(15); /* pacing required for this hub */
		idx += take;
	}
	return 0;
}

/* -------------------- High-level packets (full) -------------------- */

static const u8 sel_map[4][2] = {
	{0x01, 0x00}, /* Port 1 (INDEX, not bitmask) */
	{0x02, 0x00}, /* Port 2 */
	{0x03, 0x00}, /* Port 3 */
	{0x04, 0x00}, /* Port 4 */
};

static int sli_port_select_full(struct hid_device *hdev, int port)
{
	u8 full[12];

	if (port < 1) port = 1;
	if (port > MAX_PORTS) port = MAX_PORTS;

	full[0]  = 0xE0;
	full[1]  = 0x01;
	full[2]  = sel_map[port-1][0];  /* 01 02 03 04 */
	full[3]  = sel_map[port-1][1];  /* 00 */
	full[4]  = 0x41;
	full[5]  = 0x00;
	full[6]  = 0x44;
	full[7]  = 0x00;
	full[8]  = 0x00;
	full[9]  = 0x00;
	full[10] = 0x06;
	full[11] = 0x00;

	hid_info(hdev, "Port Select (len=12) P%d: %*ph\n", port, (int)sizeof(full), full);
	return sli_send_e0_full_segmented(hdev, full, sizeof(full));
}

static int sli_set_duty_full(struct hid_device *hdev, int duty_pct)
{
	u8 full[17];

	if (duty_pct < 0)   duty_pct = 0;
	if (duty_pct > 100) duty_pct = 100;

	full[0]  = 0xE0;
	full[1]  = 0x20;
	full[2]  = 0x00;
	full[3]  = (u8)duty_pct;   /* <duty> in the 4th byte overall */
	full[4]  = 0x00;
	full[5]  = 0x00;
	full[6]  = 0x00;
	full[7]  = 0x00;
	full[8]  = 0x4C;
	full[9]  = 0x00;
	full[10] = 0x00;
	full[11] = 0x00;
	full[12] = 0x06;
	full[13] = 0x00;
	full[14] = 0x00;
	full[15] = 0x00;
	full[16] = 0x3C;

	hid_info(hdev, "Set Duty (len=17) %d%%: %*ph\n", duty_pct, (int)sizeof(full), full);
	return sli_send_e0_full_segmented(hdev, full, sizeof(full));
}

static int sli_set_port_pct(struct hid_device *hdev, int port, int pct)
{
	int rc;
	rc = sli_port_select_full(hdev, port);
	if (rc < 0) return rc;
	msleep(15);
	rc = sli_set_duty_full(hdev, pct);
	if (rc < 0) return rc;
	msleep(15);
	return 0;
}

/* -------------------- /proc interface -------------------- */

static ssize_t sli_write_fan_speed(struct file *file, const char __user *ubuf,
				   size_t len, loff_t *offp)
{
	struct sli_port *p = PDE_DATA(file_inode(file));
	char kbuf[32];
	long val;
	int rc;

	if (!p || !p->parent) return -ENODEV;
	if (len >= sizeof(kbuf)) len = sizeof(kbuf) - 1;
	if (copy_from_user(kbuf, ubuf, len)) return -EFAULT;
	kbuf[len] = '\0';

	rc = kstrtol(kbuf, 10, &val);
	if (rc) return rc;

	if (val < 0) val = 0;
	if (val > 100) val = 100;

	hid_info(p->parent->hdev, "proc: set Port_%d fan_speed=%ld%%\n", p->port_id, val);

	rc = sli_set_port_pct(p->parent->hdev, p->port_id, (int)val);
	if (rc < 0) {
		hid_err(p->parent->hdev, "set_port_pct failed rc=%d\n", rc);
		return rc;
	}
	p->last_duty = (int)val;
	return len;
}

static ssize_t sli_read_rpm(struct file *file, char __user *ubuf, size_t len, loff_t *offp)
{
	/* Stub; no telemetry parsing yet. */
	char buf[32];
	int n;
	struct sli_port *p = PDE_DATA(file_inode(file));
	if (!p || !p->parent) return -ENODEV;
	n = scnprintf(buf, sizeof(buf), "%d\n", 0);
	return simple_read_from_buffer(ubuf, len, offp, buf, n);
}

static int sli_status_show(struct seq_file *m, void *v)
{
	struct sli_dev *sdev = m->private;
	int i;
	seq_puts(m, "driver=Lian_Li_SL_INFINITY\n");
	seq_printf(m, "vid=0x%04x pid=0x%04x\n", ENE_VID, ENE_PID);
	seq_puts(m, "transport=control_set_report_segmented_e0\n");
	seq_puts(m, "packet_format=full_12_17_segmented6\n");
	for (i = 0; i < MAX_PORTS; i++) {
		int duty = sdev->ports[i].last_duty;
		seq_printf(m, "port_%d: duty=%d rpm=%d\n", i+1, duty, 0);
	}
	return 0;
}

static int sli_status_open(struct inode *inode, struct file *file)
{
	return single_open(file, sli_status_show, PDE_DATA(inode));
}

static const struct proc_ops sli_proc_speed_fops = {
	.proc_write = sli_write_fan_speed,
};

static const struct proc_ops sli_proc_rpm_fops = {
	.proc_read  = sli_read_rpm,
};

static const struct proc_ops sli_proc_status_fops = {
	.proc_open  = sli_status_open,
	.proc_read  = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static int sli_create_proc(struct sli_dev *sdev)
{
	int i;
	char name[64];
	struct proc_dir_entry *root;

	root = proc_mkdir(PROC_ROOT_NAME, NULL);
	if (!root) return -ENOMEM;
	sdev->proc_root = root;

	/* status */
	if (!proc_create_data("status", 0444, root, &sli_proc_status_fops, sdev))
		return -ENOMEM;

	for (i = 0; i < MAX_PORTS; i++) {
		struct proc_dir_entry *dir;
		snprintf(name, sizeof(name), "Port_%d", i+1);
		dir = proc_mkdir(name, root);
		if (!dir) return -ENOMEM;

		sdev->ports[i].dir      = dir;
		sdev->ports[i].port_id  = i+1;
		sdev->ports[i].parent   = sdev;
		sdev->ports[i].last_duty = -1;

		if (!proc_create_data("fan_speed", 0222, dir, &sli_proc_speed_fops, &sdev->ports[i]))
			return -ENOMEM;
		if (!proc_create_data("rpm", 0444, dir, &sli_proc_rpm_fops, &sdev->ports[i]))
			return -ENOMEM;

		hid_info(sdev->hdev, "Created /proc/%s/%s/fan_speed for port %d\n",
		         PROC_ROOT_NAME, name, i+1);
	}
	return 0;
}

static void sli_remove_proc(struct sli_dev *sdev)
{
	int i;
	if (!sdev->proc_root) return;
	for (i = 0; i < MAX_PORTS; i++) {
		if (sdev->ports[i].dir) {
			proc_remove(sdev->ports[i].dir);
			sdev->ports[i].dir = NULL;
		}
	}
	proc_remove(sdev->proc_root);
	sdev->proc_root = NULL;
}

/* -------------------- HID glue -------------------- */

static int sli_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
	int rc;
	struct sli_dev *sdev;

	hid_info(hdev, DRV_NAME ": HID device connected\n");

	rc = hid_parse(hdev);
	if (rc) {
		hid_err(hdev, "hid_parse failed rc=%d\n", rc);
		return rc;
	}
	rc = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (rc) {
		hid_err(hdev, "hid_hw_start failed rc=%d\n", rc);
		return rc;
	}

	sdev = devm_kzalloc(&hdev->dev, sizeof(*sdev), GFP_KERNEL);
	if (!sdev) {
		rc = -ENOMEM;
		goto stop_hw;
	}
	sdev->hdev = hdev;
	hid_set_drvdata(hdev, sdev);

	/* Optional init sequences; these two reflect your logs */
	{
		u8 init1[7] = {0xE0, 0x50, 0x00, 0x01, 0x00, 0x00, 0x00};
		u8 init2[7] = {0xE0, 0x10, 0x00, 0x60, 0x01, 0x00, 0x00};
		hid_info(hdev, DRV_NAME ": init seq #1: %*ph\n", (int)sizeof(init1), init1);
		sli_send_e0_segment_try_paths(hdev, init1);
		msleep(15);
		hid_info(hdev, DRV_NAME ": init seq #2: %*ph\n", (int)sizeof(init2), init2);
		sli_send_e0_segment_try_paths(hdev, init2);
		msleep(15);
	}

	rc = sli_create_proc(sdev);
	if (rc) {
		hid_err(hdev, "proc setup failed rc=%d\n", rc);
		goto stop_hw;
	}

	hid_info(hdev, DRV_NAME ": initialized successfully\n");
	return 0;

stop_hw:
	hid_hw_stop(hdev);
	return rc;
}

static void sli_remove(struct hid_device *hdev)
{
	struct sli_dev *sdev = hid_get_drvdata(hdev);
	hid_info(hdev, DRV_NAME ": removing\n");
	if (sdev) {
		sli_remove_proc(sdev);
	}
	hid_hw_stop(hdev);
}

static const struct hid_device_id sli_table[] = {
	{ HID_USB_DEVICE(ENE_VID, ENE_PID) },
	{ }
};
MODULE_DEVICE_TABLE(hid, sli_table);

static struct hid_driver sli_driver = {
	.name     = DRV_NAME,
	.id_table = sli_table,
	.probe    = sli_probe,
	.remove   = sli_remove,
};

module_hid_driver(sli_driver);

MODULE_AUTHOR("ChatGPT (drop-in) – tailored for your ENE Lian Li hub");
MODULE_DESCRIPTION("ENE / Lian Li SL-Infinity Minimal Fan Control (segmented E0 over Control SET_REPORT)");
MODULE_LICENSE("GPL");