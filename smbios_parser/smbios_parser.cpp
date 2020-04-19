#include <windows.h>
#include "smbios.hpp"

using namespace smbios;

int main()
{
	// Query size of SMBIOS data.
	const DWORD smbios_data_size = GetSystemFirmwareTable('RSMB', 0, nullptr, 0);

	// Allocate memory for SMBIOS data
	const auto heap_handle = GetProcessHeap();
	const auto smbios_data = static_cast<raw_smbios_data*>(HeapAlloc(heap_handle, 0,
	                                                                 static_cast<size_t>(smbios_data_size)));
	if (!smbios_data)
	{
		return 0;
	}

	// Retrieve the SMBIOS table
	const DWORD bytes_written = GetSystemFirmwareTable('RSMB', 0, smbios_data, smbios_data_size);
	if (bytes_written != smbios_data_size)
	{
		return 0;
	}

	// Process the SMBIOS data and free the memory under an exit label
	parser meta;
	const auto buff = smbios_data->smbios_table_data;
	const auto buff_size = static_cast<size_t>(smbios_data_size);

	meta.feed(buff, buff_size);

	for (auto& header : meta.headers)
	{
		string_array_t strings;
		parser::extract_strings(header, strings);

		switch (header->type)
		{
		case types::baseboard_info:
			{
				const auto x = reinterpret_cast<baseboard_info*>(header);

				if (x->length == 0)
					break;

				printf("ManufacturerName  %s\n", strings[x->manufacturer_name]);
				printf("ProductName       %s\n", strings[x->product_name]);
				printf("\n");
			}
			break;

		case types::bios_info:
			{
				const auto x = reinterpret_cast<bios_info*>(header);

				if (x->length == 0)
					break;

				printf("vendor  %s\n", strings[x->vendor]);
				printf("version       %s\n", strings[x->version]);
				printf("\n");
			}
			break;


		case types::memory_device:
			{
				const auto x = reinterpret_cast<mem_device*>(header);

				if (x->total_width == 0)
					break;

				printf("Memory device %s (%s):\n", strings[x->device_locator], strings[x->bank_locator]);
				printf("  Data width:    %d bit\n", x->data_width);
				printf("  Size:          %d M\n", x->size);
				printf("  Speed:         %d MHz\n", x->speed);
				printf("  Clock speed:   %d MHz\n", x->clock_speed);
				printf("  Manufacturer:  %s\n", strings[x->manufacturer]);
				printf("  S/N:           %s\n", strings[x->serial_number]);
				printf("  Tag number:    %s\n", strings[x->assert_tag]);
				printf("  Part number:   %s\n", strings[x->part_number]);
				printf("\n");
			}
			break;
		case types::processor_info:
			{
				const auto x = reinterpret_cast<proc_info*>(header);

				if (x->length == 0)
					break;
				printf("  manufacturer     %s\n", strings[x->manufacturer]);
				printf("  ID               %ld\n", static_cast<long>(x->id));
				printf("  threads               %hhu\n", x->threads);
				printf("  cores               %hhu\n", x->cores);
			}
			break;

		default: ;
		}
	}

	HeapFree(heap_handle, 0, smbios_data);

	return 0;
}
