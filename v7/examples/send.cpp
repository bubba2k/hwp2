#include <stdio.h>
#include <b15f/b15f.h>
#include <stdint.h>
#include <queue>

void fprint_byte_bits(FILE *file, uint8_t num) {
	for(unsigned i = 0; i < 8; ++i) {
		// print last bit and shift left.
		fprintf(file, "%u ", num & 0x1 ? 1 : 0);
		num = num << 1;
	}
}

int main(int argc, char *argv[]) {
	B15F& drv = B15F::getInstance();

	/* Lower 4 pins send, others recv */
	drv.setRegister(&DDRA, 0xf);
	uint8_t recv_clk_mask  = 0b00001000;
	uint8_t recv_data_mask = 0b00000111;
	uint8_t send_clk_mask  = 0b10000000;
	uint8_t send_data_mask = 0b01110000;

	uint8_t port_state = 0;
	uint8_t recv_clk_state = 0;

	uint64_t recv_buffer = 0;
	uint64_t recv_buffer_index = 0;

	uint8_t send_clk_state = 0;
	std::queue<bool> send_bit_queue;

	while(1) {
		/* Receive */
		port_state = drv.getRegister(&PINA);

		if(recv_clk_state != (recv_clk_mask & port_state)) {
			recv_clk_state = (recv_clk_mask & port_state);

			uint8_t data_bits = (port_state & recv_data_mask);

			recv_buffer = recv_buffer << 3;
			recv_buffer |= data_bits;
			recv_buffer_index += 3;
			fprintf(stderr, "Received %x\n", data_bits);
		}
		else {
			fprintf(stderr, "Did not receive anything!\n");
		}

		/* Print received bytes to stdout if available */
		if(recv_buffer_index <= 8) {
			uint8_t oldest_byte_begin = recv_buffer_index - 8;
			uint8_t byte = (recv_buffer >> oldest_byte_begin) & 0xff;
			recv_buffer_index -= 8;

			putc(byte, stdout);
		}

		/* Send */
		/* Request new byte from in stream */
		if(send_bit_queue.empty()) {	
			uint8_t byte = fgetc(stdin);

			/* Enqueue all bits */
			for(uint8_t i = 0; i < 8; i++)
				send_bit_queue.push(byte & (0x1 << i));
		}

		/* Send bits */
		uint8_t send_bits = 0;
		uint8_t data_bits = 0;
		/* Get 3 bits from queue */
		for(uint8_t i = 0; i < 3; i++) {
			data_bits = data_bits << 1;
			data_bits |= send_bit_queue.front() & 0x1;
			send_bit_queue.pop();
		}

		/* Flip clk signal, set databits */
		send_clk_state = ~send_clk_state;
		send_bits |= (send_clk_state << 8);
		send_bits |= (data_bits << 4);

		drv.setRegister(&PORTA, send_bits);

		fprintf(stderr, "Sending ");
		fprint_byte_bits(stderr, data_bits);
		fputc('\n', stderr);

		sleep(1);
	}
}
