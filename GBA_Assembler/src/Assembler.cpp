#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_map>

static std::unordered_map<std::string, size_t> s_LabelMap;

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
		//Use "i" as an index into "currentLine", Use "j" as a count of the amount of consecutive '/' chars that have been seen
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
			s_LabelMap.insert({ currentLine.substr(0, j), currentLineNumber });
			j++;
			currentLine.erase(0, j);
		}

		//6. Convert Instructions (except Branchs) Into Machine Code
		//TODO


		//?. Put Line In Output File
		outputStream.write(currentLine.c_str(), currentLine.size());
		outputStream.write("\n", 1);
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

	s_LabelMap.clear();
	return 0;
}