#include "aes.h"

#define addRoundK \
	*S ^= *K; \
	S++;	  \
	K++;	  \

#define sBoxTurn \
	*S = sBox[*S];\
	S++;		  \
	*S = sBox[*S];\
	S++;		  \
	*S = sBox[*S];\
	S++;		  \
	*S = sBox[*S];\
	S++;		  \
	*S = sBox[*S];\
	S++;		  \
	*S = sBox[*S];\
	S++;		  \
	*S = sBox[*S];\
	S++;		  \
	*S = sBox[*S];\
	S++;		  \
	*S = sBox[*S];\
	S++;		  \
	*S = sBox[*S];\
	S++;		  \
	*S = sBox[*S];\
	S++;		  \
	*S = sBox[*S];\
	S++;		  \
	*S = sBox[*S];\
	S++;		  \
	*S = sBox[*S];\
	S++;		  \
	*S = sBox[*S];\
	S++;		  \
	*S = sBox[*S];

#define row2 \
	temp = *T;\
	*T = *T1;\
	T -= 8;\
	*T1 = *T;\
	T1 -= 8;\
	*T = *T1;\
	*T1 = temp;

#define row4\
	temp = *F;\
	*F = *T1; \
	F += 8;\
	*T1 = *F;\
	T1 += 8;\
	*F = *T1;\
	*T1 = temp;

CryptAES::CryptAES()
	: key(nullptr),
	sBoxMass(nullptr),
	secondRow(0),
	fourthRow(0)
{

}

CryptAES::CryptAES(Key* valueKey)
	: key(valueKey)
{

}

void CryptAES::encryption(AESDataBlock& block)
{
	aesData = block;

	unsigned Nk = key->getNumberOfRoundsW(); // Количество 32-битных слов в ключе

	addRoundKey();

	unsigned i = 4;

	for (; i < Nk; i += 4)
	{
		subBytes();
		shiftRows();
		mixColumns();
		addRoundKey(i);
	}

	subBytes();
	shiftRows();
	addRoundKey(i);
}

void CryptAES::subBytes()
{
	register unsigned char* S = aesData._ch;
	register unsigned char* sBox = sBoxMass;

	sBoxTurn
}

void CryptAES::shiftRows()
{
	register unsigned char* T = aesData._ch + secondRow;
	register unsigned char* T1 = aesData._ch + secondRow - 4;

	unsigned char temp;

	row2

	register unsigned char* F = aesData._ch + fourthRow;
	T1 = aesData._ch + fourthRow + 4;

	row4

	T = aesData._ch;

	swapBytes(T + 2, T + 10);
	swapBytes(T + 6, T + 14);
}

void CryptAES::addRoundKey(unsigned shift)
{
	register unsigned* S = aesData._u;
	register unsigned* K = &(key->getKeyW(shift));

	addRoundK
	addRoundK
	addRoundK
	*S ^= *K;
}

void CryptAES::swapBytes(register unsigned char* left, register unsigned char* right)
{
	register unsigned char temp = *left;
	*left = *right;
	*right = temp;
}