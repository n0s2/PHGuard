#include"hook.h"

PSYSTEM_SERVICE_TABLE SSDT��ַ = 0;

UINT64 ��ȡSSDT��ַ()
{
	PUCHAR msr = (PUCHAR)__readmsr(0xC0000082);
	PUCHAR startaddr = 0, Endaddr = 0;
	PUCHAR i = NULL;
	UCHAR b1, b2, b3;
	ULONG temp = 0;
	ULONGLONG addr = 0;

	KdPrint(("msr c0000082��ֵ:%x\n", msr));
	if (*(msr + 0x9) == 0x00)
	{
		startaddr = msr;
		Endaddr = startaddr + 0x500;
	}
	else if (*(msr + 0x9) == 0x70)
	{
		PUCHAR pKiSystemCall64Shadow = msr;
		PUCHAR EndSearchAddress = pKiSystemCall64Shadow + 0x500;
		PUCHAR i = NULL;
		INT Temp = 0;
		for (i = pKiSystemCall64Shadow; i < EndSearchAddress; i++)
		{
			if (MmIsAddressValid(i) && MmIsAddressValid(i + 5))
			{
				if (*i == 0xe9 && *(i + 5) == 0xc3)
				{
					memcpy(&Temp, i + 1, 4);
					startaddr = Temp + (i + 5);
					KdPrint(("KiSystemServiceUser�ĵ�ַ:%x\n", startaddr));
					Endaddr = startaddr + 0x500;
				}
			}
		}
	}

	for (i = startaddr; i < Endaddr; i++)
	{
		b1 = *i;
		b2 = *(i + 1);
		b3 = *(i + 2);
		if (b1 == 0x4c && b2 == 0x8d && b3 == 0x15)
		{
			memcpy(&temp, i + 3, 4);
			addr = (ULONGLONG)temp + (ULONGLONG)i + 7;
			KdPrint(("SSDT��ַ:%x\n", addr));
			return addr;
		}
	}
	return 0;
}

UINT64 ��ȡ������ַ(ULONG dwIndex)
{
	PULONG lpBase = SSDT��ַ->ServiceTableBase;
	ULONG dwCount = (ULONG)SSDT��ַ->NumberOfServices;
	UINT64 lpAddr = 0;
	ULONG dwOffset = lpBase[dwIndex];

	if (dwOffset & 0x80000000)
		dwOffset = (dwOffset >> 4) | 0xF0000000;
	else
		dwOffset >>= 4;
	lpAddr = (UINT64)((PUCHAR)lpBase + (LONG)dwOffset);

	return lpAddr;
}

KIRQL �ر�д����()
{
	KIRQL irql = KeRaiseIrqlToDpcLevel();
	ULONG_PTR cr0 = __readcr0();

	cr0 &= 0xfffffffffffeffff;
	_disable();
	__writecr0(cr0);

	return irql;
}

VOID ����д����(KIRQL irql)
{

	ULONG_PTR cr0 = __readcr0();
	cr0 |= 0x10000;
	__writecr0(cr0);
	_enable();

	KeLowerIrql(irql);
}

/*
Ҫ��д��ָ�������һЩ��������LDE���������������㣬����ʱ���Ⲣ�����ף���������HOOK NtOpenProcess��ʱ��
������Ҫ��д���ֽ���Ϊ20����LDEֻ�����16����������ԭ�����ĵ�ַ��ʵ��������Ҫ�ĵ�ַ��4���ֽ�
����� ԭ���� ����������ԭAPI�������޸ĵ��ĺ�����ָ���������Ҫ����������ԭAPI����
*/
VOID ��ʼHOOK(UINT64 HOOK������ַ, UINT64 ��������ַ, USHORT ��д�ĳ���, PVOID* ԭ����)
{
	UCHAR ����������[] = "\xFF\x25\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";
	UCHAR ����ԭ����[] = "\xFF\x25\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";
	memcpy(���������� + 6, &��������ַ, 8);

	/*
	�������ֵ14����תָ����ܳ��� �����ָ���ַΪ0x410000
	0x410000 jmp qword ptr [0x410006]
	0x410006 xxxxxxxx
	����0x410006�д���������ĵ�ַ
	*/
	UINT64 ����ԭ�����ĵ�ַ = HOOK������ַ + ��д�ĳ���;
	memcpy(����ԭ���� + 6, &����ԭ�����ĵ�ַ, 8);
	*ԭ���� = ExAllocatePool(NonPagedPool, ��д�ĳ��� + 14);
	RtlFillMemory(*ԭ����, ��д�ĳ��� + 14, 0x90);

	KIRQL irql = �ر�д����();
	memcpy(*ԭ����, (PVOID)HOOK������ַ, ��д�ĳ���);
	memcpy((PCHAR)(*ԭ����) + ��д�ĳ���, ����ԭ����, 14);

	KIRQL dpc_irql = KeRaiseIrqlToDpcLevel();
	RtlFillMemory((void*)HOOK������ַ, ��д�ĳ���, 0x90);
	memcpy((PVOID)HOOK������ַ, &����������, 14);
	KeLowerIrql(dpc_irql);
	����д����(irql);
}

VOID �ָ�HOOK(UINT64 HOOK������ַ, USHORT ��д�ĳ���, PVOID ԭ����)
{
	KIRQL irql = �ر�д����();
	memcpy((PVOID)HOOK������ַ, ԭ����, ��д�ĳ���);
	����д����(irql);
	ExFreePool(ԭ����);
}
