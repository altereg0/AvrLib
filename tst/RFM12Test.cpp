#include <gtest/gtest.h>
#include "HopeRF/RFM12.hpp"

namespace RFM12Test {

using namespace HopeRF;

class MockSPIMaster {
public:
    void setClockPrescaler(SPIPrescaler p) const {

    }
    uint8_t transceive(uint8_t out) const {
        return 0;
    }
    void send(uint8_t out) const {

    }
};

class MockSSPin {
public:
    bool isOutput = false;
    void configureAsOutput() {
        isOutput = true;
    }
    void setHigh () const {
    }
    void setLow() const {

    }
};

class MockIntPin {
public:
    typedef HAL::Atmel::InterruptVectors::VectorINT0_ INT;
    bool isInput = false;
    void configureAsInputWithPullup() {
        isInput = true;
    }
    void interruptOnLow() {

    }
    void interruptOff() {

    }
    bool isLow() {
        return false;
    }
};

struct MockComparator {
    typedef HAL::Atmel::InterruptVectors::VectorTIMER0_COMPA_ INT;

    static constexpr uint8_t prescalerPower2 = 8;
    typedef uint8_t value_t;
    value_t target = 0;

    void setTarget(value_t _target) {
        target = _target;
    }

    void interruptOn() {

    }

    void interruptOff() {

    }

    value_t getValue() {
        return 5;
    }

    template <uint32_t value, typename return_t>
    static constexpr return_t microseconds2counts() {
        return (return_t) value;
    }
};

MockSPIMaster rfm_spi;
MockSSPin rfm_ss_pin;
MockIntPin rfm_int_pin;
MockComparator rfm_comparator;

TEST(RFM12Test, rfm12_configures_pins_correctly) {
    RFM12<MockSPIMaster, MockSSPin, MockIntPin, MockComparator> rfm(rfm_spi, rfm_ss_pin, rfm_int_pin, rfm_comparator, RFM12Band::_868Mhz);

    EXPECT_TRUE(rfm_ss_pin.isOutput);
    EXPECT_TRUE(rfm_int_pin.isInput);
}

}
