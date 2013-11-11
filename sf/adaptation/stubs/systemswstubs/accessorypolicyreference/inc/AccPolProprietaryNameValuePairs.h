/*
* Copyright (c) 2002-2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  This header provides the proprietary Name/Value defintions of Name/Value pairs.
*
*/


#ifndef ACCPOLPROPRIETARYNAMEVALUEPAIRS_H
#define ACCPOLPROPRIETARYNAMEVALUEPAIRS_H

//  INCLUDES
#include <AccPolCommonNameValuePairs.h>

// CONSTANTS

// Accessory capabilities can be described as name/value pairs. To use accessorys
// capabilities from generic identification, the user needs constants. These 
// constants are referenced with name/value pairs.
// Name constants are defined to a certain numerical space. 
// In this numerical space each of these constats is presented with a term called
// "name const literal". E.g. For name KAccAcousticEchoControl name const literal is
// "1#1#1".
// Accessory capabilities can be described the following values: name, value type,
// dynamic type, I/O type, description and bitmask.
// Name:           Acoustic Echo Control
// --------------> Name of the capability 
// Value Type:     TBool
// --------------> Type of the value
// Dynamic Type:   Dynamic/Static
// --------------> Indicates if value can change while accessory is connected.
//                 If value is dynamic type the client should registrate for 
//                 listening indications for value changes.
// I/O Type:       From accessory to terminal, input.
// --------------> I/O type indicates the direction how the state update is done. 
//                 E.g.output indicated that user can use setValueL method. If
//                 accessory updates the value I/O type is input and if it done
//                 by the terminal I/O type is output.
// Description:    Accessory supports acoustic echo control. If value is true terminal
//                 must disable its own echo control. To indicate if accessory has 
//                 internal acoustic echo control.
// --------------> Description of the capability.

// Format of capability:
// =====================
//
// 0b 000001 00000 00010
//    ------ ----- -----
//    |      |     |
//    |      |     |_5 bits for capability. Tells which bit is one (*).
//    |      |_5 bits for namespace,
//    |_6 bits for capability group. Tells which bit is one (*).
//
// (*):
// 0b00 -> bit number 1 is one -> value is 0b0001=1
// 0b01 -> bit number 2 is one -> value is 0b0010=2
// 0b10 -> bit number 3 is one -> value is 0b0100=4
// 0b11 -> bit number 4 is one -> value is 0b1000=8
//

// Proprietary NAME-VALUE PAIRS:

// Name:           Proprietary capability.
// Value:          N/A
// Dynamic Type:   N/A
// I/O Type:       N/A
// Description:    N/A
//const TUint32 KAccProprietaryCapability = 0x8000;

#endif      // ACCPOLPROPRIETARYNAMEVALUEPAIRS_H   
            
// End of File
