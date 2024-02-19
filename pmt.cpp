#include "std.hpp"
#include "boost.hpp"

#include "solution.hpp"

#include "cmdparser.hpp"

int main1(int argc, char **argv) {
    Pmt::File file("cpi/cpi.cpp");
    Pmt::File file2("cpi/structure.hpp");
    Pmt::File file3("cpi/structure.cpp");
    Pmt::TargetSpec target("cpi-app", "-g", "");
    Pmt::Project project(target, {file, file2, file3});
    write_json(std::cout, project.tonode());
    std::cout << project.compilecmd(file.name) << std::endl;
    std::cout << project.compilecmd(file2.name) << std::endl;
    std::cout << project.compilecmd(file3.name) << std::endl;
    std::cout << project.linkcmd() << std::endl;
    // Pmt::exec(project.compilecmd());
    project.save("cpi/project.json");
    return 0;
}

int main2(int argc, char **argv) {
    std::cout << "loading" << std::endl;
    Pmt::Project project("cpi/project.json");
    std::cout << "project has " << (project.haschanged() ? "" : "not ") << "changed" << std::endl;
    std::cout << "updating settings: " << std::endl;
    project.compile(true);
    std::cout << "project has " << (project.haschanged() ? "" : "not ") << "changed" << std::endl;
    project.save("cpi/project.json");
    return 0;
}

int main3(int argc, char **argv) {
    /*cpi loads its own settings and recompiles itself*/
    Pmt::Project project("cpi/project.json");
    project.addfile("cpi/exec.hpp");
    project.addfile("cpi/exec.cpp");
    project.spec.opts = "-fdiagnostics-color=always -Werror -g -DBOOST_BIND_GLOBAL_PLACEHOLDERS -lboost_program_options";
    project.save("cpi/project.json");
    project.build(true);
    project.save("cpi/project.json");
    return 0;
}

int main5(int argc, char **argv) {
    Pmt::Solution solution("cpi/solution.json");
    solution.load();
    solution.build(true);
    return 0;
}

enum ArgumentMode {
    DECL_STRUCTURE,
    DECL_PROJECT,
    DECL_TARGETNAME,
    COMPILER_OPTIONS,
    LINKER_OPTIONS,
    LIST_FILES,
    UNITTEST_SYMBOL
};

void separateoptions(std::string& arg, std::vector<std::string>& args) {
    if(!std::string("-pt").compare(arg)) {
        args.push_back(std::string("-p"));
        args.push_back(std::string("-t"));
    }
    else if(!std::string("-spt").compare(arg)) {
        args.push_back(std::string("-s")); // assume target name = structure name = mainapp name
        args.push_back(std::string("-p"));
        args.push_back(std::string("-t"));
    }
    else if (!std::string("-v").compare(arg) || !std::string("--version").compare(arg)) {
        std::cout << "version: " << PMTVERSION << std::endl;
    }
    else if (!std::string("-h").compare(arg) || !std::string("--help").compare(arg)) {
        std::cout << "help: " << 
        "   --structure|-s opens a new structure and subs every following command to the last declared structure\n \
        --project|-p opens a new project and subs every following command .... bla\n \
        --target|-t expects the name of the project target\n \
        --compiler-options|-c opens a list of compiler options and subs every following command .... bla\n \
        --linker-options|-l opens a list of linker options and subs every following command .... bla\n \
        --files|-f opens a list of files and subs .... you get the idea\n \
        --unittest-symbol|-u defines an in-code preprocessor symbol that is unique in the unittest application\n \
        \twhile the main application gets passed the unique symbol MAIN" << std::endl;
    }
    else {
        args.push_back(arg);
    }
}

void fe_execution(std::vector<std::string>& args) {
    if(args.size() >= 2) {
        auto arg0 = args[0], fn = args[1];
        bool e = !std::string("-e").compare(arg0) || !std::string("--exec").compare(arg0),
                fe = !std::string("-fe").compare(arg0) || !std::string("--force-exec").compare(arg0);
        if((e || fe) && std::filesystem::exists(std::filesystem::path({fn}))) {
            try {
                Pmt::Solution s(fn);
                s.load();
                if(s.haschanged() || fe) {
                    bool succeeded = s.build(fe);
                    if(!succeeded)
                        exit(1);
                }
                else {
                    //throw Pmt::PmtException({"solution ", fn, " up to date"});
                    std::cout << "solution " << fn << " is up to date" << std::endl; // otherwise "crashes" with return code -1
                    exit(0);
                }
            }
            catch (std::exception& e) {
                std::cout << e.what() << std::endl;
                exit(-1);
            }
        }
    }
}

int main6(int argc, char **argv) {
    std::vector<std::string> args;
    for(unsigned i = 1; i < argc; i++) {
        auto arg = std::string(argv[i]);
        separateoptions(arg, args);
    }

    /*shortcut of this whole processing:
        --exec|-e <filemame> try to load and execute project*/

    fe_execution(args);

    /*processing*/
    unsigned mode = DECL_STRUCTURE; /* s:0, p:1, t:2, o:3, f:4*/
    bool expecting = false;
    std::shared_ptr<Pmt::Solution> solution;
    Pmt::Project *currentproject;
    for(auto& arg : args) {
        /*mode switching*/
        if(!std::string("-s").compare(arg) || !std::string("--structure").compare(arg)) {
            mode = DECL_STRUCTURE;
            expecting = false;
        }
        if(!std::string("-p").compare(arg) || !std::string("--project").compare(arg)) {
            mode = DECL_PROJECT;
            expecting = false;
        }
        if(!std::string("-t").compare(arg) || !std::string("--target").compare(arg)) {
            if(mode >= DECL_PROJECT) {
                mode = DECL_TARGETNAME;
                expecting = false;
            }
            else
                throw Pmt::PmtException({"currently not in mode (", 
                    mode == 0 ? "decl. struct." : (
                    mode == 1 ? "decl. proj" : (
                    mode == 2 ? "decl. target" : (
                    mode == 3 ? "comp. options" : (
                    mode == 4 ? "link. options" : "decl. files"
                )))), ") to accept a target name"});
        }
        if(!std::string("-c").compare(arg) || !std::string("--compiler-options").compare(arg)) {
            if(mode >= DECL_TARGETNAME) {
                mode = COMPILER_OPTIONS;
                expecting = false;
            } else
                throw Pmt::PmtException({"currently not in mode (", 
                    mode == 0 ? "decl. struct." : (
                    mode == 1 ? "decl. proj" : (
                    mode == 2 ? "decl. target" : (
                    mode == 3 ? "comp. options" : (
                    mode == 4 ? "link. options" : "decl. files"
                )))), ") to accept a list of options"});
        }
        if(!std::string("-l").compare(arg) || !std::string("--linker-options").compare(arg)) {
            if(mode >= DECL_TARGETNAME) { // can be skipped, compiler flags then remain empty
                mode = LINKER_OPTIONS;
                expecting = false;
            } else
                throw Pmt::PmtException({"currently not in mode (", 
                    mode == 0 ? "decl. struct." : (
                    mode == 1 ? "decl. proj" : (
                    mode == 2 ? "decl. target" : (
                    mode == 3 ? "comp. options" : (
                    mode == 4 ? "link. options" : "decl. files"
                )))), ") to accept a list of options"});
        }
        if(!std::string("-f").compare(arg) || !std::string("--files").compare(arg)) {
            if(mode >= DECL_TARGETNAME) { // can be skipped, linker flags then remain empty
                mode = LIST_FILES;
                expecting = false;
            } else
                throw Pmt::PmtException({"currently not in mode (", 
                    mode == 0 ? "decl. struct." : (
                    mode == 1 ? "decl. proj" : (
                    mode == 2 ? "decl. target" : (
                    mode == 3 ? "comp. options" : (
                    mode == 4 ? "link. options" : "decl. files"
                )))), ") to accept a list of files"});
        }
        if(!std::string("-u").compare(arg) || !std::string("--unittest-symbol").compare(arg)) {
            if(mode >= LIST_FILES) {
                mode = UNITTEST_SYMBOL;
                expecting = false;
            } else
                throw Pmt::PmtException({"currently not in mode (", 
                    mode == 0 ? "decl. struct." : (
                    mode == 1 ? "decl. proj" : (
                    mode == 2 ? "decl. target" : (
                    mode == 3 ? "comp. options" : (
                    mode == 4 ? "link. options" : "decl. files"
                )))), ") to accept a list of files"}); 
        }
        
        /*processing depending on mode*/
        switch(mode) {
            case DECL_STRUCTURE:
                if(solution) /* for declaring *another* structure*/
                {
                    /*what if mainapp isn't properly initialized? */
                    std::stringstream ss;
                    ss << solution->mainapp.spec.name << ".json";
                    solution->solutionfilename = ss.str();
                    solution->save();
                    solution.reset(); /*clear the pointer*/
                }
                break;
            case DECL_PROJECT:
                if(!solution) {
                    std::cout << "solution empty => creating new" << std::endl;
                    solution = std::make_shared<Pmt::Solution>();
                    std::cout << "project empty => creating new" << std::endl;
                    solution->mainapp = Pmt::Project();
                    currentproject = &solution->mainapp;
                    solution->mainapp.spec = Pmt::TargetSpec();
                    expecting = true;
                }
                else {
                    std::cout << "appending new subproject" << std::endl;
                    solution->projects.push_back(Pmt::Project());
                    currentproject = &solution->projects.back();
                }
                break;
            case DECL_TARGETNAME:
                if(expecting)
                    currentproject->spec.name = arg;
                else {
                    std::cout << "target spec is empty => assigning new" << std::endl;
                    currentproject->spec = Pmt::TargetSpec();
                    expecting = true;
                }
                break;
            case COMPILER_OPTIONS:
                if(expecting) {
                    std::stringstream ss;
                    ss << currentproject->spec.opts << (currentproject->spec.opts.size() > 0 ? " " : "") << arg;
                    currentproject->spec.opts = ss.str();
                }
                else {
                    std::cout << "adding to compiler options" << std::endl;
                    expecting = true;
                }
                break;
            case LINKER_OPTIONS:
                if(expecting) {
                    std::stringstream ss;
                    ss << currentproject->spec.lflags << (currentproject->spec.lflags.size() > 0 ? " " : "") << arg;
                    currentproject->spec.lflags = ss.str();
                }
                else {
                    std::cout << "adding to linker options" << std::endl;
                    expecting = true;
                }
                break;
            case LIST_FILES:
                if(expecting)
                    currentproject->addfile(arg);
                else {
                    std::cout << "adding to file list" << std::endl;
                    expecting = true;
                }
                break;
            case UNITTEST_SYMBOL:
                if(expecting)
                    currentproject->unittestsymbol = arg;
                else {
                    std::cout << "adding to unittest symbol" << std::endl;
                    expecting = true;
                }
                break;
            default:
                throw Pmt::PmtException({"can't accept ", arg, " while expecting ",
                mode == 0 ? "a structure name or project" : (
                mode == 1 ? "a targetname" : (
                mode == 2 ? "a list of options" : "a list of files"))});
        }
    }

    if(solution)
    {
        std::stringstream ss;
        ss << solution->mainapp.spec.name << ".json";
        solution->solutionfilename = ss.str();
        solution->save();
    }

    return 0;
}

int main( int argc, char **argv) {
    Pmt::CmdParser parser(argc, argv);
    parser.parseInstructions();
}