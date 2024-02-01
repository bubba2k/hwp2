#include <iostream>
#include <b15f/b15f.h>
#include <bitset>

int main(int argc, char *argv[]) {
	B15F& drv = B15F::getInstance();
	unsigned char channel_state = 0x0;

	while(1) {
		// Reset B15F pins before transmission begin
		channel_state = drv.getRegister(&PINA);
		std::cout << std::bitset<8>(channel_state) << std::endl;
	}

	return 0;
}
