#include <iostream>
#include <Windows.h>
#include <locale>
#include <stdlib.h>
#include <wchar.h>
#include <bitset>
#include <conio.h>

#include "key.h"
#include "constants.h"
#include "crypter.h"
#include "aes.h"
#include "aes_decrypt.h"
#include "aes_encrypt.h"
#include "CryptCBCMode.h"
#include "DecryptCBCMode.h"
#include "EncryptCBCMode.h"
#include "ProcessFile.h"

void nameFiles(TCHAR* name, TCHAR* save)
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFilter = TEXT("All\0*\0\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = name;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(TCHAR)*MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	std::cout << TEXT("�������� ���� ���������: ");

	while (!GetOpenFileName(&ofn))
	{
		std::cout << TEXT("\n���� ��������� �� ������\n");
		std::cout << TEXT("��������� �����.");
	}

	std::cout << name << std::endl;

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFilter = TEXT("All\0*\0\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = save;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(TCHAR)*MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	std::cout << TEXT("�������� ���� ����������: ");

	while (!GetSaveFileName(&ofn))
	{
		std::cout << TEXT("\n���� ���������� �� ������\n");
		std::cout << TEXT("��������� �����.");
	}

	std::cout << save << std::endl;
}

void timeInfo(clock_t time)
{
	std::cout << TEXT("����� ����������: ") << static_cast<double>(time) / 1000 << TEXT(" ���. ") << std::endl;
}

class CoverCode
{
public:
	CoverCode(){}
	~CoverCode();

	void cipher();

private:
	bool m_Type;
	bool m_Async;

	Key* m_Key;
	Crypter<AESDataBlock>* m_CrypterPtr1;
	Crypter<AESDataBlock>* m_CrypterPtr2;
	ProcessFile<AESDataBlock>* m_ProcessFilePtr;

private:
	void setKeys();
	void chooseMode();

	template <typename DataType>
	bool checkInput(DataType&, TCHAR*, unsigned, unsigned);
	std::bitset<32> reverseByte(std::bitset<32> value);
};

CoverCode::~CoverCode()
{
	if (m_CrypterPtr1)
	{
		delete m_CrypterPtr1;
		m_CrypterPtr1 = nullptr;
	}

	if (m_CrypterPtr2)
	{
		delete m_CrypterPtr2;
		m_CrypterPtr2 = nullptr;
	}

	if (m_Key)
	{
		delete m_Key;
		m_Key = nullptr;
	}
}

void CoverCode::cipher()
{
	TCHAR name[MAX_PATH];
	TCHAR save[MAX_PATH];

	nameFiles(name, save);
	chooseMode();

	m_ProcessFilePtr = new ProcessFile<AESDataBlock>(m_CrypterPtr2 ? m_CrypterPtr2 : m_CrypterPtr1, name, save);

	std::cout << TEXT("---�����������---") << std::endl;

	clock_t time = clock();

	if (m_Async)
	{
		if (m_ProcessFilePtr->asyncProcess(m_Type))
		{
			std::cout << TEXT("������ �����������") << std::endl;
		}
		else
		{
			m_ProcessFilePtr->~ProcessFile();
			timeInfo(clock() - time);
		}
	}
	else
	{
		if (m_ProcessFilePtr->process(m_Type))
		{
			std::cout << TEXT("������ �����������") << std::endl;
		}
		else
		{
			m_ProcessFilePtr->~ProcessFile();
			timeInfo(clock() - time);
		}
	}
	std::cout << "--------------------------------------------" << std::endl;
}

template <typename DataType>
bool CoverCode::checkInput(DataType& data, TCHAR* mass, unsigned massLength, unsigned inputLength)
{
	bool flag;
	TCHAR* input = new TCHAR[inputLength];

	int c = 0;
	int counter = 0;

	while (true)
	{
		c = _getch(); // ��������� ������ � �������

					  // ���� �������� ������  - esc ��� enter
		if (c == 27 || c == 13)
		{
			break;
		}

		// ���� ������� ������ ����� backspace � ����� �� ������
		if (c == 8 && counter != 0)
		{
			input[counter--] = ' '; // ������� ��������� ��������� ������
									// �������� ������ � �������
			putchar('\b');
			putchar(' ');
			putchar('\b');
			continue;
		}

		// ���� �������� ������ �� ����� backspace � ����� �� �������� �� �����
		if (c != 8 && c != 0 && counter < inputLength)
		{
			// �������� ������������ ����������� � ������
			for (size_t i = 0; i < inputLength; i++)
			{
				flag = true;
				for (size_t j = 0; j < massLength && flag; j++)
				{
					if (c == mass[j])
						flag = false;
				}
				if (flag)
				{
					break;
				}
			}

			if (flag == false)
			{
				input[counter++] = c; // ��������� �������� ������ � �����
				putchar(c); // ������� ������ � �������
			}
		}
	}
	std::cout << std::endl;
	data = strtol(input, nullptr, 16);
	return flag;
}

std::bitset<32> CoverCode::reverseByte(std::bitset<32> value)
{
	std::bitset<8> byte;

	for (unsigned i = 0; i < value.size(); i += 8)
	{
		for (unsigned j = 0, z = i; j < 8; ++z, ++j)
		{
			byte[j] = value[z];
		}

		for (unsigned j = i, z = byte.size() - 1; j < i + 8 && z >= 0; ++j, --z)
		{
			value[j] = byte[z];
		}
	}

	std::bitset<32> result;
	for (unsigned i = 0, j = value.size() - 1; i < value.size() && j >= 0; --j, ++i)
	{
		result[i] = value[j];
	}
	return result;
}

void CoverCode::setKeys()
{
	TCHAR hexArray[] = { TEXT('0'), TEXT('1'), TEXT('2'), TEXT('3'), TEXT('4'), TEXT('5'), TEXT('6'), TEXT('7'),
						TEXT('8'), TEXT('9'), TEXT('a'), TEXT('b'), TEXT('c'), TEXT('d'), TEXT('e'), TEXT('f'),
						TEXT('A'), TEXT('B'), TEXT('C'), TEXT('D'), TEXT('E'), TEXT('F') };

	std::cout << TEXT("�������� �������� (1 ������: 0 - ����������, 1 - ������������): ");
	while (checkInput(m_Type, hexArray, 2, 1))
	{
		std::cout << TEXT("������������ �����!") << std::endl;
		std::cout << "-----------------------------------------" << std::endl;
		std::cout << TEXT("�������� �������� (1 ������: 0 - ����������, 1 - ������������): ");
	}
	std::cout << "---------------------------------------------" << std::endl;

	unsigned* key;
	unsigned char length = 0;

	std::cout << TEXT("�������� ��� ��������� (1 ������: 0 - AES-128, 1 - AES-192, 2 - AES-256): ");

	while (checkInput(length, hexArray, 3, 1))
	{
		std::cout << TEXT("������������ �����!") << std::endl;
		std::cout << "---------------------------------------------" << std::endl;
		std::cout << TEXT("�������� ��� ��������� (1 ������: 0 - AES-128, 1 - AES-192, 2 - AES-256): ");
	}

	std::cout << "---------------------------------------------" << std::endl;

	switch (length)
	{
		case 0: 
		{
			length = 4;
			break; 
		}
		case 1: 
		{
			length = 6;
			break; 
		}
		case 2: 
		{
			length = 8;
			break; 
		}
	}
	
	key = new unsigned[length];
	
	TCHAR keyFile[MAX_PATH];

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFilter = TEXT("All\0*\0\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = keyFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(TCHAR)*MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	std::cout << TEXT("�������� ���� � ������: ");

	while (!GetOpenFileName(&ofn))
	{
		std::cout << TEXT("\n���� � ������ �� ������\n");
		std::cout << TEXT("��������� �����.");
	}

	std::cout << keyFile << std::endl;
	
	HANDLE openFile;

	unsigned counter = 0;
	unsigned* buf = new unsigned();

	if ((openFile = CreateFile(keyFile, GENERIC_READ, NULL, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)) != INVALID_HANDLE_VALUE)
	{
		while (ReadFile(openFile, buf, sizeof(unsigned), nullptr, nullptr))
		{
			if (counter < length)
			{
				std::bitset<32> afterPermutation = reverseByte(*buf);
				key[counter++] = afterPermutation.to_ulong();
			}
			else
			{
				break;
			}
		}
	}
	delete buf;
	CloseHandle(openFile);
	
	m_Key = new Key();

	if (m_Type)
	{
		m_Key->setDecryptKey(key, length);
	}
	else
	{
		m_Key->setEncryptKey(key, length);
	}

	delete[] key;
}

void CoverCode::chooseMode()
{
	setKeys();
	unsigned char n = 0;

	TCHAR hexArray[] = { TEXT('0'), TEXT('1'), TEXT('2'), TEXT('3'), TEXT('4'), TEXT('5'),
		TEXT('6'), TEXT('7'), TEXT('8'), TEXT('9'), TEXT('a'), TEXT('b'), TEXT('c'), TEXT('d'), TEXT('e'), TEXT('f'),
		TEXT('A'), TEXT('B'), TEXT('C'), TEXT('D'), TEXT('E'), TEXT('F') };

	if (m_Type)
	{
		m_CrypterPtr1 = new DecryptAES(m_Key);
	}
	else
	{
		m_CrypterPtr1 = new EncryptAES(m_Key);
	}

	std::cout << TEXT("�������� ����� ������ (1 ������: 0 - ECB, 1 - CBC): ");

	while (checkInput(n, hexArray, 2, 1))
	{
		std::cout << TEXT("������������ �����!") << std::endl;
		std::cout << "-----------------------------------------" << std::endl;

		std::cout << TEXT("�������� ����� ������ (1 ������: 0 - ECB, 1 - CBC): ");
	}

	std::cout << "-----------------------------------------" << std::endl;

	if (n)
	{
		AESDataBlock iv;
		std::cout << TEXT("������� ������ ����� IV (16 ��������, hex): ");

		while (checkInput(iv.m_ULL[0], hexArray, 22, 16))
		{
			std::cout << TEXT("������������ IV!") << std::endl;
			std::cout << "-----------------------------------------" << std::endl;
			std::cout << TEXT("������� ������ ����� IV (16 ��������, hex): ");
		}

		std::cout << "-----------------------------------------" << std::endl;
		std::cout << TEXT("������� ������ ����� IV (16 ��������, hex): ");

		while (checkInput(iv.m_ULL[1], hexArray, 22, 16))
		{
			std::cout << TEXT("������������ IV!") << std::endl;
			std::cout << "-----------------------------------------" << std::endl;
			std::cout << TEXT("������� ������ ����� IV (16 ��������, hex): ");
		}

		std::cout << "-----------------------------------------" << std::endl;

		if (m_Type)
		{
			m_CrypterPtr2 = new DecryptCBCMode(iv, m_CrypterPtr1);
		}
		else
		{
			m_CrypterPtr2 = new EncryptCBCMode(iv, m_CrypterPtr1);
		}
	}
	else
	{
		m_CrypterPtr2 = nullptr;
	}

	std::cout << TEXT("�������� ����� ������ � ������ (1 ������: 0 - ����������, 1 - �����������): ");
	
	while (checkInput(m_Async, hexArray, 2, 1))
	{
		std::cout << TEXT("������������ �����!") << std::endl;
		std::cout << "-----------------------------------------" << std::endl;
		std::cout << TEXT("�������� ����� ������ � ������ (1 ������: 0 - ����������, 1 - �����������): ");
	}

	std::cout << "-----------------------------------------" << std::endl;
}

int main()
{
	setlocale(0, "");
	char answer = 'y';

	CoverCode obj;

	do
	{
		obj.cipher();

		std::cout << TEXT("���������� ������? (y/n): ");
		std::cin >> answer;

	} while (answer != 'n' && answer != 'N');

	return 0;
}