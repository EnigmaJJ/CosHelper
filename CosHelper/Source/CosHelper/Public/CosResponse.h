// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CosBase.h"
#include "CosResponse.generated.h"

enum class ECosHelperFileInfoType : uint8;

class IHttpResponse;

UCLASS(BlueprintType)
class COSHELPER_API UCosResponse : public UCosBase
{
	GENERATED_BODY()

public:
	UCosResponse();
	virtual ~UCosResponse() override;

	//~ Begin UCosBase
	virtual const TArray<uint8>& GetContent() const override;
	//~ End UCosBase

	UFUNCTION(BlueprintCallable)
	FString GetContentAsString() const;

	UFUNCTION(BlueprintCallable)
	bool IsOK() const;

	UFUNCTION(BlueprintCallable)
	int32 GetResponseCode() const;

	UFUNCTION(BlueprintCallable)
	FString GetFileInfo(ECosHelperFileInfoType InFileInfoType);

protected:
	FORCEINLINE void SetHttpResponse(TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> InHttpResponse) { HttpResponse = InHttpResponse; }

	void GenerateFileInfos(ECosHelperFileInfoType InFileInfoType);

protected:
	friend class UCosHelper;

	TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> HttpResponse;
	TMap<ECosHelperFileInfoType, FString> FileInfos;
};
