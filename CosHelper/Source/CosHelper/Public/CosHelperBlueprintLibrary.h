// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CosHelperTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CosHelperBlueprintLibrary.generated.h"

struct FCosHelperInitializeInfo;

class UCosRequest;
class UCosResponse;
class UCosHelper;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnCosRequestCompletedDynamic, const UCosResponse*, CosResponse);

UCLASS()
class COSHELPER_API UCosHelperBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "CosHelper")
	static UCosHelper* ConstructCosHelper(const FCosHelperInitializeInfo& InitializeInfo);

	UFUNCTION(BlueprintCallable, Category = "CosHelper")
	static void DestroyCosHelper(UCosHelper* CosHelper);

	UFUNCTION(BlueprintCallable, Category = "CosHelper")
	static UCosRequest* GetFileInfo(UCosHelper* CosHelper
	                              , const FString& URIPathName
	                              , const FString& URLParameters
	                              , UPARAM(meta=(Bitmask, BitmaskEnum=ECosHelperFileInfoType)) int32 FileInfoType
	                              , FOnCosRequestCompletedDynamic OnCosRequestCompleted);

	UFUNCTION(BlueprintCallable, Category = "CosHelper")
	static UCosRequest* DownloadFile(UCosHelper* CosHelper
	                               , const FString& URIPathName
	                               , const FString& URLParameters
	                               , const FString& SavedPathName
	                               , FOnCosRequestCompletedDynamic OnCosRequestCompleted);

	UFUNCTION(BlueprintCallable, Category = "CosHelper")
	static UCosRequest* UploadFile(UCosHelper* CosHelper
	                             , const FString& FilePathName
	                             , const FString& URIPathName
	                             , const FString& URLParameters
	                             , FOnCosRequestCompletedDynamic OnCosRequestCompleted);
};
