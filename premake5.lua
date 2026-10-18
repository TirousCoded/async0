

workspace "Async0"

	startproject "Async0"
	
	language "C++"
	cppdialect "C++20"
	
	targetdir "bin/%{cfg.buildcfg}"
	objdir "bin-int/%{cfg.buildcfg}"
	
	configurations
	{
		"Debug",
		"Release"
	}
	
	platforms
	{
		"x86",
		"x64"
	}
	
	filter "configurations:Debug"
	
		symbols "On"
		defines
		{
			"DEBUG"
		}
	
	filter "configurations:Release"
	
		optimize "On"
		defines
		{
			"NDEBUG"
		}
		
	filter "platforms:x86"
	
		system "Windows"
		architecture "x86"
		
	filter "platforms:x64"
	
		system "Windows"
		architecture "x86_64"
	
project "Async0"
	
	kind "ConsoleApp"
	targetname "Async0"
	
	files
	{
		"**.h",
		"**.cpp",
		"**.c",
		"**.txt",
		"**.taul",
		"**.yama"
	}
	
	includedirs
	{
	}
	
	links
	{
	}

	buildoptions
	{
	}

