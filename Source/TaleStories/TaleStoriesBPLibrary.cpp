#include "TaleStoriesBPLibrary.h"
#include "Async/Async.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Common/TcpSocketBuilder.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"

TUniquePtr<GrpcReproxer> UTaleStoriesBPLibrary::ReproxerInstance = nullptr;

void UTaleStoriesBPLibrary::LoginWithGoogle(FString ClientId, FString ServerAddress, FOnGoogleLoginComplete OnComplete)
{
	UE_LOG(LogTemp, Log, TEXT("✅ Starting Auth Process..."));

	// Инициализируем библиотеку, если еще не создана
	if (!ReproxerInstance.IsValid())
	{
		ReproxerInstance = MakeUnique<GrpcReproxer>(TCHAR_TO_UTF8(*ServerAddress));
	}

	int32 Port = 1234;
	FString RedirectUri = FString::Printf(TEXT("http://localhost:%d"), Port);

	// Формируем URL для получения AUTH CODE
	FString AuthUrl = FString::Printf(
		TEXT(
			"https://accounts.google.com/o/oauth2/v2/auth?client_id=%s&redirect_uri=%s&response_type=code&scope=openid%%20email&prompt=consent"),
		*ClientId, *RedirectUri);

	FIPv4Endpoint Endpoint(FIPv4Address::Any, Port);
	FSocket* ListenSocket = FTcpSocketBuilder(TEXT("GoogleAuthListener"))
	                        .AsReusable()
	                        .BoundToEndpoint(Endpoint)
	                        .Listening(8);

	if (!ListenSocket)
	{
		OnComplete.ExecuteIfBound(false, TEXT("Failed to bind local port 1234"));
		return;
	}

	FPlatformProcess::LaunchURL(*AuthUrl, nullptr, nullptr);

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ListenSocket, RedirectUri, OnComplete]()
	{
		bool bSuccess = false;
		FString ErrorMsg = TEXT("Timeout");
		uint32 StartTime = FPlatformTime::Seconds();

		while (!bSuccess && (FPlatformTime::Seconds() - StartTime) < 60)
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

						FString Payload = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(ReceivedData.GetData())));

						FString Code;
						if (Payload.Split(TEXT("?code="), nullptr, &Code))
						{
							Code.Split(TEXT(" "), &Code, nullptr);
							Code.Split(TEXT("&"), &Code, nullptr);
							Code = FGenericPlatformHttp::UrlDecode(Code);

							// Мы получили CODE! Теперь отдаем его в lib_ue для обмена на JWT через наш Kotlin
							if (ReproxerInstance->Authenticate(TCHAR_TO_UTF8(*Code), TCHAR_TO_UTF8(*RedirectUri)))
							{
								bSuccess = true;
								ErrorMsg = TEXT("");
								UE_LOG(LogTemp, Log, TEXT("Server Authenticate"));
							}
							else
							{
								UE_LOG(LogTemp, Warning, TEXT("Authentication failed: %s"), *RedirectUri);
								ErrorMsg = TEXT("Server rejected Auth Code");
							}

							FString Response = TEXT(
								"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h2>Auth Successful! Return to Unreal.</h2></body></html>");
							int32 Sent = 0;
							ConnectionSocket->Send((uint8*)TCHAR_TO_UTF8(*Response), Response.Len(), Sent);
						}
					}
					ConnectionSocket->Close();
					ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket);
				}
			}
		}

		ListenSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);

		// Возвращаемся в Game Thread для вызова делегата
		AsyncTask(ENamedThreads::GameThread, [OnComplete, bSuccess, ErrorMsg]()
		{
			UE_LOG(LogTemp, Log, TEXT("Run OnComplete for LoginWithGoogle"));
			OnComplete.ExecuteIfBound(bSuccess, ErrorMsg);
		});
	});
}

bool UTaleStoriesBPLibrary::JoinGameRoom(FString RoomName, FString& OutDedicatedAddr, FString& OutErrorMessage)
{
	if (!ReproxerInstance.IsValid()) return false;

	std::string server_addr;
	if (ReproxerInstance->JoinRoom(TCHAR_TO_UTF8(*RoomName), server_addr))
	{
		OutDedicatedAddr = FString(server_addr.c_str());
		
		UE_LOG(LogTemp, Log, TEXT("Joined Room: %s"), *OutDedicatedAddr);

		// СРАЗУ переключаем библиотеку на работу с новым выделенным сервером
		ReproxerInstance->ConnectToDedicated(server_addr);

		return true;
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to join room %s"), *RoomName);
	}

	OutErrorMessage = TEXT("Lobby could not allocate room");
	return false;
}

bool UTaleStoriesBPLibrary::PingDedicatedServer(int64& OutServerTime)
{
	if (!ReproxerInstance.IsValid()) return false;
	return ReproxerInstance->PingDedicated(OutServerTime);
}
