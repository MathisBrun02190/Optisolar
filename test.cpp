#include <gpiod.hpp>
#include <unistd.h>
#include <iostream>

int main() {
    try {
        // Ouvre le chip GPIO
        gpiod::chip chip("gpiochip0");

        // Récupère les lignes GPIO 13 et 19
        gpiod::line led1 = chip.get_line(13);
        gpiod::line led2 = chip.get_line(19);

        // Demande les lignes en sortie
        gpiod::line_request req1 = gpiod::line_request::get_output("led1");
        gpiod::line_request req2 = gpiod::line_request::get_output("led2");

        led1.request(req1, 0);
        led2.request(req2, 0);

        // Boucle clignotante
        while (true) {
            led1.set_value(1);
            led2.set_value(1);
            sleep(1);

            led1.set_value(0);
            led2.set_value(0);
            sleep(1);
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
