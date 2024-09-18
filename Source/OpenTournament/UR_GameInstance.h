// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CommonGameInstance.h"

#include "UR_GameInstance.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////


enum class ECommonUserOnlineContext : uint8;
class AUR_PlayerController;

/**
 *
 */
UCLASS(Config = Game)
class OPENTOURNAMENT_API UUR_GameInstance
    : public UCommonGameInstance
{
    GENERATED_BODY()

public:

    UUR_GameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    AUR_PlayerController* GetPrimaryPlayerController() const;

    virtual bool CanJoinRequestedSession() const override;
    virtual void HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext) override;

    //virtual void ReceivedNetworkEncryptionToken(const FString& EncryptionToken, const FOnEncryptionKeyResponse& Delegate) override;
    //virtual void ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse& Delegate) override;


    virtual void Init() override;
    virtual void Shutdown() override;

    void OnPreClientTravelToSession(FString& URL);


    // @! TODO : Restrict to certain build types / configurations?
    /**
    * FTest_AddLocalPlayer
    * Simulates creating ULocalPlayer and AController.
    * Exposed this function to Blueprint for Feature Testing
    */
    UFUNCTION(BlueprintCallable, Category = "GameInstance")
    virtual void FTest_AddLocalPlayer(int32 ControllerId);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UFUNCTION()
    virtual void BeginLoadingScreen(const FString& MapName);

    UFUNCTION()
    virtual void EndLoadingScreen(UWorld* InLoadedWorld);
};
