#include <iostream>
#include <filesystem>
#include <fstream>

#define ASSEMBLER_NUM_INSTRUCTIONS 3

static char** AllInstructions = nullptr;
static size_t* AllInstructionLengths = nullptr;

static void Init()
{
	AllInstructions = new char*[ASSEMBLER_NUM_INSTRUCTIONS];
	AllInstructionLengths = new size_t[ASSEMBLER_NUM_INSTRUCTIONS];

	AllInstructions[0] = new char[3];
	AllInstructions[0][0] = 'A';
	AllInstructions[0][1] = 'D';
	AllInstructions[0][2] = 'C';
	AllInstructionLengths[0] = 3;

	AllInstructions[1] = new char[3];
	AllInstructions[1][0] = 'A';
	AllInstructions[1][1] = 'D';
	AllInstructions[1][2] = 'D';
	AllInstructionLengths[1] = 3;

	AllInstructions[2] = new char[3];
	AllInstructions[2][0] = 'A';
	AllInstructions[2][1] = 'N';
	AllInstructions[2][2] = 'D';
	AllInstructionLengths[2] = 3;
}

static void Terminate()
{
	::operator delete(AllInstructionLengths, ASSEMBLER_NUM_INSTRUCTIONS * sizeof(size_t));

	::operator delete(AllInstructions[0], 3);
	::operator delete(AllInstructions[1], 3);
	::operator delete(AllInstructions[2], 3);

	::operator delete(AllInstructions, ASSEMBLER_NUM_INSTRUCTIONS * sizeof(char*));
}

/*

static std::string AssembleADCInstruction(const std::string& adcInstructionParameters)
{
	return "";
}

static std::string AssembleADDInstruction(const std::string& addInstructionParameters)
{
	size_t i = 0;
	for (; i < addInstructionParameters.size(); i++)
	{
		if (addInstructionParameters[i] != ' ')
			break;
	}
	std::string InstructionParameters = addInstructionParameters.substr(i);


	return "";
}

static std::string AssembleANDInstruction(const std::string& andInstructionParameters)
{
	return "";
}

static std::string AssembleInstruction(const std::string& instruction)
{
	size_t i = 0;
	for (; i < instruction.size(); i++)
	{
		if (instruction[i] == ' ')
			break;
	}
	std::string mainInstruction = instruction.substr(0, i);
	size_t mainInstructionSize = instruction.substr(0, i).size();
	size_t indexOfCorrectInstruction = UINT64_MAX;
	for (i = 0; i < ASSEMBLER_NUM_INSTRUCTIONS; i++)
	{
		if (mainInstructionSize == AllInstructionLengths[i])
		{
			size_t matches = 0;
			for (size_t j = 0; j < mainInstructionSize; j++)
			{
				if (mainInstruction[j] == AllInstructions[i][j])
					matches++;
			}
			if (matches == mainInstructionSize)
			{
				indexOfCorrectInstruction = i;
				i = ASSEMBLER_NUM_INSTRUCTIONS;
			}
		}
	}
	switch (indexOfCorrectInstruction)
	{
	case 0:
		return AssembleADCInstruction(instruction.substr(i + 1));
	case 1:
		return AssembleADDInstruction(instruction.substr(i + 1));
	case 2:
		return AssembleANDInstruction(instruction.substr(i + 1));
	default:
		return "";
	}
}

static bool Assemble(const std::filesystem::path& filePath, size_t fileSize, const std::filesystem::path& intDir)
{
	std::fstream inputStream;
	std::fstream outputStream;
	inputStream.open(filePath, std::ios::in | std::ios::binary);
	if (!inputStream.is_open())
	{
		inputStream.close();
		return false;
	}

	size_t numLines = 0;
	std::string currentLine;
	std::filesystem::path currentIntFile;
	while (std::getline(inputStream, currentLine))
	{
		numLines++;
		size_t i = 0;
		size_t size = currentLine.size();
		while (i < size)
		{
			if (currentLine[i] < ' ')
			{
				currentLine.erase(i, 1);
				size--;
			}
			else
			{
				i++;
			}
		}

		if (currentLine.size() == 0)
			continue;

		if (currentLine.back() == ':')
		{
			currentLine.pop_back();
			currentLine.push_back('.');
			currentLine.push_back('t');
			currentLine.push_back('x');
			currentLine.push_back('t');
			currentIntFile = intDir / currentLine;
			outputStream.open(currentIntFile, std::ios::out | std::ios::binary);
			outputStream.close();
			continue;
		}

		std::string binaryInstruction = AssembleInstruction(currentLine);
		if (binaryInstruction == "")
		{
			inputStream.close();
			std::cout << "Error on Line " << numLines << " in " << filePath << std::endl;
			return false;
		}
		outputStream.open(currentIntFile, std::ios::out | std::ios::binary | std::ios::app);
		outputStream.write(binaryInstruction.c_str(), binaryInstruction.size());
		outputStream.write("\n", 1);
		outputStream.close();
	}

	inputStream.close();
	std::cout << "Successfully Assembled " << filePath << "..." << std::endl;
	return true;
}
*/

static bool PreProcess(const std::filesystem::path& sourcePath, const std::filesystem::path& filePath, size_t fileSize, const std::filesystem::path& intDir)
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

	size_t currentLineNumber = 0;
	std::string currentLine;
	while (std::getline(inputStream, currentLine))
	{
		currentLineNumber++;

		size_t i = 0;
		size_t j = 0;

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

		//If Empty Line, Ignore
		if (currentLine.empty())
			continue;

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

		//If Empty Line, Ignore
		if (currentLine.empty())
			continue;

		//3a. Check For Label
		//Use "i" as an index into "currentLine", Use "j" as the index of the ':' char
		i = 0;
		j = 0;
		for (; i < currentLine.size(); i++)
		{
			if (currentLine[i] == ':')
			{
				j = i;
				i = currentLine.size();
			}
		}
		if (currentLine[0] == ':')
		{
			inputStream.close();
			outputStream.close();
			std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
			std::cout << "The Label on this line must be named!!!" << std::endl;
			return false;
		}

		//3b. Get Rid Of All Spaces In The Label
		//Use "i" as an index into "currentLine", Use "j" as the index of the ':' char
		if (j != 0)
		{
			i = 0;
			j++;
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
			if (currentLine[0] == ':')
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The Label on this line must be named!!!" << std::endl;
				return false;
			}

			//3c. Put the Label in the output file and get rid of the label in "currentLine"
			//Use "i" as an index into "currentLine", Use "j" as the index of the char after the ':' char
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
			outputStream.write(currentLine.substr(0, j).c_str(), j);
			outputStream.write("\n", 1);
			currentLine.erase(0, j);

			//If Empty Line, Ignore
			if (currentLine.empty())
				continue;
		}

		//4. Get Rid Of Leading Spaces to the Instruction (Or Byte Sequence)
		//Use "i" as an index into "currentLine", Use "j" as the index of the first char that is not a space
		i = 0;
		j = 0;
		for (; i < currentLine.size(); i++)
		{
			if (currentLine[i] != ' ')
			{
				j = i;
				i = currentLine.size();
			}
		}
		currentLine.erase(0, j);

		//If Empty Line, Ignore
		if (currentLine.empty())
			continue;

		//5a. Find whether it is a Byte Sequence
		if (currentLine[0] == '{')
		{
			//5b. Get Rid Of All Spaces In The Byte Sequence
			//Use "i" as an index into "currentLine", Use "j" as the current size of "currentLine"
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

			//5c. Check to see whether There is a Closed Bracket in the Byte Sequence
			if (currentLine.back() != '}')
			{
				inputStream.close();
				outputStream.close();
				std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
				std::cout << "The Byte Sequence On This Line Does Not Have a Closed Bracket (to signify an end)" << std::endl;
				return false;
			}

			//5d. Check whether all bytes are valid hex digits and the sequence is comma seperated
			//Use "i" as an index into "currentLine", Use "j" as the count of the amount of consecutive valid hex digits
			i = 1;
			j = 0;
			for (; i < currentLine.size(); i++)
			{
				if ((currentLine[i] >= '0' && currentLine[i] <= '9') || (currentLine[i] >= 'A' && currentLine[i] <= 'F'))
					j++;
				else if ((currentLine[i] == ',' || currentLine[i] == '}') && j == 2)
					j = 0;
				else
				{
					inputStream.close();
					outputStream.close();
					std::cout << "Error on Line " << currentLineNumber << " in " << std::filesystem::relative(filePath, sourcePath) << std::endl;
					std::cout << "The Byte Sequence On This Line Is Not Valid Either Because it is not comma seperated or it does not contain valid hex values" << std::endl;
					return false;
				}
			}

			//5e. Get Rid Of All Commas In The Byte Sequence
			//Use "i" as an index into "currentLine", Use "j" as the current size of "currentLine"
			i = 0;
			j = currentLine.size();
			while (i < j)
			{
				if (currentLine[i] == ',')
				{
					currentLine.erase(i, 1);
					j--;
				}
				else
				{
					i++;
				}
			}

			//5f. Put Byte Sequence Into Output File
			outputStream.write(currentLine.c_str(), currentLine.size());
			outputStream.write("\n", 1);
		}
		else
		{
			//6. Find Out The Instruction Opcode
			//TODO

			//?. Put the line in the output file
			outputStream.write(currentLine.c_str(), currentLine.size());
			outputStream.write("\n", 1);
		}
	}

	inputStream.close();
	outputStream.close();
	std::cout << "Successfully PreProcessed " << std::filesystem::relative(filePath, sourcePath) << "..." << std::endl;
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

	Init();

	std::filesystem::path intermediatePath = sourcePath / "AssemblerInt";
	std::filesystem::remove_all(intermediatePath);
	std::filesystem::create_directory(intermediatePath);
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(sourcePath))
	{
		if (dirEntry.is_directory())
			continue;

		std::filesystem::path relativePath = std::filesystem::relative(dirEntry.path(), sourcePath);
		if (dirEntry.path().extension().string() == ".asm")
		{
			std::cout << "PreProcessing " << relativePath << "..." << std::endl;
			bool preprocessed = PreProcess(sourcePath, dirEntry.path(), dirEntry.file_size(), intermediatePath);
			if (!preprocessed)
				break;
		}
		else
		{
			std::cout << "Ignoring " << relativePath << " because it is not an assembly file..." << std::endl;
		}
	}

	Terminate();
	return 0;
}