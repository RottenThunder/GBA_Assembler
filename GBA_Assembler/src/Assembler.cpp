#include <iostream>
#include <filesystem>
#include <fstream>

#define ASSEMBLER_VERSION_MAJOR 1
#define ASSEMBLER_VERSION_MINOR 0
#define ASSEMBLER_VERSION_PATCH 0

#if defined ASSEMBLER_CONFIG_DEBUG
#define ASSEMBLER_OUTPUT_SYMBOL_0 '0'
#define ASSEMBLER_OUTPUT_SYMBOL_1 '1'
#define ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER '2'
#define ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION '\n'
#define ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING "\n"

#define ASSEMBLER_INSTRUCTION_CHAR_SIZE 17
#endif

#if defined ASSEMBLER_CONFIG_RELEASE
#define ASSEMBLER_OUTPUT_SYMBOL_0 '\0'
#define ASSEMBLER_OUTPUT_SYMBOL_1 '\1'
#define ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER '\2'
#define ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION '\3'
#define ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING "\3"

#define ASSEMBLER_INSTRUCTION_CHAR_SIZE 16
#endif

#define ASSEMBLER_BRANCH_TYPE char
#define ASSEMBLER_BRANCH_EQ 0
#define ASSEMBLER_BRANCH_NE 1
#define ASSEMBLER_BRANCH_CS_HS 2
#define ASSEMBLER_BRANCH_CC_LO 3
#define ASSEMBLER_BRANCH_MI 4
#define ASSEMBLER_BRANCH_PL 5
#define ASSEMBLER_BRANCH_VS 6
#define ASSEMBLER_BRANCH_VC 7
#define ASSEMBLER_BRANCH_HI 8
#define ASSEMBLER_BRANCH_LS 9
#define ASSEMBLER_BRANCH_GE 10
#define ASSEMBLER_BRANCH_LT 11
#define ASSEMBLER_BRANCH_GT 12
#define ASSEMBLER_BRANCH_LE 13
#define ASSEMBLER_BRANCH_AL 14
#define ASSEMBLER_BRANCH_NV 15
#define ASSEMBLER_BRANCH_LINK 16

#define ASSEMBLER_MEM_BIOS    0x00000000
#define ASSEMBLER_MEM_ERAM    0x02000000
#define ASSEMBLER_MEM_IRAM    0x03000000
#define ASSEMBLER_MEM_IO      0x04000000
#define ASSEMBLER_MEM_PALETTE 0x05000000
#define ASSEMBLER_MEM_VRAM    0x06000000
#define ASSEMBLER_MEM_OAM     0x07000000
#define ASSEMBLER_MEM_ROM     0x08000000

struct LabelInfo
{
	std::string label;
	uint64_t fileNumber;
	uint64_t instructionNumber;
};

struct BranchInfo
{
	ASSEMBLER_BRANCH_TYPE type;
	uint64_t fileNumber;
	uint64_t InstructionNumber;
	std::string label;
};

struct ByteSequenceInfo
{
	std::string label;
	std::vector<char> bytes;
	uint64_t alignment;
	uint64_t address;
};

struct PtrSequenceInfo
{
	std::string label;
	std::vector<std::string> pointers;
	std::vector<char> bytes;
	uint64_t address;
};

struct MovAddressInfo
{
	std::string label;
	int destinationRegister;
	uint64_t fileNumber;
	uint64_t InstructionNumber;
};

struct JoinInfo
{
	uint64_t parentFile;
	uint64_t childFile;
};

static std::vector<LabelInfo> s_LabelMap;
static std::vector<BranchInfo> s_BranchMap;
static std::vector<ByteSequenceInfo> s_ByteSequenceMap;
static std::vector<PtrSequenceInfo> s_PtrSequenceMap;
static std::vector<MovAddressInfo> s_MovAddressMap;
static std::vector<JoinInfo> s_JoinMap;

static bool s_SuccessfulIntConversion;
static uint64_t s_CurrentByteSequenceAlignment;

static int StringToDecimalInt(const std::string& str)
{
	int result;
	s_SuccessfulIntConversion = true;
	try
	{
		result = std::stoi(str, nullptr, 10);
	}
	catch (std::invalid_argument const&)
	{
		s_SuccessfulIntConversion = false;
	}
	catch (std::out_of_range const&)
	{
		s_SuccessfulIntConversion = false;
	}

	return result;
}

static int StringToHexadecimalInt(const std::string& str)
{
	int result;
	s_SuccessfulIntConversion = true;
	try
	{
		result = std::stoi(str, nullptr, 16);
	}
	catch (std::invalid_argument const&)
	{
		s_SuccessfulIntConversion = false;
	}
	catch (std::out_of_range const&)
	{
		s_SuccessfulIntConversion = false;
	}

	return result;
}

static void ProcessBranchInstruction(BranchInfo info, std::string& placeholder)
{
	size_t i = 0;
	size_t j = 0;
	for (; i < info.label.size(); i++)
	{
		if (info.label[i] != ' ')
		{
			j = i;
			i = info.label.size();
		}
	}
	info.label.erase(0, j);
	s_BranchMap.push_back(info);
	placeholder.clear();
	for (i = 0; i < 20 * 8; i++)
	{
#ifdef ASSEMBLER_CONFIG_DEBUG
		if (i % 16 == 0 && i != 0)
			placeholder.push_back(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION);
#endif

		placeholder.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
	}
}

static bool ProcessNOPInstruction(std::string& nopParameters)
{
	for (size_t i = 0; i < nopParameters.size(); i++)
	{
		if (nopParameters[i] != ' ')
			return false;
	}
	nopParameters.clear();
	nopParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	nopParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	nopParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	nopParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	nopParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	nopParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	nopParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	nopParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	nopParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	nopParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	nopParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	nopParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	nopParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	nopParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	nopParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	nopParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	return true;
}

static bool ProcessADCInstruction(std::string& adcParameters)
{
	for (size_t i = 0; i < adcParameters.size(); i++)
	{
		if (adcParameters[i] != ' ')
		{
			adcParameters.erase(0, i);
			i = adcParameters.size();
		}
	}

	if (adcParameters.size() < 3)
		return false;

	if (adcParameters[0] != 'R')
		return false;
	if (adcParameters[1] > '7' || adcParameters[1] < '0')
		return false;
	if (adcParameters[2] != ' ')
		return false;

	char Rd = adcParameters[1] - '0';

	for (size_t i = 3; i < adcParameters.size(); i++)
	{
		if (adcParameters[i] != ' ')
		{
			adcParameters.erase(3, i - 3);
			i = adcParameters.size();
		}
	}

	if (adcParameters.size() < 5)
		return false;

	if (adcParameters[3] != 'R')
		return false;
	if (adcParameters[4] > '7' || adcParameters[4] < '0')
		return false;

	char Rm = adcParameters[4] - '0';

	for (size_t i = 5; i < adcParameters.size(); i++)
	{
		if (adcParameters[i] != ' ')
			return false;
	}

	adcParameters.clear();

	adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);

	if (Rm & 4)
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rm & 2)
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rm & 1)
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 4)
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 2)
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 1)
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	return true;
}

static bool ProcessADDInstruction(std::string& addParameters)
{
	for (size_t i = 0; i < addParameters.size(); i++)
	{
		if (addParameters[i] != ' ')
		{
			addParameters.erase(0, i);
			i = addParameters.size();
		}
	}

	if (addParameters.size() < 3)
		return false;

	if (addParameters[0] != 'R')
		return false;
	if (addParameters[1] > '7' || addParameters[1] < '0')
		return false;
	if (addParameters[2] != ' ')
		return false;

	char Rd = addParameters[1] - '0';

	for (size_t i = 3; i < addParameters.size(); i++)
	{
		if (addParameters[i] != ' ')
		{
			addParameters.erase(3, i - 3);
			i = addParameters.size();
		}
	}

	if (addParameters.size() < 4)
		return false;

	if (addParameters[3] == '#')
	{
		if (addParameters.size() < 5)
			return false;

		int immediate = StringToDecimalInt(addParameters.substr(4));
		if (!s_SuccessfulIntConversion)
			return false;

		if (immediate < 0 || immediate > 255)
			return false;

		addParameters.clear();
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 4)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 2)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 1)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (immediate & 128)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (immediate & 64)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (immediate & 32)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (immediate & 16)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (immediate & 8)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (immediate & 4)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (immediate & 2)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (immediate & 1)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		return true;
	}

	if (addParameters.size() < 6)
		return false;

	if (addParameters[3] != 'R')
		return false;
	if (addParameters[4] > '7' || addParameters[4] < '0')
		return false;
	if (addParameters[5] != ' ')
		return false;

	char Rn = addParameters[4] - '0';

	for (size_t i = 6; i < addParameters.size(); i++)
	{
		if (addParameters[i] != ' ')
		{
			addParameters.erase(6, i - 6);
			i = addParameters.size();
		}
	}

	if (addParameters.size() < 7)
		return false;

	if (addParameters[6] == '#')
	{
		if (addParameters.size() < 8)
			return false;

		int immediate = StringToDecimalInt(addParameters.substr(7));
		if (!s_SuccessfulIntConversion)
			return false;

		if (immediate < 0 || immediate > 7)
			return false;

		addParameters.clear();
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (immediate & 4)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (immediate & 2)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (immediate & 1)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rn & 4)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rn & 2)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rn & 1)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 4)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 2)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 1)
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		return true;
	}

	if (addParameters.size() < 8)
		return false;

	if (addParameters[6] != 'R')
		return false;
	if (addParameters[7] > '7' || addParameters[7] < '0')
		return false;

	for (size_t i = 8; i < addParameters.size(); i++)
	{
		if (addParameters[i] != ' ')
			return false;
	}

	char Rm = addParameters[7] - '0';

	addParameters.clear();
	addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rm & 4)
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rm & 2)
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rm & 1)
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rn & 4)
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rn & 2)
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rn & 1)
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 4)
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 2)
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 1)
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	return true;
}

static bool ProcessADDHIInstruction(std::string& addhiParameters)
{
	for (size_t i = 0; i < addhiParameters.size(); i++)
	{
		if (addhiParameters[i] != ' ')
		{
			addhiParameters.erase(0, i);
			i = addhiParameters.size();
		}
	}

	if (addhiParameters.size() < 3)
		return false;

	if (addhiParameters[0] != 'R')
		return false;
	if (addhiParameters[2] != ' ')
		return false;

	int Rd = StringToHexadecimalInt(addhiParameters.substr(1, 1));
	if (!s_SuccessfulIntConversion)
		return false;

	for (size_t i = 3; i < addhiParameters.size(); i++)
	{
		if (addhiParameters[i] != ' ')
		{
			addhiParameters.erase(3, i - 3);
			i = addhiParameters.size();
		}
	}

	if (addhiParameters.size() < 5)
		return false;

	if (addhiParameters[3] != 'R')
		return false;

	int Rm = StringToHexadecimalInt(addhiParameters.substr(4, 1));
	if (!s_SuccessfulIntConversion)
		return false;

	for (size_t i = 5; i < addhiParameters.size(); i++)
	{
		if (addhiParameters[i] != ' ')
			return false;
	}

	addhiParameters.clear();

	if (Rd < 8 && Rm < 8)
	{
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rm & 4)
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rm & 2)
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rm & 1)
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 4)
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 2)
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 1)
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 4)
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 2)
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 1)
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		return true;
	}

	addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 8)
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rm & 8)
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rm & 4)
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rm & 2)
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rm & 1)
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 4)
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 2)
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 1)
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addhiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	return true;
}

static bool ProcessADDSPInstruction(std::string& addspParameters)
{
	for (size_t i = 0; i < addspParameters.size(); i++)
	{
		if (addspParameters[i] != ' ')
		{
			addspParameters.erase(0, i);
			i = addspParameters.size();
		}
	}
	
	if (addspParameters.size() < 2)
		return false;
	if (addspParameters[0] != '#')
		return false;

	int immediate = StringToDecimalInt(addspParameters.substr(1));
	if (!s_SuccessfulIntConversion)
		return false;

	if (immediate < 0 || immediate > 127)
		return false;

	addspParameters.clear();
	addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);

	if (immediate & 64)
		addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (immediate & 32)
		addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (immediate & 16)
		addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (immediate & 8)
		addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (immediate & 4)
		addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (immediate & 2)
		addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (immediate & 1)
		addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		addspParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	return true;
}

static bool ProcessANDInstruction(std::string& andParameters)
{
	if (!ProcessADCInstruction(andParameters))
		return false;

	andParameters[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
	andParameters[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
	return true;
}

static bool ProcessASRInstruction(std::string& asrParameters)
{
	for (size_t i = 0; i < asrParameters.size(); i++)
	{
		if (asrParameters[i] != ' ')
		{
			asrParameters.erase(0, i);
			i = asrParameters.size();
		}
	}

	if (asrParameters.size() < 3)
		return false;

	if (asrParameters[0] != 'R')
		return false;
	if (asrParameters[1] > '7' || asrParameters[1] < '0')
		return false;
	if (asrParameters[2] != ' ')
		return false;

	char Rd = asrParameters[1] - '0';

	for (size_t i = 3; i < asrParameters.size(); i++)
	{
		if (asrParameters[i] != ' ')
		{
			asrParameters.erase(3, i - 3);
			i = asrParameters.size();
		}
	}

	if (asrParameters.size() < 5)
		return false;

	if (asrParameters[3] != 'R')
		return false;
	if (asrParameters[4] > '7' || asrParameters[4] < '0')
		return false;

	char Rm = asrParameters[4] - '0';

	size_t immediate = 0;
	for (size_t i = 5; i < asrParameters.size(); i++)
	{
		if (asrParameters[i] == '#')
		{
			immediate = i;
			break;
		}
		else if (asrParameters[i] != ' ')
			return false;
	}

	if (immediate != 0)
	{
		immediate++;
		if (asrParameters.size() == immediate)
			return false;

		int convertedValue = StringToDecimalInt(asrParameters.substr(immediate));
		if (!s_SuccessfulIntConversion)
			return false;

		if (convertedValue < 1 || convertedValue > 32)
			return false;

		if (convertedValue == 32)
			convertedValue = 0;

		asrParameters.clear();

		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (convertedValue & 16)
			asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (convertedValue & 8)
			asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (convertedValue & 4)
			asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (convertedValue & 2)
			asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (convertedValue & 1)
			asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	}
	else
	{
		asrParameters.clear();

		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	}

	if (Rm & 4)
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rm & 2)
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rm & 1)
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 4)
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 2)
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 1)
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		asrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	return true;
}

static bool ProcessBICInstruction(std::string& bicParameters)
{
	if (!ProcessADCInstruction(bicParameters))
		return false;

	bicParameters[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
	bicParameters[8] = ASSEMBLER_OUTPUT_SYMBOL_1;
	bicParameters[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
	return true;
}

static bool ProcessRETURNInstruction(std::string& returnParameters)
{
	if (!ProcessNOPInstruction(returnParameters))
		return false;

	returnParameters[0] = ASSEMBLER_OUTPUT_SYMBOL_1;
	returnParameters[2] = ASSEMBLER_OUTPUT_SYMBOL_1;
	returnParameters[3] = ASSEMBLER_OUTPUT_SYMBOL_1;
	returnParameters[4] = ASSEMBLER_OUTPUT_SYMBOL_1;
	returnParameters[5] = ASSEMBLER_OUTPUT_SYMBOL_1;
	returnParameters[7] = ASSEMBLER_OUTPUT_SYMBOL_1;
	return true;
}

static bool ProcessCMNInstruction(std::string& cmnParameters)
{
	if (!ProcessADCInstruction(cmnParameters))
		return false;

	cmnParameters[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
	cmnParameters[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
	cmnParameters[8] = ASSEMBLER_OUTPUT_SYMBOL_1;
	return true;
}

static bool ProcessCMPInstruction(std::string& cmpParameters)
{
	for (size_t i = 0; i < cmpParameters.size(); i++)
	{
		if (cmpParameters[i] != ' ')
		{
			cmpParameters.erase(0, i);
			i = cmpParameters.size();
		}
	}

	if (cmpParameters.size() < 3)
		return false;

	if (cmpParameters[0] != 'R')
		return false;

	int Rd = StringToHexadecimalInt(cmpParameters.substr(1, 1));
	if (!s_SuccessfulIntConversion)
		return false;

	if (cmpParameters[2] != ' ')
		return false;

	for (size_t i = 3; i < cmpParameters.size(); i++)
	{
		if (cmpParameters[i] != ' ')
		{
			cmpParameters.erase(3, i - 3);
			i = cmpParameters.size();
		}
	}

	if (cmpParameters.size() < 4)
		return false;

	if (cmpParameters[3] == '#')
	{
		if (cmpParameters.size() == 4)
			return false;

		int convertedValue = StringToDecimalInt(cmpParameters.substr(4));
		if (!s_SuccessfulIntConversion)
			return false;

		if (convertedValue < 0 || convertedValue > 255)
			return false;

		cmpParameters.clear();

		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);

		if (Rd & 4)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 2)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 1)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (convertedValue & 128)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (convertedValue & 64)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (convertedValue & 32)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (convertedValue & 16)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (convertedValue & 8)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (convertedValue & 4)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (convertedValue & 2)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (convertedValue & 1)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		return true;
	}

	if (cmpParameters.size() < 5)
		return false;

	if (cmpParameters[3] != 'R')
		return false;

	int Rm = StringToHexadecimalInt(cmpParameters.substr(4, 1));
	if (!s_SuccessfulIntConversion)
		return false;

	cmpParameters.clear();
	if (Rd > 7 || Rm > 7)
	{
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);

		if (Rd & 8)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rm & 8)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rm & 4)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rm & 2)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rm & 1)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 4)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 2)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 1)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	}
	else
	{
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rm & 4)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rm & 2)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rm & 1)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 4)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 2)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rd & 1)
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			cmpParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	}

	return true;
}

static bool ProcessXORInstruction(std::string& xorParameters)
{
	if (!ProcessADCInstruction(xorParameters))
		return false;

	xorParameters[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
	return true;
}

static bool ProcessLDMIAInstruction(std::string& ldmiaParameters)
{
	return false;
}

static bool ProcessLDRInstruction(std::string& ldrParameters)
{
	for (size_t i = 0; i < ldrParameters.size(); i++)
	{
		if (ldrParameters[i] != ' ')
		{
			ldrParameters.erase(0, i);
			i = ldrParameters.size();
		}
	}

	if (ldrParameters.size() < 3)
		return false;

	if (ldrParameters[0] != 'R')
		return false;
	if (ldrParameters[1] > '7' || ldrParameters[1] < '0')
		return false;
	if (ldrParameters[2] != ' ')
		return false;

	char Rd = ldrParameters[1] - '0';

	for (size_t i = 3; i < ldrParameters.size(); i++)
	{
		if (ldrParameters[i] != ' ')
		{
			ldrParameters.erase(3, i - 3);
			i = ldrParameters.size();
		}
	}

	if (ldrParameters.size() < 6)
		return false;

	if (ldrParameters[3] != 'R')
		return false;
	if (ldrParameters[4] > '7' || ldrParameters[4] < '0')
		return false;
	if (ldrParameters[5] != ' ')
		return false;

	char Rn = ldrParameters[4] - '0';

	for (size_t i = 6; i < ldrParameters.size(); i++)
	{
		if (ldrParameters[i] != ' ')
		{
			ldrParameters.erase(6, i - 6);
			i = ldrParameters.size();
		}
	}

	if (ldrParameters[6] == '#')
	{
		int immediate = StringToDecimalInt(ldrParameters.substr(7));
		if (!s_SuccessfulIntConversion)
			return false;

		if (immediate < 0 || immediate > 31)
			return false;

		ldrParameters.clear();
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);

		if (immediate & 16)
			ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (immediate & 8)
			ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (immediate & 4)
			ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (immediate & 2)
			ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (immediate & 1)
			ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	}
	else if (ldrParameters[6] == 'R')
	{
		if (ldrParameters[7] > '7' || ldrParameters[7] < '0')
			return false;

		for (size_t i = 8; i < ldrParameters.size(); i++)
		{
			if (ldrParameters[i] != ' ')
				return false;
		}

		char Rm = ldrParameters[7] - '0';

		ldrParameters.clear();
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rm & 4)
			ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rm & 2)
			ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

		if (Rm & 1)
			ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
		else
			ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	}
	else
	{
		return false;
	}

	if (Rn & 4)
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rn & 2)
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rn & 1)
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 4)
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 2)
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 1)
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		ldrParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	return true;
}

static bool ProcessLDRBInstruction(std::string& ldrbParameters)
{
	if (!ProcessLDRInstruction(ldrbParameters))
		return false;

	if (ldrbParameters[2] == ASSEMBLER_OUTPUT_SYMBOL_1)
	{
		ldrbParameters[3] = ASSEMBLER_OUTPUT_SYMBOL_1;
	}
	else
	{
		ldrbParameters[5] = ASSEMBLER_OUTPUT_SYMBOL_1;
	}

	return true;
}

static bool ProcessLDRHInstruction(std::string& ldrhParameters)
{
	if (!ProcessLDRInstruction(ldrhParameters))
		return false;

	if (ldrhParameters[2] == ASSEMBLER_OUTPUT_SYMBOL_1)
	{
		ldrhParameters[0] = ASSEMBLER_OUTPUT_SYMBOL_1;
		ldrhParameters[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
		ldrhParameters[2] = ASSEMBLER_OUTPUT_SYMBOL_0;
	}
	else
	{
		ldrhParameters[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
	}

	return true;
}

static bool ProcessLDRSBInstruction(std::string& ldrsbParameters)
{
	for (size_t i = 0; i < ldrsbParameters.size(); i++)
	{
		if (ldrsbParameters[i] != ' ')
		{
			ldrsbParameters.erase(0, i);
			i = ldrsbParameters.size();
		}
	}

	if (ldrsbParameters.size() < 3)
		return false;

	if (ldrsbParameters[0] != 'R')
		return false;
	if (ldrsbParameters[1] > '7' || ldrsbParameters[1] < '0')
		return false;
	if (ldrsbParameters[2] != ' ')
		return false;

	char Rd = ldrsbParameters[1] - '0';

	for (size_t i = 3; i < ldrsbParameters.size(); i++)
	{
		if (ldrsbParameters[i] != ' ')
		{
			ldrsbParameters.erase(3, i - 3);
			i = ldrsbParameters.size();
		}
	}

	if (ldrsbParameters.size() < 6)
		return false;

	if (ldrsbParameters[3] != 'R')
		return false;
	if (ldrsbParameters[4] > '7' || ldrsbParameters[4] < '0')
		return false;
	if (ldrsbParameters[5] != ' ')
		return false;

	char Rn = ldrsbParameters[4] - '0';

	for (size_t i = 6; i < ldrsbParameters.size(); i++)
	{
		if (ldrsbParameters[i] != ' ')
		{
			ldrsbParameters.erase(6, i - 6);
			i = ldrsbParameters.size();
		}
	}

	if (ldrsbParameters.size() < 8)
		return false;

	if (ldrsbParameters[6] != 'R')
		return false;
	if (ldrsbParameters[7] > '7' || ldrsbParameters[7] < '0')
		return false;

	for (size_t i = 8; i < ldrsbParameters.size(); i++)
	{
		if (ldrsbParameters[i] != ' ')
			return false;
	}

	char Rm = ldrsbParameters[7] - '0';

	ldrsbParameters.clear();
	ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);

	if (Rm & 4)
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rm & 2)
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rm & 1)
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rn & 4)
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rn & 2)
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rn & 1)
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 4)
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 2)
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (Rd & 1)
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		ldrsbParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	return true;
}

static bool ProcessLDRSHInstruction(std::string& ldrshParameters)
{
	if (!ProcessLDRSBInstruction(ldrshParameters))
		return false;

	ldrshParameters[4] = ASSEMBLER_OUTPUT_SYMBOL_1;

	return false;
}

static bool ProcessLSLInstruction(std::string& lslParameters)
{
	if (!ProcessASRInstruction(lslParameters))
		return false;

	if (lslParameters[1] == ASSEMBLER_OUTPUT_SYMBOL_0)
	{
		lslParameters[3] = ASSEMBLER_OUTPUT_SYMBOL_0;
	}
	else
	{
		lslParameters[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
		lslParameters[8] = ASSEMBLER_OUTPUT_SYMBOL_1;
	}

	return true;
}

static bool ProcessLSRInstruction(std::string& lsrParameters)
{
	if (!ProcessASRInstruction(lsrParameters))
		return false;

	if (lsrParameters[1] == ASSEMBLER_OUTPUT_SYMBOL_0)
	{
		lsrParameters[3] = ASSEMBLER_OUTPUT_SYMBOL_0;
		lsrParameters[4] = ASSEMBLER_OUTPUT_SYMBOL_1;
	}
	else
	{
		lsrParameters[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
		lsrParameters[8] = ASSEMBLER_OUTPUT_SYMBOL_1;
		lsrParameters[9] = ASSEMBLER_OUTPUT_SYMBOL_1;
	}

	return true;
}

static bool ProcessMOVInstruction(std::string& movParameters)
{
	if (!ProcessCMPInstruction(movParameters))
		return false;

	if (movParameters[1] == ASSEMBLER_OUTPUT_SYMBOL_0)
	{
		movParameters[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
	}
	else
	{
		if (movParameters[5] == ASSEMBLER_OUTPUT_SYMBOL_0)
		{
			movParameters[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
			movParameters[3] = ASSEMBLER_OUTPUT_SYMBOL_1;
			movParameters[4] = ASSEMBLER_OUTPUT_SYMBOL_1;
			movParameters[5] = ASSEMBLER_OUTPUT_SYMBOL_1;
			movParameters[6] = ASSEMBLER_OUTPUT_SYMBOL_0;
			movParameters[8] = ASSEMBLER_OUTPUT_SYMBOL_0;
		}
		else
		{
			movParameters[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
			movParameters[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
		}
	}

	return true;
}

static bool ProcessMOVAInstruction(std::string& movaParameters, uint64_t fileNumber, uint64_t instructionNumber)
{
	for (size_t i = 0; i < movaParameters.size(); i++)
	{
		if (movaParameters[i] != ' ')
		{
			movaParameters.erase(0, i);
			i = movaParameters.size();
		}
	}

	if (movaParameters.size() < 3)
		return false;

	if (movaParameters[0] != 'R')
		return false;

	int Rd = StringToDecimalInt(movaParameters.substr(1, 1));
	if (!s_SuccessfulIntConversion)
		return false;

	if (movaParameters[2] != ' ')
		return false;

	for (size_t i = 3; i < movaParameters.size(); i++)
	{
		if (movaParameters[i] != ' ')
		{
			movaParameters.erase(3, i - 3);
			i = movaParameters.size();
		}
	}

	if (movaParameters.size() < 4)
		return false;

	if (movaParameters[3] != '&')
		return false;

	if (Rd > 7)
		return false;

	s_MovAddressMap.push_back({ movaParameters.substr(4, UINT64_MAX), Rd, fileNumber, instructionNumber });
	movaParameters.clear();
	for (size_t i = 0; i < 14 * 8; i++)
	{
#ifdef ASSEMBLER_CONFIG_DEBUG
		if (i % 16 == 0 && i != 0)
			movaParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION);
#endif

		movaParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
	}

	return true;
}

static bool ProcessMULInstruction(std::string& mulParameters)
{
	if (!ProcessADCInstruction(mulParameters))
		return false;

	mulParameters[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
	return true;
}

static bool ProcessMVNInstruction(std::string& mvnParameters)
{
	if (!ProcessADCInstruction(mvnParameters))
		return false;

	mvnParameters[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
	mvnParameters[8] = ASSEMBLER_OUTPUT_SYMBOL_1;
	return true;
}

static bool ProcessNEGInstruction(std::string& negParameters)
{
	if (!ProcessADCInstruction(negParameters))
		return false;

	negParameters[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
	negParameters[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
	return true;
}

static bool ProcessORRInstruction(std::string& orrParameters)
{
	if (!ProcessADCInstruction(orrParameters))
		return false;

	orrParameters[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
	orrParameters[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
	return true;
}

static bool ProcessRORInstruction(std::string& rorParameters)
{
	if (!ProcessADCInstruction(rorParameters))
		return false;

	rorParameters[8] = ASSEMBLER_OUTPUT_SYMBOL_1;
	return true;
}

static bool ProcessSBCInstruction(std::string& sbcParameters)
{
	if (!ProcessADCInstruction(sbcParameters))
		return false;

	sbcParameters[8] = ASSEMBLER_OUTPUT_SYMBOL_1;
	sbcParameters[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
	return true;
}

static bool ProcessSTMIAInstruction(std::string& stmiaParameters)
{
	return false;
}

static bool ProcessSTRInstruction(std::string& strParameters)
{
	if (!ProcessLDRInstruction(strParameters))
		return false;

	strParameters[4] = ASSEMBLER_OUTPUT_SYMBOL_0;

	return true;
}

static bool ProcessSTRBInstruction(std::string& strbParameters)
{
	if (!ProcessLDRInstruction(strbParameters))
		return false;

	if (strbParameters[2] == ASSEMBLER_OUTPUT_SYMBOL_1)
	{
		strbParameters[3] = ASSEMBLER_OUTPUT_SYMBOL_1;
		strbParameters[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
	}
	else
	{
		strbParameters[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
		strbParameters[5] = ASSEMBLER_OUTPUT_SYMBOL_1;
	}

	return true;
}

static bool ProcessSTRHInstruction(std::string& strhParameters)
{
	if (!ProcessLDRInstruction(strhParameters))
		return false;

	if (strhParameters[2] == ASSEMBLER_OUTPUT_SYMBOL_1)
	{
		strhParameters[0] = ASSEMBLER_OUTPUT_SYMBOL_1;
		strhParameters[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
		strhParameters[2] = ASSEMBLER_OUTPUT_SYMBOL_0;
		strhParameters[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
	}
	else
	{
		strhParameters[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
		strhParameters[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
	}

	return true;
}

static bool ProcessSUBInstruction(std::string& subParameters)
{
	if (!ProcessADDInstruction(subParameters))
		return false;

	if (subParameters[2] == ASSEMBLER_OUTPUT_SYMBOL_1)
		subParameters[4] = ASSEMBLER_OUTPUT_SYMBOL_1;
	else
		subParameters[6] = ASSEMBLER_OUTPUT_SYMBOL_1;

	return true;
}

static bool ProcessSUBSPInstruction(std::string& subspParameters)
{
	if (!ProcessADDSPInstruction(subspParameters))
		return false;

	subspParameters[8] = ASSEMBLER_OUTPUT_SYMBOL_0;

	return true;
}

static bool ProcessSWIInstruction(std::string& swiParameters)
{
	for (size_t i = 0; i < swiParameters.size(); i++)
	{
		if (swiParameters[i] != ' ')
		{
			swiParameters.erase(0, i);
			i = swiParameters.size();
		}
	}

	int interuptID = StringToDecimalInt(swiParameters);
	if (!s_SuccessfulIntConversion)
		return false;

	if (interuptID < 0 || interuptID > 255)
		return false;

	swiParameters.clear();
	swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);

	if (interuptID & 128)
		swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (interuptID & 64)
		swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (interuptID & 32)
		swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (interuptID & 16)
		swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (interuptID & 8)
		swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (interuptID & 4)
		swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (interuptID & 2)
		swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	if (interuptID & 1)
		swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);
	else
		swiParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);

	return true;
}

static bool ProcessTSTInstruction(std::string& tstParameters)
{
	if (!ProcessADCInstruction(tstParameters))
		return false;

	tstParameters[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
	tstParameters[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
	tstParameters[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
	return true;
}

static bool PreProcess(const std::filesystem::path& sourcePath, const std::filesystem::path& filePath, uint64_t fileNumber, const std::filesystem::path& intDir)
{
	s_CurrentByteSequenceAlignment = 1;

	std::fstream inputStream;
	std::fstream outputStream;
	inputStream.open(filePath, std::ios::in | std::ios::binary);
	if (!inputStream.is_open())
	{
		inputStream.close();
		std::cout << "Could not open " << std::filesystem::relative(filePath, sourcePath) << std::endl;
		return false;
	}
	std::filesystem::path relativePath = std::filesystem::relative(filePath, sourcePath);
	std::string preProcessedFileName = relativePath.string();
#ifdef ASSEMBLER_CONFIG_DEBUG
	preProcessedFileName[preProcessedFileName.size() - 3] = 't';
	preProcessedFileName[preProcessedFileName.size() - 2] = 'x';
	preProcessedFileName[preProcessedFileName.size() - 1] = 't';
#endif
#ifdef ASSEMBLER_CONFIG_RELEASE
	preProcessedFileName[preProcessedFileName.size() - 3] = 'b';
	preProcessedFileName[preProcessedFileName.size() - 2] = 'i';
	preProcessedFileName[preProcessedFileName.size() - 1] = 'n';
#endif
	std::filesystem::create_directories(intDir / relativePath.parent_path());
	outputStream.open(intDir / preProcessedFileName, std::ios::out | std::ios::binary);

	std::string mostRecentLabel;
	uint64_t currentInstructionNumber = 0;
	size_t currentLineNumber = 0;
	std::string currentLine;
	while (std::getline(inputStream, currentLine))
	{
		size_t i = 0;
		size_t j = 0;

		currentLineNumber++;

		//1. Get Rid Of Any Non-Printable Characters
		//Use "i" as an index into "currentLine", Use "j" as the current size of "currentLine"
		j = currentLine.size();
		while (i < j)
		{
			if (currentLine[i] < ' ')
			{
				currentLine.erase(i, 1);
				j--;
			}
			else
			{
				i++;
			}
		}

		//2. Delete All Comments
		//Use "i" as an index into "currentLine", Use "j" as a count of the amount of consecutive '/' chars that have been seen
		i = 0;
		j = 0;
		for (; i < currentLine.size(); i++)
		{
			if (currentLine[i] == '/')
				j++;
			else
				j = 0;

			if (j == 2)
			{
				j = i;
				j++;
				i = currentLine.size();
			}
		}
		if (j >= 2)
			currentLine.erase(j - 2, UINT64_MAX);


		//3. Replace All Commas With Spaces
		//Use "i" as an index into "currentLine"
		for (i = 0; i < currentLine.size(); i++)
		{
			if (currentLine[i] == ',')
				currentLine[i] = ' ';
		}

		//4. Process Preprocessor Directives
		//Use "i" and "j" for a variety of things
		j = 0;
		i = currentLine.find('~');
		if (i != std::string::npos)
		{
			i++;
			if (i == currentLine.find("join"))
			{
				i += 4;
				if (i == currentLine.size())
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
					std::cout << "The ~join Preprocessor Directive On This Line Does Not Have A File Path To Join Associated With It" << std::endl;
					return false;
				}
				while (i < currentLine.size())
				{
					if (currentLine[i] == ' ')
						i++;
					else
					{
						j = i;
						i = currentLine.size();
					}
				}
				if (j == 0)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
					std::cout << "The ~join Preprocessor Directive On This Line Does Not Have A File Path To Join Associated With It" << std::endl;
					return false;
				}
				if (currentLine[j] != '"')
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
					std::cout << "The ~join Preprocessor Directive On This Line Does Not Have A File Path (In Inverted Commas) To Join Associated With It" << std::endl;
					return false;
				}
				j++;
				while (j < currentLine.size())
				{
					if (currentLine[j] == '"')
					{
						i = j;
						j = currentLine.size();
					}
					j++;
				}
				if (j == currentLine.size())
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
					std::cout << "The ~join Preprocessor Directive On This Line Does Not Have A File Path (In Inverted Commas) To Join Associated With It" << std::endl;
					return false;
				}
				j = currentLine.find('"');
				std::filesystem::path joinFilePath = currentLine.substr(j + 1, i - j - 1);
				joinFilePath = sourcePath / joinFilePath;
				j = currentLine.find('~');
				currentLine.erase(j, i - j + 1);
				if (!std::filesystem::exists(joinFilePath))
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
					std::cout << "The ~join Preprocessor Directive On This Line Provides A File Path That Does Not Exist" << std::endl;
					return false;
				}
				i = 0;
				s_JoinMap.emplace_back(fileNumber, 0);
				for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(sourcePath))
				{
					if (dirEntry.is_directory())
						goto dontChange;

					if (dirEntry.path().extension().string() != ".asm")
						goto dontChange;

					if (dirEntry.path() == joinFilePath)
						s_JoinMap.back().childFile = i;

					goto Change;

					dontChange: i--;
					Change: i++;
				}
				if (s_JoinMap.back().parentFile == s_JoinMap.back().childFile)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
					std::cout << "The ~join Preprocessor Directive On This Line Provides A File Path That Is The Same As The File It Was Found In" << std::endl;
					return false;
				}
			}
			else if (i == currentLine.find("align"))
			{
				i += 5;
				if (currentLine.size() < i + 1)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
					std::cout << "The ~align Preprocessor Directive On This Line Has Incorrect Syntax" << std::endl;
					return false;
				}
				while (i < currentLine.size())
				{
					if (currentLine[i] == ' ')
						i++;
					else
					{
						j = i;
						i = currentLine.size();
					}
				}
				if (j == 0)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
					std::cout << "The ~align Preprocessor Directive On This Line Does Not Have A Valid Alignment Number Associated With It" << std::endl;
					return false;
				}
				if (currentLine[j] != '"')
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
					std::cout << "The ~align Preprocessor Directive On This Line Does Not Have A Valid Alignment Number (In Inverted Commas) Associated With It" << std::endl;
					return false;
				}
				j++;
				while (j < currentLine.size())
				{
					if (currentLine[j] == '"')
					{
						i = j;
						j = currentLine.size();
					}
					j++;
				}
				if (j == currentLine.size())
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
					std::cout << "The ~align Preprocessor Directive On This Line Does Not Have A Valid Alignment Number (In Inverted Commas) Associated With It" << std::endl;
					return false;
				}
				j = currentLine.find('"');
				int alignmentNumber = StringToDecimalInt(currentLine.substr(j + 1, i - j - 1));
				if (!s_SuccessfulIntConversion)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
					std::cout << "The ~align Preprocessor Directive On This Line Does Not Have A Valid Alignment Number Associated With It" << std::endl;
					return false;
				}
				if (alignmentNumber < 0 || alignmentNumber > 255)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
					std::cout << "The ~align Preprocessor Directive On This Line Does Not Have A Valid Alignment Number Associated With It" << std::endl;
					return false;
				}
				uint64_t alignment = 1;
				while (alignment < 255)
				{
					if (alignment == alignmentNumber)
						s_CurrentByteSequenceAlignment = alignment;

					alignment <<= 1;
				}
				j = currentLine.find('~');
				currentLine.erase(j, i - j + 1);
			}
			else
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The Preprocessor Directive On This Line Is Not A Recognised Directive" << std::endl;
				return false;
			}
		}

		//5. Store Labels into a map
		//Use "i" as an index into "currentLine", Use "j" as the index of the ':' char
		i = 0;
		j = 0;
		for (; i < currentLine.size(); i++)
		{
			if (currentLine[i] == ':')
			{
				j = i;
				j++;
				i = currentLine.size();
			}
		}
		if (j != 0)
		{
			j--;
			for (i = 0; i < j; i++)
			{
				if (currentLine[i] == ' ')
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
					std::cout << "The Label On This Line Contains Spaces Which Is Not Valid" << std::endl;
					return false;
				}
			}
			mostRecentLabel = currentLine.substr(0, j);
			if (mostRecentLabel.empty())
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The Label On This Line Must Contain Printable Characters" << std::endl;
				return false;
			}
			for (i = 0; i < s_LabelMap.size(); i++)
			{
				if (s_LabelMap[i].label == mostRecentLabel)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
					std::cout << "The Label On This Line Is Already Defined" << std::endl;
					return false;
				}
			}
			s_LabelMap.push_back({ mostRecentLabel, fileNumber, currentInstructionNumber });
			j++;
			currentLine.erase(0, j);
			j = 0;
		}

		//6. Get Rid of Leading Spaces
		//Use "i" as an index into "currentLine", Use "j" as the index of the first char that is not a space
		for (i = 0; i < currentLine.size(); i++)
		{
			if (currentLine[i] != ' ')
			{
				j = i;
				i = currentLine.size();
			}
		}
		currentLine.erase(0, j);

		//If Line Is Empty, Ignore
		if (currentLine.size() == 0)
			continue;

		//7. Process The Byte Sequence If There Is One
		//Use "i" as an index into "currentLine", Use "j" as the current size of "currentLine" AND LATER Use "j" to know which hexadecimal digit you are on
		if (currentLine[0] == '{')
		{
			currentLine.erase(0, 1);
			i = 0;
			j = currentLine.size();
			while (i < j)
			{
				if (currentLine[i] == ' ')
				{
					currentLine.erase(i, 1);
					j--;
				}
				else
				{
					i++;
				}
			}

			if (currentLine.back() != '}')
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The Byte Sequence On This Line Does Not Have An Ending Curly Bracket" << std::endl;
				return false;
			}
			currentLine.pop_back();

			if ((currentLine.size() % 2) == 1)
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The Byte Sequence On This Line Has Half A Byte Missing" << std::endl;
				return false;
			}

			if (mostRecentLabel.empty())
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The Byte Sequence On This Line Does Not Have A Label To Identify It" << std::endl;
				return false;
			}

			for (i = 0; i < s_ByteSequenceMap.size(); i++)
			{
				if (s_ByteSequenceMap[i].label == mostRecentLabel)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
					std::cout << "The Label That Identifies The Byte Sequence On This Line Is Already Defined" << std::endl;
					return false;
				}
			}

			s_LabelMap.pop_back();
			s_ByteSequenceMap.push_back({ mostRecentLabel, std::vector<char>(), s_CurrentByteSequenceAlignment, 0 });

			i = 0;
			j = 0;
			for (; i < currentLine.size(); i++)
			{
				if (j == 0)
				{
					if (currentLine[i] >= '0' && currentLine[i] <= '9')
					{
						s_ByteSequenceMap.back().bytes.push_back((currentLine[i] - '0') << 4);
					}
					else if (currentLine[i] >= 'A' && currentLine[i] <= 'F')
					{
						s_ByteSequenceMap.back().bytes.push_back(((currentLine[i] - 'A') + 10) << 4);
					}
					else
					{
						inputStream.close();
						outputStream.close();
						std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
						std::cout << "The Byte Sequence On This Line Has An Invalid Hexadecimal Digit" << std::endl;
						return false;
					}

					j = 1;
				}
				else
				{
					if (currentLine[i] >= '0' && currentLine[i] <= '9')
					{
						s_ByteSequenceMap.back().bytes.back() += (currentLine[i] - '0');
					}
					else if (currentLine[i] >= 'A' && currentLine[i] <= 'F')
					{
						s_ByteSequenceMap.back().bytes.back() += ((currentLine[i] - 'A') + 10);
					}
					else
					{
						inputStream.close();
						outputStream.close();
						std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
						std::cout << "The Byte Sequence On This Line Has An Invalid Hexadecimal Digit" << std::endl;
						return false;
					}

					j = 0;
				}
			}

			continue;
		}

		//8. Process The Pointer Sequence If There Is One
		//Use "i" as an index into "currentLine", Use "j" as a way of knowing how to seperate the names of the pointers
		if (currentLine[0] == '[')
		{
			currentLine.erase(0, 1);

			i = currentLine.size() - 1;
			j = UINT64_MAX;

			for (; i < UINT64_MAX; i--)
			{
				if (currentLine[i] != ' ')
				{
					if (currentLine[i] == ']')
						j = i;

					i = 0;
				}
			}
			if (j == UINT64_MAX)
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The Pointer Sequence On This Line Does Not Have An Ending Square Bracket Or Has Invalid Syntax After The Ending Square Bracket" << std::endl;
				return false;
			}

			currentLine.erase(j, UINT64_MAX);

			if (mostRecentLabel.empty())
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The Pointer Sequence On This Line Does Not Have A Label To Identify It" << std::endl;
				return false;
			}

			for (i = 0; i < s_PtrSequenceMap.size(); i++)
			{
				if (s_PtrSequenceMap[i].label == mostRecentLabel)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
					std::cout << "The Label That Identifies The Pointer Sequence On This Line Is Already Defined" << std::endl;
					return false;
				}
			}

			s_LabelMap.pop_back();
			s_PtrSequenceMap.push_back({ mostRecentLabel, std::vector<std::string>(), std::vector<char>(), 0 });

			i = 0;
			j = 1;
			s_PtrSequenceMap.back().pointers.emplace_back();
			for (; i < currentLine.size(); i++)
			{
				if (currentLine[i] == ' ')
				{
					if (j == 0)
						s_PtrSequenceMap.back().pointers.emplace_back();

					j = 1;
				}
				else
				{
					s_PtrSequenceMap.back().pointers.back().push_back(currentLine[i]);
					j = 0;
				}
			}

			continue;
		}

		//9. Find The Instruction
		//Use "i" as an index into "currentLine", Use "j" as the index of the end of the first word
		i = 0;
		j = 0;
		for (; i < currentLine.size(); i++)
		{
			if (currentLine[i] == ' ' || i == currentLine.size() - 1)
			{
				if (currentLine[i] == ' ')
					j = i;
				else
					j = i + 1;
				i = currentLine.size();
			}
		}

		//10. Convert Instructions (except Branchs) Into Machine Code
		std::string instruction = currentLine.substr(0, j);
		i = 0;
		j = 0;
		if (instruction == "ADC")
		{
			currentLine.erase(0, 3);
			if (!ProcessADCInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The ADC Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "ADD")
		{
			currentLine.erase(0, 3);
			if (!ProcessADDInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The ADD Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "ADDHI")
		{
			currentLine.erase(0, 5);
			if (!ProcessADDHIInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The ADDHI Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "ADDSP")
		{
			currentLine.erase(0, 5);
			if (!ProcessADDSPInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The ADDSP Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "AND")
		{
			currentLine.erase(0, 3);
			if (!ProcessANDInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The AND Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "ASR")
		{
			currentLine.erase(0, 3);
			if (!ProcessASRInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The ASR Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "B")
		{
			currentLine.erase(0, 1);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_AL, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			for (; i < ASSEMBLER_INSTRUCTION_CHAR_SIZE * 2; i++)
				currentLine.pop_back();

			currentInstructionNumber += 7;
		}
		else if (instruction == "BEQ")
		{
			currentLine.erase(0, 3);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_EQ, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BNE")
		{
			currentLine.erase(0, 3);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_NE, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BCS" || instruction == "BHS")
		{
			currentLine.erase(0, 3);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_CS_HS, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BCC" || instruction == "BLO")
		{
			currentLine.erase(0, 3);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_CC_LO, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BMI")
		{
			currentLine.erase(0, 3);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_MI, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BPL")
		{
			currentLine.erase(0, 3);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_PL, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BVS")
		{
			currentLine.erase(0, 3);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_VS, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BVC")
		{
			currentLine.erase(0, 3);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_VC, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BHI")
		{
			currentLine.erase(0, 3);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_HI, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BLS")
		{
			currentLine.erase(0, 3);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_LS, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BGE")
		{
			currentLine.erase(0, 3);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_GE, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BLT")
		{
			currentLine.erase(0, 3);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_LT, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BGT")
		{
			currentLine.erase(0, 3);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_GT, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BLE")
		{
			currentLine.erase(0, 3);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_LE, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BAL")
		{
			currentLine.erase(0, 3);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_AL, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			for (; i < ASSEMBLER_INSTRUCTION_CHAR_SIZE * 2; i++)
				currentLine.pop_back();
			currentInstructionNumber += 7;
		}
		else if (instruction == "BNV")
		{
			inputStream.close();
			outputStream.close();
			std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
			std::cout << "This Line Contains A BNV Instruction Which Will Give Unpredictable Results" << std::endl;
			return false;
		}
		else if (instruction == "CALL")
		{
			currentLine.erase(0, 4);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_LINK, fileNumber, currentInstructionNumber, currentLine }, currentLine);
#ifdef ASSEMBLER_CONFIG_DEBUG
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION);
#endif
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);

#ifdef ASSEMBLER_CONFIG_DEBUG
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION);
#endif
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_PLACEHOLDER);

			currentInstructionNumber += 11;
		}
		else if (instruction == "RETURN")
		{
			currentLine.erase(0, 6);
			if (!ProcessRETURNInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The RETURN Instruction On This Line Should Not Have Any Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "BIC")
		{
			currentLine.erase(0, 3);
			if (!ProcessBICInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The BIC Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "CMN")
		{
			currentLine.erase(0, 3);
			if (!ProcessCMNInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The CMN Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "CMP")
		{
			currentLine.erase(0, 3);
			if (!ProcessCMPInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The CMP Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "XOR")
		{
			currentLine.erase(0, 3);
			if (!ProcessXORInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The XOR Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "LDMIA")
		{
			currentLine.erase(0, 5);
			if (!ProcessLDMIAInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The LDMIA Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "LDR")
		{
			currentLine.erase(0, 3);
			if (!ProcessLDRInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The LDR Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "LDRB")
		{
			currentLine.erase(0, 4);
			if (!ProcessLDRBInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The LDRB Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "LDRH")
		{
			currentLine.erase(0, 4);
			if (!ProcessLDRHInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The LDRH Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "LDRSB")
		{
			currentLine.erase(0, 5);
			if (!ProcessLDRSBInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The LDRSB Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "LDRSH")
		{
			currentLine.erase(0, 5);
			if (!ProcessLDRSHInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The LDRSH Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "LSL")
		{
			currentLine.erase(0, 3);
			if (!ProcessLSLInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The LSL Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "LSR")
		{
			currentLine.erase(0, 3);
			if (!ProcessLSRInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The LSR Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "MOV")
		{
			currentLine.erase(0, 3);
			if (!ProcessMOVInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The MOV Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "MOVA")
		{
			currentLine.erase(0, 4);
			if (!ProcessMOVAInstruction(currentLine, fileNumber, currentInstructionNumber))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The MOVA Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
			currentInstructionNumber += 6;
		}
		else if (instruction == "MUL")
		{
			currentLine.erase(0, 3);
			if (!ProcessMULInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The MUL Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "MVN")
		{
			currentLine.erase(0, 3);
			if (!ProcessMVNInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The MVN Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "NEG")
		{
			currentLine.erase(0, 3);
			if (!ProcessNEGInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The NEG Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "ORR")
		{
			currentLine.erase(0, 3);
			if (!ProcessORRInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The ORR Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "ROR")
		{
			currentLine.erase(0, 3);
			if (!ProcessRORInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The ROR Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "SBC")
		{
			currentLine.erase(0, 3);
			if (!ProcessSBCInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The SBC Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "STMIA")
		{
			currentLine.erase(0, 5);
			if (!ProcessSTMIAInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The STMIA Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "STR")
		{
			currentLine.erase(0, 3);
			if (!ProcessSTRInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The STR Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "STRB")
		{
			currentLine.erase(0, 4);
			if (!ProcessSTRBInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The STRB Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "STRH")
		{
			currentLine.erase(0, 4);
			if (!ProcessSTRHInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The STRH Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "SUB")
		{
			currentLine.erase(0, 3);
			if (!ProcessSUBInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The SUB Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "SUBSP")
		{
			currentLine.erase(0, 5);
			if (!ProcessSUBSPInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The SUBSP Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "SWI")
		{
			currentLine.erase(0, 3);
			if (!ProcessSWIInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The SWI Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "TST")
		{
			currentLine.erase(0, 3);
			if (!ProcessTSTInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The TST Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "NOP")
		{
			currentLine.erase(0, 3);
			if (!ProcessNOPInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
				std::cout << "The NOP Instruction On This Line Should Not Have Any Parameters" << std::endl;
				return false;
			}
		}
		else
		{
			inputStream.close();
			outputStream.close();
			std::cout << "Error on Line " << currentLineNumber << " in " << relativePath << std::endl;
			std::cout << "The Instruction On This Line Is Invalid" << std::endl;
			return false;
		}


		//11. Put Line In Output File
		outputStream.write(currentLine.c_str(), currentLine.size());
#ifdef ASSEMBLER_CONFIG_DEBUG
		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif
		currentInstructionNumber++;
	}

	inputStream.close();
	outputStream.close();
	std::cout << "Successfully PreProcessed " << relativePath << "..." << std::endl;
	return true;
}

static bool Assemble(const std::filesystem::path& sourcePath, const std::filesystem::path& intermediatePath)
{
	std::fstream inputStream;
	std::fstream outputStream;
	size_t indexOfEntryPoint;

	for (indexOfEntryPoint = 0; indexOfEntryPoint < s_LabelMap.size(); indexOfEntryPoint++)
	{
		if (s_LabelMap[indexOfEntryPoint].label == "ENTRY")
			break;
	}

	if (indexOfEntryPoint == s_LabelMap.size())
	{
		std::cout << "Error In Assembling..." << std::endl;
		std::cout << "The Entry Point is Not Defined, Add The Label ENTRY To Where You Want The Program To Start" << std::endl;
		return false;
	}

	//Join All The Files That Need To Be Joined
	for (size_t i = 0; i < s_JoinMap.size(); i++)
	{
		std::filesystem::path parentFile = intermediatePath;
		std::filesystem::path childFile = intermediatePath;
		uint64_t originalAmountOfParentFileInstructions = 0;
		uint64_t fileCounter = 0;
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(intermediatePath))
		{
			if (dirEntry.is_directory())
				continue;

			if (s_JoinMap[i].parentFile == fileCounter)
			{
				parentFile = dirEntry.path();
				originalAmountOfParentFileInstructions = dirEntry.file_size() / ASSEMBLER_INSTRUCTION_CHAR_SIZE;
				if (childFile != intermediatePath)
					break;
			}
			else if (s_JoinMap[i].childFile == fileCounter)
			{
				childFile = dirEntry.path();
				if (parentFile != intermediatePath)
					break;
			}

			fileCounter++;
		}

		inputStream.open(childFile, std::ios::in | std::ios::binary);
		outputStream.open(parentFile, std::ios::out | std::ios::binary | std::ios::app);

		char nextByte;
		while (inputStream.read(&nextByte, 1))
			outputStream.write(&nextByte, 1);

		inputStream.close();
		outputStream.close();

		std::filesystem::remove(childFile);

		//Fix The Label Map
		for (size_t l = 0; l < s_LabelMap.size(); l++)
		{
			if (s_LabelMap[l].fileNumber == s_JoinMap[i].childFile)
			{
				s_LabelMap[l].fileNumber = s_JoinMap[i].parentFile;
				s_LabelMap[l].instructionNumber += originalAmountOfParentFileInstructions;
			}
			if (s_LabelMap[l].fileNumber > s_JoinMap[i].childFile)
				s_LabelMap[l].fileNumber--;
		}

		//Fix The Branch Map
		for (size_t b = 0; b < s_BranchMap.size(); b++)
		{
			if (s_BranchMap[b].fileNumber == s_JoinMap[i].childFile)
			{
				s_BranchMap[b].fileNumber = s_JoinMap[i].parentFile;
				s_BranchMap[b].InstructionNumber += originalAmountOfParentFileInstructions;
			}
			if (s_BranchMap[b].fileNumber > s_JoinMap[i].childFile)
				s_BranchMap[b].fileNumber--;
		}

		//Fix The MovAddress Map
		for (size_t m = 0; m < s_MovAddressMap.size(); m++)
		{
			if (s_MovAddressMap[m].fileNumber == s_JoinMap[i].childFile)
			{
				s_MovAddressMap[m].fileNumber = s_JoinMap[i].parentFile;
				s_MovAddressMap[m].InstructionNumber += originalAmountOfParentFileInstructions;
			}
			if (s_MovAddressMap[m].fileNumber > s_JoinMap[i].childFile)
				s_MovAddressMap[m].fileNumber--;
		}

		//Fix The Join Map
		for (size_t j = i + 1; j < s_JoinMap.size(); j++)
		{
			if (s_JoinMap[j].parentFile > s_JoinMap[i].childFile)
				s_JoinMap[j].parentFile--;
			if (s_JoinMap[j].childFile > s_JoinMap[i].childFile)
				s_JoinMap[j].childFile--;
		}
	}

	//Combine All Files Into One
	std::filesystem::path combinedFilePath;
	uint64_t fileCounter = 0;
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(intermediatePath))
	{
		if (dirEntry.is_directory())
			continue;

		if (fileCounter == 0)
			combinedFilePath = dirEntry.path();
		else
		{
			size_t originalAmountOfParentFileInstructions = std::filesystem::file_size(combinedFilePath) / ASSEMBLER_INSTRUCTION_CHAR_SIZE;

			inputStream.open(dirEntry.path(), std::ios::in | std::ios::binary);
			outputStream.open(combinedFilePath, std::ios::out | std::ios::binary | std::ios::app);

			char nextByte;
			while (inputStream.read(&nextByte, 1))
				outputStream.write(&nextByte, 1);

			inputStream.close();
			outputStream.close();

			//Fix The Label Map
			for (size_t l = 0; l < s_LabelMap.size(); l++)
			{
				if (s_LabelMap[l].fileNumber == fileCounter)
				{
					s_LabelMap[l].fileNumber = 0;
					s_LabelMap[l].instructionNumber += originalAmountOfParentFileInstructions;
				}
			}

			//Fix The Branch Map
			for (size_t b = 0; b < s_BranchMap.size(); b++)
			{
				if (s_BranchMap[b].fileNumber == fileCounter)
				{
					s_BranchMap[b].fileNumber = 0;
					s_BranchMap[b].InstructionNumber += originalAmountOfParentFileInstructions;
				}
			}

			//Fix The MovAddress Map
			for (size_t m = 0; m < s_MovAddressMap.size(); m++)
			{
				if (s_MovAddressMap[m].fileNumber == fileCounter)
				{
					s_MovAddressMap[m].fileNumber = 0;
					s_MovAddressMap[m].InstructionNumber += originalAmountOfParentFileInstructions;
				}
			}
		}

		fileCounter++;
	}


	//Evaluate Branchs
	outputStream.open(combinedFilePath, std::ios::in | std::ios::out | std::ios::binary);
	for (size_t i = 0; i < s_BranchMap.size(); i++)
	{
		size_t l;
		for (l = 0; l < s_LabelMap.size(); l++)
		{
			if (s_LabelMap[l].label == s_BranchMap[i].label)
				break;
		}
		if (l == s_LabelMap.size())
		{
			size_t j;
			for (j = 0; j < s_ByteSequenceMap.size(); j++)
			{
				if (s_ByteSequenceMap[j].label == s_BranchMap[i].label)
				{
					std::cout << "Error In Assembling..." << std::endl;
					std::cout << "The Label " << s_BranchMap[i].label << " Is A Byte Sequence, Which Can Not Be Branched To" << std::endl;
					j = s_ByteSequenceMap.size() + 1;
				}
			}
			for (j = 0; j < s_PtrSequenceMap.size(); j++)
			{
				if (s_PtrSequenceMap[j].label == s_BranchMap[i].label)
				{
					std::cout << "Error In Assembling..." << std::endl;
					std::cout << "The Label " << s_BranchMap[i].label << " Is A Pointer Sequence, Which Can Not Be Branched To" << std::endl;
					j = s_PtrSequenceMap.size() + 1;
				}
			}
			if (j == s_ByteSequenceMap.size())
			{
				std::cout << "Error In Assembling..." << std::endl;
				std::cout << "The Label " << s_BranchMap[i].label << " Is Not Defined" << std::endl;
			}

			return false;
		}

		uint64_t pointer = s_LabelMap[l].instructionNumber;
		pointer *= 2;
		pointer += 0x08000000;
		pointer += 192;
		pointer += 28;
		pointer &= (UINT32_MAX - 1);
		pointer++;

		outputStream.seekp(ASSEMBLER_INSTRUCTION_CHAR_SIZE * s_BranchMap[i].InstructionNumber);
		if (s_BranchMap[i].type != ASSEMBLER_BRANCH_AL)
		{
			if (s_BranchMap[i].type == ASSEMBLER_BRANCH_LINK)
			{
				//Link

				char linkingBranchSymbols[16];

				linkingBranchSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[8] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[10] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[11] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[12] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[13] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[14] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[15] = ASSEMBLER_OUTPUT_SYMBOL_0;
				outputStream.write(linkingBranchSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
				outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif

				linkingBranchSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[8] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[10] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[11] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[12] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[13] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[14] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[15] = ASSEMBLER_OUTPUT_SYMBOL_1;
				outputStream.write(linkingBranchSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
				outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif

				linkingBranchSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[8] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[10] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[11] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[12] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[13] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[14] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[15] = ASSEMBLER_OUTPUT_SYMBOL_0;
				outputStream.write(linkingBranchSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
				outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif

				linkingBranchSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[8] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[10] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[11] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[12] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[13] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[14] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[15] = ASSEMBLER_OUTPUT_SYMBOL_0;
				outputStream.write(linkingBranchSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
				outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif
			}
			else
			{
				//Find out the Condition

				char conditionalBranchSymbols[16];

				conditionalBranchSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_1;
				conditionalBranchSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_1;
				conditionalBranchSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_1;
				if (s_BranchMap[i].type & 0b1000)
					conditionalBranchSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_1;
				else
					conditionalBranchSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;

				if (s_BranchMap[i].type & 0b0100)
					conditionalBranchSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_1;
				else
					conditionalBranchSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_0;

				if (s_BranchMap[i].type & 0b0010)
					conditionalBranchSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
				else
					conditionalBranchSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_0;

				if (s_BranchMap[i].type & 0b0001)
					conditionalBranchSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_1;
				else
					conditionalBranchSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_0;

				conditionalBranchSymbols[8] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[10] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[11] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[12] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[13] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[14] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[15] = ASSEMBLER_OUTPUT_SYMBOL_0;
				outputStream.write(conditionalBranchSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
				outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif

				conditionalBranchSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_1;
				conditionalBranchSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_1;
				conditionalBranchSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_1;
				conditionalBranchSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[8] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[10] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[11] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[12] = ASSEMBLER_OUTPUT_SYMBOL_0;
				conditionalBranchSymbols[13] = ASSEMBLER_OUTPUT_SYMBOL_1;
				conditionalBranchSymbols[14] = ASSEMBLER_OUTPUT_SYMBOL_1;
				conditionalBranchSymbols[15] = ASSEMBLER_OUTPUT_SYMBOL_1;
				outputStream.write(conditionalBranchSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
				outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif
			}
		}

		/*
		[Byte1, Byte2, Byte3, Byte4] = address+1

		MOV R7, Byte1
		LSL R7, R7, #8
		ADD R7, Byte2
		LSL R7, R7, #8
		ADD R7, Byte3
		LSL R7, R7, #8
		ADD R7, Byte4
		BX R7
		
		00100111
		Byte1
		00000010
		00111111
		00110111
		Byte2
		00000010
		00111111
		00110111
		Byte3
		00000010
		00111111
		00110111
		Byte4
		01000111
		00111000
		*/

		char newSymbols[16];

		//MOV R7, Byte1
		newSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_1;
		for (size_t i = 8; i < 16; i++)
		{
			if (pointer & (0x80000000 >> (i - 8)))
				newSymbols[i] = ASSEMBLER_OUTPUT_SYMBOL_1;
			else
				newSymbols[i] = ASSEMBLER_OUTPUT_SYMBOL_0;
		}
		outputStream.write(newSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif

		//LSL R7, R7, #8
		newSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[8] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[10] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[11] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[12] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[13] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[14] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[15] = ASSEMBLER_OUTPUT_SYMBOL_1;
		outputStream.write(newSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif

		//ADD R7, Byte2
		newSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_1;
		for (size_t i = 8; i < 16; i++)
		{
			if (pointer & (0x00800000 >> (i - 8)))
				newSymbols[i] = ASSEMBLER_OUTPUT_SYMBOL_1;
			else
				newSymbols[i] = ASSEMBLER_OUTPUT_SYMBOL_0;
		}
		outputStream.write(newSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif

		//LSL R7, R7, #8
		newSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[8] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[10] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[11] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[12] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[13] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[14] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[15] = ASSEMBLER_OUTPUT_SYMBOL_1;
		outputStream.write(newSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif

		//ADD R7, Byte3
		newSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_1;
		for (size_t i = 8; i < 16; i++)
		{
			if (pointer & (0x00008000 >> (i - 8)))
				newSymbols[i] = ASSEMBLER_OUTPUT_SYMBOL_1;
			else
				newSymbols[i] = ASSEMBLER_OUTPUT_SYMBOL_0;
		}
		outputStream.write(newSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif

		//LSL R7, R7, #8
		newSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[8] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[10] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[11] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[12] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[13] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[14] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[15] = ASSEMBLER_OUTPUT_SYMBOL_1;
		outputStream.write(newSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif

		//ADD R7, Byte4
		newSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_1;
		for (size_t i = 8; i < 16; i++)
		{
			if (pointer & (0x00000080 >> (i - 8)))
				newSymbols[i] = ASSEMBLER_OUTPUT_SYMBOL_1;
			else
				newSymbols[i] = ASSEMBLER_OUTPUT_SYMBOL_0;
		}
		outputStream.write(newSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif

		//BX R7
		newSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[8] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[10] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[11] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[12] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[13] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[14] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[15] = ASSEMBLER_OUTPUT_SYMBOL_0;
		outputStream.write(newSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif
	}
	outputStream.close();

	//Evaluate Byte Sequence Offsets
	size_t numberOfBytes = std::filesystem::file_size(combinedFilePath);
	numberOfBytes /= ASSEMBLER_INSTRUCTION_CHAR_SIZE;
	numberOfBytes *= 2;

	for (size_t i = 0; i < s_ByteSequenceMap.size(); i++)
	{
		s_ByteSequenceMap[i].address = 0x08000000;
		s_ByteSequenceMap[i].address += 192;
		s_ByteSequenceMap[i].address += 28;
		s_ByteSequenceMap[i].address += numberOfBytes;
		uint64_t amountLeadingZeros = s_ByteSequenceMap[i].address % s_ByteSequenceMap[i].alignment;
		amountLeadingZeros = s_ByteSequenceMap[i].alignment - amountLeadingZeros;
		if (amountLeadingZeros == s_ByteSequenceMap[i].alignment)
			amountLeadingZeros = 0;
		for (size_t j = 0; j < amountLeadingZeros; j++)
			s_ByteSequenceMap[i].bytes.insert(s_ByteSequenceMap[i].bytes.begin(), 0);
		s_ByteSequenceMap[i].address += amountLeadingZeros;
		numberOfBytes += s_ByteSequenceMap[i].bytes.size();
	}

	//Evaluate Ptr Sequence Pointers & Offsets
	for (size_t i = 0; i < s_PtrSequenceMap.size(); i++)
	{
		for (size_t k = 0; k < s_PtrSequenceMap[i].pointers.size(); k++)
		{
			for (size_t b = 0; b < s_ByteSequenceMap.size(); b++)
			{
				if (s_PtrSequenceMap[i].pointers[k] == s_ByteSequenceMap[b].label)
				{
					s_PtrSequenceMap[i].bytes.push_back((char)(s_ByteSequenceMap[b].address & 0x000000FF));
					s_PtrSequenceMap[i].bytes.push_back((char)((s_ByteSequenceMap[b].address & 0x0000FF00) >> 8));
					s_PtrSequenceMap[i].bytes.push_back((char)((s_ByteSequenceMap[b].address & 0x00FF0000) >> 16));
					s_PtrSequenceMap[i].bytes.push_back((char)((s_ByteSequenceMap[b].address & 0xFF000000) >> 24));
					b = s_ByteSequenceMap.size();
				}
			}
			for (size_t l = 0; l < s_LabelMap.size(); l++)
			{
				if (s_PtrSequenceMap[i].pointers[k] == s_LabelMap[l].label)
				{
					std::cout << "Error In Assembling..." << std::endl;
					std::cout << "The Label " << s_LabelMap[i].label << " Is An Instruction Label, Which Can Not Be Converted To A Pointer" << std::endl;
					return false;
				}
			}
		}

		s_PtrSequenceMap[i].address = 0x08000000;
		s_PtrSequenceMap[i].address += 192;
		s_PtrSequenceMap[i].address += 28;
		s_PtrSequenceMap[i].address += numberOfBytes;
		uint64_t amountLeadingZeros = s_PtrSequenceMap[i].address % 4;
		amountLeadingZeros = 4 - amountLeadingZeros;
		if (amountLeadingZeros == 4)
			amountLeadingZeros = 0;
		for (size_t j = 0; j < amountLeadingZeros; j++)
			s_PtrSequenceMap[i].bytes.insert(s_PtrSequenceMap[i].bytes.begin(), 0);
		s_PtrSequenceMap[i].address += amountLeadingZeros;
		numberOfBytes += s_PtrSequenceMap[i].bytes.size();
	}

	//Translate The MOV Address Instructions
	outputStream.open(combinedFilePath, std::ios::in | std::ios::out | std::ios::binary);
	for (size_t i = 0; i < s_MovAddressMap.size(); i++)
	{
		size_t j = 0;
		uint64_t address = UINT64_MAX;
		for (; j < s_LabelMap.size(); j++)
		{
			if (s_MovAddressMap[i].label.substr(0, 8) == "MEM_BIOS")
			{
				address = ASSEMBLER_MEM_BIOS;
				if (s_MovAddressMap[i].label.size() >= 9)
				{
					int index = StringToDecimalInt(s_MovAddressMap[i].label.substr(8));
					if (!s_SuccessfulIntConversion || index < 0)
					{
						std::cout << "Error In Assembling..." << std::endl;
						std::cout << "The Label " << s_MovAddressMap[i].label << " Is Not Defined" << std::endl;
						return false;
					}
					address += index;
				}
				break;
			}
			if (s_MovAddressMap[i].label.substr(0, 8) == "MEM_ERAM")
			{
				address = ASSEMBLER_MEM_ERAM;
				if (s_MovAddressMap[i].label.size() >= 9)
				{
					int index = StringToDecimalInt(s_MovAddressMap[i].label.substr(8));
					if (!s_SuccessfulIntConversion || index < 0)
					{
						std::cout << "Error In Assembling..." << std::endl;
						std::cout << "The Label " << s_MovAddressMap[i].label << " Is Not Defined" << std::endl;
						return false;
					}
					address += index;
				}
				break;
			}
			if (s_MovAddressMap[i].label.substr(0, 8) == "MEM_IRAM")
			{
				address = ASSEMBLER_MEM_IRAM;
				if (s_MovAddressMap[i].label.size() >= 9)
				{
					int index = StringToDecimalInt(s_MovAddressMap[i].label.substr(8));
					if (!s_SuccessfulIntConversion || index < 0)
					{
						std::cout << "Error In Assembling..." << std::endl;
						std::cout << "The Label " << s_MovAddressMap[i].label << " Is Not Defined" << std::endl;
						return false;
					}
					address += index;
				}
				break;
			}
			if (s_MovAddressMap[i].label.substr(0, 6) == "MEM_IO")
			{
				address = ASSEMBLER_MEM_IO;
				if (s_MovAddressMap[i].label.size() >= 7)
				{
					int index = StringToDecimalInt(s_MovAddressMap[i].label.substr(6));
					if (!s_SuccessfulIntConversion || index < 0)
					{
						std::cout << "Error In Assembling..." << std::endl;
						std::cout << "The Label " << s_MovAddressMap[i].label << " Is Not Defined" << std::endl;
						return false;
					}
					address += index;
				}
				break;
			}
			if (s_MovAddressMap[i].label.substr(0, 11) == "MEM_PALETTE")
			{
				address = ASSEMBLER_MEM_PALETTE;
				if (s_MovAddressMap[i].label.size() >= 12)
				{
					int index = StringToDecimalInt(s_MovAddressMap[i].label.substr(11));
					if (!s_SuccessfulIntConversion || index < 0)
					{
						std::cout << "Error In Assembling..." << std::endl;
						std::cout << "The Label " << s_MovAddressMap[i].label << " Is Not Defined" << std::endl;
						return false;
					}
					address += index;
				}
				break;
			}
			if (s_MovAddressMap[i].label.substr(0, 12) == "MEM_OPALETTE")
			{
				address = ASSEMBLER_MEM_PALETTE + 512;
				if (s_MovAddressMap[i].label.size() >= 13)
				{
					int index = StringToDecimalInt(s_MovAddressMap[i].label.substr(12));
					if (!s_SuccessfulIntConversion || index < 0)
					{
						std::cout << "Error In Assembling..." << std::endl;
						std::cout << "The Label " << s_MovAddressMap[i].label << " Is Not Defined" << std::endl;
						return false;
					}
					address += index;
				}
				break;
			}
			if (s_MovAddressMap[i].label.substr(0, 8) == "MEM_VRAM")
			{
				address = ASSEMBLER_MEM_VRAM;
				if (s_MovAddressMap[i].label.size() >= 9)
				{
					int index = StringToDecimalInt(s_MovAddressMap[i].label.substr(8));
					if (!s_SuccessfulIntConversion || index < 0)
					{
						std::cout << "Error In Assembling..." << std::endl;
						std::cout << "The Label " << s_MovAddressMap[i].label << " Is Not Defined" << std::endl;
						return false;
					}
					address += index;
				}
				break;
			}
			if (s_MovAddressMap[i].label.substr(0, 9) == "MEM_OVRAM")
			{
				address = ASSEMBLER_MEM_VRAM + 65536;
				if (s_MovAddressMap[i].label.size() >= 10)
				{
					int index = StringToDecimalInt(s_MovAddressMap[i].label.substr(9));
					if (!s_SuccessfulIntConversion || index < 0)
					{
						std::cout << "Error In Assembling..." << std::endl;
						std::cout << "The Label " << s_MovAddressMap[i].label << " Is Not Defined" << std::endl;
						return false;
					}
					address += index;
				}
				break;
			}
			if (s_MovAddressMap[i].label.substr(0, 7) == "MEM_OAM")
			{
				address = ASSEMBLER_MEM_OAM;
				if (s_MovAddressMap[i].label.size() >= 8)
				{
					int index = StringToDecimalInt(s_MovAddressMap[i].label.substr(7));
					if (!s_SuccessfulIntConversion || index < 0)
					{
						std::cout << "Error In Assembling..." << std::endl;
						std::cout << "The Label " << s_MovAddressMap[i].label << " Is Not Defined" << std::endl;
						return false;
					}
					address += index;
				}
				break;
			}
			if (s_MovAddressMap[i].label.substr(0, 7) == "MEM_ROM")
			{
				address = ASSEMBLER_MEM_ROM;
				if (s_MovAddressMap[i].label.size() >= 8)
				{
					int index = StringToDecimalInt(s_MovAddressMap[i].label.substr(7));
					if (!s_SuccessfulIntConversion || index < 0)
					{
						std::cout << "Error In Assembling..." << std::endl;
						std::cout << "The Label " << s_MovAddressMap[i].label << " Is Not Defined" << std::endl;
						return false;
					}
					address += index;
				}
				break;
			}

			if (s_MovAddressMap[i].label == s_LabelMap[j].label)
			{
				address = s_LabelMap[j].instructionNumber;
				address *= 2;
				address += 0x08000000;
				address += 192;
				address += 28;
				break;
			}
		}
		if (address == UINT64_MAX)
		{
			for (j = 0; j < s_ByteSequenceMap.size(); j++)
			{
				if (s_MovAddressMap[i].label == s_ByteSequenceMap[j].label)
				{
					address = s_ByteSequenceMap[j].address;
					break;
				}
			}
			if (address == UINT64_MAX)
			{
				for (j = 0; j < s_PtrSequenceMap.size(); j++)
				{
					if (s_MovAddressMap[i].label == s_PtrSequenceMap[j].label)
					{
						address = s_PtrSequenceMap[j].address;
						break;
					}
				}
				if (address == UINT64_MAX)
				{
					std::cout << "Error In Assembling..." << std::endl;
					std::cout << "The Label " << s_MovAddressMap[i].label << " Is Not Defined" << std::endl;
					return false;
				}
			}
		}
		outputStream.seekp(s_MovAddressMap[i].InstructionNumber * ASSEMBLER_INSTRUCTION_CHAR_SIZE);

		char destinationRegisterSymbols[3];
		char newSymbols[16];

		if (s_MovAddressMap[i].destinationRegister & 4)
			destinationRegisterSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_1;
		else
			destinationRegisterSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_0;
		if (s_MovAddressMap[i].destinationRegister & 2)
			destinationRegisterSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_1;
		else
			destinationRegisterSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
		if (s_MovAddressMap[i].destinationRegister & 1)
			destinationRegisterSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_1;
		else
			destinationRegisterSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_0;

		//MOV Rd, Byte1
		newSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[5] = destinationRegisterSymbols[0];
		newSymbols[6] = destinationRegisterSymbols[1];
		newSymbols[7] = destinationRegisterSymbols[2];
		for (size_t k = 8; k < 16; k++)
		{
			if (address & (0x80000000 >> (k - 8)))
				newSymbols[k] = ASSEMBLER_OUTPUT_SYMBOL_1;
			else
				newSymbols[k] = ASSEMBLER_OUTPUT_SYMBOL_0;
		}
		outputStream.write(newSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif

		//LSL Rd, Rd, #8
		newSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[8] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[10] = destinationRegisterSymbols[0];
		newSymbols[11] = destinationRegisterSymbols[1];
		newSymbols[12] = destinationRegisterSymbols[2];
		newSymbols[13] = destinationRegisterSymbols[0];
		newSymbols[14] = destinationRegisterSymbols[1];
		newSymbols[15] = destinationRegisterSymbols[2];
		outputStream.write(newSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif

		//ADD Rd, Byte2
		newSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[5] = destinationRegisterSymbols[0];
		newSymbols[6] = destinationRegisterSymbols[1];
		newSymbols[7] = destinationRegisterSymbols[2];
		for (size_t k = 8; k < 16; k++)
		{
			if (address & (0x00800000 >> (k - 8)))
				newSymbols[k] = ASSEMBLER_OUTPUT_SYMBOL_1;
			else
				newSymbols[k] = ASSEMBLER_OUTPUT_SYMBOL_0;
		}
		outputStream.write(newSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif

		//LSL Rd, Rd, #8
		newSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[8] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[10] = destinationRegisterSymbols[0];
		newSymbols[11] = destinationRegisterSymbols[1];
		newSymbols[12] = destinationRegisterSymbols[2];
		newSymbols[13] = destinationRegisterSymbols[0];
		newSymbols[14] = destinationRegisterSymbols[1];
		newSymbols[15] = destinationRegisterSymbols[2];
		outputStream.write(newSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif

		//ADD Rd, Byte3
		newSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[5] = destinationRegisterSymbols[0];
		newSymbols[6] = destinationRegisterSymbols[1];
		newSymbols[7] = destinationRegisterSymbols[2];
		for (size_t k = 8; k < 16; k++)
		{
			if (address & (0x00008000 >> (k - 8)))
				newSymbols[k] = ASSEMBLER_OUTPUT_SYMBOL_1;
			else
				newSymbols[k] = ASSEMBLER_OUTPUT_SYMBOL_0;
		}
		outputStream.write(newSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif

		//LSL Rd, Rd, #8
		newSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[5] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[8] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[9] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[10] = destinationRegisterSymbols[0];
		newSymbols[11] = destinationRegisterSymbols[1];
		newSymbols[12] = destinationRegisterSymbols[2];
		newSymbols[13] = destinationRegisterSymbols[0];
		newSymbols[14] = destinationRegisterSymbols[1];
		newSymbols[15] = destinationRegisterSymbols[2];
		outputStream.write(newSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif

		//ADD Rd, Byte4
		newSymbols[0] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[1] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[2] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[3] = ASSEMBLER_OUTPUT_SYMBOL_1;
		newSymbols[4] = ASSEMBLER_OUTPUT_SYMBOL_0;
		newSymbols[5] = destinationRegisterSymbols[0];
		newSymbols[6] = destinationRegisterSymbols[1];
		newSymbols[7] = destinationRegisterSymbols[2];
		for (size_t k = 8; k < 16; k++)
		{
			if (address & (0x00000080 >> (k - 8)))
				newSymbols[k] = ASSEMBLER_OUTPUT_SYMBOL_1;
			else
				newSymbols[k] = ASSEMBLER_OUTPUT_SYMBOL_0;
		}
		outputStream.write(newSymbols, 16);

#ifdef ASSEMBLER_CONFIG_DEBUG
		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
#endif
		
	}
	outputStream.close();


	//Assemble
	std::filesystem::path assembledBinaryPath = sourcePath / "MyGame.gba";
	outputStream.open(assembledBinaryPath, std::ios::out | std::ios::binary);

	for (size_t i = 0; i < 192; i++)
		outputStream << '\xff';

	uint64_t addressOfEntryPoint = s_LabelMap[indexOfEntryPoint].instructionNumber;
	addressOfEntryPoint *= 2;
	addressOfEntryPoint += 0x08000000;
	addressOfEntryPoint += 192;
	addressOfEntryPoint += 28;
	addressOfEntryPoint &= (UINT32_MAX - 1);
	addressOfEntryPoint++;

	char startInstructions[4];

	//MOV R12, #0x08000000
	startInstructions[0] = (char)0b00000010;
	startInstructions[1] = (char)0b11000011;
	startInstructions[2] = (char)0b10100000;
	startInstructions[3] = (char)0b11100011;
	outputStream.write(startInstructions, 4);

	//ADD R12, R12, #205
	startInstructions[0] = (char)0b11001101;
	startInstructions[1] = (char)0b11000000;
	startInstructions[2] = (char)0b10001100;
	startInstructions[3] = (char)0b11100010;
	outputStream.write(startInstructions, 4);

	//BX R12
	startInstructions[0] = (char)0b00011100;
	startInstructions[1] = (char)0b11111111;
	startInstructions[2] = (char)0b00101111;
	startInstructions[3] = (char)0b11100001;
	outputStream.write(startInstructions, 4);

	//MOV R7, Byte1 & LSL R7, R7, #8
	startInstructions[0] = (char)((addressOfEntryPoint & 0xFF000000) >> 24);
	startInstructions[1] = (char)0b00100111;
	startInstructions[2] = (char)0b00111111;
	startInstructions[3] = (char)0b00000010;
	outputStream.write(startInstructions, 4);

	//ADD R7, Byte2 & LSL R7, R7, #8
	startInstructions[0] = (char)((addressOfEntryPoint & 0x00FF0000) >> 16);
	startInstructions[1] = (char)0b00110111;
	startInstructions[2] = (char)0b00111111;
	startInstructions[3] = (char)0b00000010;
	outputStream.write(startInstructions, 4);

	//ADD R7, Byte3 & LSL R7, R7, #8
	startInstructions[0] = (char)((addressOfEntryPoint & 0x0000FF00) >> 8);
	startInstructions[1] = (char)0b00110111;
	startInstructions[2] = (char)0b00111111;
	startInstructions[3] = (char)0b00000010;
	outputStream.write(startInstructions, 4);

	//ADD R7, Byte4 & BX R7
	startInstructions[0] = (char)(addressOfEntryPoint & 0x000000FF);
	startInstructions[1] = (char)0b00110111;
	startInstructions[2] = (char)0b00111000;
	startInstructions[3] = (char)0b01000111;
	outputStream.write(startInstructions, 4);

	inputStream.open(combinedFilePath, std::ios::in | std::ios::binary);
	char nextInstruction[ASSEMBLER_INSTRUCTION_CHAR_SIZE];
	char instructionByte1 = 0;
	char instructionByte2 = 0;
	while (inputStream.read(nextInstruction, ASSEMBLER_INSTRUCTION_CHAR_SIZE))
	{
		size_t i = 0;
		for (; i < 8; i++)
		{
			if (nextInstruction[i] == ASSEMBLER_OUTPUT_SYMBOL_1)
				instructionByte1 += (1 << (7 - i));
			else if (nextInstruction[i] != ASSEMBLER_OUTPUT_SYMBOL_0)
			{
				std::cout << "Error In Assembling..." << std::endl;
				std::cout << "An Instruction Was Not Recorded Correctly" << std::endl;
				return false;
			}
		}

		for (; i < 16; i++)
		{
			if (nextInstruction[i] == ASSEMBLER_OUTPUT_SYMBOL_1)
				instructionByte2 += (1 << (15 - i));
			else if (nextInstruction[i] != ASSEMBLER_OUTPUT_SYMBOL_0)
			{
				std::cout << "Error In Assembling..." << std::endl;
				std::cout << "An Instruction Was Not Recorded Correctly" << std::endl;
				return false;
			}
		}

#ifdef ASSEMBLER_CONFIG_DEBUG
		if (nextInstruction[i] != '\n')
		{
			std::cout << "Error In Assembling..." << std::endl;
			std::cout << "An Instruction Was Not Recorded Correctly" << std::endl;
			return false;
		}
#endif

		outputStream.write(&instructionByte2, 1);
		outputStream.write(&instructionByte1, 1);

		instructionByte1 = 0;
		instructionByte2 = 0;
	}
	inputStream.close();

	for (size_t i = 0; i < s_ByteSequenceMap.size(); i++)
		outputStream.write(s_ByteSequenceMap[i].bytes.data(), s_ByteSequenceMap[i].bytes.size());

	for (size_t i = 0; i < s_PtrSequenceMap.size(); i++)
		outputStream.write(s_PtrSequenceMap[i].bytes.data(), s_PtrSequenceMap[i].bytes.size());

	outputStream.close();

	//Create Header
	outputStream.open(assembledBinaryPath, std::ios::in | std::ios::out | std::ios::binary);

	outputStream << '\x2E' << '\x00' << '\x00' << '\xEA';

	outputStream.close();


	std::cout << "Successfully Assembled " << "MyGame.gba" << std::endl;
	return true;
}

int main(int argc, char** argv)
{
#ifdef ASSEMBLER_CONFIG_DEBUG
	std::filesystem::path sourcePath = "asmsrc";
#endif

#ifdef ASSEMBLER_CONFIG_RELEASE
	if (argc <= 1)
	{
		std::cout << "GBA_Assembler can only be called from the command line!" << std::endl;
		std::cout << "Press Enter to close this application...";
		std::cin.get();
		return 0;
	}

	if (argc >= 3)
	{
		std::cout << "GBA_Assembler only takes one arguement and that is the folder where all the source files are kept!" << std::endl;
		return 0;
	}

	std::filesystem::path sourcePath = argv[1];
#endif

#ifdef ASSEMBLER_CONFIG_DEBUG
	std::filesystem::path intermediatePath = sourcePath / "AssemblerInt";
	std::filesystem::remove_all(intermediatePath);
#endif

#ifdef ASSEMBLER_CONFIG_RELEASE
	std::filesystem::path intermediatePath = sourcePath;
	while (std::filesystem::exists(intermediatePath))
		intermediatePath /= "0";
#endif

	std::filesystem::create_directory(intermediatePath);
	size_t currentFileNumber = 0;
	bool preprocessedAll = false;
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(sourcePath))
	{
		if (dirEntry.is_directory())
			continue;

		std::filesystem::path relativePath = std::filesystem::relative(dirEntry.path(), sourcePath);
		if (dirEntry.path().extension().string() == ".asm")
		{
			std::cout << "PreProcessing " << relativePath << "..." << std::endl;
			preprocessedAll = PreProcess(sourcePath, dirEntry.path(), currentFileNumber, intermediatePath);
			currentFileNumber++;
			if (!preprocessedAll)
				break;
		}
		else
		{
			std::cout << "Ignoring " << relativePath << " because it is not an assembly file..." << std::endl;
		}
	}
	if (preprocessedAll)
	{
		std::cout << "Assembling..." << std::endl;
		if (Assemble(sourcePath, intermediatePath))
		{
			size_t binaryFileSize = std::filesystem::file_size(sourcePath / "MyGame.gba");
			size_t appendedFileSize = 1;
			while (binaryFileSize > appendedFileSize)
				appendedFileSize <<= 1;

			appendedFileSize -= binaryFileSize;
			std::fstream outputStream;
			outputStream.open(sourcePath / "MyGame.gba", std::ios::out | std::ios::binary | std::ios::app);
			for (size_t i = 0; i < appendedFileSize; i++)
				outputStream << '\xFF';
			outputStream.close();
		}
	}
#ifdef ASSEMBLER_CONFIG_RELEASE
	std::filesystem::remove_all(intermediatePath);
#endif

	s_LabelMap.clear();
	s_BranchMap.clear();
	s_ByteSequenceMap.clear();
	s_PtrSequenceMap.clear();
	s_MovAddressMap.clear();
	s_JoinMap.clear();
	s_LabelMap.shrink_to_fit();
	s_BranchMap.shrink_to_fit();
	s_ByteSequenceMap.shrink_to_fit();
	s_PtrSequenceMap.shrink_to_fit();
	s_MovAddressMap.shrink_to_fit();
	s_JoinMap.shrink_to_fit();
	return 0;
}