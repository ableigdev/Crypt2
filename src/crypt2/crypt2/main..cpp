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

	std::cout << TEXT("Выберите файл источника: ");

	while (!GetOpenFileName(&ofn))
	{
		std::cout << TEXT("\nФайл источника не выбран\n");
		std::cout << TEXT("Повторите выбор.");
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

	std::cout << TEXT("Выберите файл результата: ");

	while (!GetSaveFileName(&ofn))
	{
		std::cout << TEXT("\nФайл результата не выбран\n");
		std::cout << TEXT("Повторите выбор.");
	}

	std::cout << save << std::endl;
}

void timeInfo(clock_t time)
{
	std::cout << TEXT("Время выполнения: ") << static_cast<double>(time) / 1000 << TEXT(" сек. ") << std::endl;
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

	std::cout << TEXT("---Кодирование---") << std::endl;

	clock_t time = clock();

	if (m_Async)
	{
		if (m_ProcessFilePtr->asyncProcess(m_Type))
		{
			std::cout << TEXT("Ошибка кодирования") << std::endl;
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
			std::cout << TEXT("Ошибка кодирования") << std::endl;
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
bool CoverCode::checkInput(DataType& data, TCHAR* array1, unsigned array1Length, unsigned inputLength)
{
	bool flag;
	std::basic_string<TCHAR> input;

	int c = 0;

	while (true)
	{
		c = _getch();

		if (c == 27 || c == 13)
		{
			break;
		}

		if (c == 8 && !input.empty())
		{
			input.pop_back();
			std::cout << "\b \b";
			continue;
		}

		if (c != 8 && input.size() < inputLength)
		{
			input += c;
			putchar(c);
		}		
	}
	std::cout << std::endl;

	for (size_t i = 0; i < inputLength; ++i)
	{
		flag = true;
		for (size_t j = 0; j < array1Length && flag; ++j)
		{
			if (input[i] == array1[j])
			{
				flag = false;
			}
		}

		if (flag)
		{
			std::cin.clear();
			std::cin.sync();
			return flag;
		}		
	}
	data = strtol(input.data(), nullptr, 16);
	std::cin.clear();
	std::cin.sync();
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

	std::cout << TEXT("Выберите действие (1 символ: 0 - шифрование, 1 - дешифрование): ");
	while (checkInput(m_Type, hexArray, 2, 1))
	{
		std::cout << TEXT("Некорректный выбор!") << std::endl;
		std::cout << "-----------------------------------------" << std::endl;
		std::cout << TEXT("Выберите действие (1 символ: 0 - шифрование, 1 - дешифрование): ");
	}
	std::cout << "---------------------------------------------" << std::endl;

	unsigned* key;
	unsigned char length = 0;

	std::cout << TEXT("Выберите тип алгоритма (1 символ: 0 - AES-128, 1 - AES-192, 2 - AES-256): ");

	while (checkInput(length, hexArray, 3, 1))
	{
		std::cout << TEXT("Некорректный выбор!") << std::endl;
		std::cout << "---------------------------------------------" << std::endl;
		std::cout << TEXT("Выберите тип алгоритма (1 символ: 0 - AES-128, 1 - AES-192, 2 - AES-256): ");
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
	/*
	key[0] = 0x00010203;
	key[1] = 0x04050607;
	key[2] = 0x08090a0b;
	key[3] = 0x0c0d0e0f;
	key[4] = 0x10111213;
	key[5] = 0x14151617;
	key[6] = 0x18191a1b;
	key[7] = 0x1c1d1e1f;
	*/
	/*
	key = new unsigned[8];
	for (size_t i = 0; i < length; i++)
	{
		std::cout << TEXT("Введите ") << i + 1 << TEXT("-ю часть ключа (8 символов, hex): ");
		while (checkInput(key[i], hexArray, 16, 8))
		{
			std::cout << TEXT("Некорректный ключ!") << std::endl;
			std::cout << "-----------------------------------------" << std::endl;
			std::cout << TEXT("Введите ") << i + 1 << TEXT("-ю часть ключа (8 символов, hex): ");
		}
		std::cout << "-----------------------------------------" << std::endl;
	}
	*/
	
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

	std::cout << TEXT("Выберите файл с ключем: ");

	while (!GetOpenFileName(&ofn))
	{
		std::cout << TEXT("\nФайл с ключем не выбран\n");
		std::cout << TEXT("Повторите выбор.");
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

	std::cout << TEXT("Выберите режим работы (1 символ: 0 - ECB, 1 - CBC): ");

	while (checkInput(n, hexArray, 2, 1))
	{
		std::cout << TEXT("Некорректный выбор!") << std::endl;
		std::cout << "-----------------------------------------" << std::endl;

		std::cout << TEXT("Выберите режим работы (1 символ: 0 - ECB, 1 - CBC): ");
	}

	std::cout << "-----------------------------------------" << std::endl;

	if (n)
	{
		AESDataBlock iv;
		std::cout << TEXT("Введите первую часть IV (16 символов, hex): ");

		while (checkInput(iv.m_ULL[0], hexArray, 22, 16))
		{
			std::cout << TEXT("Некорректный IV!") << std::endl;
			std::cout << "-----------------------------------------" << std::endl;
			std::cout << TEXT("Введите первую часть IV (16 символов, hex): ");
		}

		std::cout << "-----------------------------------------" << std::endl;
		std::cout << TEXT("Введите вторую часть IV (16 символов, hex): ");

		while (checkInput(iv.m_ULL[1], hexArray, 22, 16))
		{
			std::cout << TEXT("Некорректный IV!") << std::endl;
			std::cout << "-----------------------------------------" << std::endl;
			std::cout << TEXT("Введите вторую часть IV (16 символов, hex): ");
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

	std::cout << TEXT("Выберите режим работы с файлом (1 символ: 0 - синхронный, 1 - асинхронный): ");
	
	while (checkInput(m_Async, hexArray, 2, 1))
	{
		std::cout << TEXT("Некорректный выбор!") << std::endl;
		std::cout << "-----------------------------------------" << std::endl;
		std::cout << TEXT("Выберите режим работы с файлом (1 символ: 0 - синхронный, 1 - асинхронный): ");
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

		std::cout << TEXT("Продолжить работу? (y/n): ");
		std::cin >> answer;

	} while (answer != 'n' && answer != 'N');

	system("pause");

	return 0;
}