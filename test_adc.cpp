#include <iostream>
#include <cmath>
#include <pigpio.h>
#include <unistd.h>

#define STEP_PER_REVOLUTION 2048
#define MAX_SPEED 500.0f
#define ACCELERATION 50.0f
#define INITIAL_SPEED 200.0f

#define DEADZONE 50
#define ADC_ADDR 0x48

#define LED_PIN 21

// ─────────────────────────────────────────────
// Classe AccelStepper avec DEBUG
// ─────────────────────────────────────────────
class AccelStepper {
public:

    AccelStepper(const char* name, int pin1, int pin2, int pin3, int pin4)
        : _name(name), _pos(0), _target(0), _speed(0),
        _maxSpeed(MAX_SPEED), _accel(ACCELERATION),
        _stepIndex(0)
    {
        _pins[0] = pin1;
        _pins[1] = pin2;
        _pins[2] = pin3;
        _pins[3] = pin4;

        for (int p : _pins) {
            gpioSetMode(p, PI_OUTPUT);
            gpioWrite(p, 0);
        }

        _lastTime = gpioTick();
    }

    void setMaxSpeed(float speed) { _maxSpeed = speed; }
    void setAcceleration(float accel) { _accel = accel; }
    void setSpeed(float speed) { _speed = speed; }

    void setCurrentPosition(long pos) {
        _pos = pos;
        _target = pos;
    }

    void moveTo(long target) {
        _target = target;
        std::cout << "[" << _name << "] moveTo(" << target << ")\n";
    }

    long currentPosition() const { return _pos; }
    long distanceToGo() const { return _target - _pos; }

    void run() {
        long dist = distanceToGo();

        if (dist == 0) {
            return;
        }

        uint32_t now = gpioTick();
        float dt = (now - _lastTime) * 1e-6f;

        float stepsToStop = (_speed * _speed) / (2.0f * _accel);

        if (std::abs(dist) <= stepsToStop)
            _speed -= _accel * dt;
        else
            _speed += _accel * dt;

        if (_speed < 0) _speed = 0;
        if (_speed > _maxSpeed) _speed = _maxSpeed;

        if (_speed == 0) return;

        float interval = 1.0f / _speed;
        if (dt < interval) return;

        _lastTime = now;

        int dir = (dist > 0) ? 1 : -1;
        _pos += dir;

        _stepIndex = (_stepIndex + dir + 8) % 8;

        _applyStep();

        // DEBUG moteur
        std::cout
            << "[" << _name << "] "
            << "pos=" << _pos
            << " target=" << _target
            << " dist=" << dist
            << " speed=" << _speed
            << " stepIndex=" << _stepIndex
            << "\n";
    }

private:

    const int _halfStep[8][4] = {
        {1, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 1},
        {0, 0, 0, 1},
        {1, 0, 0, 1}
    };

    void _applyStep() {
        for (int i = 0; i < 4; i++)
            gpioWrite(_pins[i], _halfStep[_stepIndex][i]);
    }

    const char* _name;
    int _pins[4];
    long _pos, _target;
    float _speed, _maxSpeed, _accel;
    int _stepIndex;
    uint32_t _lastTime;
};

// ─────────────────────────────────────────────
// ADC
// ─────────────────────────────────────────────
int adc_read(int handle, int channel) {
    char buf[1];
    buf[0] = 0x40 | (channel & 0x03);

    i2cWriteDevice(handle, buf, 1);
    gpioDelay(1500);
    return i2cReadByte(handle);
}

// ─────────────────────────────────────────────
// MOTEURS
// ─────────────────────────────────────────────
AccelStepper* stepper_az;
AccelStepper* stepper_el;

// ─────────────────────────────────────────────
// TRACKING
// ─────────────────────────────────────────────
void solarTracking(int adc) {
    int left = adc_read(adc, 0);
    int right = adc_read(adc, 1);

    std::cout << "[ADC] left=" << left << " right=" << right << "\n";

    int diff = left - right;

    if (std::abs(diff) < DEADZONE) {
        std::cout << "[TRACKING] Stable\n";
        return;
    }

    if (diff > 0) {
        std::cout << "[TRACKING] Move RIGHT\n";
        stepper_az->moveTo(stepper_az->currentPosition() + 5);
    }
    else {
        std::cout << "[TRACKING] Move LEFT\n";
        stepper_az->moveTo(stepper_az->currentPosition() - 5);
    }
}

// ─────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────
int main() {

    if (gpioInitialise() < 0) {
        std::cerr << "Erreur pigpio" << std::endl;
        return 1;
    }

    gpioSetMode(LED_PIN, PI_OUTPUT);

    int adc = i2cOpen(1, ADC_ADDR, 0);
    if (adc < 0) {
        std::cerr << "Erreur ouverture ADC" << std::endl;
        return 1;
    }

    stepper_az = new AccelStepper("AZ", 4, 27, 22, 17);
    stepper_el = new AccelStepper("EL", 18, 23, 24, 25);

    stepper_az->setMaxSpeed(1000);
    stepper_el->setMaxSpeed(1000);

    stepper_az->setAcceleration(50);
    stepper_el->setAcceleration(50);

    stepper_az->setSpeed(200);
    stepper_el->setSpeed(200);

    uint32_t lastBlink = gpioTick();
    bool ledState = false;

    while (true) {

        if (gpioTick() - lastBlink > 500000) {
            ledState = !ledState;
            gpioWrite(LED_PIN, ledState);
            lastBlink = gpioTick();
        }

        solarTracking(adc);

        stepper_az->run();
        stepper_el->run();

        gpioDelay(10);
    }

    i2cClose(adc);
    gpioTerminate();
    return 0;
}
