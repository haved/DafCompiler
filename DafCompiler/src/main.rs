use std::env;
mod logger;

fn main() {
    handle_args(env::args().collect());
}

fn handle_args(args:Vec<String>) {
    if args.len() <= 1 {
        logger::log_2(logger::FATAL_ERROR, "No input file specified");
    }
}
