#pragma once

#include "CryptCBCMode.h"

class EncryptCBCMode : public CryptCBCMode
{
public:
	EncryptCBCMode(AESDataBlock iv, Crypter* ptr)
		: CryptCBCMode(iv, ptr)
	{

	}

	virtual void encryption(AESDataBlock& block)
	{
		block.m_ULL[0] = block.m_ULL[0] ^ IV.m_ULL[0];
		block.m_ULL[1] = block.m_ULL[1] ^ IV.m_ULL[1];
		crypterPtr->encryption(block);
		IV.m_ULL[0] = block.m_ULL[0];
		IV.m_ULL[1] = block.m_ULL[1];
	}
};