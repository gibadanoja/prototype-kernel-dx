/* arch/arm/mach-msm/proc_comm.c
 *
 * Copyright (C) 2007-2008 Google, Inc.
 * Copyright (c) 2009-2012, Code Aurora Forum. All rights reserved.
 * Author: Brian Swetland <swetland@google.com>
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

#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <mach/msm_iomap.h>
#include <mach/system.h>
#include <mach/proc_comm.h>

#include "smd_private.h"

static inline void notify_other_proc_comm(void)
{
	
	wmb();
#if defined(CONFIG_ARCH_MSM7X30)
	__raw_writel(1 << 6, MSM_APCS_GCC_BASE + 0x8);
#elif defined(CONFIG_ARCH_MSM8X60)
	__raw_writel(1 << 5, MSM_GCC_BASE + 0x8);
#else
	__raw_writel(1, MSM_CSR_BASE + 0x400 + (6) * 4);
#endif
}

#define APP_COMMAND 0x00
#define APP_STATUS  0x04
#define APP_DATA1   0x08
#define APP_DATA2   0x0C

#define MDM_COMMAND 0x10
#define MDM_STATUS  0x14
#define MDM_DATA1   0x18
#define MDM_DATA2   0x1C

static DEFINE_SPINLOCK(proc_comm_lock);
static int msm_proc_comm_disable;

static int proc_comm_wait_for(unsigned addr, unsigned value)
{
	while (1) {
		
		mb();
		if (readl_relaxed(addr) == value)
			return 0;

		if (smsm_check_for_modem_crash())
			return -EAGAIN;

		udelay(5);
	}
}

void msm_proc_comm_reset_modem_now(void)
{
	unsigned base = (unsigned)MSM_SHARED_RAM_BASE;
	unsigned long flags;

	spin_lock_irqsave(&proc_comm_lock, flags);

again:
	if (proc_comm_wait_for(base + MDM_STATUS, PCOM_READY))
		goto again;

	writel_relaxed(PCOM_RESET_MODEM, base + APP_COMMAND);
	writel_relaxed(0, base + APP_DATA1);
	writel_relaxed(0, base + APP_DATA2);

	spin_unlock_irqrestore(&proc_comm_lock, flags);

	
	wmb();
	notify_other_proc_comm();

	return;
}
EXPORT_SYMBOL(msm_proc_comm_reset_modem_now);

int msm_proc_comm(unsigned cmd, unsigned *data1, unsigned *data2)
{
	unsigned base = (unsigned)MSM_SHARED_RAM_BASE;
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&proc_comm_lock, flags);

	if (msm_proc_comm_disable) {
		ret = -EIO;
		goto end;
	}


again:
	if (proc_comm_wait_for(base + MDM_STATUS, PCOM_READY))
		goto again;

#if (defined(CONFIG_MACH_PRIMODS) || defined(CONFIG_MACH_PROTOU) || defined(CONFIG_MACH_DUMMY) || defined(CONFIG_MACH_DUMMY) || defined(CONFIG_MACH_DUMMY) || defined(CONFIG_MACH_DUMMY) || defined(CONFIG_MACH_DUMMY) || defined(CONFIG_MACH_DUMMY))
       if ((cmd == PCOM_CLKCTL_RPC_DISABLE) && (*data1 == 0x1F)) {
               
               ret = 0;
               goto end;
       }
#endif
	writel_relaxed(cmd, base + APP_COMMAND);
	writel_relaxed(data1 ? *data1 : 0, base + APP_DATA1);
	writel_relaxed(data2 ? *data2 : 0, base + APP_DATA2);

	
	wmb();
	notify_other_proc_comm();

	if (proc_comm_wait_for(base + APP_COMMAND, PCOM_CMD_DONE))
		goto again;

	if (readl_relaxed(base + APP_STATUS) == PCOM_CMD_SUCCESS) {
		if (data1)
			*data1 = readl_relaxed(base + APP_DATA1);
		if (data2)
			*data2 = readl_relaxed(base + APP_DATA2);
		ret = 0;
	} else {
		ret = -EIO;
	}

	writel_relaxed(PCOM_CMD_IDLE, base + APP_COMMAND);

	switch (cmd) {
	case PCOM_RESET_CHIP:
	case PCOM_RESET_CHIP_IMM:
	case PCOM_RESET_APPS:
#if 1
		
#else
		msm_proc_comm_disable = 1;
		printk(KERN_ERR "msm: proc_comm: proc comm disabled\n");
#endif
		break;
	}
end:
	
	wmb();
	spin_unlock_irqrestore(&proc_comm_lock, flags);
	return ret;
}
EXPORT_SYMBOL(msm_proc_comm);
