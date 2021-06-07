// Fill out your copyright notice in the Description page of Project Settings.

#include "CosHelperBlueprintLibrary.h"
#include "CosHelper.h"

UCosHelper* UCosHelperBlueprintLibrary::ConstructCosHelper(const FCosHelperInitializeInfo& InitializeInfo)
{
	UCosHelper* CosHelper = NewObject<UCosHelper>();
	CosHelper->AddToRoot();
	CosHelper->Initialize(InitializeInfo);

	return CosHelper;
}

void UCosHelperBlueprintLibrary::DestroyCosHelper(UCosHelper* CosHelper)
{
	if (nullptr == CosHelper)
	{
		return;
	}

	if (CosHelper->IsValidLowLevel())
	{
		CosHelper->ConditionalBeginDestroy();
	}
	CosHelper = nullptr;
}

UCosRequest* UCosHelperBlueprintLibrary::DownloadFile(UCosHelper* CosHelper
                                                    , const FString& URIPathName
                                                    , const FString& URLParameters
                                                    , const FString& SavedPathName
                                                    , FOnCosRequestCompletedDynamic OnCosRequestCompleted)
{
	if (nullptr == CosHelper)
	{
		return nullptr;
	}

	const TWeakObjectPtr<UCosRequest> CosRequest =
		CosHelper->DownloadFile(URIPathName
		                      , URLParameters
		                      , SavedPathName
		                      , UCosHelper::FOnCosRequestCompleted::CreateLambda([OnCosRequestCompleted](const UCosResponse& CosResponse)
		                      {
		                        if (OnCosRequestCompleted.IsBound())
		                        {
		                          OnCosRequestCompleted.Execute(&CosResponse);
		                        }
		                      }));
	if (!CosRequest.IsValid())
	{
		return nullptr;
	}

	return CosRequest.Get();
}

UCosRequest* UCosHelperBlueprintLibrary::UploadFile(UCosHelper* CosHelper
                                                  , const FString& FilePathName
                                                  , const FString& URIPathName
                                                  , const FString& URLParameters
                                                  , FOnCosRequestCompletedDynamic OnCosRequestCompleted)
{
	if (nullptr == CosHelper)
	{
		return nullptr;
	}

	const TWeakObjectPtr<UCosRequest> CosRequest =
		CosHelper->UploadFile(FilePathName
		                    , URIPathName
		                    , URLParameters
		                    , UCosHelper::FOnCosRequestCompleted::CreateLambda([OnCosRequestCompleted](const UCosResponse& CosResponse)
		                    {
		                      if (OnCosRequestCompleted.IsBound())
		                      {
		                        OnCosRequestCompleted.Execute(&CosResponse);
		                      }
		                    }));
	if (!CosRequest.IsValid())
	{
		return nullptr;
	}

	return CosRequest.Get();
}
