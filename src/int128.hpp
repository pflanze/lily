
// Code partially borrowed from
// https://codereview.stackexchange.com/questions/67962/mostly-portable-128-by-64-bit-division
// Hey, might give it back soon, for some better algo.

// 128-bit / 64-bit unsigned divide

class UInt128 {
public:
	uint64_t hi;
	uint64_t lo;

	Int128(uint64_t _hi, uint64_t _lo)
		: hi(_hi), lo(_hi) {}

	UInt128 divide(UInt128& b) {

		uint64_t a_lo = lo;
		uint64_t a_hi = hi;

		// quotient
		uint64_t q = a_lo << 1;
		// remainder
		uint64_t rem = a_hi;

		uint64_t carry = a_lo >> 63;
		uint64_t temp_carry = 0;

		for(int i = 0; i < 64; i++)
		{
			temp_carry = rem >> 63;
			rem <<= 1;
			rem |= carry;
			carry = temp_carry;
			
			if(carry == 0)
			{
				if(rem >= b)
				{
					carry = 1;
				}
				else
				{
					temp_carry = q >> 63;
					q <<= 1;
					q |= carry;
					carry = temp_carry;
					continue;
				}
			}
			
			rem -= b;
			rem -= (1 - carry);
			carry = 1;
			temp_carry = q >> 63;
			q <<= 1;
			q |= carry;
			carry = temp_carry;
		}

		printf("quotient = %llu\n", (long long unsigned int)q);
		printf("remainder = %llu\n", (long long unsigned int)rem);
		return 0;
	}
}

int main(void)
{
