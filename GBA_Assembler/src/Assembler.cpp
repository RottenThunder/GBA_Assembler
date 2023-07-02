#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_map>

static std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> s_LabelMap;
static std::unordered_map<std::string, std::vector<char>> s_ByteSequenceMap;

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

#ifdef ASSEMBLER_CONFIG_DEBUG
	adcParameters.push_back('0');
	adcParameters.push_back('1');
	adcParameters.push_back('0');
	adcParameters.push_back('0');
	adcParameters.push_back('0');
	adcParameters.push_back('0');
	adcParameters.push_back('0');
	adcParameters.push_back('1');
	adcParameters.push_back('0');
	adcParameters.push_back('1');
	adcParameters.push_back(((Rm & 4) >> 2) + '0');
	adcParameters.push_back(((Rm & 2) >> 1) + '0');
	adcParameters.push_back((Rm & 1) + '0');
	adcParameters.push_back(((Rd & 4) >> 2) + '0');
	adcParameters.push_back(((Rd & 2) >> 1) + '0');
	adcParameters.push_back((Rd & 1) + '0');
#endif
#ifdef ASSEMBLER_CONFIG_RELEASE
	adcParameters.push_back('\0');
	adcParameters.push_back('\1');
	adcParameters.push_back('\0');
	adcParameters.push_back('\0');
	adcParameters.push_back('\0');
	adcParameters.push_back('\0');
	adcParameters.push_back('\0');
	adcParameters.push_back('\1');
	adcParameters.push_back('\0');
	adcParameters.push_back('\1');
	adcParameters.push_back((Rm & 4) >> 2);
	adcParameters.push_back((Rm & 2) >> 1);
	adcParameters.push_back(Rm & 1);
	adcParameters.push_back((Rd & 4) >> 2);
	adcParameters.push_back((Rd & 2) >> 1);
	adcParameters.push_back(Rd & 1);
#endif

	return true;
}

static bool ProcessADDInstruction(std::string& addParameters)
{
	return true;
}

static bool ProcessANDInstruction(std::string& andParameters)
{
	if (!ProcessADCInstruction(andParameters))
		return false;

	andParameters[7]--;
	andParameters[9]--;
	return true;
}

static bool ProcessASRInstruction(std::string& asrParameters)
{
	return true;
}

static bool ProcessBICInstruction(std::string& bicParameters)
{
	if (!ProcessADCInstruction(bicParameters))
		return false;

	bicParameters[6]++;
	bicParameters[8]++;
	bicParameters[9]--;
	return true;
}

static bool ProcessCMNInstruction(std::string& cmnParameters)
{
	if (!ProcessADCInstruction(cmnParameters))
		return false;

	cmnParameters[6]++;
	cmnParameters[7]--;
	cmnParameters[8]++;
	return true;
}

static bool ProcessCMPInstruction(std::string& cmpParameters)
{
	return true;
}

static bool ProcessEORInstruction(std::string& eorParameters)
{
	if (!ProcessADCInstruction(eorParameters))
		return false;

	eorParameters[7]--;
	return true;
}

static bool ProcessLDMIAInstruction(std::string& ldmiaParameters)
{
	return true;
}

static bool ProcessLDRInstruction(std::string& ldrParameters)
{
	return true;
}

static bool ProcessLDRBInstruction(std::string& ldrbParameters)
{
	return true;
}

static bool ProcessLDRHInstruction(std::string& ldrhParameters)
{
	return true;
}

static bool ProcessLDRSBInstruction(std::string& ldrsbParameters)
{
	return true;
}

static bool ProcessLDRSHInstruction(std::string& ldrshParameters)
{
	return true;
}

static bool ProcessLSLInstruction(std::string& lslParameters)
{
	return true;
}

static bool ProcessLSRInstruction(std::string& lsrParameters)
{
	return true;
}

static bool ProcessMOVInstruction(std::string& movParameters)
{
	return true;
}

static bool ProcessMULInstruction(std::string& mulParameters)
{
	if (!ProcessADCInstruction(mulParameters))
		return false;

	mulParameters[6]++;
	return true;
}

static bool ProcessMVNInstruction(std::string& mvnParameters)
{
	if (!ProcessADCInstruction(mvnParameters))
		return false;

	mvnParameters[6]++;
	mvnParameters[8]++;
	return true;
}

static bool ProcessNEGInstruction(std::string& negParameters)
{
	if (!ProcessADCInstruction(negParameters))
		return false;

	negParameters[6]++;
	negParameters[7]--;
	return true;
}

static bool ProcessORRInstruction(std::string& orrParameters)
{
	if (!ProcessADCInstruction(orrParameters))
		return false;

	orrParameters[6]++;
	orrParameters[9]--;
	return true;
}

static bool ProcessPOPInstruction(std::string& popParameters)
{
	return true;
}

static bool ProcessPUSHInstruction(std::string& pushParameters)
{
	return true;
}

static bool ProcessRORInstruction(std::string& rorParameters)
{
	if (!ProcessADCInstruction(rorParameters))
		return false;

	rorParameters[8]++;
	return true;
}

static bool ProcessSBCInstruction(std::string& sbcParameters)
{
	if (!ProcessADCInstruction(sbcParameters))
		return false;

	sbcParameters[8]++;
	sbcParameters[9]--;
	return true;
}

static bool ProcessSTMIAInstruction(std::string& stmiaParameters)
{
	return true;
}

static bool ProcessSTRInstruction(std::string& strParameters)
{
	return true;
}

static bool ProcessSTRBInstruction(std::string& strbParameters)
{
	return true;
}

static bool ProcessSTRHInstruction(std::string& strhParameters)
{
	return true;
}

static bool ProcessSUBInstruction(std::string& subParameters)
{
	return true;
}

static bool ProcessSWIInstruction(std::string& swiParameters)
{
	return true;
}

static bool ProcessTSTInstruction(std::string& tstParameters)
{
	if (!ProcessADCInstruction(tstParameters))
		return false;

	tstParameters[6]++;
	tstParameters[7]--;
	tstParameters[9]--;
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
	std::string preProcessedFileName = filePath.filename().string();
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

		//4. Process Preprocessor commands like "#org"
		//TODO

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
					std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
					std::cout << "The Label On This Line Contains Spaces Which Is Not Valid" << std::endl;
					return false;
				}
			}
			mostRecentLabel = currentLine.substr(0, j);
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

		//8. Find The Instruction
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

		//9. Convert Instructions (except Branchs) Into Machine Code
		std::string instruction = currentLine.substr(0, j);
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
		else
		{
			inputStream.close();
			outputStream.close();
			std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
			std::cout << "The Instruction On This Line Is Invalid" << std::endl;
			return false;
		}


		//10. Put Line In Output File
		outputStream.write(currentLine.c_str(), currentLine.size());
		outputStream.write("\n", 1);
		currentInstructionNumber++;
	}

	inputStream.close();
	outputStream.close();
	std::cout << "Successfully PreProcessed " << std::filesystem::relative(filePath, sourcePath) << "..." << std::endl;
	return true;
}

static bool Assemble(const std::filesystem::path& intermediatePath)
{
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
		Assemble(intermediatePath);
	}

	s_LabelMap.clear();
	s_ByteSequenceMap.clear();
	return 0;
}