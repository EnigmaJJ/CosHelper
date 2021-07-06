// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CosHelperTypes.generated.h"

UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class ECosHelperFileInfoType : uint8
{
	None                     = 0 UMETA(Hidden),
	ContentLength            = 1 << 0,
	MD5                      = 1 << 1,
	LastModifiedUtcTimestamp = 1 << 2,
};
ENUM_CLASS_FLAGS(ECosHelperFileInfoType)

USTRUCT(BlueprintType)
struct COSHELPER_API FCosHelperInitializeInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	bool bUseAuthorization{ false };

	/** 签名过期时长，单位为秒 */
	UPROPERTY(BlueprintReadWrite)
	int32 SignExpirationTime{ 60 };

	UPROPERTY(BlueprintReadWrite)
	int64 AppId;

	UPROPERTY(BlueprintReadWrite)
	FString BucketName;

	UPROPERTY(BlueprintReadWrite)
	FString SecretId;

	UPROPERTY(BlueprintReadWrite)
	FString SecretKey;

	UPROPERTY(BlueprintReadWrite)
	FString Region;
};
