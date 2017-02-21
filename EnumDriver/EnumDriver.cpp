// EnumDriver.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "stdafx.h"  
#include <windows.h>  
#include <stdlib.h>  
#include <stdio.h>  
// ���庯������ֵ  
//typedef ULONG NTSTATUS;
// ���ֽ��ַ����ṹ����  
typedef struct _UNICODE_STRING {
	USHORT  Length;
	USHORT  MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
// �������Զ���  
typedef struct _OBJECT_ATTRIBUTES {
	ULONG Length;
	HANDLE RootDirectory;
	UNICODE_STRING *ObjectName;
	ULONG Attributes;
	PSECURITY_DESCRIPTOR SecurityDescriptor;
	PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;
// ������Ϣ����  
typedef struct _DIRECTORY_BASIC_INFORMATION {
	UNICODE_STRING ObjectName;
	UNICODE_STRING ObjectTypeName;
} DIRECTORY_BASIC_INFORMATION, *PDIRECTORY_BASIC_INFORMATION;
// ����ֵ��״̬���Ͷ���  
#define OBJ_CASE_INSENSITIVE    0x00000040L  
#define DIRECTORY_QUERY            (0x0001)  
#define STATUS_SUCCESS            ((NTSTATUS)0x00000000L) // ntsubauth  
#define STATUS_MORE_ENTRIES        ((NTSTATUS)0x00000105L)  
#define STATUS_BUFFER_TOO_SMALL    ((NTSTATUS)0xC0000023L)  
// ��ʼ���������Ժ궨��  
#define InitializeObjectAttributes( p, n, a, r, s ) { \
	(p)->Length = sizeof(OBJECT_ATTRIBUTES); \
	(p)->RootDirectory = r; \
	(p)->Attributes = a; \
	(p)->ObjectName = n; \
	(p)->SecurityDescriptor = s; \
	(p)->SecurityQualityOfService = NULL; \
}
// �ַ�����ʼ��  
typedef VOID(CALLBACK* RTLINITUNICODESTRING)(PUNICODE_STRING, PCWSTR);

RTLINITUNICODESTRING RtlInitUnicodeString;
// �򿪶���  
typedef NTSTATUS(WINAPI *ZWOPENDIRECTORYOBJECT)(
	OUT PHANDLE DirectoryHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes
	);
ZWOPENDIRECTORYOBJECT ZwOpenDirectoryObject;
// ��ѯ����  
typedef
NTSTATUS
(WINAPI *ZWQUERYDIRECTORYOBJECT)(
IN HANDLE DirectoryHandle,
OUT PVOID Buffer,
IN ULONG BufferLength,
IN BOOLEAN ReturnSingleEntry,
IN BOOLEAN RestartScan,
IN OUT PULONG Context,
OUT PULONG ReturnLength OPTIONAL
);
ZWQUERYDIRECTORYOBJECT ZwQueryDirectoryObject;
// �ر��Ѿ��򿪵Ķ���  
typedef NTSTATUS (WINAPI *ZWCLOSE)(IN HANDLE Handle);
ZWCLOSE ZwClose;
int _tmain(int argc, _TCHAR* argv[])
{
	HMODULE hNtdll = NULL;
	UNICODE_STRING     strDirName;
	OBJECT_ATTRIBUTES  oba;
	NTSTATUS           ntStatus;
	HANDLE             hDirectory;

	hNtdll = LoadLibrary(_T("ntdll.dll"));
	if (NULL == hNtdll)
	{
		printf("[%s]--Load ntdll.dll failed(%ld).\r\n", __FUNCTION__, GetLastError());
		system("pause");
		return 0;
	}
	printf("[%s]--Load ntdll.dll sucess now get proc.\r\n", __FUNCTION__);
	RtlInitUnicodeString = (RTLINITUNICODESTRING)GetProcAddress(hNtdll, "RtlInitUnicodeString");
	ZwOpenDirectoryObject = (ZWOPENDIRECTORYOBJECT)GetProcAddress(hNtdll, "ZwOpenDirectoryObject");
	ZwQueryDirectoryObject = (ZWQUERYDIRECTORYOBJECT)GetProcAddress(hNtdll, "ZwQueryDirectoryObject");
	ZwClose = (ZWCLOSE)GetProcAddress(hNtdll, "ZwClose");

	RtlInitUnicodeString(&strDirName, _T("\\Driver"));
	InitializeObjectAttributes(&oba, &strDirName, OBJ_CASE_INSENSITIVE, NULL, NULL);
	printf("[%s]--Open directory object now.\r\n", __FUNCTION__);
	ntStatus = ZwOpenDirectoryObject(&hDirectory, DIRECTORY_QUERY, &oba);
	if (ntStatus != STATUS_SUCCESS)
	{
		printf("[%s]--Open directory object failed(%ld).\r\n", __FUNCTION__, GetLastError());
		system("PAUSE");
		return 0;
	}
	printf("[%s]--Open directory object success.\r\n", __FUNCTION__);
	PDIRECTORY_BASIC_INFORMATION   pBuffer = NULL;
	PDIRECTORY_BASIC_INFORMATION   pBuffer2 = NULL;
	ULONG    ulLength = 0x800;    // 2048  
	ULONG    ulContext = 0;
	ULONG    ulRet = 0;
	// ��ѯĿ¼����  
	do
	{
		if (pBuffer != NULL)
		{
			free(pBuffer);
		}
		ulLength = ulLength * 2;
		pBuffer = (PDIRECTORY_BASIC_INFORMATION)malloc(ulLength);
		if (NULL == pBuffer)
		{
			printf("[%s]--Malloc failed(%ld).\r\n", __FUNCTION__, GetLastError());
			if (pBuffer != NULL)
			{
				free(pBuffer);
			}
			if (hDirectory != NULL)
			{
				ZwClose(hDirectory);
			}

			system("PAUSE");
			return 0;
		}
		ntStatus = ZwQueryDirectoryObject(hDirectory, pBuffer, ulLength, FALSE, TRUE, &ulContext, &ulRet);
		printf("[%s]--ZwQueryDirectoryObject out return is %ld.\r\n", __FUNCTION__, ulRet);
	} while (ntStatus == STATUS_MORE_ENTRIES || ntStatus == STATUS_BUFFER_TOO_SMALL);

	if (STATUS_SUCCESS == ntStatus)
	{
		printf("[%s]--ZwQueryDirectoryObject success.\r\n", __FUNCTION__);
		pBuffer2 = pBuffer;
		while ((pBuffer2->ObjectName.Length != 0) && (pBuffer2->ObjectTypeName.Length != 0))
		{
			printf("ObjectName: [%S]---ObjectTypeName: [%S]\n", pBuffer2->ObjectName.Buffer, pBuffer2->ObjectTypeName.Buffer);
			pBuffer2++;
		}
	}
	else
	{
		printf("[%s]--ZwQueryDirectoryObject failed(%ld).\r\n", __FUNCTION__, GetLastError());
	}

	if (pBuffer != NULL)
	{
		free(pBuffer);
	}
	if (hDirectory != NULL)
	{
		ZwClose(hDirectory);
	}
	
	system("PAUSE");
	return 0;
}
