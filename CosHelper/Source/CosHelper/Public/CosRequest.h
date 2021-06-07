// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CosBase.h"
#include "CosRequest.generated.h"

class IHttpRequest;

UCLASS(BlueprintType)
class COSHELPER_API UCosRequest : public UCosBase
{
	GENERATED_BODY()

public:
	UCosRequest();
	virtual ~UCosRequest() override;

	FORCEINLINE void SetHttpRequest(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> InHttpRequest) { HttpRequest = InHttpRequest; }

	//~ Begin UCosBase
	virtual const TArray<uint8>& GetContent() const override;
	//~ End UCosBase

protected:
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest;
};
