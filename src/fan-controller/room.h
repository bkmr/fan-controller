#ifndef ROOM_H
#define ROOM_H

class SwitchLed {
private:
    int _switchPin;
    int _ledPin;

public:
    SwitchLed() { }

    SwitchLed(int switchPin, int ledPin) {
        _switchPin = switchPin;
        _ledPin = ledPin;
    }

    int switchPin() { return _switchPin; }
    int ledPin() { return _ledPin; }
};

class Room {
private:
    SwitchLed _switchLed;
    uint8_t _address;

public:
    enum FanSpeed {
        FAN_OFF = 0,
        FAN_LOW = 1,
        FAN_MEDIUM = 2,
        FAN_HIGH = 3
    };

    Room(SwitchLed swLed, uint8_t address) {
        _switchLed = swLed;
        _address = address;
        fanSpeed = FanSpeed::FAN_OFF;
    };

    SwitchLed switchLed() { return _switchLed; }
    uint8_t address() { return _address; }

    FanSpeed fanSpeed;
};

#endif // ROOM_H