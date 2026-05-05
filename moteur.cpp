#include <iostream>
#include <vector>
#include <pigpio.h>
#include <unistd.h>

const std::vector<int> MOTOR1 = { 4, 27, 22, 17 };
const std::vector<int> MOTOR2 = { 18, 23, 24, 25 };

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

const int MOVE_STEPS = 150;
const int STEP_DELAY = 50000; // µs

void stepMotor(const std::vector<int>& pins, int stepIndex) {
    for (int i = 0; i < 4; i++)
        gpioWrite(pins[i], SEQ[stepIndex % 8][i]);
}

void moveMotor(const std::vector<int>& pins, int steps, int direction, int& currentStep) {
    for (int i = 0; i < steps; i++) {
        currentStep = (currentStep + direction + 8) % 8;
        stepMotor(pins, currentStep);
        // Timing précis avec gpioTick comme le code test
        usleep(STEP_DELAY);
    }
}

int main() {
    if (gpioInitialise() < 0) {
        std::cerr << "Erreur d'initialisation pigpio" << std::endl;
        return 1;
    }

    for (int pin : MOTOR1) gpioSetMode(pin, PI_OUTPUT);
    for (int pin : MOTOR2) gpioSetMode(pin, PI_OUTPUT);

    int stepM1 = 0;
    int stepM2 = 0;

    std::cout << "Démarrage" << std::endl;

    while (true) {
        std::cout << "M1 → aller" << std::endl;
        moveMotor(MOTOR1, MOVE_STEPS, +1, stepM1);
        usleep(50000);

        std::cout << "M1 ← retour" << std::endl;
        moveMotor(MOTOR1, MOVE_STEPS, -1, stepM1);
        usleep(50000);

        std::cout << "M2 → aller" << std::endl;
        moveMotor(MOTOR2, MOVE_STEPS, +1, stepM2);
        usleep(50000);

        std::cout << "M2 ← retour" << std::endl;
        moveMotor(MOTOR2, MOVE_STEPS, -1, stepM2);
        usleep(50000);
    }

    gpioTerminate();
    return 0;
}