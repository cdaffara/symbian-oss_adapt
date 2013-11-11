
#ifndef ASTORIAAPISTORAGE_H
#define ASTORIAAPISTORAGE_H


class CyAsSymbianStorageDriver
{
	
public:
	IMPORT_C static int Open(void);
	IMPORT_C static int Close(void);
	IMPORT_C static int Read(int, int, void*);
	IMPORT_C static int Write(int, int, void*);

	IMPORT_C static TInt GetMediaType(void);
	IMPORT_C static TInt GetIsRemovable(void);
	IMPORT_C static TInt GetIsWriteable(void);
	IMPORT_C static TUint16 GetBlockSize(void);
	IMPORT_C static TUint GetNumberOfUnits(void);
	IMPORT_C static TInt GetIsLocked(void);
	IMPORT_C static TUint GetEraseBlockSize(void);
	IMPORT_C static TUint GetUnitSize(void);	
	IMPORT_C static TUint GetStartBlock(void);	
	IMPORT_C static TInt64 GetMediaSize(void);
	
};

#endif
