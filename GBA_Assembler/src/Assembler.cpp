#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_map>

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

struct BranchInfo
{
	ASSEMBLER_BRANCH_TYPE type;
	uint64_t fileNumber;
	uint64_t InstructionNumber;
	std::string label;
};

static std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> s_LabelMap;
static std::vector<BranchInfo> s_BranchMap;
static std::unordered_map<std::string, std::vector<char>> s_ByteSequenceMap;
static std::vector<std::pair<uint64_t, uint64_t>> s_JoinMap;

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
		if (i % 16 == 0 && i != 0)
			placeholder.push_back(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION);

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

	if (adcParameters[3] != 'R')
		return false;
	if (adcParameters[4] > '7' || adcParameters[4] < '0')
		return false;

	for (size_t i = 5; i < adcParameters.size(); i++)
	{
		if (adcParameters[i] != ' ')
			return false;
	}

	char Rm = adcParameters[4] - '0';

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

	if (((Rm & 4) >> 2) == 0)
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	else
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);

	if (((Rm & 2) >> 1) == 0)
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	else
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);

	if ((Rm & 1) == 0)
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	else
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);

	if (((Rd & 4) >> 2) == 0)
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	else
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);

	if (((Rd & 2) >> 1) == 0)
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	else
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);

	if ((Rd & 1) == 0)
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_0);
	else
		adcParameters.push_back(ASSEMBLER_OUTPUT_SYMBOL_1);

	return true;
}

static bool ProcessADDInstruction(std::string& addParameters)
{
	return false;
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
	return false;
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

static bool ProcessBXInstruction(std::string& bxParameters)
{
	if (!ProcessNOPInstruction(bxParameters))
		return false;

	bxParameters[1] = ASSEMBLER_OUTPUT_SYMBOL_1;
	bxParameters[5] = ASSEMBLER_OUTPUT_SYMBOL_1;
	bxParameters[6] = ASSEMBLER_OUTPUT_SYMBOL_1;
	bxParameters[7] = ASSEMBLER_OUTPUT_SYMBOL_1;
	bxParameters[9] = ASSEMBLER_OUTPUT_SYMBOL_1;
	bxParameters[10] = ASSEMBLER_OUTPUT_SYMBOL_1;
	bxParameters[11] = ASSEMBLER_OUTPUT_SYMBOL_1;
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
	return false;
}

static bool ProcessEORInstruction(std::string& eorParameters)
{
	if (!ProcessADCInstruction(eorParameters))
		return false;

	eorParameters[7] = ASSEMBLER_OUTPUT_SYMBOL_0;
	return true;
}

static bool ProcessLDMIAInstruction(std::string& ldmiaParameters)
{
	return false;
}

static bool ProcessLDRInstruction(std::string& ldrParameters)
{
	return false;
}

static bool ProcessLDRBInstruction(std::string& ldrbParameters)
{
	return false;
}

static bool ProcessLDRHInstruction(std::string& ldrhParameters)
{
	return false;
}

static bool ProcessLDRSBInstruction(std::string& ldrsbParameters)
{
	return false;
}

static bool ProcessLDRSHInstruction(std::string& ldrshParameters)
{
	return false;
}

static bool ProcessLSLInstruction(std::string& lslParameters)
{
	return false;
}

static bool ProcessLSRInstruction(std::string& lsrParameters)
{
	return false;
}

static bool ProcessMOVInstruction(std::string& movParameters)
{
	return false;
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

static bool ProcessPOPInstruction(std::string& popParameters)
{
	return false;
}

static bool ProcessPUSHInstruction(std::string& pushParameters)
{
	return false;
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
	return false;
}

static bool ProcessSTRBInstruction(std::string& strbParameters)
{
	return false;
}

static bool ProcessSTRHInstruction(std::string& strhParameters)
{
	return false;
}

static bool ProcessSUBInstruction(std::string& subParameters)
{
	return false;
}

static bool ProcessSWIInstruction(std::string& swiParameters)
{
	return false;
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


		//4. Replace MEM Macros With Their Integer Equivalent
		//Use "i" as the index of start of the MEM Macro, Use "j" as the index of the ']' char
		j = 0;
		i = currentLine.find("MEM_BIOS");
		if (i != std::string::npos)
		{
			i += 8;
			if (currentLine.size() == i)
			{
				i -= 8;
				currentLine.erase(i, 8);
				currentLine.insert(i, std::to_string(ASSEMBLER_MEM_BIOS));
			}
			else if (currentLine[i] == '[')
			{
				j = i;
				j++;
				while (j < currentLine.size())
				{
					if (currentLine[j] == ']')
						break;
					else
						j++;
				}
				if (j == currentLine.size() || j - i == 1)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
					std::cout << "The Macro MEM_BIOS On This Line Has A Syntax Error With It's Index" << std::endl;
					return false;
				}
				std::string translatedNumber = std::to_string(ASSEMBLER_MEM_BIOS + std::stoi(currentLine.substr(i + 1, j - i - 1)));
				i -= 8;
				j++;
				currentLine.erase(i, j - i);
				currentLine.insert(i, translatedNumber);
			}
			else
			{
				i -= 8;
				currentLine.erase(i, 8);
				currentLine.insert(i, std::to_string(ASSEMBLER_MEM_BIOS));
			}
		}
		i = currentLine.find("MEM_ERAM");
		if (i != std::string::npos)
		{
			i += 8;
			if (currentLine.size() == i)
			{
				i -= 8;
				currentLine.erase(i, 8);
				currentLine.insert(i, std::to_string(ASSEMBLER_MEM_ERAM));
			}
			else if (currentLine[i] == '[')
			{
				j = i;
				j++;
				while (j < currentLine.size())
				{
					if (currentLine[j] == ']')
						break;
					else
						j++;
				}
				if (j == currentLine.size() || j - i == 1)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
					std::cout << "The Macro MEM_ERAM On This Line Has A Syntax Error With It's Index" << std::endl;
					return false;
				}
				std::string translatedNumber = std::to_string(ASSEMBLER_MEM_ERAM + std::stoi(currentLine.substr(i + 1, j - i - 1)));
				i -= 8;
				j++;
				currentLine.erase(i, j - i);
				currentLine.insert(i, translatedNumber);
			}
			else
			{
				i -= 8;
				currentLine.erase(i, 8);
				currentLine.insert(i, std::to_string(ASSEMBLER_MEM_ERAM));
			}
		}
		i = currentLine.find("MEM_IRAM");
		if (i != std::string::npos)
		{
			i += 8;
			if (currentLine.size() == i)
			{
				i -= 8;
				currentLine.erase(i, 8);
				currentLine.insert(i, std::to_string(ASSEMBLER_MEM_IRAM));
			}
			else if (currentLine[i] == '[')
			{
				j = i;
				j++;
				while (j < currentLine.size())
				{
					if (currentLine[j] == ']')
						break;
					else
						j++;
				}
				if (j == currentLine.size() || j - i == 1)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
					std::cout << "The Macro MEM_IRAM On This Line Has A Syntax Error With It's Index" << std::endl;
					return false;
				}
				std::string translatedNumber = std::to_string(ASSEMBLER_MEM_IRAM + std::stoi(currentLine.substr(i + 1, j - i - 1)));
				i -= 8;
				j++;
				currentLine.erase(i, j - i);
				currentLine.insert(i, translatedNumber);
			}
			else
			{
				i -= 8;
				currentLine.erase(i, 8);
				currentLine.insert(i, std::to_string(ASSEMBLER_MEM_IRAM));
			}
		}
		i = currentLine.find("MEM_IO");
		if (i != std::string::npos)
		{
			i += 6;
			if (currentLine.size() == i)
			{
				i -= 6;
				currentLine.erase(i, 6);
				currentLine.insert(i, std::to_string(ASSEMBLER_MEM_IO));
			}
			else if (currentLine[i] == '[')
			{
				j = i;
				j++;
				while (j < currentLine.size())
				{
					if (currentLine[j] == ']')
						break;
					else
						j++;
				}
				if (j == currentLine.size() || j - i == 1)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
					std::cout << "The Macro MEM_IO On This Line Has A Syntax Error With It's Index" << std::endl;
					return false;
				}
				std::string translatedNumber = std::to_string(ASSEMBLER_MEM_IO + std::stoi(currentLine.substr(i + 1, j - i - 1)));
				i -= 6;
				j++;
				currentLine.erase(i, j - i);
				currentLine.insert(i, translatedNumber);
			}
			else
			{
				i -= 6;
				currentLine.erase(i, 6);
				currentLine.insert(i, std::to_string(ASSEMBLER_MEM_IO));
			}
		}
		i = currentLine.find("MEM_PALETTE");
		if (i != std::string::npos)
		{
			i += 11;
			if (currentLine.size() == i)
			{
				i -= 11;
				currentLine.erase(i, 11);
				currentLine.insert(i, std::to_string(ASSEMBLER_MEM_PALETTE));
			}
			else if (currentLine[i] == '[')
			{
				j = i;
				j++;
				while (j < currentLine.size())
				{
					if (currentLine[j] == ']')
						break;
					else
						j++;
				}
				if (j == currentLine.size() || j - i == 1)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
					std::cout << "The Macro MEM_PALETTE On This Line Has A Syntax Error With It's Index" << std::endl;
					return false;
				}
				std::string translatedNumber = std::to_string(ASSEMBLER_MEM_PALETTE + std::stoi(currentLine.substr(i + 1, j - i - 1)));
				i -= 11;
				j++;
				currentLine.erase(i, j - i);
				currentLine.insert(i, translatedNumber);
			}
			else
			{
				i -= 11;
				currentLine.erase(i, 11);
				currentLine.insert(i, std::to_string(ASSEMBLER_MEM_PALETTE));
			}
		}
		i = currentLine.find("MEM_VRAM");
		if (i != std::string::npos)
		{
			i += 8;
			if (currentLine.size() == i)
			{
				i -= 8;
				currentLine.erase(i, 8);
				currentLine.insert(i, std::to_string(ASSEMBLER_MEM_VRAM));
			}
			else if (currentLine[i] == '[')
			{
				j = i;
				j++;
				while (j < currentLine.size())
				{
					if (currentLine[j] == ']')
						break;
					else
						j++;
				}
				if (j == currentLine.size() || j - i == 1)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
					std::cout << "The Macro MEM_VRAM On This Line Has A Syntax Error With It's Index" << std::endl;
					return false;
				}
				std::string translatedNumber = std::to_string(ASSEMBLER_MEM_VRAM + std::stoi(currentLine.substr(i + 1, j - i - 1)));
				i -= 8;
				j++;
				currentLine.erase(i, j - i);
				currentLine.insert(i, translatedNumber);
			}
			else
			{
				i -= 8;
				currentLine.erase(i, 8);
				currentLine.insert(i, std::to_string(ASSEMBLER_MEM_VRAM));
			}
		}
		i = currentLine.find("MEM_OAM");
		if (i != std::string::npos)
		{
			i += 7;
			if (currentLine.size() == i)
			{
				i -= 7;
				currentLine.erase(i, 7);
				currentLine.insert(i, std::to_string(ASSEMBLER_MEM_OAM));
			}
			else if (currentLine[i] == '[')
			{
				j = i;
				j++;
				while (j < currentLine.size())
				{
					if (currentLine[j] == ']')
						break;
					else
						j++;
				}
				if (j == currentLine.size() || j - i == 1)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
					std::cout << "The Macro MEM_OAM On This Line Has A Syntax Error With It's Index" << std::endl;
					return false;
				}
				std::string translatedNumber = std::to_string(ASSEMBLER_MEM_OAM + std::stoi(currentLine.substr(i + 1, j - i - 1)));
				i -= 7;
				j++;
				currentLine.erase(i, j - i);
				currentLine.insert(i, translatedNumber);
			}
			else
			{
				i -= 7;
				currentLine.erase(i, 7);
				currentLine.insert(i, std::to_string(ASSEMBLER_MEM_OAM));
			}
		}
		i = currentLine.find("MEM_ROM");
		if (i != std::string::npos)
		{
			i += 7;
			if (currentLine.size() == i)
			{
				i -= 7;
				currentLine.erase(i, 7);
				currentLine.insert(i, std::to_string(ASSEMBLER_MEM_ROM));
			}
			else if (currentLine[i] == '[')
			{
				j = i;
				j++;
				while (j < currentLine.size())
				{
					if (currentLine[j] == ']')
						break;
					else
						j++;
				}
				if (j == currentLine.size() || j - i == 1)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
					std::cout << "The Macro MEM_ROM On This Line Has A Syntax Error With It's Index" << std::endl;
					return false;
				}
				std::string translatedNumber = std::to_string(ASSEMBLER_MEM_ROM + std::stoi(currentLine.substr(i + 1, j - i - 1)));
				i -= 7;
				j++;
				currentLine.erase(i, j - i);
				currentLine.insert(i, translatedNumber);
			}
			else
			{
				i -= 7;
				currentLine.erase(i, 7);
				currentLine.insert(i, std::to_string(ASSEMBLER_MEM_ROM));
			}
		}

		//5. Process Preprocessor Directives
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
					std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
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
					std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
					std::cout << "The ~join Preprocessor Directive On This Line Does Not Have A File Path To Join Associated With It" << std::endl;
					return false;
				}
				if (currentLine[j] != '"')
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
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
					std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
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
					std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
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
						s_JoinMap.back().second = i;

					goto Change;

					dontChange: i--;
					Change: i++;
				}
				if (s_JoinMap.back().first == s_JoinMap.back().second)
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
					std::cout << "The ~join Preprocessor Directive On This Line Provides A File Path That Is The Same As The File It Was Found In" << std::endl;
					return false;
				}
			}
			else
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The Preprocessor Directive On This Line Is Not A Recognised Directive" << std::endl;
				return false;
			}
		}

		//6. Store Labels into a map
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
					std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
					std::cout << "The Label On This Line Contains Spaces Which Is Not Valid" << std::endl;
					return false;
				}
			}
			mostRecentLabel = currentLine.substr(0, j);
			if (mostRecentLabel.empty())
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The Label On This Line Must Contain Printable Characters" << std::endl;
				return false;
			}
			if (s_LabelMap.contains(mostRecentLabel))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The Label On This Line Is Already Defined" << std::endl;
				return false;
			}
			s_LabelMap.insert({ mostRecentLabel, { fileNumber, currentInstructionNumber } });
			j++;
			currentLine.erase(0, j);
			j = 0;
		}

		//7. Get Rid of Leading Spaces
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

		//8. Process The Byte Sequence If There Is One
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
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The Byte Sequence On This Line Does Not Have An Ending Curly Bracket" << std::endl;
				return false;
			}
			currentLine.pop_back();

			if ((currentLine.size() % 2) == 1)
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The Byte Sequence On This Line Has Half A Byte Missing" << std::endl;
				return false;
			}

			if (mostRecentLabel.empty())
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The Byte Sequence On This Line Does Not Have A Label To Identify It" << std::endl;
				return false;
			}

			if (s_ByteSequenceMap.contains(mostRecentLabel))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The Label That Identifies The Byte Sequence On This Line Is Already Defined" << std::endl;
				return false;
			}

			s_LabelMap.erase(mostRecentLabel);
			s_ByteSequenceMap.insert({ mostRecentLabel, std::vector<char>() });

			i = 0;
			j = 0;
			for (; i < currentLine.size(); i++)
			{
				if (j == 0)
				{
					if (currentLine[i] >= '0' && currentLine[i] <= '9')
					{
						s_ByteSequenceMap[mostRecentLabel].push_back((currentLine[i] - '0') << 4);
					}
					else if (currentLine[i] >= 'A' && currentLine[i] <= 'F')
					{
						s_ByteSequenceMap[mostRecentLabel].push_back(((currentLine[i] - 'A') + 10) << 4);
					}
					else
					{
						inputStream.close();
						outputStream.close();
						std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
						std::cout << "The Byte Sequence On This Line Has An Invalid Hexadecimal Digit" << std::endl;
						return false;
					}

					j = 1;
				}
				else
				{
					if (currentLine[i] >= '0' && currentLine[i] <= '9')
					{
						s_ByteSequenceMap[mostRecentLabel].back() += (currentLine[i] - '0');
					}
					else if (currentLine[i] >= 'A' && currentLine[i] <= 'F')
					{
						s_ByteSequenceMap[mostRecentLabel].back() += ((currentLine[i] - 'A') + 10);
					}
					else
					{
						inputStream.close();
						outputStream.close();
						std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
						std::cout << "The Byte Sequence On This Line Has An Invalid Hexadecimal Digit" << std::endl;
						return false;
					}

					j = 0;
				}
			}

			continue;
		}

		//9. Find The Instruction
		//Use "i" as an index into "currentLine", Use "j" as the index of the first char that is a space
		i = 0;
		j = 0;
		for (; i < currentLine.size(); i++)
		{
			if (currentLine[i] == ' ')
			{
				j = i;
				i = currentLine.size();
			}
		}

		//10. Convert Instructions (except Branchs) Into Machine Code
		std::string instruction = currentLine.substr(0, j);
		i = 0;
		j = 0;
		if (instruction == "ADC")
		{
			currentLine.erase(0, 4);
			if (!ProcessADCInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The ADC Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "ADD")
		{
			currentLine.erase(0, 4);
			if (!ProcessADDInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The ADD Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "AND")
		{
			currentLine.erase(0, 4);
			if (!ProcessANDInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The AND Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "ASR")
		{
			currentLine.erase(0, 4);
			if (!ProcessASRInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The ASR Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "B")
		{
			currentLine.erase(0, 2);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_AL, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			for (; i < ASSEMBLER_INSTRUCTION_CHAR_SIZE * 2; i++)
				currentLine.pop_back();

			currentInstructionNumber += 7;
		}
		else if (instruction == "BEQ")
		{
			currentLine.erase(0, 4);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_EQ, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BNE")
		{
			currentLine.erase(0, 4);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_NE, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BCS" || instruction == "BHS")
		{
			currentLine.erase(0, 4);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_CS_HS, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BCC" || instruction == "BLO")
		{
			currentLine.erase(0, 4);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_CC_LO, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BMI")
		{
			currentLine.erase(0, 4);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_MI, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BPL")
		{
			currentLine.erase(0, 4);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_PL, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BVS")
		{
			currentLine.erase(0, 4);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_VS, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BVC")
		{
			currentLine.erase(0, 4);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_VC, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BHI")
		{
			currentLine.erase(0, 4);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_HI, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BLS")
		{
			currentLine.erase(0, 4);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_LS, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BGE")
		{
			currentLine.erase(0, 4);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_GE, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BLT")
		{
			currentLine.erase(0, 4);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_LT, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BGT")
		{
			currentLine.erase(0, 4);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_GT, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BLE")
		{
			currentLine.erase(0, 4);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_LE, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentInstructionNumber += 9;
		}
		else if (instruction == "BAL")
		{
			currentLine.erase(0, 4);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_AL, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			for (; i < ASSEMBLER_INSTRUCTION_CHAR_SIZE * 2; i++)
				currentLine.pop_back();
			currentInstructionNumber += 7;
		}
		else if (instruction == "BNV")
		{
			inputStream.close();
			outputStream.close();
			std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
			std::cout << "This Line Contains A BNV Instruction Which Will Give Unpredictable Results" << std::endl;
			return false;
		}
		else if (instruction == "BL")
		{
			currentLine.erase(0, 3);
			ProcessBranchInstruction({ ASSEMBLER_BRANCH_LINK, fileNumber, currentInstructionNumber, currentLine }, currentLine);
			currentLine.push_back(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION);
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
			currentInstructionNumber += 10;
		}
		else if (instruction == "BX")
		{
			currentLine.erase(0, 2);
			if (!ProcessBXInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The BX Instruction On This Line Should Not Have Any Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "BIC")
		{
			currentLine.erase(0, 4);
			if (!ProcessBICInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The BIC Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "CMN")
		{
			currentLine.erase(0, 4);
			if (!ProcessCMNInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The CMN Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "CMP")
		{
			currentLine.erase(0, 4);
			if (!ProcessCMPInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The CMP Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "EOR")
		{
			currentLine.erase(0, 4);
			if (!ProcessEORInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The EOR Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "LDMIA")
		{
			currentLine.erase(0, 6);
			if (!ProcessLDMIAInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The LDMIA Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "LDR")
		{
			currentLine.erase(0, 4);
			if (!ProcessLDRInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The LDR Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "LDRB")
		{
			currentLine.erase(0, 5);
			if (!ProcessLDRBInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The LDRB Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "LDRH")
		{
			currentLine.erase(0, 5);
			if (!ProcessLDRHInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The LDRH Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "LDRSB")
		{
			currentLine.erase(0, 6);
			if (!ProcessLDRSBInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The LDRSB Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "LDRSH")
		{
			currentLine.erase(0, 6);
			if (!ProcessLDRSHInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The LDRSH Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "LSL")
		{
			currentLine.erase(0, 4);
			if (!ProcessLSLInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The LSL Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "LSR")
		{
			currentLine.erase(0, 4);
			if (!ProcessLSRInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The LSR Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "MOV")
		{
			currentLine.erase(0, 4);
			if (!ProcessMOVInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The MOV Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "MUL")
		{
			currentLine.erase(0, 4);
			if (!ProcessMULInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The MUL Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "MVN")
		{
			currentLine.erase(0, 4);
			if (!ProcessMVNInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The MVN Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "NEG")
		{
			currentLine.erase(0, 4);
			if (!ProcessNEGInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The NEG Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "ORR")
		{
			currentLine.erase(0, 4);
			if (!ProcessORRInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The ORR Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "POP")
		{
			currentLine.erase(0, 4);
			if (!ProcessPOPInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The POP Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "PUSH")
		{
			currentLine.erase(0, 5);
			if (!ProcessPUSHInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The PUSH Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "ROR")
		{
			currentLine.erase(0, 4);
			if (!ProcessRORInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The ROR Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "SBC")
		{
			currentLine.erase(0, 4);
			if (!ProcessSBCInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The SBC Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "STMIA")
		{
			currentLine.erase(0, 6);
			if (!ProcessSTMIAInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The STMIA Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "STR")
		{
			currentLine.erase(0, 4);
			if (!ProcessSTRInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The STR Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "STRB")
		{
			currentLine.erase(0, 5);
			if (!ProcessSTRBInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The STRB Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "STRH")
		{
			currentLine.erase(0, 5);
			if (!ProcessSTRHInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The STRH Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "SUB")
		{
			currentLine.erase(0, 4);
			if (!ProcessSUBInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The SUB Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "SWI")
		{
			currentLine.erase(0, 4);
			if (!ProcessSWIInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The SWI Instruction On This Line Has Invalid Parameters" << std::endl;
				return false;
			}
		}
		else if (instruction == "TST")
		{
			currentLine.erase(0, 4);
			if (!ProcessTSTInstruction(currentLine))
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
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
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The NOP Instruction On This Line Should Not Have Any Parameters" << std::endl;
				return false;
			}
		}
		else
		{
			inputStream.close();
			outputStream.close();
			std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
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
	std::cout << "Successfully PreProcessed " << std::filesystem::relative(filePath, sourcePath) << "..." << std::endl;
	return true;
}

static bool Assemble(const std::filesystem::path& sourcePath, const std::filesystem::path& intermediatePath)
{
	std::fstream inputStream;
	std::fstream outputStream;

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

			if (s_JoinMap[i].first == fileCounter)
			{
				parentFile = dirEntry.path();
				originalAmountOfParentFileInstructions = dirEntry.file_size() / ASSEMBLER_INSTRUCTION_CHAR_SIZE;
				if (childFile != intermediatePath)
					break;
			}
			else if (s_JoinMap[i].second == fileCounter)
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
		for (auto& [label, info] : s_LabelMap)
		{
			if (info.first == s_JoinMap[i].second)
			{
				info.first = s_JoinMap[i].first;
				info.second += originalAmountOfParentFileInstructions;
			}
			if (info.first > s_JoinMap[i].second)
				info.first--;
		}

		//Fix The Branch Map
		for (size_t b = 0; b < s_BranchMap.size(); b++)
		{
			if (s_BranchMap[b].fileNumber == s_JoinMap[i].second)
			{
				s_BranchMap[b].fileNumber = s_JoinMap[i].first;
				s_BranchMap[b].InstructionNumber += originalAmountOfParentFileInstructions;
			}
			if (s_BranchMap[b].fileNumber > s_JoinMap[i].second)
				s_BranchMap[b].fileNumber--;
		}

		//Fix The Join Map
		for (size_t j = i + 1; j < s_JoinMap.size(); j++)
		{
			if (s_JoinMap[j].first > s_JoinMap[i].second)
				s_JoinMap[j].first--;
			if (s_JoinMap[j].second > s_JoinMap[i].second)
				s_JoinMap[j].second--;
		}
	}

	//Combine All Files Into One
	std::filesystem::directory_entry combinedFilePath;
	uint64_t fileCounter = 0;
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(intermediatePath))
	{
		if (dirEntry.is_directory())
			continue;

		if (fileCounter == 0)
			combinedFilePath = dirEntry;
		else
		{
			size_t originalAmountOfParentFileInstructions = combinedFilePath.file_size() / ASSEMBLER_INSTRUCTION_CHAR_SIZE;

			inputStream.open(dirEntry.path(), std::ios::in | std::ios::binary);
			outputStream.open(combinedFilePath.path(), std::ios::out | std::ios::binary | std::ios::app);

			char nextByte;
			while (inputStream.read(&nextByte, 1))
				outputStream.write(&nextByte, 1);

			inputStream.close();
			outputStream.close();

			//Fix The Label Map
			for (auto& [label, info] : s_LabelMap)
			{
				if (info.first == fileCounter)
				{
					info.first = 0;
					info.second += originalAmountOfParentFileInstructions;
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
		}

		fileCounter++;
	}


	//Evaluate Branchs
	outputStream.open(combinedFilePath.path(), std::ios::in | std::ios::out | std::ios::binary);
	for (size_t i = 0; i < s_BranchMap.size(); i++)
	{
		if (!s_LabelMap.contains(s_BranchMap[i].label))
		{
			if (s_ByteSequenceMap.contains(s_BranchMap[i].label))
			{
				std::cout << "Error In Assembling..." << std::endl;
				std::cout << "The Label " << s_BranchMap[i].label << " Is A Byte Sequence, Which Can Not Be Branched To" << std::endl;
			}
			else
			{
				std::cout << "Error In Assembling..." << std::endl;
				std::cout << "The Label " << s_BranchMap[i].label << " Is Not Defined" << std::endl;
			}

			return false;
		}

		uint64_t pointer = s_LabelMap.at(s_BranchMap[i].label).second;
		pointer *= 2;
		pointer += 0x08000000;
		pointer += 192;
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

				outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);

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

				outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);

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
				linkingBranchSymbols[12] = ASSEMBLER_OUTPUT_SYMBOL_0;
				linkingBranchSymbols[13] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[14] = ASSEMBLER_OUTPUT_SYMBOL_1;
				linkingBranchSymbols[15] = ASSEMBLER_OUTPUT_SYMBOL_1;
				outputStream.write(linkingBranchSymbols, 16);

				outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
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

				outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);

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

				outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
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

		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);

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

		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);

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

		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);

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

		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);

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

		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);

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

		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);

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

		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);

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

		outputStream.write(ASSEMBLER_OUTPUT_SYMBOL_END_OF_INSTRUCTION_STRING, 1);
	}
	outputStream.close();

	//Evaluate Byte Sequence Offsets
	//TODO

	//Assemble
	//TODO


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

	std::filesystem::path intermediatePath = sourcePath / "AssemblerInt";
	std::filesystem::remove_all(intermediatePath);
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
		Assemble(sourcePath, intermediatePath);
	}

	s_LabelMap.clear();
	s_BranchMap.clear();
	s_BranchMap.shrink_to_fit();
	s_ByteSequenceMap.clear();
	s_JoinMap.clear();
	s_JoinMap.shrink_to_fit();
	return 0;
}