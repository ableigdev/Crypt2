#pragma once

#include "aes.h"

class DecryptAES : public CryptAES
{
public:
	DecryptAES();
	DecryptAES(Key*);

	virtual void encryption(AESDataBlock&);

private:
	void mixColumns();
};