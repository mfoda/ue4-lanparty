#pragma once

#include "LanPartyMessage.generated.h"

UENUM(BlueprintType)
enum class ELanPartyMessageType : uint8
{
    None,        
    Discovery,
    JoinParty,
    LeaveParty,
    SkipCountdown, 
    ServerReady,
};

USTRUCT(BlueprintType)
struct LANPARTY_API FLanPartyMessage
{
    GENERATED_BODY()

    /** Positional index starting from 1 */
    UPROPERTY()
        uint8 PlayerIndex;

    /** IP Address of message sender */
    UPROPERTY()
        FString IpAddress;

    /** Type of message being sent */
    UPROPERTY()
        ELanPartyMessageType PartyMessageType;

    /** Whether or not message sender is in current party */
    UPROPERTY()
        bool IsInCurrentParty;

    /** Time next match will start */
    UPROPERTY()
        int64 MatchStartTimeTicks;

    FLanPartyMessage(): PlayerIndex(0), PartyMessageType(ELanPartyMessageType::None), IsInCurrentParty(false),
                        MatchStartTimeTicks(FDateTime::MinValue().GetTicks())
    { }

    explicit FLanPartyMessage(const uint8 InPlayerIndex, const FString &InSenderIpAddress,
                              const ELanPartyMessageType InPartyMessageType, const bool InIsInCurrentParty,
                              const int64 InMatchStartTimeTicks) :
    PlayerIndex(InPlayerIndex),
    IpAddress(InSenderIpAddress),
    PartyMessageType(InPartyMessageType), 
    IsInCurrentParty(InIsInCurrentParty),
    MatchStartTimeTicks(InMatchStartTimeTicks)
    { }
};