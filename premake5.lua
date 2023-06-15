workspace "GBA_Assembler"
	architecture "x86_64"
	startproject "GBA_Assembler"

	configurations
	{
		"Debug",
		"Release"
	}

project "GBA_Assembler"
	location "GBA_Assembler"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "On"

	targetdir ("Bin/%{cfg.buildcfg}-%{cfg.architecture}-%{cfg.system}/%{prj.name}")
	objdir ("Bin-Int/%{cfg.buildcfg}-%{cfg.architecture}-%{cfg.system}/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src"
	}

	filter "system:windows"
		systemversion "latest"

	filter "system:linux"
		pic "On"
		systemversion "latest"

	filter "configurations:Debug"
		defines "ASSEMBLER_CONFIG_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "ASSEMBLER_CONFIG_RELEASE"
		runtime "Release"
		optimize "On"
