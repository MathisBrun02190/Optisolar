#include <iostream>
#include <vector>
#include <pigpio.h>
#include <unistd.h>

const std::vector<int> MOTOR1 = { 4, 27, 22, 17 };
const std::vector<int> MOTOR2 = { 18, 23, 24, 25 };

// Séquence de pas (full-step)
const int SEQ[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
	{0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

void stepMotor(const std::vector<int>& pins, int stepIndex) {
    for (int i = 0; i < 4; i++)
        gpioWrite(pins[i], SEQ[stepIndex][i]);
}

int main() {

    if (gpioInitialise() < 0) {
        std::cerr << "Erreur d'initialisation pigpio" << std::endl;
        return 1;
    }

    // Configurer les 8 broches en sortie
    for (int pin : MOTOR1) gpioSetMode(pin, PI_OUTPUT);
    for (int pin : MOTOR2) gpioSetMode(pin, PI_OUTPUT);

    std::cout << "Les deux moteurs tournent...ou pas" << std::endl;

    while (true) {
        for (int step = 0; step < 8; step++) {

            // Moteur 1
            stepMotor(MOTOR1, step);

            // Moteur 2
            stepMotor(MOTOR2, step);

            usleep(1000); // vitesse (plus petit = plus rapide)
        }
    }

    gpioTerminate();
    return 0;
}
