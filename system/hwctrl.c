// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2020 Martin Whitaker.
//
// Derived from an extract of memtest86+ lib.c:
//
// lib.c - MemTest-86  Version 3.4
//
// Released under version 2 of the Gnu Public License.
// By Chris Brady

#include "stddef.h"

#include "boot.h"
#include "bootparams.h"
#include "efi.h"

#include "hwctrl.h"
#include "io.h"

#include "unistd.h"

//------------------------------------------------------------------------------
// Public Functions
//------------------------------------------------------------------------------

extern efi_info_t saved_efi_info;

static void efi_reset(uint8_t reset_type)
{
    static efi_runtime_services_t *rs_table = NULL;

#ifdef __x86_64__
    if (saved_efi_info.loader_signature == EFI64_LOADER_SIGNATURE) {
        uintptr_t system_table_addr = ((uintptr_t)saved_efi_info.sys_tab_hi << 32) | saved_efi_info.sys_tab;
        if (system_table_addr != 0) {
            efi64_system_table_t *sys_table = (efi64_system_table_t *)system_table_addr;
            rs_table = (efi_runtime_services_t *)sys_table->runtime_services;
            rs_table->reset_system(reset_type, 0, 0);
        }
    }
#endif
    if (saved_efi_info.loader_signature == EFI32_LOADER_SIGNATURE) {
        uintptr_t system_table_addr = saved_efi_info.sys_tab;
        if (system_table_addr != 0) {
            efi32_system_table_t *sys_table = (efi32_system_table_t *)system_table_addr;
            rs_table = (efi_runtime_services_t *)(uintptr_t)sys_table->runtime_services;
            rs_table->reset_system(reset_type, 0, 0);
        }
    }
}

void reboot(void)
{
    // Use cf9 method as first try
    uint8_t cf9 = inb(0xcf9) & ~6;
    outb(cf9|2, 0xcf9); // Request hard reset
    usleep(50);
    outb(cf9|6, 0xcf9); // Actually do the reset
    usleep(50);

    // If we have UEFI, try EFI reset service
    if(saved_efi_info.loader_signature) {
        efi_reset(EFI_RESET_COLD);
        usleep(1000000);
    }

     // Still here? try the PORT 0x64 method
    outb(0xfe, 0x64);
    usleep(150000);

    if(!saved_efi_info.loader_signature) {
        // In last resort, (very) obsolete reboot method using BIOS
        *((uint16_t *)0x472) = 0x1234;
    }
}

void floppy_off()
{
    // Stop the floppy motor.
    outb(0x8, 0x3f2);
}

void cursor_off()
{
    // Set HW cursor off screen.
    outb(0x0f, 0x3d4);
    outb(0xff, 0x3d5);

    outb(0x0e, 0x3d4);
    outb(0xff, 0x3d5);
}
