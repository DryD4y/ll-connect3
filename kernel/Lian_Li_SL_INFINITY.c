// SPDX-License-Identifier: GPL-2.0
/*
 * Lian Li SL Infinity Fan Hub Linux Driver
 * ENE LianLi-SL-infinity-v1.4 (VID=0x0CF2, PID=0xA102)
 *
 * Exposes:
 *   /proc/Lian_li_SL_INFINITY/Port_X/fan_speed   (write 0–100)
 *
 * Author: AI + Joey
 */

 #include <linux/module.h>
 #include <linux/kernel.h>
 #include <linux/usb.h>
 #include <linux/hid.h>
 #include <linux/proc_fs.h>
 #include <linux/uaccess.h>
 #include <linux/slab.h>
 #include <linux/delay.h>
 
 #define VENDOR_ID  0x0CF2
 #define PRODUCT_ID 0xA102
 
 struct sli_hub {
	 struct hid_device *hdev;
	 struct proc_dir_entry *procdir;
 };
 
 struct sli_port {
	 int index;  /* 0..3 */
	 struct sli_hub *hub;
 };
 
 /* send FEATURE report segment */
 static int sli_send_segment(struct hid_device *hdev, const u8 *buf, size_t len)
 {
	 int rc;
	 rc = hid_hw_raw_request(hdev, 0xE0, (u8 *)buf, len,
							 HID_FEATURE_REPORT, HID_REQ_SET_REPORT);
	 if (rc < 0)
		 return rc;
	 msleep(18);
	 return 0;
 }
 
/* set fan speed for a specific port - based on actual protocol analysis */
static int sli_set_fan_speed(struct sli_port *p, u8 duty)
{
	 struct hid_device *hdev = p->hub->hdev;
	 u8 port_num = p->index + 1;  /* Convert 0-based to 1-based port number */
	 u8 cmd[7];

	 /* Individual port control protocol discovered from PCAP analysis:
	  * Port 1: e0 20 00 <duty> 00 00 00
	  * Port 2: e0 21 00 <duty> 00 00 00  
	  * Port 3: e0 22 00 <duty> 00 00 00
	  * Port 4: e0 23 00 <duty> 00 00 00
	  */
	 cmd[0] = 0xE0;           /* Report ID */
	 cmd[1] = 0x20 + p->index; /* Port selector: 0x20, 0x21, 0x22, 0x23 */
	 cmd[2] = 0x00;           /* Reserved */
	 cmd[3] = duty;           /* Duty cycle (0-100) */
	 cmd[4] = 0x00;           /* Reserved */
	 cmd[5] = 0x00;           /* Reserved */
	 cmd[6] = 0x00;           /* Reserved */

	 pr_info("SLI: Setting port %d to %d%% (duty=0x%02x)\n",
			 port_num, duty, duty);

	 /* Send individual port control command */
	 int rc = sli_send_segment(hdev, cmd, sizeof(cmd));
	 if (rc < 0) {
		 pr_err("SLI: Port %d command failed: %d\n", port_num, rc);
		 return rc;
	 }
	 
	 pr_info("SLI: Successfully set port %d to %d%%\n", port_num, duty);
	 return 0;
}
 
 /* write handler: percentage -> duty */
 static ssize_t sli_write_fan_speed(struct file *file,
									const char __user *ubuf,
									size_t count, loff_t *ppos)
 {
	 struct sli_port *p = pde_data(file_inode(file));
	 char buf[16];
	 int val;
	 u8 duty;
	 int rc;
 
	 if (count >= sizeof(buf))
		 return -EINVAL;
	 if (copy_from_user(buf, ubuf, count))
		 return -EFAULT;
	 buf[count] = '\0';
 
	 if (kstrtoint(buf, 10, &val) < 0)
		 return -EINVAL;
	 if (val < 0) val = 0;
	 if (val > 100) val = 100;  /* Clamp to max percentage */

	 /* Direct percentage control - no RPM conversion needed */
	 duty = (u8)val;  /* Use value directly as percentage */

	 rc = sli_set_fan_speed(p, duty);
	 if (rc < 0) return rc;
 
	 pr_info("SLI: Port %d set to %d%% (duty=0x%02x)\n",
			 p->index+1, duty, duty);
	 return count;
 }
 
 static const struct proc_ops sli_proc_ops = {
	 .proc_write = sli_write_fan_speed,
 };
 
 /* HID probe */
 static int sli_probe(struct hid_device *hdev, const struct hid_device_id *id)
 {
	 struct sli_hub *hub;
	 int i;
 
	 hub = devm_kzalloc(&hdev->dev, sizeof(*hub), GFP_KERNEL);
	 if (!hub) return -ENOMEM;
	 hub->hdev = hdev;
	 hid_set_drvdata(hdev, hub);
 
	 if (hid_parse(hdev)) return -EINVAL;
	 if (hid_hw_start(hdev, HID_CONNECT_DEFAULT)) return -EINVAL;
 
	 hub->procdir = proc_mkdir("Lian_li_SL_INFINITY", NULL);
	 if (!hub->procdir) return -ENOMEM;
 
	 for (i = 0; i < 4; i++) {
		 struct sli_port *p = kzalloc(sizeof(*p), GFP_KERNEL);
		 char name[16];
		 if (!p) continue;
		 p->index = i;
		 p->hub = hub;
 
		 snprintf(name, sizeof(name), "Port_%d", i+1);
		 {
			 struct proc_dir_entry *portdir = proc_mkdir(name, hub->procdir);
			 if (portdir) {
				 proc_create_data("fan_speed", 0222, portdir,
								  &sli_proc_ops, p);
			 }
		 }
	 }
 
	 pr_info("SLI: HID device initialized\n");
	 return 0;
 }
 
 static void sli_remove(struct hid_device *hdev)
 {
	 struct sli_hub *hub = hid_get_drvdata(hdev);
	 if (hub && hub->procdir) {
		 remove_proc_subtree("Lian_li_SL_INFINITY", NULL);
	 }
	 hid_hw_stop(hdev);
	 pr_info("SLI: HID device removed\n");
 }
 
 static const struct hid_device_id sli_table[] = {
	 { HID_USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
	 { }
 };
 MODULE_DEVICE_TABLE(hid, sli_table);
 
 static struct hid_driver sli_driver = {
	 .name = "Lian_Li_SL_INFINITY",
	 .id_table = sli_table,
	 .probe = sli_probe,
	 .remove = sli_remove,
 };
 
 module_hid_driver(sli_driver);
 
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("AI + Joey");
 MODULE_DESCRIPTION("Lian Li SL Infinity Fan Hub Driver");
 