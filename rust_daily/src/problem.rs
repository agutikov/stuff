


pub(crate) trait Problem {
    fn solution(&mut self) -> &mut dyn Solution;
    fn description(&self) -> String;
}

pub(crate) trait Solution {
    fn execute(&mut self);
}


