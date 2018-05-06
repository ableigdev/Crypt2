#pragma once

#include "constants.h"

template <typename BlockType>
class Crypter
{
public:
	virtual void encryption(BlockType&) = 0;
};
