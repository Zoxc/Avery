#pragma once
#include "arch.hpp"

namespace ACPI
{
	struct RSDP
	{
		static const uint64_t signature_magic;

		uint64_t signature;
		uint8_t checksum;
		char oem[6];
		uint8_t revision;
		uint32_t address;
	} __attribute__((packed));

	struct SDT
	{
		union
		{
			uint32_t signature;
			char signature_string[4];
		};
		uint32_t length;
		uint8_t revision;
		uint8_t checksum;
		char oem_id[6];
		char oem_table_id[8];
		uint32_t oem_revision;
		uint32_t creator_id;
		uint32_t creator_revision;
	} __attribute__((packed));

	struct MADT:
		public SDT
	{
		static const uint32_t signature_magic;

		uint32_t local_interrupt_controller;
		uint32_t flags;

		enum EntryType
		{
			ProcessorLocalAPICEntry,
			IOAPICEntry,
			InterruptSourceOverrideEntry,
			NMISourceEntry,
			LocalAPICNMIEntry,
			LocalAPICAddressOverrideEntry
		};

		struct Entry
		{
			uint8_t type;
			uint8_t length;
		} __attribute__((packed));

		struct ProcessorLocalAPIC:
			public Entry
		{
			uint8_t processor_id;
			uint8_t apic_id;
			uint32_t flags;

			static const size_t flag_enabled = 1;
		} __attribute__((packed));

		struct IOAPIC:
			public Entry
		{
			uint8_t id;
			uint8_t reserved;
			uint32_t address;
			uint32_t global_int_start;
		} __attribute__((packed));

		struct LocalAPICAddressOverride:
			public Entry
		{
			uint16_t reserved;
			uint64_t apic_address;
		} __attribute__((packed));

		struct InterruptSourceOverride:
			public Entry
		{
			uint8_t bus;
			uint8_t source;
			uint32_t global_int;
			uint16_t flags;
		} __attribute__((packed));
	} __attribute__((packed));

	struct RSDT:
		public SDT
	{
		static const uint32_t signature_magic;

		uint32_t tables[1]; // Variable length
	} __attribute__((packed));

	void set_table(addr_t table);
	void initialize();
};
