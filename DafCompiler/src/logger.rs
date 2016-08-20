
pub const FATAL_ERROR: i32 = 0;
pub const ERROR: i32 = 1;
pub const WARNING : i32 = 2;
pub const INFO: i32 = 3;
pub const NOTE: i32 = 4;

const LOG_LEVELS: &'static [ &'static str ] = &["fatal_error", "error",
    "warning", "info", "note"];
static EXEC_NAME: &'static str = "dafc";

pub fn log_2(level:i32, message:&'static str) {
    log_3(EXEC_NAME, level, message);
}

pub fn log_3(location:&'static str, level:i32, message:&'static str) {
    println!("{}: {}: {}", LOG_LEVELS[level], level, message);
}