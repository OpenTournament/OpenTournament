// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.


#include "UR_FireModeBase.h"

void UUR_FireModeBase::SetBusy(bool bNewBusy)
{
    if (bNewBusy != bIsBusy)
    {
        bIsBusy = bNewBusy;
        if (BaseInterface)
        {
            IUR_FireModeBaseInterface::Execute_FireModeChangedStatus(BaseInterface.GetObject(), this);
        }
    }
}
