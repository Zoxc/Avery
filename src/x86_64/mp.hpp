#pragma once
#include "arch.hpp"

namespace MP
{
	enum EntryType
	{
		ProcessorEntry,
		BusEntry,
		IOAPICEntry,
		IOInterruptEntry,
		LocalInterruptEntry
	};

	struct Interrupt
	{
		uint8_t type;
		uint8_t interrupt_type;
		uint16_t flags;
		uint8_t source_bus_id;
		uint8_t source_bus_irq;
		uint8_t dest_apic_id;
		uint8_t dest_apic_int;
	} __attribute__((packed));

	verify_size(Interrupt, 8);

	struct IOAPIC
	{
		uint8_t type;
		uint8_t ioapic_id;
		uint8_t ioapic_version;
		uint8_t flags;
		uint32_t address;
	} __attribute__((packed));

	verify_size(IOAPIC, 8);

	struct Bus
	{
		uint8_t type;
		uint8_t bus_id;
		char bus_type[6];
	} __attribute__((packed));

	verify_size(Bus, 8);

	struct Processor
	{
		uint8_t type;
		uint8_t local_apic;
		uint8_t local_apic_version;
		uint8_t flags;
		uint32_t signature;
		uint32_t features;
		uint32_t reserved[2];
	} __attribute__((packed));

	verify_size(Processor, 20);

	struct Configuration
	{
		static const size_t signature_magic = 0x504D4350;

		uint32_t signature;
		uint16_t base_table_size;
		uint8_t spec_rev;
		uint8_t checksum;
		char oem_id[8];
		char product_id[12];
		uint32_t oem_table_address;
		uint16_t oem_table_size;
		uint16_t entry_count;
		uint32_t lapic_address;
		uint16_t extended_table_size;
		uint8_t extended_table_checksum;
		uint8_t reserved;
	} __attribute__((packed));

	verify_size(Configuration, 44);

	struct Pointer
	{
		static const size_t signature_magic = 0x5F504D5F;

		uint32_t signature;
		uint32_t config_address;
		uint8_t length;
		uint8_t spec_rev;
		uint8_t checksum;
		uint8_t features[5];
	} __attribute__((packed));

	verify_size(Pointer, 16);

	void search();
	void initialize();
};
