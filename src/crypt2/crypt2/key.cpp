#include "key.h"
#include "constants.h"

Key::Key()
	: m_Key(nullptr),
	m_NumberOfW(0)
{

}

Key::Key(unsigned* k, unsigned char wok)
{
	setEncryptKey(k, wok);
}

void Key::setEncryptKey(unsigned* k, unsigned char wok)
{
	expandKeys(k, wok);

	for (size_t i = 0, temp = 0; i < m_NumberOfW; ++i, temp = 0)
	{
		for (size_t t = 0; t < 32; t += 8)
		{
			temp |= ((m_Key[i] >> t) & 0xff) << (24 - t);
		}
		m_Key[i] = temp;
	}
}

void Key::setDecryptKey(unsigned* k, unsigned char wok)
{
	expandKeys(k, wok);

	for (size_t i = 4, tempBlock = 0; i < m_NumberOfW - 4; ++i)
	{
		tempBlock = (MultiplyBy14[m_Key[i] >> 24 & 0xff] ^ MultiplyBy11[(m_Key[i] >> 16) & 0xff] ^ MultiplyBy13[(m_Key[i] >> 8 & 0xff)] ^ MultiplyBy9[(m_Key[i] & 0xff)]) << 24;
		tempBlock |= (MultiplyBy14[m_Key[i] >> 16 & 0xff] ^ MultiplyBy11[m_Key[i] >> 8 & 0xff] ^ MultiplyBy13[(m_Key[i] & 0xff)] ^ MultiplyBy9[(m_Key[i] >> 24 & 0xff)]) << 16;
		tempBlock |= (MultiplyBy14[m_Key[i] >> 8 & 0xff] ^ MultiplyBy11[m_Key[i] & 0xff] ^ MultiplyBy13[(m_Key[i] >> 24 & 0xff)] ^ MultiplyBy9[(m_Key[i] >> 16 & 0xff)]) << 8;
		tempBlock |= MultiplyBy14[m_Key[i] & 0xff] ^ MultiplyBy11[m_Key[i] >> 24 & 0xff] ^ MultiplyBy13[(m_Key[i] >> 16 & 0xff)] ^ MultiplyBy9[(m_Key[i] >> 8 & 0xff)];
		m_Key[i] = tempBlock;
	}

	for (size_t i = 0, temp = 0; i < m_NumberOfW; ++i, temp = 0)
	{
		for (size_t t = 0; t < 32; t += 8)
		{
			temp |= ((m_Key[i] >> t) & 0xff) << (24 - t);
		}
		m_Key[i] = temp;
	}

	for (size_t j = 4; j < m_NumberOfW / 2; j += 4)
	{
		for (size_t temp, i = 0; i < 4; ++i)
		{
			temp = m_Key[i + j - 4];
			m_Key[i + j - 4] = m_Key[m_NumberOfW - j + i];
			m_Key[m_NumberOfW - j + i] = temp;
		}
	}
}


unsigned& Key::getKeyW(unsigned i)
{
	return m_Key[i];
}


unsigned char Key::getNumberOfRoundsW()
{
	return m_NumberOfW - 4;
}

Key::~Key()
{
	if (m_Key)
	{
		delete[] m_Key;
	}
}

unsigned Key::subBytes(unsigned source)
{
	unsigned rez = 0;
	for (size_t i = 0; i < 32; i += 8)
	{
		rez |= sBox[(source >> i) & 0xff] << i;
	}
	return rez;
}

unsigned Key::leftShift(const unsigned& currentKey, const unsigned char count)
{
	return (currentKey >> (32 - count)) | (currentKey << count);
}

void Key::expandKeys(unsigned* k, unsigned char wok)
{
	m_NumberOfW = 4 * (wok + 6) + 4;
	m_Key = new unsigned[m_NumberOfW];

	for (size_t i = 0; i < wok; i++)
	{
		m_Key[i] = k[i];
	}

	unsigned w;
	for (size_t i = wok; i < m_NumberOfW; i++)
	{
		w = m_Key[i - 1];
		if (i % wok == 0)
		{
			w = subBytes(leftShift(w, 8)) ^ rcon[i / wok - 1] /*Rcon(i / wok - 1)*/;
		}
		else if ((wok > 6) && (i % wok == 4))
		{
			w = subBytes(w);
		}
		m_Key[i] = m_Key[i - wok] ^ w;
	}
}