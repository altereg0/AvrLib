#ifndef FIFODECL_HPP_
#define FIFODECL_HPP_

#include "AtomicScope.hpp"
#include "Streams/ReadResult.hpp"
#include "Streams/StreamingDecl.hpp"

/**
 * A FIFO queue of bytes, with a maximum size of 254.
 */
class AbstractFifo: public Streams::Impl::StreamingDefaultWriteIfSpace<AbstractFifo> {
    constexpr static uint8_t NO_MARK = 255;

    volatile uint8_t * const buffer;
    const uint8_t bufferSize;
    volatile uint8_t readPos = 0;
    volatile uint8_t writePos = 0;
    uint8_t writeMark = NO_MARK;
    uint8_t readMark = NO_MARK;
    uint8_t abortedWrites = 0;
    bool reading = false;
    bool writing = false;

public:
    inline bool isWriting() const {
        return writing;
    }
    inline bool isReading() const {
        return reading;
    }

private:
    inline uint8_t markedOrWritePos() const {
        return isWriting() ? writeMark : writePos;
    }
    inline uint8_t markedOrReadPos() const {
        return isReading() ? readMark : readPos;
    }
    inline bool _isFull() const {
        uint8_t lastWritePos = markedOrReadPos();
        if (lastWritePos == 0) {
            lastWritePos = bufferSize - 1;
        } else {
            lastWritePos--;
        }
        return writePos == lastWritePos;
    }
    inline bool _hasSpace() const {
        uint8_t lastWritePos = markedOrReadPos();
        if (lastWritePos == 0) {
            lastWritePos = bufferSize - 1;
        } else {
            lastWritePos--;
        }
        return writePos != lastWritePos;
    }

public:
    AbstractFifo(uint8_t * const _buffer, const uint8_t _bufferSize): buffer(_buffer), bufferSize(_bufferSize) {}

    inline uint8_t getAbortedWrites() const {
        return abortedWrites;
    }

    bool isEmpty() const;

    bool hasContent() const;

    bool isFull() const;

    inline bool hasSpace() const {
        return !isFull();
    }

    uint8_t getSize() const;

    uint8_t getReadAvailable() const;

    uint8_t getSpace() const;

    inline uint8_t getCapacity() const {
        return bufferSize - 1;
    }

    uint8_t peek() const;


    void writeStart();

    void writeEnd();

    void writeAbort();

    void readStart();

    void readEnd();

    void readAbort();

    /**
     * Appends the given element, assuming there is space for it. May only be called after having
     * previously confirmed enough space by checking getAvailable(), and calling markWrite(). That
     * also makes sure that no interrupt code can get confused while unchecked appends are being made.
     */
    void uncheckedWrite(uint8_t b);

    /** Valid to call between markWrite() and commitWrite(), to reserve a spot in the queue,
     * with the actual value being set later. The caller must NOT write to [ptr] after
     * markWrite() has been called.
     *
     * Returns whether the ptr was reserved (true), or false if the Fifo was full,
     * or no write mark was set.
     */
    bool reserve(volatile uint8_t * &ptr);

    /**
     * Reads a value from the fifo, assuming that previously a check to getSize() was made,
     * and nothing else was read in the meantime.
     */
    void uncheckedRead(uint8_t &b);

    void clear();

    /** Only for use in interrupts. Inlined, and does not disable interrupt flag. */
    __attribute__((always_inline)) inline bool fastread(uint8_t &b) {
        const bool avail = markedOrWritePos() != readPos;
        if (avail) {
            b = buffer[readPos];
            readPos++;
            if (readPos >= bufferSize) {
                readPos -= bufferSize;
            }
        }
        return avail;
    }

    /** Only for use in interrupts. Inlined, and does not disable interrupt flag. */
    __attribute__((always_inline)) inline void fastwrite(uint8_t b) {
        if (_hasSpace()) {
            buffer[writePos] = b;
            writePos++;
            if (writePos >= bufferSize) {
                writePos -= bufferSize;
            }
        }
    }
};

/**
 * Statically allocated FIFO
 */
template<uint8_t Capacity>
class Fifo: public AbstractFifo {
    uint8_t buffer[Capacity + 1];
public:
    Fifo(): AbstractFifo(buffer, Capacity + 1) {}
};

#endif /* FIFODECL_HPP_ */
