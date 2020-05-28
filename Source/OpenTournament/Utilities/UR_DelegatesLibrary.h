// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "UR_DelegatesLibrary.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

class FViewport;

/////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FViewportResizedSignature, float, SizeX, float, SizeY);

/**
 *
 */
UCLASS()
class OPENTOURNAMENT_API UUR_DelegatesLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

protected:
    UUR_DelegatesLibrary();

    void InternalOnViewportResized(FViewport* Viewport, uint32 Param);

public:

    /**
    * Singleton access to bind or trigger dispatchers.
    */
    UFUNCTION(BlueprintPure, Meta = (DisplayName = "GetDelegatesLibrary"))
    static UUR_DelegatesLibrary* Get()
    {
        //return GetDefault<UUR_DelegatesLibrary>();
        return UUR_DelegatesLibrary::StaticClass()->GetDefaultObject<UUR_DelegatesLibrary>();
    }

    UPROPERTY(BlueprintAssignable)
    FViewportResizedSignature OnViewportResized;

};
