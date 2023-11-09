#include "bitmap.h"
#include "superblock.h"
#include <cstring>
#include <string>

using namespace std;

bitmap::bitmap() {
    size = 0;
}  

//******************************************************************************
bitmap::bitmap(int bitmapSize) {
    size = bitmapSize;
    // Calculate the number of bytes needed for the bitmap
    int numBytes = (size + 7) / 8;
    // Initialize all bytes to 0
    data.resize(numBytes, 0);
}

//******************************************************************************
void bitmap::resize(int newSize) {
    size = newSize;
    data.resize((size + 7) / 8, 0);
}

//******************************************************************************
bool bitmap::setBit(int pos, bool value) {
    bool rc = true;
    if (value) {
        // Set the bit at pos to 1
        data[pos / 8] |= 1 << (pos % 8);
    } else {
        // Set the bit at pos to 0
        data[pos / 8] &= ~(1 << (pos % 8));
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
    bool rc = ((pos >= 0) && (pos < size)) ? true : false;

    if (rc) {
        int byteIndex = pos / 8;
		int bitOffset = pos % 8;
        // Check if the bit is set
        rc = ((data[byteIndex] & (1 << bitOffset)) != 0) ? true : false;
    }
    return rc;
}

//******************************************************************************
int bitmap::findFirstFree() {
    int rc = -1;
    for (int i = 0; i < size; ++i) {
        if (!isBitSet(i)) {
            rc = i;
            break;
        }
    }

    // No free bits found
    return rc;
}