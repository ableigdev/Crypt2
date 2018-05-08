#pragma once

#include "aes.h"

class EncryptAES : public CryptAES
{
public:
	EncryptAES();
	EncryptAES(Key*);

	virtual void encryption(AESDataBlock&);

private:
	void mixColumns();
};