/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Compensate function definition.
*
*/


#include <e32math.h>
#include "tiltcompensation.h"

//-----------------------------------------------------------------------------
// Direction6D
//-----------------------------------------------------------------------------
//
EXPORT_C TInt Compensate(
    const TTiltCompensationInput& aInput,
    TTiltCompensationOutput& aOutput,
     )
    {
    TReal target;
    TReal source( 0 );
    TReal inputY( aInput.iMagneticVector.iY );
    TReal inputZ( aInput.iMagneticVector.iZ );
    
    if( aInput.iMagneticVector.iY > 0 && aInput.iMagneticVector.iZ > 0 )
    // 0 - 90 degrees
        {
        source = inputY / inputZ;
        }
    if( aInput.iMagneticVector.iY > 0 && aInput.iMagneticVector.iZ < 0 )
    // 90 - 180 degrees
        {
        source = inputZ / inputY * -1;
        }
    if( aInput.iMagneticVector.iY < 0 && aInput.iMagneticVector.iZ < 0 )
    // 180 - 270 degrees
        {
        source = inputY / inputZ;
        }
    if( aInput.iMagneticVector.iY < 0 && aInput.iMagneticVector.iZ > 0 )
    // 270 - 360 degrees
        {
        source = inputZ / inputY * -1;
        }
    
    Math::ATan( target, source );
    TInt16 declination( ( target * 180 ) / 3.14 );
    
    if( aInput.iMagneticVector.iY > 0 && aInput.iMagneticVector.iZ > 0 )
    // 0 - 90 degrees
        {
        }
    if( aInput.iMagneticVector.iY > 0 && aInput.iMagneticVector.iZ < 0 )
    // 90 - 180 degrees
        {
        declination = declination + 90;
        }
    if( aInput.iMagneticVector.iY < 0 && aInput.iMagneticVector.iZ < 0 )
    // 180 - 270 degrees
        {
        declination = declination + 180;
        }
    if( aInput.iMagneticVector.iY < 0 && aInput.iMagneticVector.iZ > 0 )
    // 270 - 360 degrees
        {
        declination = declination + 270;
        }
    
    if( inputY == 0 && inputZ > 0 )
        {
        declination = 0;
        }
    if( inputY > 0 && inputZ == 0 )
        {
        declination = 90;
        }
    if( inputY == 0 && inputZ < 0 )
        {
        declination = 180;
        }
    if( inputY < 0 && inputZ == 0 )
        {
        declination = 270;
        }
    
    aOutput.iTheta = declination;
    return KErrNone;
    }
    
//-----------------------------------------------------------------------------
// Direction6D
//-----------------------------------------------------------------------------
//
EXPORT_C TInt Compensate(
    const TTiltCompensationInput& aInput,
    TTiltCompensationOutput& aOutput,
    const RParamsArray& aParamsArray )
    {
    TReal target;
    TReal source( 0 );
    TReal inputY( aInput.iMagneticVector.iY );
    TReal inputZ( aInput.iMagneticVector.iZ );
    
    // Get parameters
    for( TInt i = 0; i != aParamsArray.Count(); i++ )
        {
        TInt parameter = aParamsArray[ i ];
        }
    
    if( aInput.iMagneticVector.iY > 0 && aInput.iMagneticVector.iZ > 0 )
    // 0 - 90 degrees
        {
        source = inputY / inputZ;
        }
    if( aInput.iMagneticVector.iY > 0 && aInput.iMagneticVector.iZ < 0 )
    // 90 - 180 degrees
        {
        source = inputZ / inputY * -1;
        }
    if( aInput.iMagneticVector.iY < 0 && aInput.iMagneticVector.iZ < 0 )
    // 180 - 270 degrees
        {
        source = inputY / inputZ;
        }
    if( aInput.iMagneticVector.iY < 0 && aInput.iMagneticVector.iZ > 0 )
    // 270 - 360 degrees
        {
        source = inputZ / inputY * -1;
        }
    
    Math::ATan( target, source );
    TInt16 declination( ( target * 180 ) / 3.14 );
    
    if( aInput.iMagneticVector.iY > 0 && aInput.iMagneticVector.iZ > 0 )
    // 0 - 90 degrees
        {
        }
    if( aInput.iMagneticVector.iY > 0 && aInput.iMagneticVector.iZ < 0 )
    // 90 - 180 degrees
        {
        declination = declination + 90;
        }
    if( aInput.iMagneticVector.iY < 0 && aInput.iMagneticVector.iZ < 0 )
    // 180 - 270 degrees
        {
        declination = declination + 180;
        }
    if( aInput.iMagneticVector.iY < 0 && aInput.iMagneticVector.iZ > 0 )
    // 270 - 360 degrees
        {
        declination = declination + 270;
        }
    
    if( inputY == 0 && inputZ > 0 )
        {
        declination = 0;
        }
    if( inputY > 0 && inputZ == 0 )
        {
        declination = 90;
        }
    if( inputY == 0 && inputZ < 0 )
        {
        declination = 180;
        }
    if( inputY < 0 && inputZ == 0 )
        {
        declination = 270;
        }
    
    aOutput.iTheta = declination;
    return KErrNone;
    }    

// End of File
