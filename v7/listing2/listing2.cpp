# include <iostream>
# include <b15f/b15f.h>
using namespace std;
int main ()
{
	B15F& drv = B15F::getInstance () ; //drv wird ein Objekt einer Klasse
	drv . setRegister (& DDRA ,0x00) ;
	//Bit 7-1 Eingabe,Bit 0 Ausgabe
	while (1)
	{
		// Lesen
		while (1) {
			cout << ((int)drv.getRegister(&PINA)) << endl;
			drv.delay_ms(10);
		}
	}
}
