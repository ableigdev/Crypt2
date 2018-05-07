#pragma once

#include "crypter.h"
#include "constants.h"
#include <Windows.h>

#define CLUSTER 4096

template <typename BlockType>
class ProcessFile
{
public:
	ProcessFile();
	ProcessFile(Crypter<BlockType>*, TCHAR*, TCHAR*);
	ProcessFile();

	bool process(bool);
	bool asyncProcess(bool);
	void closeFile(HANDLE&);
	bool createSourceFile(TCHAR*);
	bool createDestinationFile(TCHAR*);

private:
	HANDLE m_OpenFile;
	HANDLE m_SaveFile;
	BlockType* m_Buffer;
	unsigned m_SizeBuffer;
	ull m_SizeFile;
	unsigned m_BytesOfBlock;
	unsigned m_BytesOfBuffer;
	Crypter<BlockType>* m_CrypterPtr;

private:
	void setSizeBufferOfCluster(TCHAR*, TCHAR*);
	unsigned getSizeOfCluster(TCHAR*);
	bool checkFile();
	void processCode(unsigned);
	void asyncReadFile(BlockType*, OVERLAPPED&);
	void asyncProcessCode(BlockType*);
	void swapBuffer(BlockType**);
	void checkWait(bool, OVERLAPPED&);
};

template <typename BlockType>
ProcessFile<BlockType>::ProcessFile()
	: m_OpenFile(INVALID_HANDLE_VALUE),
	m_SaveFile(INVALID_HANDLE_VALUE),
	m_Buffer(nullptr)
{

}

template <typename BlockType>
ProcessFile<BlockType>::ProcessFile(Crypter<BlockType>* ptr, TCHAR* name, TCHAR* save)
	: m_OpenFile(INVALID_HANDLE_VALUE),
	m_SaveFile(INVALID_HANDLE_VALUE),
	m_Buffer(nullptr),
	m_BytesOfBlock(sizeof(BlockType))
	m_CrypterPtr(ptr)
{
	createSourceFile(name);
	createDestinationFile(save);
	setSizeBufferOfCluster(name, save);
}

template <typename BlockType>
ProcessFile<BlockType>::ProcessFile()
{
	delete[] m_Buffer;
	closeFile(m_OpenFile);
	closeFile(m_SaveFile);
}

template <typename BlockType>
bool ProcessFile<BlockType>::process(bool type)
{
	if (checkFile())
	{
		return true;
	}

	unsigned restBuffer = m_SizeFile % m_BytesOfBuffer;
	unsigned restBlock = restBuffer % m_BytesOfBlock;
	unsigned length = m_SizeFile / m_BytesOfBuffer;

	if (restBuffer == 1 && type)
	{
		--length;
		restBuffer += m_BytesOfBuffer;
	}

	for (size_t i = 0; i < length; ++i)
	{
		ReadFile(m_OpenFile, m_Buffer, m_BytesOfBuffer, nullptr, nullptr);
		processCode(m_SizeBuffer);
		WriteFile(m_SaveFile, m_Buffer, m_BytesOfBuffer, nullptr, nullptr);
	}

	if (restBuffer)
	{
		ReadFile(m_OpenFile, m_Buffer, restBuffer, nullptr, nullptr);

		if (type)
		{
			processCode(restBuffer / m_BytesOfBlock);
			WriteFile(m_SaveFile, m_Buffer, restBlock - restBlock * (m_BytesOfBlock - char(m_Buffer[restBlock / m_BytesOfBlock]) + restBlock), nullptr, nullptr);
		}
		else
		{
			unsigned numberBlock = restBlock / m_BytesOfBlock + bool(restBlock);
			processCode(numberBlock);
			m_Buffer[numberBlock] = restBlock;
			WriteFile(m_SaveFile, m_Buffer, numberBlock * m_BytesOfBlock + bool(restBlock), nullptr, nullptr);
		}
	}
	return false;
}

template <typename BlockType>
bool ProcessFile<BlockType>::asyncProcess(bool type)
{
	if (checkFile())
	{
		return true;
	}

	unsigned restBuffer = m_SizeFile % m_BytesOfBuffer;
	unsigned restBlock = restBuffer % m_BytesOfBlock;
	unsigned length = m_SizeFile / m_BytesOfBuffer;

	if (restBuffer == 1 && type)
	{
		--length;
		restBuffer += m_BytesOfBuffer;
	}

	if (length)
	{
		BlockType* buff[3];
		buff[0] = m_Buffer;
		buff[1] = new BlockType[m_SizeBuffer];
		buff[2] = new BlockType[m_SizeBuffer];

		bool iter = 1;
		bool rezRead[2];
		bool rezWrite = 1;

		OVERLAPPED massOl[3];
		for (size_t i = 0; i < 3; ++i)
		{
			ZeroMemory(&massOl[i], sizeof OVERLAPPED);
			massOl[i].hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		}

		rezRead[0] = ReadFile(m_OpenFile, buff[1], m_BytesOfBuffer, nullptr, &massOl[0]);
		massOl[1].Offset = massOl[0].Offset = m_BytesOfBuffer;

		for (unsigned i = 1; i < length; ++i, iter ^= 1)
		{
			rezRead[iter] = ReadFile(m_OpenFile, buff[0], m_BytesOfBuffer, nullptr, &massOl[iter]);
			massOl[1].Offset = massOl[0].Offset += m_BytesOfBuffer;

			checkWait(rezRead[iter ^ 1], massOl[iter ^ 1]);
			asyncProcessCode(buff[1]);

			checkWait(rezWrite, massOl[2]);
			swapBuff(buff);
			rezWrite = WriteFile(m_SaveFile, buff[2], m_BytesOfBuffer, nullptr, &massOl[2]);
			massOl[2].Offset += m_BytesOfBuffer;
		}

		checkWait(rezRead[iter ^ 1], massOl[iter ^ 1]);
		asyncProcessCode(buff[iter]);

		checkWait(rezWrite, massOl[2]);
		rezWrite = WriteFile(m_SaveFile, buff[iter], m_BytesOfBuffer, nullptr, &massOl[2]);
		massOl[2].Offset += m_BytesOfBuffer;

		checkWait(rezWrite, massOl[2]);
		delete[](buff[0] == m_Buffer ? buff[1] : buff[0]);
		delete[] buff[2];
		
		for (size_t i = 0; i < 3; ++i)
		{
			CloseHandle(massOl[i].hEvent);
		}
	}
	if (restBuffer)
	{
		ReadFile(m_OpenFile, m_Buffer, restBuffer, nullptr, nullptr);
		
		if (type)
		{
			processCode(restBuffer / m_BytesOfBlock);
			WriteFile(m_SaveFile, m_Buffer, restBuffer - restBlock * (m_BytesOfBlock - char(m_Buffer[restBuffer / m_BytesOfBlock]) + restBlock), nullptr, nullptr);
		}
		else
		{
			unsigned numberBlock = restBuffer / m_BytesOfBlock + bool(restBlock);
			processCode(numberBlock);
			m_Buffer[numberBlock] = restBlock;
			WriteFile(m_SaveFile, m_Buffer, numberBlock * m_BytesOfBlock + bool(restBlock), nullptr, nullptr);
		}
	}
	return false;
}

template <typename BlockType>
void ProcessFile<BlockType>::closeFile(HANDLE& handle)
{
	if (handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;
	}
}

template <typename BlockType>
bool ProcessFile<BlockType>::createSourceFile(TCHAR* name)
{
	closeFile(m_OpenFile);

	if ((m_OpenFile = CreateFile(name, GENERIC_READ, NULL, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)) != INVALID_HANDLE_VALUE)
	{
		DWORD sizeHigh = 0;
		m_SaveFile = GetFileSize(m_OpenFile, &sizeHigh) + (ull(sizeHigh) << 32);
		return true;
	}
	return false;
}

template <typename BlockType>
bool ProcessFile<BlockType>::createDestinationFile(TCHAR* name)
{
	closeFile(m_SaveFile);
	return (m_SaveFile = CreateFile(name, GENERIC_WRITE, NULL, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr)) == INVALID_HANDLE_VALUE;
}