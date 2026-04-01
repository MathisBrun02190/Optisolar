#include <iostream>

#include <vector>

#include <pigpio.h>

const std::vector<int> LED_PINS = { 21 };

int main() {

	// 1. Initialisation

	if (gpioInitialise() < 0) {

		std::cerr << "Erreur d'initialisation pigpio" << std::endl;

		return 1;

	}

	// 2. Configuration des broches en mode SORTIE

	for (int pin : LED_PINS) {

		gpioSetMode(pin, PI_OUTPUT);

	}

	std::cout << "Démarrage : les LEDs s'allument et s'éteignent en boucle." << std::endl;

	while (true) {

		// Allumer toutes les LEDs

		for (int pin : LED_PINS) {

			gpioWrite(pin, 1);

		}

		std::cout << "LEDs allumées." << std::endl;

		gpioDelay(2000000); // Attendre 2 secondes

		// Éteindre toutes les LEDs

		for (int pin : LED_PINS) {

			gpioWrite(pin, 0);

		}

		std::cout << "LEDs éteintes." << std::endl;

		gpioDelay(2000000); // Attendre 2 secondes avant de recommencer

	}

	gpioTerminate();

	return 0;

}