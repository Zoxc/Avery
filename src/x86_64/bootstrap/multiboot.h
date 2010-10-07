#pragma once
#include "common.h"

#define MULTIBOOT_HEADER_FLAG_PAGE_ALIGN (1 << 0)
#define MULTIBOOT_HEADER_FLAG_MEMORY_INFO (1 << 1)

#define MULTIBOOT_MAGIC 0x2BADB002

#define MULTIBOOT_HEADER_MAGIC 0x1BADB002

#define MULTIBOOT_CHECKSUM(flags) (-(MULTIBOOT_HEADER_MAGIC + (flags)))

struct multiboot_header
{
	uint32_t magic;
	uint32_t flags;
	uint32_t checksum;
} __attribute__((aligned (4))) __attribute__((packed));
