#ifndef BITMAP_H
#define BITMAP_H

#include <iostream>
#include <vector>

class bitmap {
	private:
		int size;
		std::vector<unsigned char> data; // Store the bitmap data as bytes
	
	public:
		bitmap(int size);

        // Returns true if bit at the given position is set
        // Otherwise, returns false if given position is out of range
		bool setBit(int pos);

        // Returns true if bit at the given position is cleared
        // Otherwise, returns false if given position is out of range
        bool clearBit(int pos);

        // Returns true if bit is set at the given position
        // Otherwise, returns false if bit is not set at the given position
        // Or given position is out of range
        bool isBitSet(int pos) const;

};

#endif