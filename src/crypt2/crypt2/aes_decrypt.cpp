#include "aes_decrypt.h"

/*
#define MixColLoopDecr\
		cur = *SAr; \
		cur_8 = unsigned char(cur >> SHIFT_8);\
		cur_16 = unsigned char(cur >> SHIFT_16);\
		cur_24 = cur >> SHIFT_24;\
		cur = unsigned char(cur);\
		*SAr = (MultiplyBy14[cur] | MultiplyBy11[cur] << SHIFT_24) | (MultiplyBy13[cur] << SHIFT_8) | (MultiplyBy9[cur] << SHIFT_16)\
				^ MultiplyBy9[cur_8] | (MultiplyBy14[cur_8] << SHIFT_8) | (MultiplyBy11[cur_8] << SHIFT_16) | (MultiplyBy13[cur_8] << SHIFT_24)\
				^ MultiplyBy13[cur_16] | (MultiplyBy9[cur_16] << SHIFT_8) | (MultiplyBy14[cur_16] << SHIFT_16) | (MultiplyBy11[cur_16] << SHIFT_24)\
				^ MultiplyBy11[cur_24] | (MultiplyBy13[cur_24] << SHIFT_16) | (MultiplyBy9[cur_24] << SHIFT_24) | (MultiplyBy14[cur_24] << SHIFT_8);\
		++SAr;

*/

#define MixColLoopDecr\
		cur = *SAr; \
		cur_8 = unsigned char(cur >> SHIFT_8);\
		cur_16 = unsigned char(cur >> SHIFT_16);\
		cur_24 = cur >> SHIFT_24;\
		cur = unsigned char(cur);\
		*SAr = (MultiplyBy14[cur] | MultiplyBy11[cur] << SHIFT_24) | (MultiplyBy13[cur] << SHIFT_8) | (MultiplyBy9[cur] << SHIFT_16)\
				^ MultiplyBy9[cur_8] | (MultiplyBy14[cur_8] << SHIFT_24) | (MultiplyBy11[cur_8] << SHIFT_8) | (MultiplyBy13[cur_8] << SHIFT_16)\
				^ MultiplyBy13[cur_16] | (MultiplyBy9[cur_16] << SHIFT_24) | (MultiplyBy14[cur_16] << SHIFT_8) | (MultiplyBy11[cur_16] << SHIFT_16)\
				^ MultiplyBy11[cur_24] | (MultiplyBy13[cur_24] << SHIFT_24) | (MultiplyBy9[cur_24] << SHIFT_8) | (MultiplyBy14[cur_24] << SHIFT_16);\
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
	/*
	for (unsigned char i = 0, temp_ch[4] = { 0 }; i < 16; i += 4)
	{
		temp_ch[0] = MultiplyBy14[aesData._ch[i]] ^ MultiplyBy11[aesData._ch[i + 1]] ^ MultiplyBy13[aesData._ch[i + 2]] ^ MultiplyBy9[aesData._ch[i + 3]];
		temp_ch[1] = MultiplyBy9[aesData._ch[i]] ^ MultiplyBy14[aesData._ch[i + 1]] ^ MultiplyBy11[aesData._ch[i + 2]] ^ MultiplyBy13[aesData._ch[i + 3]];
		temp_ch[2] = MultiplyBy13[aesData._ch[i]] ^ MultiplyBy9[aesData._ch[i + 1]] ^ MultiplyBy14[aesData._ch[i + 2]] ^ MultiplyBy11[aesData._ch[i + 3]];
		temp_ch[3] = MultiplyBy11[aesData._ch[i]] ^ MultiplyBy13[aesData._ch[i + 1]] ^ MultiplyBy9[aesData._ch[i + 2]] ^ MultiplyBy14[aesData._ch[i + 3]];
		
		for (size_t j = 0; j < 4; ++j)
		{
			aesData._ch[i + j] = temp_ch[j];
		}
	}
	*/
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