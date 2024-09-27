#include "io/mailbox.h"

#include "io/memoryMappedIO.h"
#include "lib/memory.h"

enum
{
	MAILBOX_STATUS_EMPTY = 0x40000000,
	MAILBOX_STATUS_FULL = 0x80000000
};

void mailbox_flush()
{
	while (!(mmio_read(MBOX_STATUS) & MAILBOX_STATUS_EMPTY))
	{
		mmio_read(MBOX_READ);
	}
}

uint32_t mailbox_read(uint8_t channel)
{
    // Loop until we receive something from the requested channel
	while (1)
	{
        // Wait for data
		while (mmio_read(MBOX_STATUS) & MAILBOX_STATUS_EMPTY) { }

		// Read the data
		uint32_t data = mmio_read(MBOX_READ);
		uint8_t readChannel = data & 0xF;

		if (readChannel == channel)
			return data & ~0xF;
	}
}

void mailbox_write(uint32_t data, uint8_t channel)
{
    // Wait for space in the FIFO
    while ( (mmio_read(MBOX_STATUS) & MAILBOX_STATUS_FULL) != 0) { }

	// Write the value to the requested channel
	mmio_write(MBOX_WRITE, (data & ~0xF) | channel);
}

uint32_t mailbox_write_read(uint32_t data, uint8_t channel)
{
	peripheral_entry();

	mailbox_flush();

	mailbox_write(data, channel);

	uint32_t resault = mailbox_read(channel);

	peripheral_exit();

	return resault;
}