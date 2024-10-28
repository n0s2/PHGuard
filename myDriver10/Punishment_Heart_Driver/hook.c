#include"hook.h"

PSYSTEM_SERVICE_TABLE SSDT地址 = 0;

UINT64 获取SSDT地址()
{
	PUCHAR msr = (PUCHAR)__readmsr(0xC0000082);
	PUCHAR startaddr = 0, Endaddr = 0;
	PUCHAR i = NULL;
	UCHAR b1, b2, b3;
	ULONG temp = 0;
	ULONGLONG addr = 0;

	KdPrint(("msr c0000082的值:%x\n", msr));
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
					KdPrint(("KiSystemServiceUser的地址:%x\n", startaddr));
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
			KdPrint(("SSDT地址:%x\n", addr));
			return addr;
		}
	}
	return 0;
}

UINT64 获取函数地址(ULONG dwIndex)
{
	PULONG lpBase = SSDT地址->ServiceTableBase;
	ULONG dwCount = (ULONG)SSDT地址->NumberOfServices;
	UINT64 lpAddr = 0;
	ULONG dwOffset = lpBase[dwIndex];

	if (dwOffset & 0x80000000)
		dwOffset = (dwOffset >> 4) | 0xF0000000;
	else
		dwOffset >>= 4;
	lpAddr = (UINT64)((PUCHAR)lpBase + (LONG)dwOffset);

	return lpAddr;
}

KIRQL 关闭写保护()
{
	KIRQL irql = KeRaiseIrqlToDpcLevel();
	ULONG_PTR cr0 = __readcr0();

	cr0 &= 0xfffffffffffeffff;
	_disable();
	__writecr0(cr0);

	return irql;
}

VOID 开启写保护(KIRQL irql)
{

	ULONG_PTR cr0 = __readcr0();
	cr0 |= 0x10000;
	__writecr0(cr0);
	_enable();

	KeLowerIrql(irql);
}

/*
要改写的指令长度网上一些方法是用LDE这个反编译引擎计算，但有时候这并不靠谱，比如我在HOOK NtOpenProcess的时候
发现需要改写的字节数为20，但LDE只算出了16，导致跳回原函数的地址与实际我们需要的地址差4个字节
这里的 原函数 是用来储存原API函数被修改掉的函数的指令及，后面需要调用它跳回原API函数
*/
VOID 开始HOOK(UINT64 HOOK函数地址, UINT64 代理函数地址, USHORT 改写的长度, PVOID* 原函数)
{
	UCHAR 跳到代理函数[] = "\xFF\x25\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";
	UCHAR 跳回原函数[] = "\xFF\x25\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";
	memcpy(跳到代理函数 + 6, &代理函数地址, 8);

	/*
	下面的数值14是跳转指令的总长度 假设该指令地址为0x410000
	0x410000 jmp qword ptr [0x410006]
	0x410006 xxxxxxxx
	其中0x410006中储存代理函数的地址
	*/
	UINT64 跳回原函数的地址 = HOOK函数地址 + 改写的长度;
	memcpy(跳回原函数 + 6, &跳回原函数的地址, 8);
	*原函数 = ExAllocatePool(NonPagedPool, 改写的长度 + 14);
	RtlFillMemory(*原函数, 改写的长度 + 14, 0x90);

	KIRQL irql = 关闭写保护();
	memcpy(*原函数, (PVOID)HOOK函数地址, 改写的长度);
	memcpy((PCHAR)(*原函数) + 改写的长度, 跳回原函数, 14);

	KIRQL dpc_irql = KeRaiseIrqlToDpcLevel();
	RtlFillMemory((void*)HOOK函数地址, 改写的长度, 0x90);
	memcpy((PVOID)HOOK函数地址, &跳到代理函数, 14);
	KeLowerIrql(dpc_irql);
	开启写保护(irql);
}

VOID 恢复HOOK(UINT64 HOOK函数地址, USHORT 改写的长度, PVOID 原函数)
{
	KIRQL irql = 关闭写保护();
	memcpy((PVOID)HOOK函数地址, 原函数, 改写的长度);
	开启写保护(irql);
	ExFreePool(原函数);
}
