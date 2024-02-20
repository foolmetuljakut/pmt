#include "cmdparser.hpp"

namespace Pmt
{

CmdParser::CmdParser(int argc, char **argv)
    : currentMode{Modes::Creation}, creationMode{Modes::Structure}, modificationMode{Modes::Add}, executionMode{Modes::CompileLinkApp} {
    for(unsigned i = 1; i < argc; i++) {
        auto arg = std::string(argv[i]);
        separateOptions(arg);
    }
}

void CmdParser::separateOptions(std::string &arg) {

    // replace all within one line with the first key
    std::vector<std::vector<std::string>> identityMap {
        {"-f", "--files", "--force"},
        {"-s", "--structure", "--specific"},
        {"-p", "--project"},
        {"-t", "--target"},
        {"-c", "--compile", "--compiler-options"},
        {"-l", "--link", "--linker-options"},
        {"-e", "--execute"},
        {"-r", "--re"},
        {"-R", "--remove"},
        {"-a", "--add"},
        {"-v", "--version"},
        {"-h", "--help"}
    };

    // replace groups with the given sequence
    std::map<std::string, std::vector<std::string>> groupMap {
        {"-spt", {"-s", "-p", "-t"}},
        {"-clf", {"-c", "-l", "-f"}},
        {"-pt", {"-p", "-t"}},
        {"-lf", {"-l", "-f"}},
    };

    bool match{false};

    // keys and keywords
    for(auto& identities : identityMap) {
        for(auto& term : identities) {
            if(!term.compare(arg)) {
                args.push_back(identities[0]);
                match = true;
                break;
            }
        }
        if(match) {
            break;
        }
    }
    if(match) {
        return;
    }

    // groupkeys 
    for(auto& kv :groupMap) {
        if(!kv.first.compare(arg)) {
            match = true;
            for(auto& replacement : kv.second) {
                args.push_back(replacement);
            }
        }
        if(match) {
            break;
        }
    }
    if(match) {
        return;
    }

    if(!match) {
        args.push_back(arg);
    }
}

void CmdParser::setCurrentMode() {
    if(!args[0].compare("create")) {
        currentMode = Modes::Creation;
    } else if(!args[0].compare("mod")) {
        currentMode == Modes::Modification;
    } else if(!args[0].compare("exe")) {
        currentMode == Modes::Compilation;
    } else {
        throw PmtException({"first keyword must be either 'create', 'mod' or 'exe'"});
    }
    args.erase(args.begin());
}

void CmdParser::changeCreationMode(Modes::CreationMode m)
{
    switch(creationMode) {
        case Modes::Structure:
            if(m == Modes::Project) {
                creationMode = m;
                return;
            } else {
                throw PmtException({"creation mode: transition from ", std::to_string(creationMode), " to ", std::to_string(m), " not allowed"});
            }
        case Modes::Project:
            if(m == Modes::Target) {
                creationMode = m;
                return;
            } else {
                throw PmtException({"creation mode: transition from ", std::to_string(creationMode), " to ", std::to_string(m), " not allowed"});
            }
        case Modes::Target:
            if ((m | Modes::CompilerOptions > 0) || (m | Modes::Project > 0)) {
                creationMode = m;
                return;
            } else {
                throw PmtException({"creation mode: transition from ", std::to_string(creationMode), " to ", std::to_string(m), " not allowed"});
            }
        case Modes::CompilerOptions:
            if ((m | Modes::LinkerOptions > 0) || (m | Modes::Project > 0)) {
                creationMode = m;
                return;
            } else {
                throw PmtException({"creation mode: transition from ", std::to_string(creationMode), " to ", std::to_string(m), " not allowed"});
            }
        case Modes::LinkerOptions:
            if ((m | Modes::FileList > 0) || (m | Modes::Project > 0)) {
                creationMode = m;
                return;
            } else {
                throw PmtException({"creation mode: transition from ", std::to_string(creationMode), " to ", std::to_string(m), " not allowed"});
            }
        case Modes::FileList:
            if (m == Modes::Project) {
                creationMode = m;
                return;
            } else {
                throw PmtException({"creation mode: transition from ", std::to_string(creationMode), " to ", std::to_string(m), " not allowed"});
            }
        default:
            throw PmtException({"creation mode: transition from ", std::to_string(creationMode), " to ", std::to_string(m), " not allowed"});
    }
}

#define throwCreationModeException(mode) throw PmtException({"creation mode: transition from ", std::to_string(creationMode), " to ", std::to_string(mode), " not allowed"})

void CmdParser::interpretAsCreation() {
    Solution sol;

    std::map<std::string, Modes::CreationMode> transitions {
      {"-s", Modes::Structure},
      {"-p", Modes::Project},
      {"-t", Modes::Target},
      {"-c", Modes::CompilerOptions},
      {"-l", Modes::LinkerOptions},
      {"-f", Modes::FileList}
    };

    for(auto& instr : args) {
        std::cout << instr << std::endl;

        /*bug potential:
            only change state, if the next expected state is given by instr
            otherwise by defining compiler flags or linker flags
            that are the same as a transition key by accident
            we switch a state!*/
        if(instr.rfind("-", 0) != std::string::npos) {
            if(!instr.compare("-s")) {
                if(creationMode != Modes::Structure) {
                    changeCreationMode(Modes::Structure);
                    continue;
                }
            } else {
                changeCreationMode(transitions.find(instr)->second);
                continue;
            }
        }
        
        // ! string.startsWith
        if(creationMode == Modes::Structure && 
            sol.solutionfilename.size() == 0 && 
            instr.rfind("-", 0) == std::string::npos) {
            sol.solutionfilename = instr;
        } else if(creationMode == Modes::Project) {
            if(sol.solutionfilename.size() == 0 && instr.rfind("-", 0) == std::string::npos) {
                sol.solutionfilename = instr;
                sol.mainapp.spec.name = instr;
            }
        } else if(creationMode == Modes::Target) {

            // string.startsWith
            if(instr.rfind("-", 0) != std::string::npos) {
                throw PmtException({"after specifying a target a name must be given."});
            }

            if(sol.solutionfilename.size() == 0) {
                sol.solutionfilename = instr;
                sol.mainapp.spec.name = instr;
            }

        }
    }
}

void CmdParser::interpretAsMod() {
}

void CmdParser::interpretAsExe() {
}

void CmdParser::parseInstructions() {
    setCurrentMode();

    switch(currentMode) {
        case Modes::Creation:
            interpretAsCreation();
        case Modes::Modification:
            interpretAsMod();
        case Modes::Compilation:
            interpretAsExe();
    }
}

} // namespace Pmt


