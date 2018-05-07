#pragma once

#include "crypter.h"

class CryptCBCMode : public Crypter<AESDataBlock>
{
public:
	CryptCBCMode() {}
	CryptCBCMode(AESDataBlock& iv, Crypter* ptr)
		: IV(iv),
		crypterPtr(ptr)
	{

	}

	virtual void encryption(AESDataBlock&) = 0;

protected:
	AESDataBlock IV;
	Crypter* crypterPtr;
};