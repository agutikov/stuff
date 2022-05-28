
use clap::{AppSettings, Clap};

mod problem;
mod p_015;
mod p_082;

use crate::problem::Problem;


#[derive(Debug)]
#[derive(Clap)]
#[clap(version = "1.0", author = "Aleksei G. <agutikov@gmail.com>")]
#[clap(setting = AppSettings::ColoredHelp)]
struct Opts {
    #[clap(subcommand)]
    cmd: SubCommand,
}


#[derive(Debug)]
#[derive(Clap)]
enum SubCommand {
    #[clap(version = "0.1", about = "Problem #15")]
    P015(p_015::Command),
    P082(p_082::Command),
}

fn get_problem(cmd: &mut SubCommand) -> &mut dyn Problem {
    match cmd {
        SubCommand::P015(p) => {
            return p;
        }
        SubCommand::P082(p) => {
            return p;
        }
    }
}


fn execute(cmd: &mut SubCommand) {
    let p = get_problem(cmd);
    println!("{}", p.description());
    p.solution().execute();
}



fn main() {
    let mut opts: Opts = Opts::parse();
    println!("opts: {:#?}", opts);

    execute(&mut opts.cmd);
}
