#include "common.hpp"
#include "send.hpp"
#include "receive.hpp"
#include "pack.hpp"
#include "SerialPort.hpp"

#include <bitset>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

// 4 LSBs: send
// 4 MSBs: receive

// Remember, we split our 8 bits in half, 4 for sending, 4 for receiving.
// The following three functions help with this matter.
unsigned char get_sender_bits(unsigned char channel_state) {
    return channel_state & 0xf;
}

unsigned char get_receiver_bits(unsigned char channel_state) {
    return channel_state >> 4;
}

unsigned char assemble_channel_bits(const unsigned char sender_bits, const unsigned char receiver_bits) {
    return ((sender_bits & 0xf) | (receiver_bits << 4));
}

// Utility functions to read and write byte vectors from/to filestreams.
std::vector<unsigned char> read_bytes_from_file(std::istream& file_stream, std::size_t num_bytes) {
    std::vector<char> buffer(num_bytes);

    // Read bytes from the file stream into the buffer
    file_stream.read(buffer.data(), num_bytes);

    // Resize the vector to the actual number of bytes read
    buffer.resize(file_stream.gcount());

    return std::vector<unsigned char>(buffer.begin(), buffer.end());
}

void write_bytes_to_file(std::ostream& file_stream, const std::vector<unsigned char>& bytes) {
    // Have to perform this cast... unfortunately
    std::vector<char> buffer(bytes.begin(), bytes.end());

    // Write out all the bytes
    file_stream.write(buffer.data(), bytes.size());
}

// Arbitrarily chosen number of bytes of data packed to each frame.
static const std::size_t BLOCK_SIZE = 8;

int main(int argc, char *argv[]) {

	if(argc != 2) {
		fprintf(stderr, "Usage: %s <arduino_serial_port>\n", argv[0]);
		std::exit(1);
	}

    std::istream &infile = std::cin;
    std::ostream &outfile = std::cout;

    // Open the serial ports to the arduino here...
    SerialPort serial(argv[1]);
    serial.configure();
    serial.flush();

    Sender sender;
    Receiver receiver;

    unsigned char channel_state = 0x0,
                  sender_bits = 0x0,
                  receiver_bits = 0x0;

    static std::vector<unsigned char> inframe, outframe;

    long unsigned timer = 0;

    while(true) {
        sleep(1);
        timer++;

        // Read the current channel state.
        bool receive_success = false;
        uint8_t received = serial.receive_byte(receive_success);
        if(receive_success) channel_state = received;

        // Do all our logic...
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


        sender_bits   = sender.tick(get_sender_bits(channel_state));
        receiver_bits = receiver.tick(get_receiver_bits(channel_state));

        channel_state = assemble_channel_bits(sender_bits, receiver_bits);

        std::cerr << std::bitset<8>(channel_state) << std::endl;

        // Write out the new channel state.
        // TODO: What if writing to serial fails?
        bool send_success;
        serial.send_byte(channel_state, send_success);
    }

    return 0;
}
