// Copyright Epic Games, Inc. All Rights Reserved.

#include "CosResponse.h"
#include "Interfaces/IHttpResponse.h"

UCosResponse::UCosResponse()
{
}

UCosResponse::~UCosResponse()
{
	HttpResponse = nullptr;
}

const TArray<uint8>& UCosResponse::GetContent() const
{
	if (!HttpResponse.IsValid())
	{
		return UCosBase::GetContent();
	}

	return HttpResponse->GetContent();
}

FString UCosResponse::GetContentAsString() const
{
	if (!HttpResponse.IsValid())
	{
		return TEXT("");
	}

	return HttpResponse->GetContentAsString();
}

bool UCosResponse::IsOK() const
{
	if (!HttpResponse.IsValid())
	{
		return false;
	}

	return EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode());
}

int32 UCosResponse::GetResponseCode() const
{
	if (!HttpResponse.IsValid())
	{
		return -1;
	}

	return HttpResponse->GetResponseCode();
}
