#include <b15f/b15f.h>

int main() {
	auto& drv = B15F::getInstance();

	drv.setRegister(&DDRA, 0xff);
	drv.setRegister(&PORTA, 0x0);
	drv.setRegister(&DDRA, 0x0);
}
