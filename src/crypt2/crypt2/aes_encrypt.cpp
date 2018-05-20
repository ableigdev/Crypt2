#include "aes_encrypt.h"

#define MixColLoopEncr\
		cur = *SAr; \
		cur_8 = unsigned char(cur >> SHIFT_8);\
		cur_16 = unsigned char(cur >> SHIFT_16);\
		cur_24 = cur >> SHIFT_24;\
		cur = unsigned char(cur);\
		*SAr = (MultiplyBy2[cur] | (MultiplyBy3[cur] << SHIFT_24) | (cur << SHIFT_8) | (cur << SHIFT_16)) \
		        ^ (MultiplyBy3[cur_8] | (MultiplyBy2[cur_8] << SHIFT_8) | (cur_8 << SHIFT_16) | (cur_8 << SHIFT_24)) \
				^ (cur_16 | (MultiplyBy3[cur_16] << SHIFT_8) | (MultiplyBy2[cur_16] << SHIFT_16) | (cur_16 << SHIFT_24)) \
				^ (cur_24 | (MultiplyBy3[cur_24] << SHIFT_16) | (MultiplyBy2[cur_24] << SHIFT_24) | (cur_24 << SHIFT_8));\
		++SAr;

EncryptAES::EncryptAES()
	: CryptAES()
{

}

EncryptAES::EncryptAES(Key* ptr)
	: CryptAES(ptr)
{
	sBoxMass = sBox;
	secondRow = 15;
	fourthRow = 1;
}

void EncryptAES::encryption(AESDataBlock& block)
{
	CryptAES::encryption(block);
}

inline void EncryptAES::mixColumns()
{
	register unsigned int cur_8;
	register unsigned int cur;
	register unsigned int cur_16;
	register unsigned int cur_24;

	unsigned int* SAr = aesData._u;

	MixColLoopEncr
	MixColLoopEncr
	MixColLoopEncr
	MixColLoopEncr	
}