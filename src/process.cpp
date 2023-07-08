#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// constructor
Process::Process(int pid){
  pid_ = pid;
  user_ = LinuxParser::User(pid_);
  command_ = LinuxParser::Command(pid_);
  cpuUtilization_ = (static_cast<double>(LinuxParser::ActiveJiffies(pid)))/(static_cast<double>(LinuxParser::Jiffies()));
  ram_ = LinuxParser::Ram(pid_);
  uptime_ = LinuxParser::UpTime(pid_);
}

int Process::Pid() { return pid_; }

float Process::CpuUtilization() { return cpuUtilization_; }

string Process::Command() { return command_; }

string Process::Ram() { return ram_; }

string Process::User() { return user_; }

long int Process::UpTime() { return uptime_; }

bool Process::operator<(Process const& a) const {
  return a.cpuUtilization_ < this->cpuUtilization_;
}