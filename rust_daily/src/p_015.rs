
use clap::Clap;

use crate::problem::Problem;
use crate::problem::Solution;



#[derive(Debug)]
#[derive(Clap)]
pub struct Command {
    #[clap(short, about = "Verbose debug messages")]
    pub debug: bool
}

impl Problem for Command {
    fn description(&self) -> String {
        return "P#15 Description:
            Given a stream of elements too large to store in memory,
            pick a random element from the stream with uniform probability."
            .to_string();
    }

    fn solution(&mut self) -> &mut dyn Solution {
        return self;
    }
}

impl Solution for Command {
    fn execute(&mut self) {
        println!("Execute P#15");
    }
}
