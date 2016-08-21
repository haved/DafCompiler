
pub const FATAL_ERROR: usize = 0;
pub const ERROR: usize = 1;
pub const WARNING : usize = 2;
pub const INFO: usize = 3;
pub const NOTE: usize = 4;

const LOG_LEVELS: &'static [ &'static str ] = &["fatal_error", "error", "warning", "info", "note"];

static EXEC_NAME: &'static str = "dafc";

pub fn logDaf(level:usize, message:&str) {
    log(EXEC_NAME, level, message);
}

pub fn log(location:&str, level:usize, message:&str) {
    println!("{}: {}: {}", location, LOG_LEVELS[level], message);
}