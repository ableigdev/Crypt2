#pragma once

#include "CryptCBCMode.h"

class DecryptCBCMode : public CryptCBCMode
{
public:
	DecryptCBCMode(AESDataBlock iv, Crypter* ptr)
		: CryptCBCMode(iv, ptr)
	{

	}

	virtual void encryption(AESDataBlock& block)
	{
		AESDataBlock codeBlock = block;
		crypterPtr->encryption(block);
		block.m_ULL[0] = block.m_ULL[0] ^ IV.m_ULL[0];
		block.m_ULL[1] = block.m_ULL[1] ^ IV.m_ULL[1];
		IV = codeBlock;
	}
};