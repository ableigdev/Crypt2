#include "aes_decrypt.h"

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

void DecryptAES::mixColumns()
{
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
}