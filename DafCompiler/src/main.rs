use std::env;
use std::process::exit;
use std::path::{Path, PathBuf};
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
    let mut input = handle_args(env::args().collect());
    if input.searchDirs.len() == 0 {
        input.searchDirs.push(String::from(".")); //If no search dirs are specified, try this one
    }
    if input.output.len() == 0 {
        input.output.push_str("./"); //Make the output in this folder
    }
    let mut filesForParsing = handle_input(&input);
    //Files for parsing might be relative to serach dirs
    assure_file_existance(&mut filesForParsing, &input.searchDirs); //Will search for and make input files exist
}

fn assure_file_existance(files:&mut Vec<FileForParsing>, searchDirs:&Vec<String>) {
    for ffp in files {
        let mut found = false;
        for searchDir in searchDirs {
            let mut pathTry:PathBuf = Path::new(searchDir).join(&ffp.input);
            if pathTry.is_file() {
                ffp.input = String::from(pathTry.to_str().unwrap_or("Path try error"));
                found = true;
                break;
            } else {
                pathTry.set_extension("daf");
                if pathTry.is_file() {
                    ffp.input = String::from(pathTry.to_str().unwrap_or("Path try error"));
                    found = true;
                    break;
                }
            }
        }
        if !found {
            logger::logDaf(logger::FATAL_ERROR, &format!("The file {} wasn't found", ffp.input));
        }
    }
}

fn handle_input(input:&Input) -> Vec<FileForParsing> {
    let inputCount = 1;//input.inputFiles.len();
    if inputCount == 0 {
        logger::logDaf(logger::FATAL_ERROR, "No input files passed");
    }
    let outputDir = true;//input.output.chars().last() == Some('/');
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
