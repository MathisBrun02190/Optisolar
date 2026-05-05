#include <iostream>
#include <vector>
#include <pigpio.h>

const std::vector<int> RELAIS_PINS = { 20 };

int main() {
	// 1. Initialisation
	if (gpioInitialise() < 0) {
		std::cerr << "Erreur d'initialisation pigpio" << std::endl;
		return 1;
	}

	// 2. Configuration des broches en mode SORTIE
	for (int pin : RELAIS_PINS) {
		gpioSetMode(pin, PI_OUTPUT);
	}

	std::cout << "Démarrage : les relais s'allument et s'éteignent en boucle." << std::endl;

	while (true) {
		// Allumer toutes les Relais
		for (int pin : RELAIS_PINS) {
			gpioWrite(pin, 1);
		}

		std::cout << "Relais allumés." << std::endl;

		gpioDelay(5000000); // Attendre 5 secondes

		// Éteindre toutes les Relais
		for (int pin : RELAIS_PINS) {
			gpioWrite(pin, 0);
		}

		std::cout << "Relais éteints." << std::endl;

		gpioDelay(5000000); // Attendre 5 secondes avant de recommencer
	}

	gpioTerminate();

	return 0;

}