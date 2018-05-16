#include "aes_encrypt.h"

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
	for (unsigned char i = 0, temp_ch[4] = { 0 }; i < 16; i += 4)
	{
		temp_ch[0] = MultiplyBy2[aesData._ch[i]] ^ MultiplyBy3[aesData._ch[i + 1]] ^ aesData._ch[i + 2] ^ aesData._ch[i + 3];
		temp_ch[1] = aesData._ch[i] ^ MultiplyBy2[aesData._ch[i + 1]] ^ MultiplyBy3[aesData._ch[i + 2]] ^ aesData._ch[i + 3];
		temp_ch[2] = aesData._ch[i] ^ aesData._ch[i + 1] ^ MultiplyBy2[aesData._ch[i + 2]] ^ MultiplyBy3[aesData._ch[i + 3]];
		temp_ch[3] = MultiplyBy3[aesData._ch[i]] ^ aesData._ch[i + 1] ^ aesData._ch[i + 2] ^ MultiplyBy2[aesData._ch[i + 3]];

		for (size_t j = 0; j < 4; ++j)
		{
			aesData._ch[i + j] = temp_ch[j];
		}
	}
}