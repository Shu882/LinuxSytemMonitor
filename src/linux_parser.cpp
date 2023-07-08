#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// DONE: Read and return the system memory utilization, done
float LinuxParser::MemoryUtilization() {
  double memtotal, memfree;
  string line, key;
  std::ifstream stream(kProcDirectory+kMeminfoFilename);
  if (stream.is_open()){
    while(std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == "MemTotal:") linestream >> memtotal;
        if (key == "MemFree:") linestream >> memfree;
      }
    }
  }
  float utilization = (memtotal - memfree)/memtotal;
  return utilization;
}

// DONE: Read and return the system uptime, done
long LinuxParser::UpTime() {
  long uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return uptime;
}

std::vector<long> LinuxParser::Cpu(){
  long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
  string line, key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      while(linestream >> key){
        if (key == "cpu"){
          linestream >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
        }
      }
    }
  }
  long Idle = idle + iowait; // total idle jiffies
  long NonIdle = user + nice + system + irq + softirq + steal; // total active jiffies
  long Total = Idle + NonIdle;
  std::vector<long> cpu = {Idle, NonIdle, Total};
  return cpu;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  vector<string> cpu = CpuUtilization();
  long user = std::stol(cpu[0]);
  long nice = std::stol(cpu[1]);
  long system = std::stol(cpu[2]);
  long idle = std::stol(cpu[3]);
  long iowait = std::stol(cpu[4]);
  long irq = std::stol(cpu[5]);
  long softirq = std::stol(cpu[6]);
  long steal = std::stol(cpu[7]);

  long NonIdle = user + nice + system + irq + softirq + steal; // total active jiffies
  long Idle = idle + iowait; // total idle jiffies
  long Total = Idle + NonIdle;

  return Total;
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> cpu = CpuUtilization();
  long user = std::stol(cpu[0]);
  long nice = std::stol(cpu[1]);
  long system = std::stol(cpu[2]);
  long irq = std::stol(cpu[5]);
  long softirq = std::stol(cpu[6]);
  long steal = std::stol(cpu[7]);
  long NonIdle = user + nice + system + irq + softirq + steal; // total active jiffies
  return NonIdle;
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> cpu = CpuUtilization();
  long idle = std::stol(cpu[3]);
  long iowait = std::stol(cpu[4]);
  long Idle = idle + iowait; // total idle jiffies
  return Idle;
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
  string line, key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      while(linestream >> key){
        if (key == "cpu"){
          linestream >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
        }
      }
    }
  }
  vector<string> cpu = {user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice};
  return cpu;
}

// DONE: Read and return the total number of processes, done
int LinuxParser::TotalProcesses() {
  int totalprocesses;
  string line, key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      while(linestream >> key){
        if (key == "processes"){
          linestream >> totalprocesses;
        }
      }
    }
  }
  return totalprocesses;
}

// DONE: Read and return the number of running processes, done
int LinuxParser::RunningProcesses() {
  int runningprocesses;
  string line, key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      while(linestream >> key){
        if (key == "procs_running"){
          linestream >> runningprocesses;
        }
      }
    };
  }
  return runningprocesses;
}

// DONE: Read and return the command associated with a process, done
string LinuxParser::Command(int pid) {
  string line;
  std::ifstream cmdfile(kProcDirectory+ to_string(pid)+kCmdlineFilename);
  if(cmdfile.is_open()) getline(cmdfile, line);
  return line;
}

// DONE: Read and return the memory used by a process, done
string LinuxParser::Ram(int pid) {
  string line, key, ramMB;
  int ram;
  std::ifstream statusfile(kProcDirectory+ to_string(pid)+kStatusFilename);
  if(statusfile.is_open()) {
    while(getline(statusfile, line)){
      std::istringstream linestream(line);
      while(linestream >> key){
        if (key == "VmSize:")
          linestream >> ram;
      }
    }
  }
  ramMB = to_string(ram/1000);
  return ramMB;
}

// DONE: Read and return the user ID associated with a process, done
string LinuxParser::Uid(int pid) {
  string line, key, uid;
  std::ifstream statusfile(kProcDirectory+ to_string(pid)+kStatusFilename);
  if(statusfile.is_open()) {
    while(getline(statusfile, line)){
      std::istringstream linestream(line);
      while(linestream >> key){
        if (key == "Uid:")
           linestream >> uid;
      }
    }
  }
  return uid;
}

// DONE: Read and return the user associated with a process, done
string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  string line, user, x, uid_get;
  std::ifstream passwdfile(kPasswordPath);
  if(passwdfile.is_open()){
    while(getline(passwdfile, line)){
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while(linestream >> user >> x >> uid_get){
        if (uid_get == uid){
          return user;
        }
      }
    }
  }
  return "me";
}

// DONE: Read and return the uptime of a process, done
long LinuxParser::UpTime(int pid) {
  string line, nth;
  long uptime;
  std::ifstream statfile(kProcDirectory + to_string(pid) + kStatFilename);
  if (statfile.is_open()){
    while(getline(statfile, line)){
      std::istringstream linestream(line);
      for (int i=0; i<22; ++i){
        linestream >> nth;
      }
    }
  }
  uptime = std::stol(nth) / sysconf(_SC_CLK_TCK); // in seconds
  return uptime; // the value 0 sometimes b/c the uptime is too small
}

float LinuxParser::CpuUtilization(int pid) {
  long uptime = UpTime(); // system uptime in seconds
  long utime, stime, cutime, cstime, starttime, totalTime, seconds;
  float cpuUsage;
  //get the values
  string line, nth;
  std::ifstream statfile(kProcDirectory + to_string(pid) + kStatFilename);
  if (statfile.is_open()){
    while(getline(statfile, line)){
      std::istringstream linestream(line);
      for (int i=0; i<22; ++i){
        if (i == 13) linestream >> utime;
        else if (i == 14) linestream >> stime;
        else if (i == 15) linestream >> cutime;
        else if (i == 16) linestream >> cstime;
        else if (i == 21) linestream >> starttime;
        else linestream >> nth;
      }
    }
  }
  long HZ = sysconf(_SC_CLK_TCK);
  totalTime = (utime + stime + cutime + cstime)/HZ; // total time spent on the process in seconds
  seconds = uptime - (starttime/HZ); // total elapsed time in seconds since the process started
  double total = totalTime;
  cpuUsage = total / seconds;
  if (cpuUsage < 1 && cpuUsage >=0)
    return cpuUsage;
  else
    return 0.5;// default wrong value
}
