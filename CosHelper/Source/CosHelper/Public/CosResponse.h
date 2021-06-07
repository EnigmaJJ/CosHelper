// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CosBase.h"
#include "CosResponse.generated.h"

class IHttpResponse;

UCLASS(BlueprintType)
class COSHELPER_API UCosResponse : public UCosBase
{
	GENERATED_BODY()

public:
	UCosResponse();
	virtual ~UCosResponse() override;

	FORCEINLINE void SetHttpResponse(TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> InHttpResponse) { HttpResponse = InHttpResponse; }

	//~ Begin UCosBase
	virtual const TArray<uint8>& GetContent() const override;
	//~ End UCosBase

	UFUNCTION(BlueprintCallable)
	FString GetContentAsString() const;

	UFUNCTION(BlueprintCallable)
	bool IsOK() const;

	UFUNCTION(BlueprintCallable)
	int32 GetResponseCode() const;

protected:
	TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> HttpResponse;
};
