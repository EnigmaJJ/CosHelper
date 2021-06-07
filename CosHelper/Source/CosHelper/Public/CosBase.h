// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CosBase.generated.h"

UCLASS(Abstract)
class COSHELPER_API UCosBase : public UObject
{
	GENERATED_BODY()

public:
	UCosBase();
	virtual ~UCosBase() override;

	/**
	 * The content is what needs to be uploaded for CosRequest and is what has been downloaded for CosReponse.
	 */
	UFUNCTION(BlueprintCallable)
	virtual const TArray<uint8>& GetContent() const;
};
