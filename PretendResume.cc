/*
 * Copyright (C) 2008 Pierre Lejeune
 *
 * This source file is intended to be compiled as a shared library
 * to be used as a hook for Paludis, the other Package Mangler.
 * It outputs the resume command on the console or in a file regarding the "resume-command-template" argument.
 * To use it, copy it or make a link to it into "${SHAREDIR}/paludis/install_pretend_post".
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
//#include <cctype>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>

#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/system.hh>
//#include <paludis/util/wrapped_forward_iterator.hh>

/**
 * Function to run the current hook (declared in Paludis API)
 */
paludis::HookResult paludis_hook_run(const paludis::Environment* env, const paludis::Hook& hook)
{
    paludis::HookResult result = { paludis::value_for<paludis::n::max_exit_status>(0), paludis::value_for<paludis::n::output>("") };
    int fd = 0;
/*
 * Showing all variables in hook
 * For debugging only
 */
//	for(paludis::Hook::ConstIterator h(hook.begin()), h_end(hook.end()); h != h_end; ++h)
//		std::cout << h->first << " : " << h->second << std::endl;
// Check whether $RESUME_COMMAND is defined
    if(hook.get("RESUME_COMMAND").empty())
        result.output = paludis::value_for<paludis::n::output>("No resume command");
    else
    {
        std::cout << std::endl;
// Check whether there are errors in deplist
        if(hook.get("DEPLIST_HAS_ERRORS") == "false")
        {
// Check whether resume_command_template argument is set
            std::string paludis_client = paludis::getenv_or_error("PALUDIS_CLIENT");
            std::transform(paludis_client.begin(), paludis_client.end(), paludis_client.begin(), toupper);
            std::string rctValue = paludis::getenv_with_default(paludis_client + "_CMDLINE_resume_command_template", "null");
// Output resume command to console
            if(rctValue == "null")
            {
                std::cout << "Resume command: " << hook.get("RESUME_COMMAND") << std::endl;
                result.output = paludis::value_for<paludis::n::output>("Resume Command OK");
            }
// Output resume command to file
            else
            {
                std::string rctFile = rctValue;
                if(rctFile.find("XXXXXX") != std::string::npos)
                {
                    char* rctFileCStr = new char[rctFile.length() + 1];
                    memset(rctFileCStr, '\0', rctFile.length() + 1);
                    rctFile.copy(rctFileCStr, rctFile.length());
                    fd = mkstemp(rctFileCStr);
                    rctFile = rctFileCStr;
                }
                std::fstream rctFileStream(rctFile.c_str());
                if(rctFileStream)
                {
                    rctFileStream << hook.get("RESUME_COMMAND") << std::endl;
                    rctFileStream.close();
                    chmod(rctFile.c_str(), 0644);
                    std::cout << "Resume command saved to file: " << rctFile << std::endl;
                    result.output = paludis::value_for<paludis::n::output>("Resume Command Saved To File");
                }
                else
                {
                    std::cout << "Resume command NOT saved to file: " << rctFile << std::endl;
                    std::cout << "Resume command: " << hook.get("RESUME_COMMAND") << std::endl;
                    result.output = paludis::value_for<paludis::n::output>("Resume Command NOT Saved To File");
                }
            }
			std::string root(paludis::stringify(env->root()));
			std::string exec_mode(paludis::getenv_with_default("PALUDIS_EXEC_PALUDIS", ""));
			if(exec_mode != "never" && (root.empty() || root == "/"))
			{
				std::string::size_type pos = hook.get("RESUME_COMMAND").find("sys-apps/paludis");
				if(pos != std::string::npos)
					std::cout << "WARNING : Paludis is in resume command" << std::endl;
			}
        }
// Error
        else
        {
            std::cout << "Error in resume command" << std::endl;
            result.output = paludis::value_for<paludis::n::output>("Error in resume command");
        }
    }
    return result;
}
