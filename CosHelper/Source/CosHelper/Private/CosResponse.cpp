// Copyright Epic Games, Inc. All Rights Reserved.

#include "CosResponse.h"
#include "CosHelperModule.h"
#include "CosHelperTypes.h"
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

	return bConnectedSuccessfully && EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode());
}

int32 UCosResponse::GetResponseCode() const
{
	if (!HttpResponse.IsValid())
	{
		return -1;
	}

	return HttpResponse->GetResponseCode();
}

FString UCosResponse::GetFileInfo(ECosHelperFileInfoType InFileInfoType)
{
	static FString Empty{};
	if (!FileInfos.Contains(InFileInfoType))
	{
		UE_LOG(LogCosHelper, Warning, TEXT("File info: %d does NOT exist."), static_cast<int>(InFileInfoType));
		return Empty;
	}

	return FileInfos[InFileInfoType];
}

void UCosResponse::GenerateFileInfos(ECosHelperFileInfoType InFileInfoType)
{
	if (!HttpResponse.IsValid())
	{
		return;
	}

	if (EnumHasAnyFlags(InFileInfoType, ECosHelperFileInfoType::ContentLength))
	{
		FileInfos.Add(ECosHelperFileInfoType::ContentLength, HttpResponse->GetHeader(TEXT("Content-Length")));
	}

	if (EnumHasAnyFlags(InFileInfoType, ECosHelperFileInfoType::ETag))
	{
		FileInfos.Add(ECosHelperFileInfoType::ETag, HttpResponse->GetHeader(TEXT("ETag")));
	}

	if (EnumHasAnyFlags(InFileInfoType, ECosHelperFileInfoType::LastModifiedUtcTimestamp))
	{
		FDateTime LastModifiedUtcTime{};
		if (!FDateTime::ParseHttpDate(HttpResponse->GetHeader(TEXT("Last-Modified")), LastModifiedUtcTime))
		{
			UE_LOG(LogCosHelper, Error, TEXT("Failed to parse http date: %s"), *HttpResponse->GetHeader(TEXT("Last-Modified")));
		}
		else
		{
			FileInfos.Add(ECosHelperFileInfoType::LastModifiedUtcTimestamp
			            , FString::Printf(TEXT("%lld"), LastModifiedUtcTime.ToUnixTimestamp()));
		}
	}
}
