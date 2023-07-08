#include "processor.h"
#include "linux_parser.h"

float Processor::Utilization() {
  double active_jiffies = LinuxParser::ActiveJiffies();
  double total_jiffies = LinuxParser::Jiffies();
  double idle_jiffies = LinuxParser::IdleJiffies();

  float CPU_Percentage = (active_jiffies)/total_jiffies;
  return CPU_Percentage;
}