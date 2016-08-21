use std::env;
use std::process::exit;
mod logger;

struct Input {
    output:String,
    inputFiles:Vec<String>,
    recursive:bool,
    searchDirs:Vec<String>
}

struct FileForParsing {
    input:String,
    output:String,
    recursive:bool
}

fn main() {
    let filesForParsing = handle_input(handle_args(env::args().collect()));
}

fn handle_input(input:Input) -> Vec<FileForParsing> {
    let inputCount = input.inputFiles.len();
    if inputCount == 0 {
        logger::logDaf(logger::FATAL_ERROR, "No input files passed");
    }
    let outputDir = input.output.chars().last() == Some('/');
    if (inputCount > 1 || input.recursive) && !outputDir {
        logger::logDaf(logger::FATAL_ERROR, "When compiling multiple files, the output must be a directory");
    }
    
    let mut filesForParsing = Vec::new();

    for inputFile in input.inputFiles {
        let mut outputFile = String::from(&input.output as &str);
        if outputDir {
            outputFile.push_str(&inputFile);
        }
        filesForParsing.push(FileForParsing {input: inputFile, output: outputFile, recursive: input.recursive});
    }

    filesForParsing
}

fn handle_args(args:Vec<String>) -> Input {
    if args.len() <= 1 {
        logger::logDaf(logger::FATAL_ERROR, "No input file specified");
    }

    let mut input = Input {output: String::new(), inputFiles: Vec::new(), recursive: false, searchDirs: Vec::new()};

    let mut index = 1;
    while index < args.len()  {
        let arg = &args[index];
        if arg.eq("--path") {
            index+=1;
            if index >= args.len() {
                logger::logDaf(logger::FATAL_ERROR, "Expected a path after '--path'");
            }
            input.searchDirs.push((&args[index]).to_string());
        } else if arg.eq("-r") {
            input.recursive = true;
        } else if arg.eq("-o") {
            index+=1;
            if index >= args.len() {
                logger::logDaf(logger::FATAL_ERROR, "Expected an output file or directory after '-o'");
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
    /*
    dafc --path src/ me.haved.Main -o bin/ -r //Finds me/haved/Main.daf in src/ and makes bin/me/haved/Main.o, as well as all files imported by Main.daf recursivly
    dafc Main //Places the the output file at ./Main.o
    Usage: dafc [inputFile]* [--path searchPath] [-r] [-o outputDir/File]
    */ 
    
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
