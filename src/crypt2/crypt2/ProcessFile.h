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
	~ProcessFile();

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
	m_BytesOfBlock(sizeof(BlockType)),
	m_CrypterPtr(ptr)
{
	createSourceFile(name);
	createDestinationFile(save);
	setSizeBufferOfCluster(name, save);
}

template <typename BlockType>
ProcessFile<BlockType>::~ProcessFile()
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

template <typename Block>
bool FileCode<Block>::asyncProcess(bool type)
{
	if (checkFile())
		return true;

	// ��������� ����� ��� ���������� �����, ���� �� �� ������ 8
	unsigned restBuffer = sizeFile % bytesOfBuffer;
	unsigned restBlock = restBuffer % bytesOfBlock;
	// ���������� ���������� ��������� ������
	unsigned length = sizeFile / bytesOfBuffer;

	if (restBuffer == 1 && type)
	{
		--length;
		restBuffer += bytesOfBuffer;
	}
	if (length)
	{
		Block* buff[2];
		buff[0] = buffer;
		buff[1] = new Block[sizeBuffer];
		bool iter = 1, rezRead[2], rezWrite = 1;

		OVERLAPPED massOl[2];
		for (size_t i = 0; i < 2; i++)
		{
			ZeroMemory(&massOl[i], sizeof(OVERLAPPED));
			massOl[i].hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		}

		rezRead[0] = ReadFile(openFile, buff[1], bytesOfBuffer, nullptr, &massOl[1]);
		massOl[0].Offset = bytesOfBuffer;

		for (unsigned i = 1; i < length; ++i, iter ^= 1)
		{
			rezRead[iter] = ReadFile(openFile, buff[iter ^ 1], bytesOfBuffer, nullptr, &massOl[iter ^ 1]);

			checkWait(rezRead[iter], massOl[iter ^ 1]);

			asyncProcessCode(buff[iter]);
			checkWait(FALSE, massOl[iter]);

			rezWrite = WriteFile(saveFile, buff[iter], bytesOfBuffer, nullptr, &massOl[iter]);
			checkWait(rezWrite, massOl[iter]);
			massOl[0].Offset = massOl[1].Offset += bytesOfBuffer;
		}

		checkWait(rezRead[iter ^ 1], massOl[iter]);
		asyncProcessCode(buff[iter]);

		checkWait(FALSE, massOl[iter]);
		rezWrite = WriteFile(saveFile, buff[iter], bytesOfBuffer, nullptr, &massOl[iter]);
		massOl[iter].Offset += bytesOfBuffer;

		checkWait(rezWrite, massOl[iter ^ 1]);

		delete[] buff[1];

		for (size_t i = 0; i < 2; i++)
		{
			CloseHandle(massOl[i].hEvent);
		}

	}
	if (restBuffer)
	{
		ReadFile(openFile, buffer, restBuffer, nullptr, nullptr);
		if (type)
		{
			processCode(restBuffer / bytesOfBlock);
			WriteFile(saveFile, buffer, restBuffer - restBlock * (bytesOfBlock - char(buffer[restBuffer / bytesOfBlock]) + restBlock), nullptr, nullptr);
		}
		else
		{
			unsigned numberBlock = restBuffer / bytesOfBlock + bool(restBlock);
			processCode(numberBlock);
			buffer[numberBlock] = restBlock;
			WriteFile(saveFile, buffer, numberBlock*bytesOfBlock + bool(restBlock), nullptr, nullptr);
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
		m_SizeFile = GetFileSize(m_OpenFile, &sizeHigh) + (ull(sizeHigh) << 32);
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

template <typename BlockType>
void ProcessFile<BlockType>::setSizeBufferOfCluster(TCHAR* name, TCHAR* save)
{
	unsigned resultSizeCluster;
	
	if ((m_BytesOfBuffer = getSizeOfCluster(name)) < (resultSizeCluster = getSizeOfCluster(save)))
	{
		m_BytesOfBuffer = resultSizeCluster;
	}

	m_SizeBuffer = m_BytesOfBuffer / m_BytesOfBlock;
	m_Buffer = new BlockType[m_SizeBuffer + 1];
}

template <typename BlockType>
unsigned ProcessFile<BlockType>::getSizeOfCluster(TCHAR* name)
{
	DWORD secPerClas = 0;
	DWORD bytePerSec = 0;
	TCHAR catalog[3] = { name[0], name[1], '\0' };
	return GetDiskFreeSpace(catalog, &secPerClas, &bytePerSec, nullptr, nullptr) ? secPerClas * bytePerSec : CLUSTER;
}

template <typename BlockType>
bool ProcessFile<BlockType>::checkFile()
{
	return m_OpenFile == INVALID_HANDLE_VALUE || m_SaveFile == INVALID_HANDLE_VALUE;
}

template <typename BlockType>
void ProcessFile<BlockType>::processCode(unsigned length)
{
	for (size_t i = 0; i < length; ++i)
	{
		m_CrypterPtr->encryption(m_Buffer[i]);
	}
}

template <typename BlockType>
void ProcessFile<BlockType>::asyncReadFile(BlockType* buffer, OVERLAPPED& ol)
{
	ReadFile(m_OpenFile, buffer, m_BytesOfBuffer, nullptr, &ol);
	ol.Offset += m_BytesOfBuffer;
}

template <typename BlockType>
void ProcessFile<BlockType>::asyncProcessCode(BlockType* buffer)
{
	for (size_t i = 0; i < m_SizeFile; ++i)
	{
		m_CrypterPtr->encryption(buffer[i]);
	}
}

template <typename BlockType>
void ProcessFile<BlockType>::swapBuffer(BlockType** buff)
{
	register BlockType* temp = buff[2];
	buff[2] = buff[1];
	buff[1] = buff[0];
	buff[0] = temp;
}

template <typename BlockType>
void ProcessFile<BlockType>::checkWait(BOOL rez, OVERLAPPED& ol)
{
	if (rez == FALSE)
	{
		WaitForSingleObject(ol.hEvent, INFINITY);
	}
}