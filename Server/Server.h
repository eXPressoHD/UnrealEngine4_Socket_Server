#pragma once

#include <string>
#include "Networking.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Server.generated.h"

UCLASS()
class BASICCODINGUE_API AServer : public AActor
{
    GENERATED_BODY()

public:
    FSocket* ListenerSocket;
    FSocket* ConnectionSocket;
    FIPv4Endpoint RemoteAddressForConnection;

    // Sets default values for this actor's properties
    AServer();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

private:
    void EndPlay(const EEndPlayReason::Type EndPlayReason);
    bool StartTCPReceiver(const FString& socketName, const FString& ip, const int32 port);
    bool FormatIP4ToNumber(const FString& ip, uint8(&Out)[4]);
    FSocket* CreateTCPConnectionListener(const FString& socketName,const FString& ip, const uint16 port, const int32 receiveBufferSize);
    void TCPConnectionListener();
    FString StringFromBinaryArray(const TArray<uint8>& BinaryArray);
    void TCPSocketListener();
    TArray<uint8> ConvertToUTF8(const FString& InString);

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

};