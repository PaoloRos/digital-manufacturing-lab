/*
  _____ ____  __  __   __  __       _       
 |  ___/ ___||  \/  | |  \/  | __ _(_)_ __  
 | |_  \___ \| |\/| | | |\/| |/ _` | | '_ \ 
 |  _|  ___) | |  | | | |  | | (_| | | | | |
 |_|   |____/|_|  |_| |_|  |_|\__,_|_|_| |_|
                                            
Warning: this is a bare-bones template for a FSM-based agent. It is not meant to be used as-is, but rather to be adapted to the specific needs of the user. 

To compile it with CMake, use the following CMakeLists.txt:
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the installed MADS package
find_package(Mads REQUIRED)
message(STATUS "Found Mads: ${Mads_DIR}")

add_executable(fsm path/to/main.cpp)
target_link_libraries(fsm PRIVATE Mads::Mads)
*/

#include <thread>
#include <chrono>
#include <filesystem>
#include "fsm.hpp"
#include "../cncpp.hpp"
#include "../timer.hpp"
#include "../defines.hpp"

using namespace std;
using double_d = std::chrono::duration<double>;

struct FsmData {
  string program_file;
  unique_ptr<cncpp::Machine> machine;
  unique_ptr<cncpp::Program> program;
  unique_ptr<Timer<double_d, false>> timer; // smart ptr to a Timer class istance, two template parameters: the type of the duration (double in this case) and whether to enable statistics (false in this case)
  // track of time
  data_t t_tot = 0.0, t_blk = 0.0;  // total time and time for the current block
};

int main(int argc, char *argv[]) 
{
  string machine_file = "progr-files/machine.toml";
  
  if (argc < 2) {
    cerr << cncpp::log_tag(cncpp::LogType::ERROR) << " Usage: " << argv[0] << " <program.gcode> [machine.toml]" << endl;
    return 1;
  }

  string program_file = argv[1];
  if (argc >= 3)  machine_file = argv[2];

  FsmData data{
    .program_file = program_file,
    .machine = make_unique<cncpp::Machine>(machine_file)
  };

  // setup timer

  // Creating the timer designed for a pace of tq seconds, with a maximum wait time of 50 ms to avoid blocking indefinitely in case of errors.
  double_d timer_interval(data.machine->tq());
  double_d timer_max_interval(0.05); // 50 ms max wait
  data.timer = make_unique<Timer<double_d, false>>(timer_interval, timer_max_interval); // timer interval and max wait

  // start timer

  data.timer->start();  // Start the timer

  // Hp: computation time < tq, so that we can use the timer to pace the FSM loop. If this is not the case, we would need to implement a more complex timing mechanism to avoid blocking the FSM loop.
  auto fsm = cncpp::FiniteStateMachine(&data);
  fsm.set_timing_function( [&]() {
    try {
    data.timer->wait_throw(); // Wait for the next timer tick, throwin an error if t_computation > tq (exceeding the max wait time)
  } catch (const TimerError &e) {
    cerr << cncpp::log_tag(cncpp::LogType::ERROR) << " Timer error: " << e.what() << endl;
  }
}); // Setting the pacing function

  // run FSM

  fsm.run([&](FsmData &data) {
    // any operation to be performed
  });

  cerr << cncpp::log_tag(cncpp::LogType::MESSAGE) << " Program execution completed: FSM execution ended." << endl;

  return 0;
}