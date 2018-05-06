#pragma once

#include "crypter.h"
#include "key.h"

class CryptAES : public Crypter<AESDataBlock>
{
public:
	CryptAES();
	CryptAES(Key*);

	virtual void encryption(AESDataBlock&);

protected:
	Key* key;

	union data
	{
		unsigned* _u;
		unsigned char* _ch;
		data operator=(AESDataBlock& block)
		{
			_u = (unsigned*)((void*)(&block));
			return *this;
		}
	};

	data aesData;

	unsigned char* sBoxMass;
	unsigned char secondRow;
	unsigned char fourthRow;

	virtual void mixColumn() = 0;

	void subBytes();
	void shiftRows();
	void addRoundKey(unsigned = 0);
	void swapBytes(register unsigned char*, register unsigned char*);
};