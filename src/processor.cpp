#include "processor.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  double active_jiffies = activeJiffies;
  double total_jiffies = jiffies;
  float CPU_Percentage = active_jiffies/total_jiffies;
  return CPU_Percentage;
}