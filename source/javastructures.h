#ifndef _JAVASTRUCTURES_GUARD
#define _JAVASTRUCTURES_GUARD

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

#pragma pack(pop)

#define ACC_PUBLIC		0x0001
#define ACC_FINAL		0x0010
#define ACC_SUPER		0x0020
#define ACC_INTERFACE	0x0200
#define ACC_ABSTRACT	0x0400
#define ACC_SYNTHETIC	0x1000
#define ACC_ANNOTATION	0x2000
#define ACC_ENUM		0x4000

typedef struct _attribute_info
{
	short attribute_name_index;
	int attribute_length;
	char * info;
} AttributeInfo;

typedef struct _field_info
{
	short access_flags;
	short name_index;
	short descriptor_index;
	short attributes_count;

} FieldInfo;

typedef struct _method_info
{
	short access_flags;
	short name_index;
	short descriptor_index;
	short attributes_count;
	AttributeInfo * attributes;
} MethodInfo;

typedef struct _classInfo
{
	short majorVersion;
	short minorVersion;
	short constant_pool_count;
	CPInfo * constant_pool;
	short access_flags;
	short this_class;
	short super_class;
	short interfaces_count;
	short * interfaces;
	short fields_count;
	FieldInfo * fields;
	short methods_count;
	MethodInfo * methods;
	short attributes_count;
	AttributeInfo * attributes;
} ClassInfo;

#endif