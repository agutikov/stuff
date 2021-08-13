#!/usr/bin/env python3

from pprint import pprint
from copy import copy, deepcopy


class halt:
    def exec(self, state):
        state.halt()


class branch:
    def __init__(self, next_i_0, next_i_1):
        self.next_i = {
            0: next_i_0,
            1: next_i_1
        }

    def exec(self, state):
        v = state.read()
        i = self.next_i[v]
        state.goto(i)


class move:
    def __init__(self, steps, next_i):
        self.steps = steps
        self.next_i = next_i

    def exec(self, state):
        state.do_move(self.steps)
        state.goto(self.next_i)


class write:
    def __init__(self, value, next_i):
        self.value = value
        self.next_i = next_i

    def exec(self, state):
        state.do_write(self.value)
        state.goto(self.next_i)



class tm_state:
    def __init__(self, tape_initial_state, program_graph):
        self.tape = copy(tape_initial_state)
        self.prog = deepcopy(program_graph)
        self.prog[-1] = halt()

        # stack pointer - index of the tape cell
        self.sp = 0

        # program counter - key of the program graph node - instruction key
        # node with key == 0 should always exist - it is the start node
        self.pc = 0

        self.running = True

        self.counter = 0
        self.max_counter = 100

    def halt(self):
        print(f'{self.counter} Halt!')
        self.running = False

    def goto(self, next_i):
        if next_i == -1:
            halt()
        else:
            print(f'{self.counter} {self.pc} goto {next_i}')
        self.pc = next_i

    def read(self):
        v = self.tape.setdefault(self.sp, 0)
        print(f'{self.counter} {self.pc} read [{self.sp}] -> {v}')
        return v

    def do_write(self, value):
        print(f'{self.counter} {self.pc} write [{self.sp}] <- {value}')
        self.tape[self.sp] = value

    def do_move(self, steps):
        next_sp = self.sp + steps
        s = ('+' if steps > 0 else '') + f'{steps}'
        print(f'{self.counter} {self.pc} move {s} [{self.sp}] -> [{next_sp}]')
        self.sp = next_sp

    def step(self):
        instr = self.prog[self.pc]
        instr.exec(self)
        return self.running

    def run_program(self):
        self.running = True
        self.pc = 0
        self.counter = 0
        while self.step():
            self.counter += 1
            if self.counter >= self.max_counter:
                print('Instruction counter exceeded')
                break


prog_sum_2_bits = {
    0: branch(1, 2),
    1: move(1, -1),
    2: move(1, 3),
    3: branch(4, 5),
    4: write(1, -1),
    5: write(0, -1)
}


tapes_2 = [
    #{0: 0, 1: 0},
    #{0: 0, 1: 1},
    {0: 1, 1: 0},
    #{0: 1, 1: 1}
]

for tape in tapes_2:
    tm = tm_state(tape, prog_sum_2_bits)
    print('\n\n')
    tm.run_program()
    pprint(tape)
    pprint(tm.tape)


print('\n\n')
tm = tm_state({0: 0, 1: 1}, prog_sum_2_bits)
pprint(tm.tape)
tm.run_program()
pprint(tm.tape)
tm.run_program()
pprint(tm.tape)
tm.run_program()
pprint(tm.tape)


class tmi:
    def __init__(self, value, step, next_state):
        self.val = value
        self.step = step
        self.next_i = next_state

    def exec(self, state):
        state.do_write(self.val)
        state.do_move(self.step)
        state.goto(self.next_i)


class tms:
    def __init__(self, i0, i1):
        self.instr = {
            0: i0,
            1: i1
        }

    def exec(self, state):
        v = state.read()
        self.instr[v].exec(state)

bb_2 = {
    0: tms(tmi(1, 1, 1), tmi(1, -1, 1)),
    1: tms(tmi(1, -1, 0), tmi(1, 1, -1)),
}

print('\n\n')
tm = tm_state({}, bb_2)
tm.run_program()
pprint(tm.tape)


bb_3 = {
    0: tms(tmi(1, 1, 1), tmi(1, 1, -1)),
    1: tms(tmi(0, 1, 2), tmi(1, 1, 1)),
    2: tms(tmi(1, -1, 2), tmi(1, -1, 0)),
}

print('\n\n')
tm = tm_state({}, bb_3)
tm.run_program()
pprint(tm.tape)


