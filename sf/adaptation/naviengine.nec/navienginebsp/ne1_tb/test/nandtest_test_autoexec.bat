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

trace on 65
z:
cd test
format c:
format d: /e
format d:
format i:
runtests e32test.auto.bat -st -t 20 -c
format c:
format d: /e
format d:
format i:
runtests loader.auto.bat -st -t 90 -c
format c:
format d: /e
format d:
format i:
runtests f32test.auto.bat -d c -st -t 30 -c
format c:
format d: /e
format d:
format i:
runtests f32test.auto.bat -d d -st -t 90 -c
format c:
format d: /e
format d:
format i:
runtests f32test.auto.bat -d i -st -t 45 -c
runtests rofs.auto.bat -d z -t 15 -p -c
