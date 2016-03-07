#include "Fifo.hpp"

uint8_t AbstractFifo::getSize() const {
    AtomicScope _;
    const auto write_pos = markedOrWritePos();
    const auto read_pos = markedOrReadPos();
    return (write_pos > read_pos) ? write_pos - read_pos :
           (write_pos < read_pos) ? bufferSize - read_pos + write_pos :
           0;
}

uint8_t AbstractFifo::getReadAvailable() const {
    AtomicScope _;
    const auto write_pos = markedOrWritePos();
    const auto read_pos = readPos; // an on-going read DOES remove from the available bytes to read
    return (write_pos > read_pos) ? write_pos - read_pos :
           (write_pos < read_pos) ? bufferSize - read_pos + write_pos :
           0;
}

uint8_t AbstractFifo::getSpace() const {
    AtomicScope _;
    const auto write_pos = writePos;  // an on-going write DOES count to eating up space
    const auto read_pos = markedOrReadPos();
    return (write_pos > read_pos) ? bufferSize - write_pos + read_pos - 1 :
           (write_pos < read_pos) ? read_pos - write_pos - 1 :
           bufferSize - 1;
}

void AbstractFifo::uncheckedWrite(uint8_t b) {
    buffer[writePos] = b;
    writePos++;
    if (writePos >= bufferSize) {
        writePos -= bufferSize;
    }
}

bool AbstractFifo::reserve(volatile uint8_t * &ptr) {
    AtomicScope _;
    if (isWriting() && hasSpace()) {
        ptr = buffer + writePos;
        writePos++;
        if (writePos >= bufferSize) {
            writePos -= bufferSize;
        }
        return true;
    } else {
        return false;
    }
}

void AbstractFifo::uncheckedRead(uint8_t &b) {
    b = buffer[readPos];
    readPos++;
    if (readPos >= bufferSize) {
        readPos -= bufferSize;
    }
}

void AbstractFifo::clear() {
    AtomicScope _;

    readPos = 0;
    writePos = 0;
    writeMark = NO_MARK;
    readMark = NO_MARK;
    reading = false;
    writing = false;
}

bool AbstractFifo::isEmpty() const {
    AtomicScope _;
    return markedOrWritePos() == readPos;
}

bool AbstractFifo::hasContent() const {
    AtomicScope _;
    return markedOrWritePos() != readPos;
}

bool AbstractFifo::isFull() const {
    AtomicScope _;
    return _isFull();
}

uint8_t AbstractFifo::peek() const {
    AtomicScope _;
    if (hasContent()) {
        return buffer[readPos];
    } else {
        return 0;
    }
}

void AbstractFifo::writeStart() {
    if (!isWriting()) {
       writeMark = writePos;
       writing = true;
    }
}

void AbstractFifo::writeEnd() {
    AtomicScope _;

    if (isWriting()) {
        writing = false;
        writeMark = NO_MARK;
    }
}

void AbstractFifo::writeAbort() {
    AtomicScope _;

    if (isWriting()) {
        writing = false;
        writePos = writeMark;
        writeMark = NO_MARK;
        abortedWrites++;
    }
}

void AbstractFifo::readStart() {
    if (!isReading()) {
        reading = true;
        readMark = readPos;
    }
}

void AbstractFifo::readEnd()  {
    AtomicScope _;

    if (isReading()) {
        reading = false;
        readMark = NO_MARK;
    }
}

void AbstractFifo::readAbort() {
    AtomicScope _;

    if (isReading()) {
        reading = false;
        readPos = readMark;
        readMark = NO_MARK;
    }
}

