// Copyright Epic Games, Inc. All Rights Reserved.

#include "CosHelper.h"
#include "Containers/SortedMap.h"
#include "CosHelperModule.h"
#include "CosHelperTypes.h"
#include "CosRequest.h"
#include "CosResponse.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/SecureHash.h"
#include "PlatformHttp.h"

bool UCosHelper::Initialize(const FCosHelperInitializeInfo& InitializeInfo)
{
	bUseAuthorization = InitializeInfo.bUseAuthorization;
	SignExpirationTime = static_cast<uint32>(InitializeInfo.SignExpirationTime);
	SecretId = InitializeInfo.SecretId;
	SecretKey = InitializeInfo.SecretKey;

	GenerateHost(static_cast<uint64>(InitializeInfo.AppId), InitializeInfo.BucketName, InitializeInfo.Region);

	return true;
}

TWeakObjectPtr<UCosRequest> UCosHelper::GetFileInfo(const FString& URIPathName
                                                  , const FString& URLParameters
                                                  , ECosHelperFileInfoType FileInfoType
                                                  , FOnCosRequestCompleted OnCosRequestCompleted)
{
	FRequestData* RequestData =
		CreateRequest(URIPathName
		            , URLParameters
		            , [this](TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest){
		                HttpRequest->SetVerb(TEXT("HEAD"));
		                ReplaceWithCDNHost(HttpRequest.Get());

		                return true;
		              }
		            , OnCosRequestCompleted);
	if (nullptr == RequestData)
	{
		return nullptr;
	}

	RequestData->FileInfoType = FileInfoType;

	return RequestData->CosRequest;
}

TWeakObjectPtr<UCosRequest> UCosHelper::DownloadFile(const FString& URIPathName
                                                   , const FString& URLParameters
                                                   , const FString& SavedFilePathName
                                                   , FOnCosRequestCompleted OnCosRequestCompleted)
{
	FRequestData* RequestData =
		CreateRequest(URIPathName
		            , URLParameters
		            , [this](TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest){
		                HttpRequest->SetVerb(TEXT("GET"));
		                ReplaceWithCDNHost(HttpRequest.Get());

		                return true;
		              }
		            , OnCosRequestCompleted);
	if (nullptr == RequestData)
	{
		return nullptr;
	}

	RequestData->LocalFilePathName = SavedFilePathName;

	return RequestData->CosRequest;
}

TWeakObjectPtr<UCosRequest> UCosHelper::UploadFile(const FString& FilePathName
                                                 , const FString& URIPathName
                                                 , const FString& URLParameters
                                                 , FOnCosRequestCompleted OnCosRequestCompleted)
{
	if (FilePathName.IsEmpty())
	{
		UE_LOG(LogCosHelper, Warning, TEXT("Param FilePathName is empty."));
		return nullptr;
	}

	FRequestData* RequestData =
		CreateRequest(URIPathName
		            , URLParameters
		            , [&FilePathName](TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest){
		                HttpRequest->SetVerb(TEXT("PUT"));
		                if (!HttpRequest->SetContentAsStreamedFile(FilePathName))
		                {
		                  UE_LOG(LogCosHelper, Error, TEXT("Failed to stream from file: %s"), *FilePathName);
		                  return false;
		                }
		                return true;
		              }
		            , OnCosRequestCompleted);
	if (nullptr == RequestData)
	{
		return nullptr;
	}

	RequestData->LocalFilePathName = FilePathName;

	return RequestData->CosRequest;
}

void UCosHelper::GenerateHost(uint64 AppId, const FString& BucketName, const FString& Region)
{
	/**
	 * Host的形式为：<BucketName-APPID>.cos.<Region>.myqcloud.com
	 */
	Host = FString::Printf(TEXT("%s-%llu.cos.%s.myqcloud.com"), *BucketName, AppId, *Region);
}

FString UCosHelper::GenerateAuthorization(const IHttpRequest& HttpRequest, const FString& URIPathName)
{
	/**
	 * 密钥的生成可见：https://cloud.tencent.com/document/product/436/7778
	 */

	//~ Begin 生成KeyTime
	const FDateTime Now = FDateTime::UtcNow();
	const int64 StartTimestamp = Now.ToUnixTimestamp();
	const int64 EndTimestamp = StartTimestamp + SignExpirationTime;

	const FString KeyTime = FString::Printf(TEXT("%lld;%lld"), StartTimestamp, EndTimestamp);
	//~ End 生成KeyTime

	//~ Begin 生成SignKey
	const FTCHARToUTF8 SecretKeyData{ *SecretKey };
	const FTCHARToUTF8 KeyTimeData{ *KeyTime };

	uint8 SignKeyHash[20];
	FSHA1::HMACBuffer(SecretKeyData.Get(), SecretKeyData.Length(), KeyTimeData.Get(), KeyTimeData.Length(), SignKeyHash);

	const FString SignKey = BytesToHex(SignKeyHash, sizeof(SignKeyHash)).ToLower();
	//~ End 生成SignKey

	//~ Begin 生成UrlParamList和HttpParameters
	TMap<FString, FString> URLParameters;
	GetURLParameters(HttpRequest.GetURL(), URLParameters);

	FString URLParamList, HttpParameters;
	GenerateEncodedStrings(URLParameters, URLParamList, HttpParameters);
	//~ End 生成UrlParamList和HttpParameters

	//~ Begin 生成HeaderList和HttpHeaders
	TMap<FString, FString> HeaderNamesToValues;
	GetHeaderNamesToValues(HttpRequest, HeaderNamesToValues);

	FString HeaderList, HttpHeaders;
	GenerateEncodedStrings(HeaderNamesToValues, HeaderList, HttpHeaders);
	//~ End 生成HeaderList和HttpHeaders

	//~ Begin 生成HttpString
	const FString HttpMethod = HttpRequest.GetVerb().ToLower();
	const FString HttpString =
		FString::Printf(TEXT("%s\n%s\n%s\n%s\n"), *HttpMethod, *URIPathName, *HttpParameters, *HttpHeaders);
	//~ End 生成HttpString

	//~ Begin 生成StringToSign
	const FTCHARToUTF8 HttpStringData{ *HttpString };
	uint8 HttpStringHash[20];
	FSHA1::HashBuffer(HttpStringData.Get(), HttpStringData.Length(), HttpStringHash);
	const FString HttpStringHashHex = BytesToHex(HttpStringHash, sizeof(HttpStringHash)).ToLower();

	const FString StringToSign = FString::Printf(TEXT("sha1\n%s\n%s\n"), *KeyTime, *HttpStringHashHex);
	//~ End 生成StringToSign

	//~ Begin 生成Signature
	const FTCHARToUTF8 SignKeyData{ *SignKey };
	const FTCHARToUTF8 StringToSignData{ *StringToSign };

	uint8 SignatureHash[20];
	FSHA1::HMACBuffer(SignKeyData.Get(), SignKeyData.Length(), StringToSignData.Get(), StringToSignData.Length(), SignatureHash);

	const FString Signature = BytesToHex(SignatureHash, sizeof(SignatureHash)).ToLower();
	//~ End 生成Signature

	//~ Begin 生成签名
	const FString Sign =
		FString::Printf(TEXT("q-sign-algorithm=sha1&q-ak=%s&q-sign-time=%s&q-key-time=%s&q-header-list=%s&q-url-param-list=%s&q-signature=%s"),
		*SecretId, *KeyTime, *KeyTime, *HeaderList, *URLParamList, *Signature);
	//~ End 生成签名

	return Sign;
}

bool UCosHelper::GenerateEncodedStrings(const TMap<FString, FString>& Parameters, FString& OutKeyList, FString& OutString)
{
	if (0 == Parameters.Num())
	{
		return false;
	}

	TSortedMap<FString, FString> SortedParameters;
	for (auto& Param : Parameters)
	{
		const FString& EncodedKey = FPlatformHttp::UrlEncode(Param.Key);
		const FString& EncodedValue = FPlatformHttp::UrlEncode(Param.Value);
		SortedParameters.Add(EncodedKey.ToLower(), EncodedValue);
	}

	int32 Idx = 0;
	for (auto& Param : SortedParameters)
	{
		const FString& Key = Param.Key;
		const FString& Value = Param.Value;

		OutKeyList += Key;
		OutString += FString::Printf(TEXT("%s=%s"), *Key, *Value);

		++Idx;
		if (SortedParameters.Num() != Idx)
		{
			OutKeyList += TEXT(";");
			OutString += TEXT("&");
		}
	}

	return true;
}

bool UCosHelper::GetURLParameters(const FString& URL, TMap<FString, FString>& OutParameters)
{
	if (URL.IsEmpty())
	{
		return false;
	}

	FString Path, Parameters;
	if (!URL.Split(TEXT("?"), &Path, &Parameters))
	{
		return false;
	}

	TArray<FString> ParamElements;
	const int32 ElementCount = Parameters.ParseIntoArray(ParamElements, TEXT("&"), true);
	for (int32 Idx = 0; Idx < ElementCount; ++Idx)
	{
		FString Key, Value;
		if (ParamElements[Idx].Split(TEXT("="), &Key, &Value))
		{
			OutParameters.Add(Key, Value);
		}
		else
		{
			// No value, maybe like ?acl&
			OutParameters.Add(ParamElements[Idx], FString{});
		}
	}

	return true;
}

bool UCosHelper::GetHeaderNamesToValues(const IHttpRequest& HttpRequest, TMap<FString, FString>& OutHeaderNamesToValues)
{
	const TArray<FString> AllHeaders = HttpRequest.GetAllHeaders();
	if (0 == AllHeaders.Num())
	{
		return false;
	}

	const TCHAR Separator[] = TEXT(": ");
	for (auto& Header : AllHeaders)
	{
		FString HeaderName, HeaderValue;
		Header.Split(Separator, &HeaderName, &HeaderValue);
		OutHeaderNamesToValues.Add(HeaderName, HeaderValue);
	}

	return true;
}

UCosHelper::FRequestData* UCosHelper::CreateRequest(const FString& URIPathName
                                                  , const FString& URLParameters
                                                  , TFunction<bool(TSharedRef<IHttpRequest, ESPMode::ThreadSafe>)> OnFillHttpRequest
                                                  , FOnCosRequestCompleted OnCosRequestCompleted)
{
	if (URIPathName.IsEmpty() || !URIPathName.StartsWith(TEXT("/")))
	{
		UE_LOG(LogCosHelper, Warning, TEXT("Invalid param URIPathName: %s"), *URIPathName);
		return nullptr;
	}

	TSharedPtr<FRequestData>* pRequestData = URIToRequests.Find(URIPathName);
	if (nullptr != pRequestData)  // URI is processing
	{
		TSharedPtr<FRequestData> RequestData = *pRequestData;
		if (OnCosRequestCompleted.IsBound())
		{
			RequestData->CompletedDelegateInstances.Push(OnCosRequestCompleted);
		}

		return RequestData.Get();
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetHeader(TEXT("Host"), Host);

	// We must encode special characters for URI path name, otherwise the request will fail
	const FString EncodedURIPathName = EncodePathName(URIPathName);
	const FString URL = FString::Printf(TEXT("https://%s%s?%s"), *Host, *EncodedURIPathName, *URLParameters);
	HttpRequest->SetURL(URL);

	if (!OnFillHttpRequest(HttpRequest))
	{
		UE_LOG(LogCosHelper, Error, TEXT("Failed to start processing http request!!!"));
		return nullptr;
	}

	if (bUseAuthorization)
	{
		HttpRequest->SetHeader(TEXT("Authorization"), GenerateAuthorization(HttpRequest.Get(), URIPathName));
	}

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UCosHelper::OnHttpRequestCompleted);
	if (!HttpRequest->ProcessRequest())
	{
		UE_LOG(LogCosHelper, Error, TEXT("Failed to start processing http request!!!"));
		return nullptr;
	}

	TSharedPtr<FRequestData> NewRequestData = MakeShared<FRequestData>();
	NewRequestData->HttpRequest = HttpRequest;
	NewRequestData->URIPathName = URIPathName;

	UCosRequest* CosRequest = NewObject<UCosRequest>();
	CosRequest->AddToRoot();
	CosRequest->SetHttpRequest(HttpRequest);
	NewRequestData->CosRequest = CosRequest;

	if (OnCosRequestCompleted.IsBound())
	{
		NewRequestData->CompletedDelegateInstances.Push(OnCosRequestCompleted);
	}

	URIToRequests.Add(URIPathName, NewRequestData);
	HttpToRequests.Add(&HttpRequest.Get(), NewRequestData);

	return NewRequestData.Get();
}

FString UCosHelper::EncodePathName(const FString& InPathName) const
{
	TArray<FString> OutPathNames;
	if (0 != InPathName.ParseIntoArray(OutPathNames, TEXT("/")))
	{
		for (int Idx = 0; Idx < OutPathNames.Num(); ++Idx)
		{
			FString& PathName = OutPathNames[Idx];
			PathName = FPlatformHttp::UrlEncode(PathName);
		}

		return FString::Printf(TEXT("/%s"), *FString::Join(OutPathNames, TEXT("/")));
	}

	return InPathName;
}

bool UCosHelper::ReplaceWithCDNHost(IHttpRequest& InHttpRequest) const
{
	if (CDNHost.IsEmpty())
	{
		return false;
	}
	
	FString URL = InHttpRequest.GetURL();
	URL = URL.Replace(*Host, *CDNHost);
	InHttpRequest.SetURL(URL);

	return true;
}

void UCosHelper::OnHttpRequestCompleted(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bConnectedSuccessfully)
{
	TSharedPtr<FRequestData> RequestData = HttpToRequests.FindRef(HttpRequest.Get());
	if (!RequestData.IsValid())
	{
		UE_LOG(LogCosHelper, Error, TEXT("Failed to find RequestData for URL: %s"), *HttpRequest->GetURL());
		return;
	}
	HttpToRequests.Remove(HttpRequest.Get());

	if (bConnectedSuccessfully && HttpResponse.IsValid())
	{
		if (!EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			UE_LOG(LogCosHelper
			     , Error
			     , TEXT("Failed to request URL: %s. ResponseCode: %d.\nError: %s")
			     , *HttpResponse->GetURL(), HttpResponse->GetResponseCode(), *HttpResponse->GetContentAsString());
		}
		else
		{
			const FString Verb = HttpRequest->GetVerb();
			if (Verb.Equals(TEXT("GET")))
			{
				if (!RequestData->LocalFilePathName.IsEmpty())
				{
					if (!FFileHelper::SaveArrayToFile(HttpResponse->GetContent(), *RequestData->LocalFilePathName))
					{
						UE_LOG(LogCosHelper, Error, TEXT("Failed to save file: %s"), *RequestData->LocalFilePathName);
					}
				}
			}
		}
	}

	if (0 != RequestData->CompletedDelegateInstances.Num())
	{
		UCosResponse* CosResponse = NewObject<UCosResponse>();
		CosResponse->AddToRoot();
		CosResponse->SetHttpResponse(HttpResponse);

		const FString Verb = HttpRequest->GetVerb();
		if (Verb.Equals(TEXT("HEAD")))
		{
			CosResponse->GenerateFileInfos(RequestData->FileInfoType);
		}

		for (auto& OnCompleted : RequestData->CompletedDelegateInstances)
		{
			if (OnCompleted.IsBound())
			{
				OnCompleted.Execute(*CosResponse);
			}
		}

		CosResponse->RemoveFromRoot();
	}

	URIToRequests.Remove(RequestData->URIPathName);
}

UCosHelper::FRequestData::~FRequestData()
{
	HttpRequest = nullptr;

	if (CosRequest->IsValidLowLevel())
	{
		CosRequest->RemoveFromRoot();
		CosRequest->ConditionalBeginDestroy();
		CosRequest = nullptr;
	}

	CompletedDelegateInstances.Empty();
}
