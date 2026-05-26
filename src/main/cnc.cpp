/*
  ____ _   _  ____ 
 / ___| \ | |/ ___|
| |   |  \| | |    
| |___| |\  | |___ 
 \____|_| \_|\____|
                   

 Author: Paolo Rossi
 Date: 2026-05-07
*/

#include <thread>
#include <chrono>
#include <filesystem>
#include "fsm.hpp"
#include "../cncpp.hpp"
#include "../timer.hpp"

using namespace std;
using double_d = std::chrono::duration<double>; // 'duration' allows to represent time intervals, and 'double' is the type of the count (i.e., the number of seconds, which can be fractional)

struct FsmData {
  string program_file;
  unique_ptr<cncpp::Machine> machine;
  unique_ptr<cncpp::Program> program;
  unique_ptr<Timer<double_d, false>> timer; // smart_ptr to a periodic timed loop class, with time representation as 'double_d' and disabled statistics
  data_t t_tot = 0, t_blk = 0;              // total time and time of the current block
};

int main(int const argc, char const **argv)
{
  string const k_dafault_machine_file = "tcp://localhost:9092";

  if (argc < 2) {
    cerr << cncpp::log_tag(cncpp::LogType::ERROR)
         << " Usage: " << argv[0] << " <program.gcode> [machine.toml] [tcp://host:port] (optional)" << endl;
    return 1;
  }
  if (argc >= 4) {
    cerr << cncpp::log_tag(cncpp::LogType::ERROR)
         << " Too many arguments. Usage: " << argv[0] << " <program.gcode> [machine.toml] [tcp://host:port] (optional)" << endl;
    return 1;
  }

  string program_file = argv[1];
  string machine_file = (argc >= 3) ? argv[2] : k_dafault_machine_file;

  // Initialise data struct
  FsmData data{
    .program_file = program_file,
    .machine = make_unique<cncpp::Machine>(machine_file)
  };
  if (data.machine->agent()) {
    cerr << cncpp::log_tag(cncpp::LogType::MESSAGE) << fg::blue << " Connected to MADS broker at " << machine_file << fg::reset << endl;
    data.machine->agent()->info(cerr);
  } else {
    cerr << cncpp::log_tag(cncpp::LogType::MESSAGE) << fg::green << " Loaded machine configuration file from " << machine_file << fg::reset << endl;
  }

  cerr << cncpp::log_tag(cncpp::LogType::MESSAGE) << style::bold << " Machine initialized:\n" << *data.machine << style::reset << endl;

  // Prepare timer
  double_d timer_interval(data.machine->tq());  // effective timer period
  double_d timer_max_interval(data.machine->tq_max());  // max allowed timer period (depending on the type of machine): max wait
  data.timer = make_unique<Timer<double_d, false>>(timer_interval, timer_max_interval);
  data.timer->start();

  // Create the FSM istance
  // Hp: computation time < tq, so that we can use the timer to pace the FSM 
  // loop. If this is not the case, we would need to implement a more complex 
  // timing mechanism to avoid blocking the FSM loop.
  auto fsm = cncpp::FiniteStateMachine(&data);
  // Provide the FSM with a timing callback that enforces the cycle period.
  // wait_throw() blocks until the next tick and throws on timing errors,
  // which are logged here without stopping the FSM.
  fsm.set_timing_function([&] () {
    try {
      data.timer->wait_throw();
    } catch (const TimerError &e) {
      cerr << cncpp::log_tag(cncpp::LogType::WARNING) << " Timer error: " << e.what() << endl;
    }
  }); 

  // Run the FSM
  fsm.run([&](FsmData &s) {
    // put here any op. that must be executed at each loop
  });

  cerr << cncpp::log_tag(cncpp::LogType::MESSAGE) << " Program execution completed." << endl;
  
  return 0;
}
