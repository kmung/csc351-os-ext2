#include "bitmap.h"

using namespace std;

//******************************************************************************
bitmap::bitmap(int size) {
    this->size = size;

    // Calculate the number of bytes needed for the bitmap
    int numBytes = (size + 7) / 8;
    // Initialize all bytes to 0
    data.resize(numBytes, 0);
}

//******************************************************************************
bool bitmap::setBit(int pos) {
    bool rc = pos < size ? true : false;

    if (rc) {
        int byteIndex = pos / 8;
        int bitOffset = pos % 8;
        // Set the bit at the given position
        data[byteIndex] |= (1 << bitOffset);
    }

    return rc;
}

//******************************************************************************
bool bitmap::clearBit(int pos) {
    bool rc = pos < size ? true : false;

    if (rc) {
        int byteIndex = pos / 8;
		int bitOffset = pos % 8;
        // Clear the bit at the given position
		data[byteIndex] &= ~(1 << bitOffset);
    }   

    return rc;
}

//******************************************************************************
bool bitmap::isBitSet(int pos) const {
    bool rc = pos < size ? true : false;

    if (rc) {
        int byteIndex = pos / 8;
		int bitOffset = pos % 8;
        // Check if the bit is set
        rc = ((data[byteIndex] & (1 << bitOffset)) != 0) ? true : false;
    }
}