#include "linux_parser.h"
#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

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

float LinuxParser::MemoryUtilization() {
  double memtotal, memfree;
  string line, key;
  std::ifstream stream(kProcDirectory+kMeminfoFilename);
  if (stream.is_open()){
    while(std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == filterMemTotal) linestream >> memtotal;
        if (key == filterMemFree) linestream >> memfree;
      }
    }
  }
  float utilization = (memtotal - memfree)/memtotal;
  return utilization;
}

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

long LinuxParser::Jiffies() {
  vector<string> cpu = CpuUtilization();
  long total = 0;
  for (int i=0; i < 8; ++i){
    total += std::stol(cpu[i]);
  }
  return total;
}

long LinuxParser::ActiveJiffies(int pid) {
  long utime, stime, cutime, cstime, starttime;
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
  return utime + stime + cutime + cstime;
}

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

long LinuxParser::IdleJiffies() {
  vector<string> cpu = CpuUtilization();
  long idle = std::stol(cpu[3]);
  long iowait = std::stol(cpu[4]);
  long Idle = idle + iowait; // total idle jiffies
  return Idle;
}

vector<string> LinuxParser::CpuUtilization() {
  string user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
  string line, key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      while(linestream >> key){
        if (key == filterCpu){
          linestream >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
        }
      }
    }
  }
  vector<string> cpu = {user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice};
  return cpu;
}

int LinuxParser::TotalProcesses() {
  int totalprocesses;
  string line, key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      while(linestream >> key){
        if (key == filterProcess){
          linestream >> totalprocesses;
        }
      }
    }
  }
  return totalprocesses;
}

int LinuxParser::RunningProcesses() {
  int runningprocesses;
  string line, key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      while(linestream >> key){
        if (key == filterRunningProcess){
          linestream >> runningprocesses;
        }
      }
    };
  }
  return runningprocesses;
}

string LinuxParser::Command(int pid) {
  string line;
  std::ifstream cmdfile(kProcDirectory+ to_string(pid)+kCmdlineFilename);
  if(cmdfile.is_open()) getline(cmdfile, line);
  return line;
}

string LinuxParser::Ram(int pid) {
  string line, key, ramMB;
  int ram;
  std::ifstream statusfile(kProcDirectory+ to_string(pid)+kStatusFilename);
  if(statusfile.is_open()) {
    while(getline(statusfile, line)){
      std::istringstream linestream(line);
      while(linestream >> key){
        // used VmRSS b/c it gives the exact physical memory being used as a part of the physical RAM
        // while VmSize is the virtual memory size, which could be more than the physical RAM size
        if (key == filterProcMem)
          linestream >> ram;
      }
    }
  }
  ramMB = to_string(ram/1000);
  return ramMB;
}

string LinuxParser::Uid(int pid) {
  string line, key, uid;
  std::ifstream statusfile(kProcDirectory+ to_string(pid)+kStatusFilename);
  if(statusfile.is_open()) {
    while(getline(statusfile, line)){
      std::istringstream linestream(line);
      while(linestream >> key){
        if (key == filterUid)
          linestream >> uid;
      }
    }
  }
  return uid;
}

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
  return user;
}

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

