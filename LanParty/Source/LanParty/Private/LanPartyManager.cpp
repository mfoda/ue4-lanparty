#include "LanPartyManager.h"
#include "LanParty.h"

#include "Engine/Engine.h"
#include "MessageEndpoint.h"
#include "MessageEndpointBuilder.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Misc/CommandLine.h"
#include "Timespan.h"
#include <iso646.h>

#define print(text, ...) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, FString::Printf(text, __VA_ARGS__))

ULanPartyManager::ULanPartyManager()
{
    // read cmdline args
    const auto CommandLineArguments = FCommandLine::Get();
    int32 playerIndex;
    int32 maxPlayers;

    if (FParse::Value(CommandLineArguments, TEXT("PlayerIndex"), playerIndex))
        PlayerIndex = playerIndex;
    if (FParse::Value(CommandLineArguments, TEXT("MaxPlayers"), maxPlayers))
        MaxPlayers = maxPlayers;

    // get own IP address
    auto CanBind = false;
    LocalHostAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, CanBind)->ToString(false);

    // setup messaging endpoints
    MessageSenderEndpoint   = FMessageEndpoint::Builder("FPartySenderMessageEndpoint").Build();
    MessageReceiverEndpoint = FMessageEndpoint::Builder("FPartyReceiverMessageEndpoint")
                              .Handling<FLanPartyMessage>(this, &ULanPartyManager::HandlePartyMessage).Build();

    // subscribe to party messages
    if (MessageReceiverEndpoint.IsValid())
        MessageReceiverEndpoint->Subscribe<FLanPartyMessage>();
}

void ULanPartyManager::BeginDestroy()
{
    MessageSenderEndpoint.Reset();
    MessageReceiverEndpoint.Reset();

    Super::BeginDestroy();
}

bool ULanPartyManager::IsInParty() const
{
    return CurrentParticipants.Contains(LocalHostAddress);
}

void ULanPartyManager::BroadcastDiscoveryMessage() const
{
    PublishPartyMessage(ELanPartyMessageType::Discovery);
}

void ULanPartyManager::HandlePartyMessage(const FLanPartyMessage& Message,
                                          const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& Context) 
{
    if (not Context->IsValid())
        return;

    // skip own messages
    if (Message.IpAddress == LocalHostAddress)
        return;

    // call appropriate message handler
    switch (Message.PartyMessageType) 
    { 
        case ELanPartyMessageType::None:
            UE_LOG(LogParty, Error, TEXT("Received a message with PartyEventType::None"));
            break;
        case ELanPartyMessageType::Discovery:
            OnPlayerDiscovered(Context->GetSender(), Message);
            break;
        case ELanPartyMessageType::JoinParty:
            OnPlayerJoinedParty(Message);
            break;
        case ELanPartyMessageType::LeaveParty:
            OnPlayerLeftParty(Message);
            break;
        case ELanPartyMessageType::SkipCountdown:
            OnSkipCountdown();
            break;
        case ELanPartyMessageType::ServerReady: 
            OnServerReady(Message);
        break;
    }
}

void ULanPartyManager::OnPlayerDiscovered(const FMessageAddress& SenderAddress, const FLanPartyMessage& Message)
{
    KnownEndpoints.AddUnique(SenderAddress);

    // calculate peer match start time and update if greate than ours
    const auto TheirMatchStartTime = FDateTime::Now() + FTimespan::FromSeconds(Message.MatchStartTimeOffset);

    if (TheirMatchStartTime > MatchStartTime)
        MatchStartTime = TheirMatchStartTime;

    // re-trigger event in case players joined before this instance was alive
    if (Message.IsInCurrentParty)
        OnPlayerJoinedParty(Message);
}

void ULanPartyManager::OnPlayerJoinedParty(const FLanPartyMessage& Message)
{
    const auto AlreadyInParty = CurrentParticipants.Contains(Message.IpAddress);

    CurrentParticipants.AddUnique(Message.IpAddress);

    // if (not AlreadyInParty)
        PlayerJoinedPartyDelegate.Broadcast(Message.PlayerIndex);
}

void ULanPartyManager::OnPlayerLeftParty(const FLanPartyMessage& Message)
{
    const auto AlreadyLeftParty = not CurrentParticipants.Contains(Message.IpAddress);

    CurrentParticipants.RemoveSingle(Message.IpAddress);

    // if (not AlreadyLeftParty)
        PlayerLeftPartyDelegate.Broadcast(Message.PlayerIndex);

    // reset all timers in case we are spectator and last party member left
    if (CurrentParticipants.Num() == 0)
        ResetParty();
}

void ULanPartyManager::OnServerReady(const FLanPartyMessage& Message) 
{
    const auto bIsInParty = IsInParty();
    ResetParty();

    if (bIsInParty)
        ServerReadyDelegate.Broadcast(Message.IpAddress);
}

void ULanPartyManager::OnSkipCountdown() const
{
    SkipCountdownDelegate.Broadcast();    
}

void ULanPartyManager::JoinParty()
{
    if (IsInParty())
        return;

    CurrentParticipants.AddUnique(LocalHostAddress);

    PlayerJoinedPartyDelegate.Broadcast(PlayerIndex);
    
    // manually construct message in order to update MatchStartTime
    const auto Offset = FTimespan::FromSeconds(MatchmakingLengthSeconds); 
    MatchStartTime = FDateTime::Now() + Offset;

    const auto Message = new FLanPartyMessage(PlayerIndex, LocalHostAddress, ELanPartyMessageType::JoinParty,
                                              IsInParty(), Offset.GetSeconds());

    PublishPartyMessage(Message);
}

/** Party leave is called separately on clients and servers after matchmaking in order to avoid time sync issues,
 *  thus ShouldBroadcast is used to differentiate between a normal leave and a post-matchmaking leave
 */
void ULanPartyManager::LeaveParty()
{
    if (not IsInParty())
        return;

    CurrentParticipants.RemoveSingle(LocalHostAddress);

    PlayerLeftPartyDelegate.Broadcast(PlayerIndex);

    PublishPartyMessage(ELanPartyMessageType::LeaveParty);

    // reset all timers in case we were the only party member
    if (CurrentParticipants.Num() == 0)
        ResetParty();
}

void ULanPartyManager::SkipCountdown() const
{
    if (not IsInParty())
        return;

    PublishPartyMessage(ELanPartyMessageType::SkipCountdown);
}

bool ULanPartyManager::CheckIsGameHost()
{
   if (not IsInParty())
       return false;

   const auto ServerAddress = GetServerIpAddress();
   const auto IsGameHost = ServerAddress == LocalHostAddress;

   // party leave is done separately on clients and server to avoid time sync issues
   if (IsGameHost)
   {
       ResetParty();
       PublishPartyMessage(ELanPartyMessageType::ServerReady);
   }

   return IsGameHost;
}

void ULanPartyManager::PrintPartyInfo() const
{
    auto const World = GetWorld();
    if (World)
    {
        print(TEXT("My IP: %s"), *LocalHostAddress);
        print(TEXT("Known Endpoints: %d"), KnownEndpoints.Num());
        print(TEXT("In Party: %d"), CurrentParticipants.Num());
    }
}

void ULanPartyManager::PublishPartyMessage(const ELanPartyMessageType MessageType) const
{
    if (MessageSenderEndpoint.IsValid())
    {
        const auto Offset  = FMath::Max(0.0, (MatchStartTime - FDateTime::Now()).GetTotalSeconds());
        const auto Message = new FLanPartyMessage(PlayerIndex, LocalHostAddress, MessageType, IsInParty(), Offset);
        MessageSenderEndpoint->Publish<FLanPartyMessage>(Message);
    }
}

void ULanPartyManager::PublishPartyMessage(FLanPartyMessage* Message) const
{
    if (MessageSenderEndpoint.IsValid())
    {
        MessageSenderEndpoint->Publish<FLanPartyMessage>(Message);
    }
}

FString ULanPartyManager::GetServerIpAddress()
{
    CurrentParticipants.Sort();
    return CurrentParticipants.Top();
}

void ULanPartyManager::ResetParty()
{
    CurrentParticipants.Empty();

    MatchStartTime = FDateTime::MinValue(); 

    PartyResetDelegate.Broadcast();
}

