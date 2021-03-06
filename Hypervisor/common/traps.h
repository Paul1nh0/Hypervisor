#ifndef _TRAPS_H
#define _TRAPS_H

#include "common.h"
#include "hvm.h"

typedef struct _NBP_TRAP *PNBP_TRAP;

typedef enum
{
	TRAP_DISABLED = 0,
	TRAP_GENERAL = 1,
	TRAP_MSR = 2,
	TRAP_IO = 3
} TRAP_TYPE;

typedef struct _NBP_TRAP_DATA_GENERAL
{
	ULONG TrappedVmExit;
	ULONG64 RipDelta;             // this value will be added to rip to skip the trapped instruction
} NBP_TRAP_DATA_GENERAL, *PNBP_TRAP_DATA_GENERAL;

typedef struct _NBP_TRAP_DATA_MSR
{
	ULONG32 TrappedMsr;
	UCHAR TrappedMsrAccess;
	UCHAR GuestTrappedMsrAccess;
} NBP_TRAP_DATA_MSR, *PNBP_TRAP_DATA_MSR;

typedef struct _NBP_TRAP_DATA_IO
{
	ULONG TrappedPort;
} NBP_TRAP_DATA_IO, *PNBP_TRAP_DATA_IO;

typedef BOOLEAN(*NBP_TRAP_CALLBACK) (
	PCPU Cpu,
	PGUEST_REGS GuestRegs,
	PNBP_TRAP Trap,
	BOOLEAN WillBeAlsoHandledByGuestHv
	);

typedef struct _NBP_TRAP
{
	LIST_ENTRY le;

	TRAP_TYPE TrapType;
	TRAP_TYPE SavedTrapType;

	union
	{
		NBP_TRAP_DATA_GENERAL General;
		NBP_TRAP_DATA_MSR Msr;
		NBP_TRAP_DATA_IO Io;
	};

	NBP_TRAP_CALLBACK TrapCallback;
	BOOLEAN bForwardTrapToGuest;  // FALSE if guest hypervisor doesn't want to intercept this in its own guest.
	// This will be TRUE for TRAP_MSR record when we're going to intercept MSR "rw"
	// but the guest wants to intercept only "r" or "w". 
	// Check Msr.GuestTrappedMsrAccess for correct event forwarding.
} NBP_TRAP, *PNBP_TRAP;

NTSTATUS TrRegisterTrap(PCPU Cpu, PNBP_TRAP Trap);
NTSTATUS TrInitializeGeneralTrap(
	PCPU Cpu,
	ULONG TrappedVmExit,
	UCHAR RipDelta,
	NBP_TRAP_CALLBACK TrapCallback,
	PNBP_TRAP * pInitializedTrap
	);
NTSTATUS TrDeregisterTrap(PNBP_TRAP Trap);
NTSTATUS TrDeregisterTrapList(PLIST_ENTRY TrapList);
NTSTATUS TrFindRegisteredTrap(
	PCPU Cpu,
	PGUEST_REGS GuestRegs,
	ULONG64 exitcode,
	PNBP_TRAP * pTrap
	);
NTSTATUS TrExecuteGeneralTrapHandler(
	PCPU Cpu,
	PGUEST_REGS GuestRegs,
	PNBP_TRAP Trap,
	BOOLEAN WillBeAlsoHandledByGuestHv
	);

#endif