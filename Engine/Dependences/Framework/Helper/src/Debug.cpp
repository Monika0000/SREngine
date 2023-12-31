//
// Created by Nikita on 16.11.2020.
//

#include "../inc/Debug.h"
#include "FileSystem/FileSystem.h"
#include <iostream>
#include <ResourceManager/ResourceManager.h>
#include <easy/profiler.h>

namespace Framework::Helper {
        void Debug::Print(std::string &msg, Debug::Type type) {
            g_mutex.lock();

            static std::string pref		= "-";
            static ConsoleColor color	= ConsoleColor::Black;

            switch (type) {
                case Debug::Type::Log:		    pref = "[Log] ";	        color = ConsoleColor::Cyan;				break;
                case Debug::Type::Info: 	    pref = "[Info] ";	        color = ConsoleColor::Magenta;			break;
                case Debug::Type::Debug:	    pref = "[Debug] ";	        color = ConsoleColor::Blue;				break;
                case Debug::Type::Graph:	    pref = "[Graph] ";	        color = ConsoleColor::Green;			break;
                case Debug::Type::Shader:	    pref = "[Shader] ";	        color = ConsoleColor::LightCyan;		break;
                case Debug::Type::Script:	    pref = "[Script] ";	        color = ConsoleColor::Brown;			break;
                case Debug::Type::System:	    pref = "[System] ";	        color = ConsoleColor::LightBlue;		break;
                case Debug::Type::Warn:	        pref = "[Warn] ";	        color = ConsoleColor::Yellow;			break;
                case Debug::Type::Error:	    pref = "[Error] ";	        color = ConsoleColor::LightRed;			break;
                case Debug::Type::ScriptError:	pref = "[ScriptError] ";	color = ConsoleColor::LightRed;			break;
                case Debug::Type::ScriptLog:	pref = "[ScriptLog] ";	    color = ConsoleColor::LightCyan;		break;
                default:					    pref = "[Unk] ";	        color = ConsoleColor::Black;			break;
            }

            if (Debug::g_showUseMemory) {
                std::cout << '<' << (float)ResourceManager::GetUsedMemoryLoad() / 1024.f << " KB> ";
                if (g_file.is_open())
                    g_file << '<' << (float)ResourceManager::GetUsedMemoryLoad() / 1024.f << " KB> ";
            }


            DWORD bg_color = g_theme == Theme::Light ? (WORD)(((int)ConsoleColor::LightGray << 4)) : (WORD)(((int)ConsoleColor::Black << 4));

            SetConsoleTextAttribute(Debug::g_console, (bg_color | (int)color));
            std::cout << pref;
            if (g_file.is_open())
                g_file << pref;
            int text_color = g_theme == Theme::Light ? (int)ConsoleColor::Black : (int)ConsoleColor::White;
            SetConsoleTextAttribute(Debug::g_console, (bg_color | text_color));

            std::cout << msg << std::endl;
            if (g_file.is_open())
                g_file << msg << std::endl;

            g_mutex.unlock();
        }

        void Debug::Init(const std::string& log_path, bool ShowUsedMemory, Theme colorTheme) {
            if (Helper::Debug::Profile()) EASY_PROFILER_ENABLE;

            setlocale(LC_ALL, "rus");
            setlocale(LC_NUMERIC, "C");

            srand(time(NULL));

            g_theme = colorTheme;

            InitColorTheme();

            std::string successful = FileSystem::GetPathToExe() + "/successful";
            if (FileSystem::FileExists(successful.c_str()))
                FileSystem::Delete(successful.c_str());

            g_log_path = log_path + "/log.txt";
            if (FileSystem::FileExists(g_log_path.c_str()))
                FileSystem::Delete(g_log_path.c_str());
            g_file.open(g_log_path);

            g_console = GetStdHandle(STD_OUTPUT_HANDLE);

            g_isInit = true;
            g_showUseMemory = ShowUsedMemory;

            std::string msg = "Debugger has been initialized. \n\tLog path: " + g_log_path;
            Print(msg, Type::Debug);
        }
        int Debug::Stop() {
            if (!g_countErrors && !g_countWarnings) {
                std::string msg = "Debugger has been stopped.";
                Print(msg, Type::Debug);
            }else{
                std::string msg = "Debugger has been stopped with errors!\n"
                                  "\tErrors count: "+std::to_string(g_countErrors)+
                                  "\n\tWarnings count: "+std::to_string(g_countWarnings);
                Print(msg, Type::Debug);
            }
            g_file.close();

            std::ofstream o("successful");
            o.close();

            return 0;
        }

        void Debug::InitColorTheme() {
            if (!g_ColorThemeIsEnabled)
                if (g_theme == Theme::Light)
                    system("color 70");
            g_ColorThemeIsEnabled = true;
        }
    }