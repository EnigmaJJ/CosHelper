// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "CosHelper.generated.h"

enum class ECosHelperFileInfoType : uint8;

struct FCosHelperInitializeInfo;

class UCosRequest;
class UCosResponse;

UCLASS(BlueprintType)
class COSHELPER_API UCosHelper : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_DELEGATE_OneParam(FOnCosRequestCompleted, const UCosResponse& /*CosResponse*/);

public:
	bool Initialize(const FCosHelperInitializeInfo& InitializeInfo);

	FORCEINLINE void SetCDNHost(const FString& InHost) { CDNHost = InHost; }

	/**
	 * 从服务器获取文件信息（避免下载文件）
	 * @param URIPathName 服务器上的文件路径名，文件路径名需要以'/'开头，相对于存储桶，如"/v.txt"
	 * @param URLParameters 请求参数，会添加到Http请求路径之后，如"acl=9" -> https://v.txt?acl=9
	 * @param FileInfoType 需要获取的信息类型，是ECosHelperFileInfoType枚举的组合
	 * @param OnCosRequestCompleted 文件信息获取后的回调
	 *
	 * @remark 最终生成的URL形式为：https://Host[URIPathName]?[URLParameters]
	 */
	TWeakObjectPtr<UCosRequest> GetFileInfo(const FString& URIPathName
	                                      , const FString& URLParameters
	                                      , ECosHelperFileInfoType FileInfoType
	                                      , FOnCosRequestCompleted OnCosRequestCompleted);

	/**
	 * 从服务器下载文件
	 * @param URIPathName 服务器上的文件路径名，文件路径名需要以'/'开头，相对于存储桶，如"/v.txt"
	 * @param URLParameters 请求参数，会添加到Http请求路径之后，如"acl=9" -> https://v.txt?acl=9
	 * @param SavedFilePathName 文件存储路径名，如果为空，则不会保存下载的文件
	 * @param OnCosRequestCompleted 文件下载完成后的回调
	 *
	 * @remark 最终生成的URL形式为：https://Host[URIPathName]?[URLParameters]
	 */
	TWeakObjectPtr<UCosRequest> DownloadFile(const FString& URIPathName
	                                       , const FString& URLParameters
	                                       , const FString& SavedFilePathName
	                                       , FOnCosRequestCompleted OnCosRequestCompleted);

	/**
	 * 上传文件到服务器
	 * @param FilePathName 要上传到服务器的本地文件路径名
	 * @param URIPathName 文件在服务器上存储的路径名，路径名需要以'/'开头，相对于存储桶，如"/v.txt"
	 * @param URLParameters 请求参数，会添加到Http请求路径之后，如"acl=9" -> https://v.txt?acl=9
	 * @param OnCosRequestCompleted 文件上传完成后的回调
	 *
	 * @remark 最终生成的URL形式为：https://Host[URIPathName]?[URLParameters]
	 */
	TWeakObjectPtr<UCosRequest> UploadFile(const FString& FilePathName
	                                     , const FString& URIPathName
	                                     , const FString& URLParameters
	                                     , FOnCosRequestCompleted OnCosRequestCompleted);

private:
	struct FRequestData
	{
		/** 服务器上的资源路径名，路径名以'/'开头，相对于存储桶，如"/v.txt" */
		FString URIPathName;

		/** 本地文件路径名。对于下载请求，是下载后保存在本地的文件路径名；对于上传请求，是需要上传的本地文件路径名 */
		FString LocalFilePathName;

		UCosRequest* CosRequest;
		TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest;
		TArray<FOnCosRequestCompleted> CompletedDelegateInstances;

		ECosHelperFileInfoType FileInfoType;

		~FRequestData();
	};

private:
	void GenerateHost(uint64 AppId, const FString& BucketName, const FString& Region);
	FString GenerateAuthorization(const IHttpRequest& HttpRequest, const FString& URIPathName);

	/**
	 * 根据Map中的键值对，生成2个经过URL编码的串
	 * OutKeyList的格式为：key1;key2;key3
	 * OutString的格式为：key1=value1&key2=value2;key3=value3
	 */
	bool GenerateEncodedStrings(const TMap<FString, FString>& Parameters, FString& OutKeyList, FString& OutString);

	/**
	 * 从URL中获取请求参数的键值对
	 * @remark 获取的键值对的数值不会进行URL解码，因此传入的URL不应该是经过编码的内容
	 */
	bool GetURLParameters(const FString& URL, TMap<FString, FString>& OutParameters);

	/**
	 * 将Http请求头部的数据按键值对的方式填充到OutHeaderNamesToValues中
	 */
	bool GetHeaderNamesToValues(const IHttpRequest& HttpRequest, TMap<FString, FString>& OutHeaderNamesToValues);

	FRequestData* CreateRequest(const FString& URIPathName
	                          , const FString& URLParameters
	                          , TFunction<bool(TSharedRef<IHttpRequest, ESPMode::ThreadSafe>)> OnFillHttpRequest
	                          , FOnCosRequestCompleted OnCosRequestCompleted);

	FString EncodePathName(const FString& InPathName) const;

	bool ReplaceWithCDNHost(IHttpRequest& InHttpRequest) const;

	void OnHttpRequestCompleted(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bConnectedSuccessfully);

private:
	FString Host;
	FString CDNHost;

	bool bUseAuthorization;
	uint32 SignExpirationTime;
	FString SecretId;
	FString SecretKey;

	/** Key is URIPathName */
	TMap<FString, TSharedPtr<FRequestData>> URIToRequests;

	/** HttpRequest完成时，我们需要通过HttpRequest来获取对应的RequestData */
	TMap<IHttpRequest*, TSharedPtr<FRequestData>> HttpToRequests;
};
