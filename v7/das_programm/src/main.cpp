#include "common.hpp"
#include "send.hpp"
#include "receive.hpp"
#include "pack.hpp"

#include <vector>
#include <string>
#include <fstream>
#include <b15f/b15f.h>

// Remember, we split our 8 bits in half, 4 for sending, 4 for receiving.
// The following three functions help with this matter.
unsigned char get_sender_bits(unsigned char channel_state, unsigned int sender_offset) {
    return (channel_state >> (4 * sender_offset)) & 0xff;
}

unsigned char get_receiver_bits(unsigned char channel_state, unsigned int sender_offset) {
    return (channel_state >> (4 * (1 - sender_offset))) & 0xff;
}

unsigned char assemble_channel_bits(unsigned char sender_bits, unsigned char receiver_bits, unsigned int sender_offset) {
    unsigned char sender_byte   = sender_bits   << (sender_offset * 4);
    unsigned char receiver_byte = receiver_bits << ((1 - sender_offset) * 4);

    return sender_byte | receiver_byte;
}

// Utility functions to read and write byte vectors from/to filestreams.
std::vector<unsigned char> read_bytes_from_file(std::ifstream& file_stream, std::size_t num_bytes) {
    std::vector<char> buffer(num_bytes);

    // Read bytes from the file stream into the buffer
    file_stream.read(buffer.data(), num_bytes);

    // Resize the vector to the actual number of bytes read
    buffer.resize(file_stream.gcount());

    return std::vector<unsigned char>(buffer.begin(), buffer.end());
}

void write_bytes_to_file(std::ofstream& file_stream, const std::vector<unsigned char>& bytes) {
    // Have to perform this cast... unfortunately
    std::vector<char> buffer(bytes.begin(), bytes.end());

    // Write out all the bytes
    file_stream.write(buffer.data(), bytes.size());
}

// Arbitrarily chosen number of bytes of data packed to each frame.
static const std::size_t BLOCK_SIZE = 4096;

int main(int argc, char *argv[]) {

	if(argc != 4) {
		fprintf(stderr, "Usage: %s <infile> <outfile> <sender_offset>\n", argv[0]);
		std::exit(1);
	}

    // What channel to send on?
    unsigned char sender_offset = std::stoi(argv[3]);
	if(!(sender_offset == 1 || sender_offset == 0)) {
		fprintf(stderr, "Sender offset must be 0 or 1!\n");
		std::exit(1);
	}

    std::ifstream infile(argv[1]);
    std::ofstream outfile(argv[2]);

    // B15F setup here
	B15F& drv = B15F::getInstance();
	unsigned char ddra_mask = (sender_offset == 0 ? 0b01001011 : 0b10110100);
	drv.setRegister(&DDRA, ddra_mask);

    Sender sender;
    Receiver receiver;

    unsigned char channel_state = 0x0,
                  sender_bits = 0x0,
                  receiver_bits = 0x0;

    static std::vector<unsigned char> inframe, outframe;

    while(true) {

		channel_state = drv.getRegister(&PORTA);

        if(sender.need_frame()) {
            auto data = read_bytes_from_file(infile, BLOCK_SIZE);
            pack_frame(data, inframe);
            sender.read_frame(inframe);

            // TODO: What if end of infile is reached?
        }
        if(receiver.frame_available()) {
            std::vector<unsigned char> data;
            receiver.frame_pull(outframe);
            unpack_frame(outframe, data);
            write_bytes_to_file(outfile, data);

            // TODO: What if end of outfile is reached?
        }

        sender_bits   = sender.tick(get_sender_bits(channel_state, sender_offset));
        receiver_bits = receiver.tick(get_receiver_bits(channel_state, sender_offset));

        channel_state = assemble_channel_bits(sender_bits, receiver_bits, sender_offset);

		drv.setRegister(&PORTA, channel_state);
    }

    return 0;
}
