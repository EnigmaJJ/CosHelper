// Copyright Epic Games, Inc. All Rights Reserved.

#include "CosRequest.h"
#include "Interfaces/IHttpRequest.h"

UCosRequest::UCosRequest()
{
}

UCosRequest::~UCosRequest()
{
	HttpRequest = nullptr;
}

const TArray<uint8>& UCosRequest::GetContent() const
{
	if (HttpRequest.IsValid())
	{
		return HttpRequest->GetContent();
	}

	return UCosBase::GetContent();
}
