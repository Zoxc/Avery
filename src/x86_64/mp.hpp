#pragma once
#include "arch.hpp"

namespace MP
{
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
	};

	struct Pointer
	{
		static const size_t signature_magic = 0x5F504D5F;

		uint32_t signature;
		uint32_t config_address;
		uint8_t length;
		uint8_t spec_rev;
		uint8_t checksum;
		uint8_t features[5];
	};

	void search();
	void initialize();
};
