/*
* Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  
* ne1_tb\inc\ne1_tb_power.inl
* NE1_TBVariant Power Management Inline file
* -/-/-/-/-/-/-/-/-/ class TNE1_TBPowerController /-/-/-/-/-/-/-/-/-/
*
*/



inline void TNE1_TBPowerController::RegisterPowerController(DNE1_TBPowerController* aPowerController)
	{
    __PM_ASSERT(!iPowerController); 
    iPowerController = aPowerController;
#if defined(__SMP__) && !defined(__NO_IDLE_HANDLER_PIL__)
    iIdleHandler = &aPowerController->iIdleHandler;
#endif    
    }

