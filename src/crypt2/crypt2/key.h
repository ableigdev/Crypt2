#pragma once

class Key
{
public:
	Key();
	Key(unsigned*, unsigned char);
	~Key();

	void setEncryptKey(unsigned*, unsigned char);
	void setDecryptKey(unsigned*, unsigned char);

	unsigned& getKeyW(unsigned);
	unsigned char getNumberOfRoundsW();

private:
	unsigned* m_Key;
	unsigned char m_NumberOfW;

	unsigned subBytes(unsigned);
	unsigned leftShift(const unsigned&, const unsigned char);
	void expandKeys(unsigned*, unsigned char);
};