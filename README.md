# CosHelper
CosHelper is a plugin for UE which is used to download/upload resources from/to COS of Tencent Cloud.

## Compiled and verified UE version
* UE4.26

## Update
##### 2021.06.29
Fix: values do not need to be converted to lower case in GenerateEncodedStrings method
Fix: fail to request when there are special characters in URIPathName
Fix: crash in OnHttpRequestCompleted method because the element was removed before it was dereferenced
