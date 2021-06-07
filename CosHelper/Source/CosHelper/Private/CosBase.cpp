// Copyright Epic Games, Inc. All Rights Reserved.

#include "CosBase.h"

UCosBase::UCosBase()
{
}

UCosBase::~UCosBase()
{
}

const TArray<uint8>& UCosBase::GetContent() const
{
	static TArray<uint8> Empty;
	return Empty;
}
