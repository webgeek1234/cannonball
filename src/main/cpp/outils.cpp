#include "outils.hpp"

outils::outils(void)
{
}

outils::~outils(void)
{
}

// Generate long random
//
// Source Address: 0x6C8E
// Input:          None
// Output:         Long Random

// First set of values should be:
// 2a6d7fe7
// 7fe76115
// 6115388b
// 388b9bf8
// 9bf8a878
// a878cb8d
// cb8df300
// f30037e8

// Seed for random number generator
static uint32_t rnd_seed = 0;

uint32_t outils::random()
{
	// New seed value
	uint32_t seed = rnd_seed;

	if (seed == 0)
		seed = 0x2A6D365A;

	// Random Value To Return
	uint32_t rnd = seed;
	
	seed <<= 2;
	seed += rnd;
	seed <<= 3;
	seed += rnd;

	asmhelper::move16(seed, rnd);
	asmhelper::swap32(seed);
	asmhelper::add16(seed, rnd);
	asmhelper::move16(rnd, seed);
	asmhelper::swap32(seed);

	rnd_seed = seed; // Set new seed

	return rnd;
}

// Square Root Functions
//
// Note: This isn't a direct port of the OutRun routine
//
// To Do: Test this outputs identical results

int32_t outils::next(int32_t n, int32_t i) 
{
    return (n + i/n) >> 1;
}

int32_t outils::isqrt(int32_t number) 
{
    if (number == 0)
        return 0;

    int n  = 1;
    int n1 = next(n, number);

    while(abs(n1 - n) > 1) 
    {
        n  = n1;
        n1 = next(n, number);
    }
    while((n1*n1) > number) 
    {
        n1 -= 1;
    }
    return n1;
}

int32_t outils::abs(int32_t n)
{
	return n >= 0 ? n : -n;
}

// Helper routine to add two longs
uint32_t outils::bcd_add(uint32_t src, uint32_t dst)
{
    uint8_t carry = 0;
    uint32_t final = 0;

    for (uint8_t i = 0; i < 4; i++)
    {
        int32_t res = (src & 0xF) + (dst & 0xF) + carry;

        if (res > 9)
            res += 6;

        res += (src & 0xF0) + (dst & 0xF0);

        if (res > 0x99)
        {
            res -= 0xA0;
            carry = 1;
        }
        else
        {
            carry = 0;
        }

        src >>= 8;
        dst >>= 8;

        final |= ((res & 0xFF) << (i * 8));
    }

    return final;
}

// Helper routine to add two longs
uint32_t outils::bcd_sub(uint32_t src, uint32_t dst)
{
    uint8_t carry = 0;
    uint32_t final = 0;

    for (uint8_t i = 0; i < 4; i++)
    {
        int32_t res = (dst & 0xF) - (src & 0xF) - carry;

        if ((res & 0xFF) > 0x9)
            res -= 6;

        res += (dst & 0xF0) - (src&0xF0);

        if ((res & 0xFFFF) > 0x99)
        {
            res += 0xA0;
            carry = 1;
        } 
        else 
        {
            carry = 0;
        }

        src >>= 8;
        dst >>= 8;

        final |= ((res & 0xFF) << (i * 8));
    }

    return final;
}

// Convert car increment value to a human readable speed
//
// Output:
// Final speed to output. This is a hex number but, can be directly converted to decimal.
// So, 0x180 actually would be 180 kp/h directly.
//
// Source: 0x7126
uint16_t outils::convert_speed(uint16_t car_increment)
{
    int16_t top_byte = -1; // [d3]
    int16_t lookup = car_increment; // [d2]

    do
    {
        lookup -= 100;
        top_byte++;
    }
    while (lookup >= 0);
    lookup += 100;
    return (top_byte << 8) | DEC_TO_HEX[lookup];
}

// Lookup table to convert a decimal value to hex
const uint8_t outils::DEC_TO_HEX[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99
};