#include "aes_decrypt.h"

#define MixColLoopDecr\
		cur = *SAr; \
		cur_8 = unsigned char(cur >> SHIFT_8);\
		cur_16 = unsigned char(cur >> SHIFT_16);\
		cur_24 = cur >> SHIFT_24;\
		cur = unsigned char(cur);\
		*SAr = (MultiplyBy14[cur] | (MultiplyBy9[cur] << SHIFT_8) | (MultiplyBy13[cur] << SHIFT_16) | (MultiplyBy11[cur] << SHIFT_24))\
				^ (MultiplyBy11[cur_8] | (MultiplyBy14[cur_8] << SHIFT_8) | (MultiplyBy9[cur_8] << SHIFT_16) | (MultiplyBy13[cur_8] << SHIFT_24))\
				^ (MultiplyBy13[cur_16] | (MultiplyBy11[cur_16] << SHIFT_8) | (MultiplyBy14[cur_16] << SHIFT_16) | (MultiplyBy9[cur_16] << SHIFT_24))\
				^ (MultiplyBy9[cur_24] | (MultiplyBy13[cur_24] << SHIFT_8) | (MultiplyBy11[cur_24] << SHIFT_16) | (MultiplyBy14[cur_24] << SHIFT_24));\
		++SAr;

DecryptAES::DecryptAES()
	: CryptAES()
{

}

DecryptAES::DecryptAES(Key* ptr)
	: CryptAES(ptr)
{
	sBoxMass = invSbox;
	secondRow = 13;
	fourthRow = 3;
}

void DecryptAES::encryption(AESDataBlock& block)
{
	CryptAES::encryption(block);
}

inline void DecryptAES::mixColumns()
{
	register unsigned int cur_8;
	register unsigned int cur;
	register unsigned int cur_16;
	register unsigned int cur_24;

	unsigned int* SAr = aesData._u;

	MixColLoopDecr
	MixColLoopDecr
	MixColLoopDecr
	MixColLoopDecr
}