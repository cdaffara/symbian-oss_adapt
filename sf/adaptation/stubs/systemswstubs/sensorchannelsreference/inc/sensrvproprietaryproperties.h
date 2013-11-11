/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  This header provides the proprietary property definitions.
*
*/


#ifndef SENSRVPROPRIETARYPROPERTIES_H
#define SENSRVPROPRIETARYPROPERTIES_H

#include <sensrvproperty.h>

/**
* - Name:        Name of the property. 
*                Property values from 0x0 to 0xffff are reserved for platform use.
* - Type:        Defines type of the property (TInt/TReal/TBuf)
* - Scope:       Defines a property scope. Property can be defined for a 
*                channel, for a specific item in a channel or for a server 
*                related to channel or 
* - Group:       Defines which category a property belongs
* - Mandatory:   Defines is property mandatory or in which groups property is 
*                mandatory 
* - Capability:  Capabilities needed to set this property
* - Description: Description of the property
* 
*/
//const TSensrvPropertyId KSensrvProprietaryProperty = 0x00010000;


#endif // SENSRVPROPRIETARYPROPERTIES_H
