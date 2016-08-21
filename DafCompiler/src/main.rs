use std::env;
use std::process::exit;
mod logger;

struct Input {
    output:String,
    inputFiles:Vec<String>,
    recursive:bool,
    searchDirs:Vec<String>
}

fn main() {
    let input = handle_args(env::args().collect());
}

/*
dafc --path src/ me.haved.Main -o bin/ -r //Finds me/haved/Main.daf in src/ and makes bin/me/haved/Main.o, as well as all files imported by Main.daf recursivly
dafc me.haved.Main -r -o output.o //Packs all files imported by ./me/haved/Main.daf into output.o
dafc Main //Places the the output file at ./Main.o
Usage: dafc [inputFile]* [--path searchPath] [-r] [-o outputDir/File]
*/

fn handle_args(args:Vec<String>) -> Input {
    if args.len() <= 1 {
        logger::logDaf(logger::FATAL_ERROR, "No input file specified");
        exit(-1);
    }

    let mut input = Input {output: String::new(), inputFiles: Vec::new(), recursive: false, searchDirs: Vec::new()};

    let mut index = 1;
    while index < args.len()  {
        let arg = &args[index];
        if arg.eq("--path") {
            index+=1;
            if index >= args.len() {
                logger::logDaf(logger::FATAL_ERROR, "Expected a path after '--path'");
                exit(-1);
            }
            println!("Path added: {}", &args[index]);
            input.searchDirs.push((&args[index]).to_string());
        } else if arg.eq("-r") {
            println!("Recursive parsing");
            input.recursive = true;
        } else if arg.eq("-o") {
            index+=1;
            if index >= args.len() {
                logger::logDaf(logger::FATAL_ERROR, "Expected an output file or directory after '-o'");
                exit(-1);
            }
            input.output = (&args[index]).to_string();
        } else if arg.eq("--help") {
            print_help_message();
            exit(0);
        } else {
            input.inputFiles.push((&args[index]).to_string())
        }
        index+=1;
    }

    input
}

fn print_help_message() {
    println!("
    Help page for dafc:

    Usage: dafc inputFiles [options]

    The input files can be relative to any source dir, or the current dir.
    The .daf extension may be omitted.
    The output may be a single file, or a directory.
    In which case each input file gets an object file with the same name.
    Options:
        --path <soruce path>    Adds a source path
        -o <output file/dir>    Sets the output
        -r                      Recursivly compile input files
        --help                  Print this help message");
}
