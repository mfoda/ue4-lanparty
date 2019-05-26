#pragma once

#include "LanPartyMessage.h"

#include "LanPartyManager.generated.h"

struct FMessageAddress;
class IMessageContext;
class FMessageEndpoint;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerIndexDelegate, uint8, PlayerIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerAddressDelegate, FString, ServerAddress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSkipCountdownDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTriggerDelegate);

UCLASS(BlueprintType)
class LANPARTY_API ULanPartyManager : public UObject
{
    GENERATED_BODY()

    /** Message Bus sending and receiving endpoints */
    TSharedPtr<FMessageEndpoint, ESPMode::ThreadSafe> MessageSenderEndpoint;
    TSharedPtr<FMessageEndpoint, ESPMode::ThreadSafe> MessageReceiverEndpoint;

    /** Saved sender addresses from previously received messages */
    TArray<FMessageAddress> KnownEndpoints;

    /** Max number of players in one party */
    uint8 MaxPlayers;

    /** Own ip address */
    FString LocalHostAddress;

    /** Party message handler */
    void HandlePartyMessage(const FLanPartyMessage& Message,
                            const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& Context);

    /** Handles ELanPartyMessageType::Discovery message */
    void OnPlayerDiscovered(const FMessageAddress& SenderAddress, const FLanPartyMessage& Message);

    /** Handles ELanPartyMessageType::JoinParty message */
    void OnPlayerJoinedParty(const FLanPartyMessage& Message);

    /** Handles ELanPartyMessageType::LeaveParty message */
    void OnPlayerLeftParty(const FLanPartyMessage& Message);

    /** Handles ELanPartyMessageType::ServerReady message */
    void OnServerReady(const FLanPartyMessage& Message);

    /** Handles ELanPartyMessageType::SkipCountdown message */
    void OnSkipCountdown() const;

public:
    /** Sets up message endpoints and reads config from cmdline args */
    ULanPartyManager();

    /** Overridden to dispose of MessageEndpoint resources */
    virtual void BeginDestroy() override;

    /** Player's positional index */
    UPROPERTY(BlueprintReadOnly, Category=LanParty)
        uint8 PlayerIndex = 1;

    /** Match start time for current party */
    UPROPERTY(BlueprintReadOnly, Category=LanParty)
        FDateTime MatchStartTime = FDateTime::MinValue();

    /** Matchmaking duration in seconds */
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=LanParty)
        int MatchmakingLengthSeconds = 30;

    /** Called when a player joins current party */
    UPROPERTY(BlueprintAssignable, Category=LanParty)
        FPlayerIndexDelegate PlayerJoinedPartyDelegate;

    /** Called when a player leaves current party */
    UPROPERTY(BlueprintAssignable, Category=LanParty)
        FPlayerIndexDelegate PlayerLeftPartyDelegate;

    /** Called when server is ready to host party */
    UPROPERTY(BlueprintAssignable, Category=LanParty)
        FServerAddressDelegate ServerReadyDelegate;

    /** Called when any party member chooses to skip countdown */
    UPROPERTY(BlueprintAssignable, Category=LanParty)
        FSkipCountdownDelegate SkipCountdownDelegate;

    /** Only called on non-party members when party game is started in order to update UI */
    UPROPERTY(BlueprintAssignable, Category=LanParty)
        FTriggerDelegate PartyResetDelegate;

    /** Whether this player is in current party */
    UFUNCTION(BlueprintPure, Category=LanParty)
        bool IsInParty() const;

    /** Announces this player's presence to other players on network */
    UFUNCTION(BlueprintCallable, Category=LanParty)
        void BroadcastDiscoveryMessage() const;

    /** Called when this player joins party */
    UFUNCTION(BlueprintCallable, Category=LanParty)
        void JoinParty();

    /** Called when this player leaves party */
    UFUNCTION(BlueprintCallable, Category=LanParty)
        void LeaveParty();

    /** Called when this player skips countdown */
    UFUNCTION(BlueprintCallable, Category=LanParty)
        void SkipCountdown() const;

    /** Called when countdown is finished to determine whether to host game or wait for server invite */
    UFUNCTION(BlueprintCallable, Category=LanParty)
        bool CheckIsGameHost();

    // TODO temp for debug
    /** (ip address) list of current party members */
    UPROPERTY(BlueprintReadOnly, Category=LanParty)
        TArray<FString> CurrentParticipants;

    /** Print debug info about current party */
    UFUNCTION(BlueprintCallable, Category=LanParty)
        void PrintPartyInfo() const;

private:
    /** Helper for broadcasting message from MessageType */
    void PublishPartyMessage(ELanPartyMessageType MessageType) const;

    /** Helper for broadcasting message */
    void PublishPartyMessage(FLanPartyMessage* Message) const;

    /** Uses same logic on all endpoints to uniformly identify ip address of current party */
    FString GetServerIpAddress();

    /** Clear local party list */
        void ResetParty();
};
