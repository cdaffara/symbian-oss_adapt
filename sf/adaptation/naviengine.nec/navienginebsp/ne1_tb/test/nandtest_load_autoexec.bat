@rem
@rem Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
@rem All rights reserved.
@rem This component and the accompanying materials are made available
@rem under the terms of "Eclipse Public License v1.0"
@rem which accompanies this distribution, and is available
@rem at the URL "http://www.eclipse.org/legal/epl-v10.html".
@rem
@rem Initial Contributors:
@rem Nokia Corporation - initial contribution.
@rem
@rem Contributors:
@rem
@rem Description: 
@rem

z:
nandloader -e -r -a z: cldr.img core.img rofs1.img rofs2.img rofsextension3.img rofs4.img rofs5.img rofs6.img user3.img smr1.img smr2.img -z FFFFFFFFFFFFFFFF 1000 1000 
crash
