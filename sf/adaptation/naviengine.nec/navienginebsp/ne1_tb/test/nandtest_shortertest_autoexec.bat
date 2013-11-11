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

format c:
format d: /e
format d:
format i:
format j:
rem demand paging tests (which may do nothing on non-paging ROMs):
runtests nanddemandpaginge32tests.bat -st -t 20 -c

format c:
format d: /e
format d:
format i:
format j:
runtests f32test.auto.bat -d i -st -t 45 -c
runtests rofs.auto.bat -d z -t 15 -p -c


