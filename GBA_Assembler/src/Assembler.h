#pragma once
#include <iostream>
#include <filesystem>
#include <fstream>

#define ASSEMBLER_NUM_INSTRUCTIONS 3

static char** AllInstructions = nullptr;
static size_t* AllInstructionLengths = nullptr;

static std::string AssembleADCInstruction(const std::string& adcInstructionParameters)
{
	return "";
}

static std::string AssembleADDInstruction(const std::string& addInstructionParameters)
{
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