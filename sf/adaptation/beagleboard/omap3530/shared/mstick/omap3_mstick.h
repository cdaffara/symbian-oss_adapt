// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of the License "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// omap3530/shared/mstick/omap3_mstick.h
//

#ifndef OMAP3_MSTICK_H
#define OMAP3_MSTICK_H

#include <e32cmn.h>

namespace Omap3
{
	namespace MsTick
	{
	/** Start the millisecond timer counting, call from Init3 stage of ASSP */
	IMPORT_C TInt Start();

	/** Start idle tick supression
	 * @param	aMaxSleepTicks	Maximum number of ticks to sleep for
	 * @return	If >0 the number of ticks that will be suppressed (could be < aMaxSleepTicks)
	 * @return	If ==0 supression was not enabled
	 */
	IMPORT_C TInt SuppressIdleTicks( TInt aMaxSleepTicks );

	/** End idle supression and restore tick timer to normal operation
	 *
	 * @param	aMaxSleepTicks	Original maximum number of ticks passed to SuppressIdleTicks()
	 * @return	Actual number of ticks since SuppressIdleTickw() was called
	 */
	IMPORT_C TInt EndIdleTickSuppression( TInt aMaxSleepTicks );
	}	// namespace MsTick
}	// namespace Omap3



#endif // define OMAP3_MSTICK_H


