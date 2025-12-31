#include "TaleStoriesBPLibrary.h"
#include "Async/Async.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "Serialization/JsonSerializer.h"
#include "GenericPlatform/GenericPlatformHttp.h" // Вместо Apple-специфичного

TUniquePtr<GrpcReproxer> UTaleStoriesBPLibrary::ReproxerInstance = nullptr;

bool UTaleStoriesBPLibrary::ConnectAndJoin(FString Token, FString ServerAddress)
{
	if (!ReproxerInstance.IsValid())
	{
		ReproxerInstance = MakeUnique<GrpcReproxer>(TCHAR_TO_UTF8(*ServerAddress));
	}

	if (ReproxerInstance->ConnectToServer(TCHAR_TO_UTF8(*Token)))
	{
		std::string SessionId;
		if (ReproxerInstance->JoinGameSession(SessionId))
		{
			UE_LOG(LogTemp, Log, TEXT("✅ Successfully joined session: %s"), *FString(SessionId.c_str()));
			return true;
		}
	}

	return false;
}

void UTaleStoriesBPLibrary::LoginWithGoogle(FString ClientId, FString ClientSecret, FString ServerAddress,
                                            FOnGoogleLoginComplete OnComplete)
{
	UE_LOG(LogTemp, Log, TEXT("✅ LoginWithGoogle started!"));

	int32 Port = 1234;
	FString RedirectUri = FString::Printf(TEXT("http://localhost:%d"), Port);

	FString AuthUrl = FString::Printf(
		TEXT(
			"https://accounts.google.com/o/oauth2/v2/auth?client_id=%s&redirect_uri=%s&response_type=code&scope=openid%%20email&access_type=offline&prompt=consent"),
		*ClientId, *RedirectUri);

	FIPv4Endpoint Endpoint(FIPv4Address::Any, Port);
	FSocket* ListenSocket = FTcpSocketBuilder(TEXT("GoogleAuthListener"))
	                        .AsReusable()
	                        .BoundToEndpoint(Endpoint)
	                        .Listening(8);

	if (!ListenSocket)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Socket Error: Could not bind port 1234"));
		return;
	}

	FPlatformProcess::LaunchURL(*AuthUrl, nullptr, nullptr);

	// Используем лямбду с захватом копий, чтобы избежать проблем с памятью
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
	          [ListenSocket, ClientId, ClientSecret, RedirectUri, OnComplete, ServerAddress]()
	          {
		          bool bCodeReceived = false;
		          uint32 StartTime = FPlatformTime::Seconds();

		          while (!bCodeReceived && (FPlatformTime::Seconds() - StartTime) < 60)
		          {
			          bool bHasPendingConnection = false;
			          ListenSocket->HasPendingConnection(bHasPendingConnection);

			          if (bHasPendingConnection)
			          {
				          TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->
					          CreateInternetAddr();
				          FSocket* ConnectionSocket = ListenSocket->Accept(*RemoteAddress, TEXT("GoogleConnection"));

				          if (ConnectionSocket)
				          {
					          FPlatformProcess::Sleep(0.2f);
					          TArray<uint8> ReceivedData;
					          uint32 DataSize = 0;
					          ConnectionSocket->HasPendingData(DataSize);

					          if (DataSize > 0)
					          {
						          ReceivedData.SetNumUninitialized(DataSize);
						          int32 BytesRead = 0;
						          ConnectionSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), BytesRead);

						          FString Payload = FString(
							          UTF8_TO_TCHAR(reinterpret_cast<const char*>(ReceivedData.GetData())));

						          FString Code;
						          if (Payload.Split(TEXT("?code="), nullptr, &Code))
						          {
							          Code.Split(TEXT(" "), &Code, nullptr);
							          Code.Split(TEXT("&"), &Code, nullptr);
							          Code = FGenericPlatformHttp::UrlDecode(Code);

							          bCodeReceived = true;

							          // Отвечаем браузеру
							          FString Response = TEXT(
								          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h2>Success! Return to game.</h2></body></html>");
							          int32 Sent = 0;
							          ConnectionSocket->Send((uint8*)TCHAR_TO_UTF8(*Response), Response.Len(), Sent);

							          // Обмен Code -> ID Token
							          AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
							                    [Code, ClientId, ClientSecret, RedirectUri, OnComplete, ServerAddress]()
							                    {
								                    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> TokenReq =
									                    FHttpModule::Get().CreateRequest();
								                    TokenReq->SetURL(TEXT("https://oauth2.googleapis.com/token"));
								                    TokenReq->SetVerb(TEXT("POST"));
								                    TokenReq->SetHeader(
									                    TEXT("Content-Type"),
									                    TEXT("application/x-www-form-urlencoded"));

								                    FString PostData = FString::Printf(
									                    TEXT(
										                    "code=%s&client_id=%s&client_secret=%s&redirect_uri=%s&grant_type=authorization_code"),
									                    *Code, *ClientId, *ClientSecret, *RedirectUri);

								                    TokenReq->SetContentAsString(PostData);
								                    TokenReq->OnProcessRequestComplete().BindLambda(
									                    [OnComplete, ServerAddress](
									                    FHttpRequestPtr, FHttpResponsePtr Resp,
									                    bool bSuccess)
									                    {
										                    if (bSuccess && Resp.IsValid())
										                    {
											                    TSharedPtr<FJsonObject> JsonObj;
											                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory
												                    <>::Create(Resp->GetContentAsString());
											                    if (FJsonSerializer::Deserialize(Reader, JsonObj))
											                    {
												                    FString IdToken;
												                    if (JsonObj->TryGetStringField(
													                    TEXT("id_token"), IdToken))
												                    {
													                    AsyncTask(ENamedThreads::GameThread,
														                    [OnComplete, IdToken, ServerAddress]()
														                    {
															                    OnComplete.ExecuteIfBound(
																                    IdToken, ServerAddress);
														                    });
													                    return;
												                    }
											                    }
										                    }
										                    UE_LOG(LogTemp, Error,
										                           TEXT("❌ Failed to exchange code for token"));
									                    });
								                    TokenReq->ProcessRequest();
							                    });
						          }
					          }
					          ConnectionSocket->Close();
					          ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket);
				          }
			          }
			          FPlatformProcess::Sleep(0.1f);
		          }
		          ListenSocket->Close();
		          ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
	          });
}
