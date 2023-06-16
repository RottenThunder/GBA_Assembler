#include "Assembler.h"

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
	::operator delete(AllInstructionLengths, ASSEMBLER_NUM_INSTRUCTIONS);

	::operator delete(AllInstructions[0], 3);
	::operator delete(AllInstructions[1], 3);
	::operator delete(AllInstructions[2], 3);

	::operator delete(AllInstructions, ASSEMBLER_NUM_INSTRUCTIONS);
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

	std::filesystem::path intermediatePath = sourcePath / "int";
	std::filesystem::create_directory(intermediatePath);
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(sourcePath))
	{
		if (dirEntry.is_directory())
			continue;

		std::filesystem::path relativePath = std::filesystem::relative(dirEntry.path(), sourcePath);
		if (dirEntry.path().extension().string() != ".asm")
		{
			std::cout << "Ignoring " << relativePath << " because it is not an assembly file..." << std::endl;
		}
		else
		{
			std::cout << "Assembling " << relativePath << "..." << std::endl;
			bool assembled = Assemble(dirEntry.path(), dirEntry.file_size(), intermediatePath);
			if (!assembled)
				break;
		}
	}

	Terminate();
	return 0;
}