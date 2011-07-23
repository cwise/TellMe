// TellMe.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "speed.h"

#define OS_KEY			CString("Software\\Microsoft\\Windows\\CurrentVersion")
#define OSV_VALUE		CString("Version")
#define OSVN_VALUE		CString("VersionNumber")
#define OSREGOWN_VALUE	CString("RegisteredOwner")
#define OSREGORG_VALUE	CString("RegisteredOrganization")
#define OSPRODID_VALUE	CString("ProductId")

#define CPU_KEY			CString("Hardware\\Description\\system\\CentralProcessor\\0")
#define CPU_VALUE		CString("Identifier")
#define CPUV_VALUE		CString("VendorIdentifier")

#define COMP_KEY		CString("System\\CurrentControlSet\\Control\\ComputerName\\ComputerName")
#define COMP_VALUE		CString("ComputerName")

#define USER_KEY		CString("Network\\Logon")
#define	USER_VALUE		CString("username")

#define MONITOR_KEY		CString("Enum\\MONITOR")
#define MONITOR_VALUE	CString("DeviceDesc")

#define	ISA_KEY			CString("Enum\\ISAPNP")
#define ISA_VALUE		CString("DeviceDesc")

#define PCI_KEY			CString("Enum\\PCI")
#define PCI_VALUE		CString("DeviceDesc")

#define PCMCIA_KEY		CString("Enum\\PCMCIA")
#define PCMCIA_VALUE	CString("DeviceDesc")		
		
#define	CLASS_VALUE		CString("Class")

#define SOUND_TAG		CString("MEDIA")
#define VIDEO_TAG		CString("DISPLAY")
#define NETWORK_TAG		CString("NET")
#define MODEM_TAG		CString("MODEM")

using namespace std;

CString	GetRegistryEntry(CString& pKey, CString& pValue)
{
	CRegistry	registry;
	CString		value;

	if(registry.Connect((HKEY) CRegistry::keyLocalMachine)==TRUE)
	{
		if(registry.Open(pKey))
		{
			registry.GetValue(pValue, value);
			registry.Close();
		}
	}

	return value;
}

void OutputOSInfo()
{
	cout << "Oper. System  : " << '\t' << (LPCSTR) GetRegistryEntry(OS_KEY, OSV_VALUE) << endl;
	cout << "Version Num.  : " << '\t' << (LPCSTR) GetRegistryEntry(OS_KEY, OSVN_VALUE) << endl;
	cout << "Reg. Owner    : " << '\t' << (LPCSTR) GetRegistryEntry(OS_KEY, OSREGOWN_VALUE) << endl;
	cout << "Reg. Org.     : " << '\t' << (LPCSTR) GetRegistryEntry(OS_KEY, OSREGORG_VALUE) << endl;
	cout << "Product ID    : " << '\t' << (LPCSTR) GetRegistryEntry(OS_KEY, OSPRODID_VALUE) << endl;
}

void OutputComputerName()
{
	cout << "Computer name : " << '\t' << (LPCSTR) GetRegistryEntry(COMP_KEY, COMP_VALUE) << endl;
}

void OutputUserName()
{
	cout << "User name     : " << '\t' << (LPCSTR) GetRegistryEntry(USER_KEY, USER_VALUE) << endl;	
}

void OutputCPU()
{
	cout << "CPU           : " << '\t' << (LPCSTR) GetRegistryEntry(CPU_KEY, CPU_VALUE) << endl;
	cout << "CPU Vendor    : " << '\t' << (LPCSTR) GetRegistryEntry(CPU_KEY, CPUV_VALUE) << endl;
	cout << "CPU Speed     : " << '\t' << cpunormspeed(0) << " Mhz" << endl;
}

void OutputMemory()
{
	MEMORYSTATUS	mem;

	GlobalMemoryStatus(&mem);

	cout << "RAM           : " << '\t' << (mem.dwTotalPhys/(1024*1024)) << " MB" << endl;	
}

void OutputMonitors()
{
	CRegistry	registry;
	CString		value;
	int			whichMon = 1;

	if(registry.Connect((HKEY) CRegistry::keyLocalMachine)==TRUE)
	{
		if(registry.Open(MONITOR_KEY))
		{
			// enumerate each monitor key
			// ignore the default monitor
			DWORD numMons = registry.GetNumberOfSubkeys();

			for(int i = 0;i < numMons;i++)
			{
				CString		subKeyName1, subKeyName2, className;
				
				registry.EnumerateKeys(i, subKeyName1, className);	
				if(subKeyName1!="DEFAULT_MONITOR")
				{
					if(registry.Open(MONITOR_KEY + "\\" + subKeyName1))
					{
						registry.EnumerateKeys(0, subKeyName2, className);	
						if(registry.Open(MONITOR_KEY + "\\" + subKeyName1 + "\\" + subKeyName2))
						{	
							CString		tmpString;

							registry.GetValue(MONITOR_VALUE, value);
							tmpString.Format("Monitor %d     : ", whichMon++);
							cout << (LPCSTR) tmpString << '\t' << (LPCSTR) value << endl;	
						}
					}
					// restore registry key
					registry.Open(MONITOR_KEY);
				}
			}
			registry.Close();
		}
	}
}

void OutputDrives()
{
	CString		whichDrive, tmpString;
	int			driveNum = 1;

	for(char drive = 'C';drive <= 'Z';drive++)
	{
		int	driveEnum;
		whichDrive.Format("%c:\\", drive);

		if((driveEnum = GetDriveType(whichDrive))!=DRIVE_NO_ROOT_DIR)
		{
			CString		driveType;

			switch(driveEnum)
			{
			case DRIVE_UNKNOWN:
				driveType = "Unknown";
				break;
			case DRIVE_REMOVABLE:
				driveType = "Removable";
				break;			
			case DRIVE_FIXED:
				driveType = "Fixed";
				break;		
			case DRIVE_REMOTE:
				driveType = "Remote";
				break;
			case DRIVE_CDROM:
				driveType = "CD-ROM";
				break;		
			case DRIVE_RAMDISK:
				driveType = "RAM";
				break;
			}
			
			if(driveEnum==DRIVE_FIXED)
			{
				ULARGE_INTEGER	freeBytes, totalBytes, dummy;
				float			totalSpace, freeSpace;

				GetDiskFreeSpaceEx(whichDrive, &dummy, &totalBytes, &freeBytes);
				totalSpace = ((signed __int64) totalBytes.QuadPart)/(1024*1024);
				freeSpace = ((signed __int64) freeBytes.QuadPart)/(1024*1024);
				tmpString.Format("Drive %d       : \t(%c)\t%s\t(Total=%.0fM)\t(Free=%.0fM)", driveNum++, drive, driveType, totalSpace, freeSpace);
			}
			else
				tmpString.Format("Drive %d       : \t(%c)\t%s", driveNum++, drive, driveType);

			cout << (LPCSTR) tmpString << endl;	
		}
	}
}

void OutputNetworkCards()
{
	CRegistry	registry;
	CString		value;
	int			whichCard = 1;

	if(registry.Connect((HKEY) CRegistry::keyLocalMachine)==TRUE)
	{
		if(registry.Open(ISA_KEY))
		{
			// enumerate each card key
			DWORD numCards = registry.GetNumberOfSubkeys();

			for(int i = 0;i < numCards;i++)
			{
				CString		subKeyName1, subKeyName2, className;
				
				registry.EnumerateKeys(i, subKeyName1, className);	
				if(registry.Open(ISA_KEY + "\\" + subKeyName1))
				{
					registry.EnumerateKeys(0, subKeyName2, className);	
					if(registry.Open(ISA_KEY + "\\" + subKeyName1 + "\\" + subKeyName2))
					{	
						CString		tmpString;

						registry.GetValue(CLASS_VALUE, className);
						className.MakeUpper();

						if(className==NETWORK_TAG)
						{
							registry.GetValue(ISA_VALUE, value);
							tmpString.Format("Net. Card %d   : ", whichCard++);
							cout << (LPCSTR) tmpString << '\t' << (LPCSTR) value << " (ISA)" << endl;
						}
					}
				}
				// restore registry key
				registry.Open(ISA_KEY);
			}
		}

		if(registry.Open(PCI_KEY))
		{
			// enumerate each card key
			DWORD numCards = registry.GetNumberOfSubkeys();

			for(int i = 0;i < numCards;i++)
			{
				CString		subKeyName1, subKeyName2, className;
				
				registry.EnumerateKeys(i, subKeyName1, className);	
				if(registry.Open(PCI_KEY + "\\" + subKeyName1))
				{
					registry.EnumerateKeys(0, subKeyName2, className);	
					if(registry.Open(PCI_KEY + "\\" + subKeyName1 + "\\" + subKeyName2))
					{	
						CString		tmpString;

						registry.GetValue(CLASS_VALUE, className);
						className.MakeUpper();

						if(className==NETWORK_TAG)
						{
							registry.GetValue(PCI_VALUE, value);
							tmpString.Format("Net. Card %d   : ", whichCard++);
							cout << (LPCSTR) tmpString << '\t' << (LPCSTR) value << " (PCI)" << endl;
						}
					}
				}
				// restore registry key
				registry.Open(PCI_KEY);
			}
		}

		if(registry.Open(PCMCIA_KEY))
		{
			// enumerate each card key
			DWORD numCards = registry.GetNumberOfSubkeys();

			for(int i = 0;i < numCards;i++)
			{
				CString		subKeyName1, subKeyName2, className;
				
				registry.EnumerateKeys(i, subKeyName1, className);	
				if(registry.Open(PCMCIA_KEY + "\\" + subKeyName1))
				{
					registry.EnumerateKeys(0, subKeyName2, className);	
					if(registry.Open(PCMCIA_KEY + "\\" + subKeyName1 + "\\" + subKeyName2))
					{	
						CString		tmpString;

						registry.GetValue(CLASS_VALUE, className);
						className.MakeUpper();

						if(className==NETWORK_TAG)
						{
							registry.GetValue(PCMCIA_VALUE, value);
							tmpString.Format("Net. Card %d   : ", whichCard++);
							cout << (LPCSTR) tmpString << '\t' << (LPCSTR) value << " (PCMCIA)" << endl;
						}
					}
				}
				// restore registry key
				registry.Open(PCMCIA_KEY);
			}
			registry.Close();
		}
	}
}

void OutputSoundCards()
{
	CRegistry	registry;
	CString		value;
	int			whichCard = 1;

	if(registry.Connect((HKEY) CRegistry::keyLocalMachine)==TRUE)
	{
		if(registry.Open(ISA_KEY))
		{
			// enumerate each card key
			DWORD numCards = registry.GetNumberOfSubkeys();

			for(int i = 0;i < numCards;i++)
			{
				CString		subKeyName1, subKeyName2, className;
				
				registry.EnumerateKeys(i, subKeyName1, className);	
				if(registry.Open(ISA_KEY + "\\" + subKeyName1))
				{
					registry.EnumerateKeys(0, subKeyName2, className);	
					if(registry.Open(ISA_KEY + "\\" + subKeyName1 + "\\" + subKeyName2))
					{	
						CString		tmpString;

						registry.GetValue(CLASS_VALUE, className);
						className.MakeUpper();

						if(className==SOUND_TAG)
						{
							registry.GetValue(ISA_VALUE, value);
							tmpString.Format("Sound Card %d  : ", whichCard++);
							cout << (LPCSTR) tmpString << '\t' << (LPCSTR) value << " (ISA)" << endl;
						}
					}
				}
				// restore registry key
				registry.Open(ISA_KEY);
			}
		}

		if(registry.Open(PCI_KEY))
		{
			// enumerate each card key
			DWORD numCards = registry.GetNumberOfSubkeys();

			for(int i = 0;i < numCards;i++)
			{
				CString		subKeyName1, subKeyName2, className;
				
				registry.EnumerateKeys(i, subKeyName1, className);	
				if(registry.Open(PCI_KEY + "\\" + subKeyName1))
				{
					registry.EnumerateKeys(0, subKeyName2, className);	
					if(registry.Open(PCI_KEY + "\\" + subKeyName1 + "\\" + subKeyName2))
					{	
						CString		tmpString;

						registry.GetValue(CLASS_VALUE, className);
						className.MakeUpper();

						if(className==SOUND_TAG)
						{
							registry.GetValue(PCI_VALUE, value);
							tmpString.Format("Sound Card %d  : ", whichCard++);
							cout << (LPCSTR) tmpString << '\t' << (LPCSTR) value << " (PCI)"<< endl;
						}
					}
				}
				// restore registry key
				registry.Open(PCI_KEY);
			}
			registry.Close();
		}
	}
}

void OutputModems()
{
	CRegistry	registry;
	CString		value;
	int			whichCard = 1;

	if(registry.Connect((HKEY) CRegistry::keyLocalMachine)==TRUE)
	{
		if(registry.Open(ISA_KEY))
		{
			// enumerate each card key
			DWORD numCards = registry.GetNumberOfSubkeys();

			for(int i = 0;i < numCards;i++)
			{
				CString		subKeyName1, subKeyName2, className;
				
				registry.EnumerateKeys(i, subKeyName1, className);	
				if(registry.Open(ISA_KEY + "\\" + subKeyName1))
				{
					registry.EnumerateKeys(0, subKeyName2, className);	
					if(registry.Open(ISA_KEY + "\\" + subKeyName1 + "\\" + subKeyName2))
					{	
						CString		tmpString;

						registry.GetValue(CLASS_VALUE, className);
						className.MakeUpper();

						if(className==MODEM_TAG)
						{
							registry.GetValue(ISA_VALUE, value);
							tmpString.Format("Modem Card %d  : ", whichCard++);
							cout << (LPCSTR) tmpString << '\t' << (LPCSTR) value << " (ISA)" << endl;
						}
					}
				}
				// restore registry key
				registry.Open(ISA_KEY);
			}
		}

		if(registry.Open(PCI_KEY))
		{
			// enumerate each card key
			DWORD numCards = registry.GetNumberOfSubkeys();

			for(int i = 0;i < numCards;i++)
			{
				CString		subKeyName1, subKeyName2, className;
				
				registry.EnumerateKeys(i, subKeyName1, className);	
				if(registry.Open(PCI_KEY + "\\" + subKeyName1))
				{
					registry.EnumerateKeys(0, subKeyName2, className);	
					if(registry.Open(PCI_KEY + "\\" + subKeyName1 + "\\" + subKeyName2))
					{	
						CString		tmpString;

						registry.GetValue(CLASS_VALUE, className);
						className.MakeUpper();

						if(className==MODEM_TAG)
						{
							registry.GetValue(PCI_VALUE, value);
							tmpString.Format("Modem Card %d  : ", whichCard++);
							cout << (LPCSTR) tmpString << '\t' << (LPCSTR) value << " (PCI)"<< endl;
						}
					}
				}
				// restore registry key
				registry.Open(PCI_KEY);
			}
		}

		if(registry.Open(PCMCIA_KEY))
		{
			// enumerate each card key
			DWORD numCards = registry.GetNumberOfSubkeys();

			for(int i = 0;i < numCards;i++)
			{
				CString		subKeyName1, subKeyName2, className;
				
				registry.EnumerateKeys(i, subKeyName1, className);	
				if(registry.Open(PCMCIA_KEY + "\\" + subKeyName1))
				{
					registry.EnumerateKeys(0, subKeyName2, className);	
					if(registry.Open(PCMCIA_KEY + "\\" + subKeyName1 + "\\" + subKeyName2))
					{	
						CString		tmpString;

						registry.GetValue(CLASS_VALUE, className);
						className.MakeUpper();

						if(className==MODEM_TAG)
						{
							registry.GetValue(PCMCIA_VALUE, value);
							tmpString.Format("Modem Card %d  : ", whichCard++);
							cout << (LPCSTR) tmpString << '\t' << (LPCSTR) value << " (PCMCIA)" << endl;
						}
					}
				}
				// restore registry key
				registry.Open(PCMCIA_KEY);
			}
			registry.Close();
		}
	}
}


void OutputVideoCards()
{
	CRegistry	registry;
	CString		value;
	int			whichCard = 1;

	if(registry.Connect((HKEY) CRegistry::keyLocalMachine)==TRUE)
	{
		if(registry.Open(ISA_KEY))
		{
			// enumerate each card key
			DWORD numCards = registry.GetNumberOfSubkeys();

			for(int i = 0;i < numCards;i++)
			{
				CString		subKeyName1, subKeyName2, className;
				
				registry.EnumerateKeys(i, subKeyName1, className);	
				if(registry.Open(ISA_KEY + "\\" + subKeyName1))
				{
					registry.EnumerateKeys(0, subKeyName2, className);	
					if(registry.Open(ISA_KEY + "\\" + subKeyName1 + "\\" + subKeyName2))
					{	
						CString		tmpString;

						registry.GetValue(CLASS_VALUE, className);
						className.MakeUpper();

						if(className==VIDEO_TAG)
						{
							registry.GetValue(ISA_VALUE, value);
							tmpString.Format("Video Card %d  : ", whichCard++);
							cout << (LPCSTR) tmpString << '\t' << (LPCSTR) value << " (ISA)" << endl;
						}
					}
				}
				// restore registry key
				registry.Open(ISA_KEY);
			}
		}

		if(registry.Open(PCI_KEY))
		{
			// enumerate each card key
			DWORD numCards = registry.GetNumberOfSubkeys();

			for(int i = 0;i < numCards;i++)
			{
				CString		subKeyName1, subKeyName2, className;
				
				registry.EnumerateKeys(i, subKeyName1, className);	
				if(registry.Open(PCI_KEY + "\\" + subKeyName1))
				{
					registry.EnumerateKeys(0, subKeyName2, className);	
					if(registry.Open(PCI_KEY + "\\" + subKeyName1 + "\\" + subKeyName2))
					{	
						CString		tmpString;

						registry.GetValue(CLASS_VALUE, className);
						className.MakeUpper();

						if(className==VIDEO_TAG)
						{
							registry.GetValue(PCI_VALUE, value);
							tmpString.Format("Video Card %d  : ", whichCard++);
							cout << (LPCSTR) tmpString << '\t' << (LPCSTR) value << " (PCI)"<< endl;
						}
					}
				}
				// restore registry key
				registry.Open(PCI_KEY);
			}
			registry.Close();
		}

	}
}

int main(int argc, char* argv[])
{
	CTime		today(CTime::GetCurrentTime());
	CString		timeRun;

	timeRun.Format("Time Run %d/%d/%d %d:%d", today.GetDay(), today.GetMonth(), today.GetYear(), today.GetHour(), today.GetMinute());
	cout << "TellMe - Copyright 1999 RKIL (www.rkil.com) - By Chris Wise (cwise@rkil.com)" << endl;
	cout << endl;
	cout << (LPCSTR) timeRun << endl;
	cout << endl;
	OutputComputerName();
	OutputUserName();
	OutputOSInfo();
	OutputCPU();
	OutputMemory();
	OutputDrives();
	OutputMonitors();
	OutputVideoCards();
	OutputSoundCards();
	OutputNetworkCards();
	OutputModems();

	return 0;
}

