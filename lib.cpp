#include "windows.h"
#include "stdint.h"
#include <bitset>

/// <summary>
///  Union for check one byte fields
/// </summary>
union OneByteFields {
	struct Bytes {
		unsigned b0 : 8;
		unsigned b1 : 8;
		unsigned b2 : 8;
		unsigned b3 : 8;
	} bytes;

	uint32_t value;
};

/// <summary>
///  Function: mycpuid(int regs[4], int func)
///  shell over function cpuid from asm.
///  int regs[4] - represent registers
///  int func - value of function, that should be used in cpuid call.
/// </summary>

void mycpuid(int regs[4], int func) {
	int ieax, iebx, iecx, iedx;

	__asm {
		mov eax, func
		cpuid
		mov ieax, eax
		mov iebx, ebx
		mov iecx, ecx
		mov iedx, edx
	}

	regs[0] = ieax;
	regs[1] = iebx;
	regs[2] = iecx;
	regs[3] = iedx;
}

/// <summary>
///  Function: mycpuid(int regs[4], int func)
///  shell over function cpuid from asm ( with additional parameter: ECX register)
///  int regs[4] - represent registers
///  int EAXfunc - value of function, that should be used in cpuid call.
///	 int ECXparm - value of pararmeter ECX
/// </summary>

void mycpuid_withECX(int regs[4], int EAXfunc, int ECXparam) {
	int ieax, iebx, iecx, iedx;

	__asm {
		mov eax, EAXfunc
		mov ecx, ECXparam
		cpuid
		mov ieax, eax
		mov iebx, ebx
		mov iecx, ecx
		mov iedx, edx
	}

	regs[0] = ieax;
	regs[1] = iebx;
	regs[2] = iecx;
	regs[3] = iedx;
}


/// <summary>
/// enum for matchiing massives and the size of page
/// </summary>

enum Sizes {
	_notProcessing = -1,
	_notFound,
	_4KB,
	_2MB,
	_4MB,
	_4KBand2MBor4MB,
	_2MBOr4MB,
	_1GB,
	_GENERAL
};

/// <summary>
///	Function: findInTableOfTBLInstruction(unsigned char byte)
///	To compare current byte with bytes, that responded for TBL instruction information
/// unsigned char byte - current byte.
/// <\summary>

int findInTableOfTBLInstruction(unsigned char byte) {
	// bytes, that point on TBL instructions page size
	static unsigned char codes4KB[] = { 0x01, 0x4f, 0x61, 0xb0, 0xb2, 0xb5, 0xb6 };
	static unsigned char codes2MB[] = { 0xb1 };
	static unsigned char codes4MB[] = { 0x02, 0x0b };
	static unsigned char codes4KBAnd2MBOr4MB[] = { 0x50, 0x51, 0x52 };
	static unsigned char codes2MBOr4MB[] = { 0x55, 0x76 };
	static unsigned char general = 0xFE;

	// compose current byte with bytes in arrays
	for (const auto & code : codes4KB) if (code == byte) return _4KB;
	for (const auto & code : codes2MB) if (code == byte) return _2MB;
	for (const auto & code : codes4MB) if (code == byte) return _4MB;
	for (const auto & code : codes4KBAnd2MBOr4MB) if (code == byte) return _4KBand2MBor4MB;
	for (const auto & code : codes2MBOr4MB) if (code == byte) return _2MBOr4MB;
	if (byte == general) return _GENERAL;

	// if byte not found in no one array of codes.
	return _notFound;
}

/// <summary>
///	Function: int decodeCPUIDreturn(int regs[4])
/// This function figure out the TBL instructions page size via cpuid assembler function
/// int regs[4] - represent registers
///	returned value - the number of massive, in which placed related byte
/// <\summary>

int decodeCPUIDreturn(int regs[4]) {
	union OneByteFields reg;
	int massiveNumber = 0;

	union OneByteFields temp;
	temp.value = regs[0];

	unsigned short amount = temp.bytes.b0;

	for (int i = 0; i < 4; i++) {
		reg.value = regs[i];

		// chech the most-significat bit
		if (reg.value >> 31 == 1) continue;
		else { // find information about processer in register/
			// explore 4 byte
			massiveNumber = findInTableOfTBLInstruction(reg.bytes.b3);
			if (massiveNumber != 0) break;


			// explore 3 byte
			massiveNumber = findInTableOfTBLInstruction(reg.bytes.b2);
			if (massiveNumber != 0) break;

			// explore 2 byte
			massiveNumber = findInTableOfTBLInstruction(reg.bytes.b1);
			if (massiveNumber != 0) break;

			// explore 1 byte (ignore processing 1byte in EAX register)
			if (i != 0) {
				massiveNumber = findInTableOfTBLInstruction(reg.bytes.b0);
				if (massiveNumber != 0) break;
			}

		}
	}

	return massiveNumber;
}

/// <summary>
///	Function: void formingOutString(const int processorsNumber, const int massiveNumber, wchar_t* outString)
/// This function forming output string to display in window.
/// const int processorsNumber - amount of processers
///	const int massiveNumber - the number of massive, in which placed related byte ( to find out size of page)
/// wchar_t* outString - buffer for output string
/// <\summary>

void formingOutString(const int processorsNumber, const int massiveNumber, void *outString) {
	char format[] = "Number of processors: %d; Size of TLB instructions page: %s.";

	switch (massiveNumber) {
	case _notProcessing: {
		char size[] = "not processing for this processor";
		wsprintf(reinterpret_cast<LPSTR>(outString), format, processorsNumber, size);	
	} break;

	case _notFound: {
		char size[] = "information not found";
		wsprintf(reinterpret_cast<LPSTR>(outString), format, processorsNumber, size);
	} break;

	case _4KB: {
		char size[] = "4 KB";
		wsprintf(reinterpret_cast<LPSTR>(outString), format, processorsNumber, size);
	} break;

	case _2MB: {
		char size[] = "2 MB";
		wsprintf(reinterpret_cast<LPSTR>(outString), format, processorsNumber, size);
	} break;

	case _4MB: {
		char size[] = "4 MB";
		wsprintf(reinterpret_cast<LPSTR>(outString), format, processorsNumber, size);
	} break;

	case _4KBand2MBor4MB: {
		char size[] = "4 KB and 2 MB or 4 MB";	
		wsprintf(reinterpret_cast<LPSTR>(outString), format, processorsNumber, size);
	} break;

	case _2MBOr4MB: {
		char  size[] = "2 MB or 4 MB";
		wsprintf(reinterpret_cast<LPSTR>(outString), format, processorsNumber, size);	
	} break;
	case _1GB: {
		char  size[] = "1 GB";
		wsprintf(reinterpret_cast<LPSTR>(outString), format, processorsNumber, size);
	} break;
	}
}

/// <summary>
///	Function: extern "C" __declspec(dllexport) int getProseccorInfo(void *info)
/// This function figure out the number of processors and TBL instructions page size
///    and puts them into a string in a format suitable for output
/// void *info - buffer for output string
/// <\summary>

extern "C" __declspec(dllexport) int getProseccorInfo(void *info) {
	// Copy the hardware information to the SYSTEM_INFO structure
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);

	// find out the CPU label
	int CPUInfo[4];

	// check the label of processor
	char VendorID[13];  VendorID[12] = '\0';
	mycpuid(CPUInfo, 0x0);
	memcpy(VendorID, CPUInfo + 1, sizeof(int));
	memcpy(VendorID + 4, CPUInfo + 3, sizeof(int));
	memcpy(VendorID + 8, CPUInfo + 2, sizeof(int));

	if (strcmp(VendorID, "GenuineIntel") == 0) {
		// 02H - instruction for TLB information 
		const int function = 0x02;
		mycpuid(CPUInfo, function);

		// decode information from registers
		int massiveNumber = decodeCPUIDreturn(CPUInfo);

		// save result into output string
		if(massiveNumber != _GENERAL) {
			formingOutString(siSysInfo.dwNumberOfProcessors, massiveNumber, info);
		}
		else { // check specifics
			const int specific_function = 0x18;
			int ECXparam = 0x0;
			bool found = false;

			// first call
			mycpuid_withECX(CPUInfo, specific_function, ECXparam);

			// EAX containing max numbers of calls
			const int amountOfCalls = CPUInfo[0];	

			// check and loop
	
			std::bitset<32> instructionsState(2); // EDX: 0010b: Instruction TLB.
			for(int ECXparam = 1; ECXparam <= amountOfCalls + 1 & !found; ECXparam++ ) {
				// check EDX[4:0]
				std::bitset<32> checkState = std::bitset<32>(CPUInfo[3]) & instructionsState;
				if (checkState == instructionsState) {
					//	EBX -- page size entries supported by this structure
					//	Bit 00: 4KB, Bit 01 : 2MB,  Bit 02 : 4MB , Bit 03 : 1 GB;
					std::bitset<32> check0 = std::bitset<32>(0);
					std::bitset<32> check1 = std::bitset<32>(1);
					std::bitset<32> check2 = std::bitset<32>(2);
					std::bitset<32> check3 = std::bitset<32>(3);
					std::bitset<32> EBX(CPUInfo[1]);

					if ((EBX & check3) == check3) { formingOutString(siSysInfo.dwNumberOfProcessors, _1GB, info); found = true; }
					else if ((EBX & check2) == check2) { formingOutString(siSysInfo.dwNumberOfProcessors, _4MB, info); found = true; }
					else if ((EBX & check1) == check1) { formingOutString(siSysInfo.dwNumberOfProcessors, _2MB, info); found = true; }
					else if ((EBX & check0) == check0) { formingOutString(siSysInfo.dwNumberOfProcessors, _4KB, info); found = true; }
				}
				
				// check next sub-leaf
				mycpuid_withECX(CPUInfo, specific_function, ECXparam);
			}

			// if information not found
			if(!found) formingOutString(siSysInfo.dwNumberOfProcessors, _notFound, info);
		}

	}
	else {
		formingOutString(siSysInfo.dwNumberOfProcessors, _notProcessing, info);
	}

	return 0;
}