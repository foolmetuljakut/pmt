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
}

bool CmdParser::changeCreationMode(Modes::CreationMode m)
{
    switch(currentMode) {
        case Modes::Structure:
            return m == Modes::Project;
        case Modes::Project:
            return m == Modes::Target;
        case Modes::Target:
            return (m | Modes::CompilerOptions > 0) || (m | Modes::Project > 0);
        case Modes::CompilerOptions:
            return (m | Modes::LinkerOptions > 0) || (m | Modes::Project > 0);
        case Modes::LinkerOptions:
            return (m | Modes::FileList > 0) || (m | Modes::Project > 0);
        case Modes::FileList:
            return m == Modes::Project;
        default:
            return false;
    }
}

void CmdParser::interpretAsCreation() {
    
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


