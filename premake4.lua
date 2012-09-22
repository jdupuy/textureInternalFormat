solution "OpenGL"
	configurations {
	"debug",
	"release"
	}
	platforms { "x64", "x32" }

-- ---------------------------------------------------------
-- Project 
	
		basedir "./"
		language "C++"
		location "./"
		kind "ConsoleApp" -- Shouldn't this be in configuration section ?
		files { "*.hpp", "*.cpp" }
		includedirs {
		"include",
		}
		objdir "obj"

-- Debug configurations
		configuration {"debug"}
			defines {"DEBUG"}
			flags {"Symbols", "ExtraWarnings"}

-- Release configurations
		configuration {"release"}
			defines {"NDEBUG"}
			flags {"Optimize"}

-- Linux x86 platform gmake
		configuration {"linux", "gmake", "x32"}
			linkoptions {
			"-Wl,-rpath,./lib/linux/lin32 -L./lib/linux/lin32 -lGLEW -lglut"
			}
			libdirs {
			"lib/linux/lin32"
			}

-- Linux x64 platform gmake
		configuration {"linux", "gmake", "x64"}
			linkoptions {
			"-Wl,-rpath,./lib/linux/lin64 -L./lib/linux/lin64 -lGLEW -lglut"
			}
			libdirs {
			"lib/linux/lin64"
			}

-- Visual x86
		configuration {"vs2010", "x32"}
			libdirs {
			"lib/windows/win32"
			}
			links {
			"glew32s",
			"freeglut"
			}

	project "default"

	project "rgba2"
		defines{"R2_G2_B2"}

	project "rgba4"
		defines{"R4_G4_B4"}

	project "rgba5"
		defines{"R5_G5_B5"}

	project "r3g3b2"
		defines{"R3_G3_B2"}

---- Visual x64
--		configuration {"vs2010", "x64"}
--			links {
--			"glew32s",
--			"freeglut",
--			}
--			libdirs {
--			"lib/windows/win64"
--			}


