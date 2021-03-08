
#include "Server.h"
#include "TimerManager.h" //for GetWorldTimerManager
#include <Runtime/Sockets/Public/Sockets.h>
#include <Runtime/Networking/Public/Interfaces/IPv4/IPv4Endpoint.h>
#include <Runtime/Networking/Public/Interfaces/IPv4/IPv4Address.h>
#include <Runtime/Networking/Public/Common/TcpSocketBuilder.h>
#include <Runtime/Networking/Public/Networking.h>
#include "FugenColor.h"


//INFO: Der UE spezifische Fstring ist besonders, da mit ihm gesucht, verglichen und ersetzt werden kann
//        Ist aber auch teurer als die normalen String Klassen in C++


const FString AServer::DEFAULT_RENDER = "RENDERWITHIMAGE::";
const FString AServer::FUGEN_COLOR = "fugencolor_1";
const FString AServer::ROTATE_FLOOR = "ROTATE_FLOOR::";


// Sets default values
AServer::AServer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AServer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (ListenerSocket)
	{
		ListenerSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenerSocket);
	}

	if (ConnectionSocket)
	{
		ConnectionSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket);
	}

	UE_LOG(LogTemp, Error, TEXT("Disposed previous sockets..."));
}

// Called when the game starts or when spawned
void AServer::BeginPlay()
{
	Super::BeginPlay();
	if (!StartTCPReceiver("SocketTest", "127.0.0.1", 454))
	{
		UE_LOG(LogTemp, Warning, TEXT("TCP SOCKET LISTENER FAILED!"));
		return;
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("TCP SOCKET LISTENER STARTED!"));
	}
}

// Called every frame
void AServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


bool AServer::StartTCPReceiver(
	const FString& socketName,
	const FString& ip,
	const int32 port
)
{
	ListenerSocket = CreateTCPConnectionListener(socketName, ip, port, 20000000);

	//Not created?
	if (!ListenerSocket)
	{
		UE_LOG(LogTemp, Error, TEXT("StartTCPReceiver>> Listen socket could not be created!~> % s % d"), *ip, port);
		return false;
	}
	UE_LOG(LogTemp, Warning, TEXT("StartTCPReceiver>> Listen socket created!~> % s % d"), *ip, port);

	FTimerHandle ConnectionListenTimer; GetWorldTimerManager().SetTimer(ConnectionListenTimer, this, &AServer::TCPConnectionListener, 0.01f, true);

	return true;
}

/// <summary>
/// Formatiert die vom Typ String bestehende IP in nummern
/// </summary>
/// <param name="ip"></param>
/// <param name="Out"></param>
/// <returns></returns>
bool AServer::FormatIP4ToNumber(const FString& ip, uint8(&Out)[4])
{
	//IP Formatting
	ip.Replace(TEXT(" "), TEXT(""));
	const TCHAR* delim;
	delim = TEXT(".");

	TArray<FString> Parts;
	ip.ParseIntoArray(Parts, delim, true);
	if (Parts.Num() != 4)
		return false;

	for (int32 i = 0; i < 4; ++i)
	{
		Out[i] = FCString::Atoi(*Parts[i]);
	}
	return true;
}

/// <summary>
/// Erstellt den Tcp Listener
/// </summary>
/// <param name="socketName">Name des Listeners</param>
/// <param name="ip">Ip</param>
/// <param name="port">Port</param>
/// <param name="receiveBufferSize">Buffersize die der Listener erhählt< / param>
/// <returns></returns>
FSocket* AServer::CreateTCPConnectionListener(const FString& socketName,
	const FString& ip, const uint16 port, const int32 receiveBufferSize)
{
	uint8 IP4Nums[4];
	if (!FormatIP4ToNumber(ip, IP4Nums))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid IP! Expecting 4 parts separated by ."));
		return false;
	}
	//Create Socket
	FIPv4Endpoint Endpoint(FIPv4Address(IP4Nums[0], IP4Nums[1],
		IP4Nums[2], IP4Nums[3]), port);
	FSocket* ListenSocket = FTcpSocketBuilder(*socketName)
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.Listening(8);
	//Set Buffer Size
	int32 NewSize = 0;
	ListenSocket->SetReceiveBufferSize(receiveBufferSize, NewSize);
	return ListenSocket;
}


void AServer::TCPConnectionListener()
{
	if (!ListenerSocket)
	{
		UE_LOG(LogTemp, Error, TEXT("Listener doesn't exist, but should."));
		return;
	}

	//Remote address
	TSharedRef<FInternetAddr> RemoteAddress =
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	bool Pending;

	// handle incoming connections
	if (ListenerSocket->HasPendingConnection(Pending) && Pending)
	{
		UE_LOG(LogTemp, Warning, TEXT("Connection Pending..."));
		//Already have a Connection? destroy previous
		if (ConnectionSocket)
		{
			ConnectionSocket->Close();
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket);
		}

		ConnectionSocket = ListenerSocket->Accept(*RemoteAddress,
			TEXT("TODO: Start Kick off Event"));

		if (ConnectionSocket != NULL)
		{
			//Global cache of current Remote Address
			RemoteAddressForConnection = FIPv4Endpoint(RemoteAddress);
			UE_LOG(LogTemp, Warning, TEXT("Accepted Connection!"));

			FTimerHandle SocketListenTimer;
			GetWorldTimerManager().SetTimer(SocketListenTimer, this,
				&AServer::TCPSocketListener, 0.01f, true);
		}
	}
}

FString AServer::StringFromBinaryArray(const TArray<uint8>& BinaryArray)
{
	//Set byte array to a string
	std::string cstr(reinterpret_cast<const
		char*>(BinaryArray.GetData()), BinaryArray.Num());
	return FString(cstr.c_str());
}


void AServer::TCPSocketListener()
{

	if (!ConnectionSocket) return;

	TArray<uint8> ReceivedData;
	uint32 Size;
	while (ConnectionSocket->HasPendingData(Size))
	{
		ReceivedData.Init(0.0f, FMath::Min(Size, 65507u));

		int32 Read = 0;
		ConnectionSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);

		UE_LOG(LogTemp, Warning, TEXT("Data Read! %d"), ReceivedData.Num());
	}

	if (ReceivedData.Num() <= 0)
	{
		//No Data Received
		return;
	}
	const FString ReceivedUE4String = StringFromBinaryArray(ReceivedData);
	//Logging
	UE_LOG(LogTemp, Warning, TEXT("Total Data read! %d"), ReceivedData.Num());
	UE_LOG(LogTemp, Warning, TEXT("As String!!!!! ~> %s"), *ReceivedUE4String);

	ProcessReceivedMessage(ReceivedUE4String);
}


TArray<uint8> AServer::ConvertToUTF8(const FString& InString)
{
	FString UseStr(InString);
	TCHAR* SendData = UseStr.GetCharArray().GetData();
	int32 DataLen = FCString::Strlen(SendData);
	uint8* Temp = (uint8*)TCHAR_TO_UTF8(SendData);
	TArray<uint8> SendTemp;
	for (int i = 0; i < DataLen; i++) //Durch alle Chars durchgehen...
	{
		SendTemp.Add(Temp[i]); //TArray füllen mit Char an stelle i...
	}

	return SendTemp;
}

void AServer::ProcessReceivedMessage(FString message)
{
	FString Left, Right; //Beide seiten neben des Seperators
	message.Split(TEXT("::"), &Left, &Right); //Splitten Left=Funktionalität wie rotatefloor::->Right = Wert z.b. 90 für 90°

	if (Left == FUGEN_COLOR) //Wechsel der Fugenfarbe
	{
		if (Right != "") //Wenn Wert zur Änderung vorhanden
		{
			//Hexcolor der neuen fugenfarbe !Atoi=Unsafe!
			int32 receivedHexColor = FCString::Atoi(*Right);	
			//Umwandlung zu FLinearColor da Typ für Änderung des Mats benötigt
			FLinearColor newFugenColor = HexToLinearColor(receivedHexColor);	
			//Instanz des  FugenObjektes holen und Methode zum ändern aufrufen
			AFugenColor* fugenChangeObj = Cast<AFugenColor>(GetWorld()->SpawnActor(AFugenColor::StaticClass()));
			fugenChangeObj->ChangeFugenColor(newFugenColor);
		}
	}
	else if (Left == ROTATE_FLOOR) 
	{

	}
}

void AServer::SendMessageToClient(const FString& textToSend)
{
	TArray<uint8> SendBuffer = ConvertToUTF8(textToSend);
	int32 SendReader;

	if (ConnectionSocket)
	{
		ConnectionSocket->Send(SendBuffer.GetData(), SendBuffer.Num(), SendReader);
	}
}

FLinearColor AServer::HexToLinearColor(int hexColor)
{
	typedef struct RGB {
		double r;
		double g;
		double b;
	};

	struct RGB rgbColor;
	rgbColor.r = ((hexColor >> 16) & 0xFF) / 255.0;  // Extract the RR byte
	rgbColor.g = ((hexColor >> 8) & 0xFF) / 255.0;   // Extract the GG byte
	rgbColor.b = ((hexColor) & 0xFF) / 255.0;        // Extract the BB byte

	return FLinearColor(rgbColor.r, rgbColor.g, rgbColor.b);
}