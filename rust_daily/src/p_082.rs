
use clap::Clap;

use crate::problem::Problem;
use crate::problem::Solution;



#[derive(Debug)]
#[derive(Clap)]
pub struct Command {
}

impl Problem for Command {
    fn description(&self) -> String {
        return "P#82 Description:
            Using a read7() method that returns 7 characters from a file,
            implement readN(n) which reads n characters.
            For example, given a file with the content “Hello world”,
            three read7() returns “Hello w”,
            “orld” and then “”."
            .to_string();
    }

    fn solution(&mut self) -> &mut dyn Solution {
        return self;
    }
}

impl Solution for Command {
    fn execute(&mut self) {
        println!("Execute P#82");
    }
}
