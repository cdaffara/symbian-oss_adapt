/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Reference implementation of SSY Property Provider interface
*
*/


#ifndef SSYREFERENCEPROPERTYPROVIDER_H
#define SSYREFERENCEPROPERTYPROVIDER_H

#include <ssypropertyprovider.h>

class CSsyReferenceChannel;

/**
 *  Channel property provider implementation.
 *
 *  @lib ssyreferenceplugin.lib
 *  @since S60 5.0
 */
class CSsyReferencePropertyProvider : public CBase, public MSsyPropertyProvider
    {

public:

    /**
     * Two-phase constructor
     *
     * @since S60 5.0
     * @param[in] aChannel Reference to channel this provider belongs to
     * @return CSsyReferenceChannelDataProvider* Pointer to created CSsyReferenceControl object
     */
    static CSsyReferencePropertyProvider* NewL( CSsyReferenceChannel& aChannel );

    /**
     * Virtual destructor
     *
     * @since S60 5.0
     */
    virtual ~CSsyReferencePropertyProvider();

// from base class MSsyPropertyProvider

    /**
     * From MSsyPropertyProvider
     * Check if property value affects other sensor channels already open.
     * If the new property value is legal but affects somehow other channels' properties,
     * SSY must return list of the affected channels so that the sensor server can
     * check if the client allowed to set this property. If the SSY value 
     * is not legal SSY must leave with KErrArgument-value.
     *
     * @since S60 5.0
     * @param[in] aProperty Property to be checked. 
     * @param[out] aAffectedChannels Return list of the channel which will be affected if the property 
     *       value will be set.
     * @leave KErrArgument If the property value is illegal.
     */  
    void CheckPropertyDependenciesL( const TSensrvChannelId aChannelId,
                                     const TSensrvProperty& aProperty,
                                     RSensrvChannelList& aAffectedChannels ); 

    /**
     * From MSsyPropertyProvider
     * Set property for the channel. Before the sensor server sets the property value,
     * it is checked with CheckPropertyDependenciesL()-function.
     * This means a property value should always be valid for the SSY.
     *
     * @since S60 5.0
     * @param[in] aProperty Rereference to a property object to be set
     */  
    void SetPropertyL( const TSensrvChannelId aChannelId, 
                       const TSensrvProperty& aProperty );

    /**
     * From MSsyPropertyProvider
     * Get channel property value. The property parameter contains channel id and
     * item index. SSY fills values and attributes to the property object.
     *
     * @since S60 5.0
     * @param[in, out] aProperty Reference to a property object to be filled
     *     with property values and attributes.
     */  
    void GetPropertyL( const TSensrvChannelId aChannelId, 
                       TSensrvProperty& aProperty );

    /**
     * From MSsyPropertyProvider
     * Get all channel properties. Returns all properties which are related to this channel.
     *
     * @since S60 5.0
     * @param[out] aChannelPropertyList List of the all properties of the channel.
     */  
    void GetAllPropertiesL( const TSensrvChannelId aChannelId,
                            RSensrvPropertyList& aChannelPropertyList );

    /** 
    * Returns a pointer to a specified interface - to allow future extension
    * of this class without breaking binary compatibility
    *
    * @since S60 5.0
    * @param aInterfaceUid Identifier of the interface to be retrieved
    * @param aInterface A reference to a pointer that retrieves the specified interface.
    */
    void GetPropertyProviderInterfaceL( TUid aInterfaceUid, 
                                        TAny*& aInterface );

private:

    /**
     * C++ constructor.
     * @param[in] aChannel Reference to channel this provider belongs to
     */
    CSsyReferencePropertyProvider( CSsyReferenceChannel& aChannel );

    /**
     * Symbian 2nd phase constructor.
     */
    void ConstructL();

private: // data

    /**
     * Reference to channel for which this provider belongs to
     */
    CSsyReferenceChannel& iChannel;

    };

#endif // SSYREFERENCEPROPERTYPROVIDER_H
