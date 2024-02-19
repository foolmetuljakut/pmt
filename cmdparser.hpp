#ifndef PMT_CMDPARSER_HPP
#define PMT_CMDPARSER_HPP

#include "std.hpp"
#include "pmtexception.hpp"

#define PMTVERSION "0.7"

namespace Pmt {

    namespace Modes {
        /* 
            CurrentMode: 
                Single Mode, no transitions
        */
        enum CurrentMode {
            Creation,
            Modification,
            Compilation
        };

        /*
            CreationMode: transitions
                Structure -> Project -> Target -> CompilerOptions -> LinkerOptions -> FileList
                CompilerOptions, LinkerOptions, FileList -> Project
        */
        enum CreationMode {
            Undefined = 0,
            Structure,
            Project,
            Target,
            CompilerOptions,
            LinkerOptions,
            FileList
        };

        /* 
            ModificationMode: 
                Single Mode, no transitions
        */
        enum ModificationMode {
            Add,
            Remove,
            SpecificRecompile,
            SpecificRelink
        };

        /* 
            ModificationMode: 
                Switches, can be wildly combined
        */
        enum ExecutionMode {
            CompileLinkApp,
            CompileLinkTest,
            ForceRecompile
        };
    };

    class CmdParser {
        Modes::CurrentMode currentMode;
        Modes::CreationMode creationMode;
        Modes::ModificationMode modificationMode;
        Modes::ExecutionMode executionMode;

        std::vector<std::string> args;
        void separateOptions(std::string& arg);
        void setCurrentMode();
        bool changeCreationMode(Modes::CreationMode m);
        void interpretAsCreation();
        void interpretAsMod();
        void interpretAsExe();
    public:
        CmdParser(int argc, char **argv);
        void parseInstructions();
    };

};

#endif //PMT_CMDPARSER_HPP