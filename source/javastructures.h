#ifndef _JAVASTRUCTURES_GUARD
#define _JAVASTRUCTURES_GUARD

typedef struct _versionInfo
{
	int majorVersion;
	int minorVersion;
} VersionInfo;

#define CONSTANT_Class					7
#define CONSTANT_Fieldref				9
#define CONSTANT_Methodref				10
#define CONSTANT_InterfaceMethodref		11
#define CONSTANT_String					8
#define CONSTANT_Integer				3
#define CONSTANT_Float					4
#define CONSTANT_Long					5
#define CONSTANT_Double					6
#define CONSTANT_NameAndType			12
#define CONSTANT_Utf8					1

#pragma pack(push)
#pragma pack(1)
typedef struct _cp_info
{
	union
	{
		char * chars;
		struct
		{
			// 4 bytes
			union
			{
				int highBytes;
				int bytes;
				int class_index;
				int name_index;
				int string_index;
				int reference_kind;
			};
			// 4 bytes
			union
			{
				int lowBytes;
				int name_and_type_index;
				int reference_index;
				int descriptor_index;
			};
		};
	};
	char tag;
	short length;
} CPInfo;

typedef struct _constantInfo
{
	int nConstants;
	CPInfo * constantPool;
} ConstantInfo;

#pragma pack(pop)

#endif